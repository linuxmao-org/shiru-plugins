/*
 * Converter of VST patches in FXB format.
 *
 */

#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstddef>

#include "../plugins/chipwave/ChipWaveShared.cpp"
static Parameter parameters[Parameter_Count];

enum {
    kModeBank,
    kModeUndefined,
};

struct FxPreset {
    std::string name;
    float param[Parameter_Count] = {};
};

bool load_bank(FILE *in, std::vector<FxPreset> &presets);
bool save_presets(const FxPreset *presets, uint32_t count, const char *out_path);
bool is_default_preset(const FxPreset &fxp);
bool path_has_extension(const char *path, const char *ext);

int main(int argc, char *argv[])
{
    const char *out_path = nullptr;

    if (argc <= 1) {
        fprintf(
            stderr,
            "* Preset Conversion WinVST -> LV2\n"
            "Usage: %s <-o my-pset.lv2> <my-bank.fxb>\n",
            argv[0]);
        return 1;
    }

    for (int opt; (opt = getopt(argc, argv, "o:")) != -1;) {
        switch (opt) {
        case 'o':
            out_path = optarg;
            break;
        default:
            return 1;
        }
    }

    if (argc - optind != 1) {
        fprintf(stderr, "Bad number of arguments: please indicate a FXB file.\n");
        return 1;
    }

    if (!out_path) {
        fprintf(stderr, "Indicate the name of the output \".lv2\" bundle. (the option \"-o\")\n");
        return 1;
    }

    ///
    for (uint32_t i = 0; i < Parameter_Count; ++i)
        InitParameter(i, parameters[i]);

    ///
    int mode = kModeUndefined;
    const char *in_path = argv[optind];

    if (path_has_extension(in_path, ".fxb")) {
        mode = kModeBank;
    }
    // else if (path_has_extension(in_path, ".fxp")) {
    //     mode = kModeProgram;
    // }

    if (mode == kModeUndefined) {
        fprintf(stderr, "Unrecognized file extension.\n");
        return 1;
    }

    FILE *in_file = fopen(in_path, "rb");
    if (!in_file) {
        fprintf(stderr, "Failed to open file.\n");
        return 1;
    }

    std::vector<FxPreset> presets;

    switch (mode) {
    case kModeBank:
        if (!load_bank(in_file, presets)) {
            fprintf(stderr, "Could not read the bank file.\n");
            return 1;
        }
        break;
    }

    if (presets.empty()) {
        fprintf(stderr, "The file does not contain presets.\n");
        return 1;
    }

    if (!save_presets(presets.data(), presets.size(), out_path)) {
        fprintf(stderr, "Could not save presets.\n");
        return 1;
    }

    return 0;
}

static bool fread_be32(void *dst, FILE *in)
{
    uint8_t bytes[4];
    if (fread(bytes, 4, 1, in) != 1)
        return false;
    *reinterpret_cast<uint32_t *>(dst) =
        bytes[3] | (bytes[2] << 8) | (bytes[1] << 16) | (bytes[0] << 24);
    return true;
}

static bool fread_le32(void *dst, FILE *in)
{
    uint8_t bytes[4];
    if (fread(bytes, 4, 1, in) != 1)
        return false;
    *reinterpret_cast<uint32_t *>(dst) =
        bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
    return true;
}

static constexpr float ShiruTag(const char *s)
{
    return (float)(s[0] | (s[1] << 7) | (s[2] << 14) | (s[3] << 21));
}

bool load_bank(FILE *in, std::vector<FxPreset> &presets)
{
    char chunk_magic[4];
    uint32_t byte_size;
    char fx_magic[4];
    uint32_t version;
    uint32_t fx_id;
    uint32_t fx_version;
    uint32_t num_programs;

    if (fread(chunk_magic, 4, 1, in) != 1 || !fread_be32(&byte_size, in) ||
        fread(fx_magic, 4, 1, in) != 1 || !fread_be32(&version, in) ||
        !fread_be32(&fx_id, in) || !fread_be32(&fx_version, in) ||
        !fread_be32(&num_programs, in) || fseek(in, +128, SEEK_CUR) != 0)
    {
        return false;
    }

    if (memcmp(chunk_magic, "CcnK", 4) || memcmp(fx_magic, "FBCh", 4))
        return false;

    uint32_t chunk_size;
    if (!fread_be32(&chunk_size, in))
        return false;

    if (chunk_size >= 1024 * 1024)
        return false; // check for absurdly high size field

    uint32_t chunk_item_count = chunk_size / sizeof(float);
    if (chunk_item_count == 0)
        return false;

    {
        float tag;
        if (!fread_le32(&tag, in) || tag != ShiruTag("DATA"))
            return false;
    }

    uint32_t program_number = 0;
    std::map<uint32_t, FxPreset> map_presets;

    for (uint32_t i = 1; i < chunk_item_count - 1;) {
        float tag;
        if (++i, !fread_le32(&tag, in))
            return false;

        if (tag == ShiruTag("PROG")) {
            float value;
            if (++i, !fread_le32(&value, in))
                return false;
            program_number = (uint32_t)value;

            // fprintf(stderr, "* Program %u\n", program_number);
        }
        else if (tag == ShiruTag("NAME")) {
            std::string &program_name = map_presets[program_number].name;

            program_name.clear();
            program_name.reserve(64);

            float value;
            do {
                if (++i, !fread_le32(&value, in))
                    return false;
                program_name.push_back((char)value);
            } while (value != 0);

            // fprintf(stderr, "* Name %s\n", program_name.c_str());
        }
        else if (tag == ShiruTag("DATA") || tag == ShiruTag("DONE")) {
            return false;
        }
        else {
            float value;
            if (++i, !fread_le32(&value, in))
                return false;

            uint32_t param_index = ~0u;

            for (uint32_t i = 0; i < Parameter_Count && param_index == ~0u; ++i) {
                const char *current_symbol = parameters[i].symbol.buffer();
                if (tag == ShiruTag(current_symbol))
                    param_index = i;
            }

            if (param_index == ~0u)
                return false;

            const Parameter &param = parameters[param_index];
            ParameterRanges ranges = param.ranges;

            // from Vst 0-1 to normal range
            value = ranges.min + value * (ranges.max - ranges.min);

            // fprintf(stderr, "  - %s: %f\n", param.name.buffer(), value);

            map_presets[program_number].param[param_index] = value;
        }
    }

    {
        float tag;
        if (!fread_le32(&tag, in) || tag != ShiruTag("DONE"))
            return false;
    }

    for (const auto &pset_key_val : map_presets) {
        const FxPreset &fxp = pset_key_val.second;
        presets.push_back(fxp);
    }

    // remove the filler at end with empty entries
    while (!presets.empty() && is_default_preset(presets.back()))
        presets.pop_back();

    return true;
}

bool save_presets(const FxPreset *presets, uint32_t count, const char *out_path)
{
    mkdir(out_path, 0755);

    std::string ttl_path = std::string(out_path) + "/manifest.ttl";

    FILE *out = fopen(ttl_path.c_str(), "wb");
    if (!out)
        return false;

    fprintf(
        out,
        "@prefix atom: <http://lv2plug.in/ns/ext/atom#> ." "\n"
        "@prefix lv2: <http://lv2plug.in/ns/lv2core#> ." "\n"
        "@prefix pset: <http://lv2plug.in/ns/ext/presets#> ." "\n"
        "@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> ." "\n"
        "@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> ." "\n"
        "@prefix state: <http://lv2plug.in/ns/ext/state#> ." "\n"
        "@prefix xsd: <http://www.w3.org/2001/XMLSchema#> ." "\n");

    for (uint32_t i = 0; i < count; ++i) {
        const FxPreset &fxp = presets[i];

        fprintf(
            out,
            "" "\n"
            "<" DISTRHO_PLUGIN_URI "#MyImportedPreset%03u>" "\n"
            "\t" "a pset:Preset ;" "\n"
            "\t" "lv2:appliesTo <" DISTRHO_PLUGIN_URI "> ;" "\n"
            "\t" "rdfs:label \"\"\"%s\"\"\" ;" "\n",
            i, fxp.name.c_str());

        for (uint32_t i = 0; i < Parameter_Count; ++i) {
            fprintf(
                out,
                "\t" "%s [" "\n"
                "\t\t" "lv2:symbol \"\"\"%s\"\"\" ;" "\n"
                "\t\t" "pset:value %f ;" "\n"
                "\t" "%s" "\n",
                (i == 0) ? "lv2:port" : "] ,",
                parameters[i].symbol.buffer(),
                fxp.param[i],
                (i + 1 == Parameter_Count) ? "] ;" : "");
        }

        fprintf(
            out,
            "\t" "state:state [" "\n"
            "\t\t" "<urn:distrho:ProgramName> \"\"\"%s\"\"\"" "\n"
            "\t" "] ." "\n",
            fxp.name.c_str());
    }

    fflush(out);

    int err = ferror(out);
    fclose(out);

    if (err != 0) {
        unlink(ttl_path.c_str());
        return false;
    }

    return true;
}

bool is_default_preset(const FxPreset &fxp)
{
    for (uint32_t i = 0; i < Parameter_Count; ++i) {
        if (fxp.param[i] != parameters[i].ranges.def)
            return false;
    }
    return true;
}

bool path_has_extension(const char *path, const char *ext)
{
    size_t path_len = strlen(path);
    size_t ext_len = strlen(ext);

    if (ext_len > path_len)
        return false;

    for (size_t i = 0; i < ext_len; ++i) {
        char c1 = path[path_len - ext_len + i];
        char c2 = ext[i];
        c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 - 'A' + 'a') : c1;
        c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 - 'A' + 'a') : c2;
        if (c1 != c2)
            return false;
    }

    return true;
}
