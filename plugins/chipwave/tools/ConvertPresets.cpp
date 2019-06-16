#include "../ChipWaveShared.hpp"
#include <endian.h>
#include <string>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>

static float str_tag(const char s[4])
{
    return (float)(((s)[0])|(((s)[1])<<7)|(((s)[2])<<14)|(((s)[3]<<21)));
}

static bool read_u32_be(FILE *fh, uint32_t *num)
{
    if (fread(num, sizeof(*num), 1, fh) != 1)
        return false;
    *num = be32toh(*num);
    return true;
}

static bool read_f32(FILE *fh, float *num)
{
    union { uint32_t i; float f; } u;
    if (fread(&u.i, sizeof(u.i), 1, fh) != 1)
        return false;
    //u.i = be32toh(u.i);
    *num = u.f;
    return true;
}

static bool read_name(FILE *fh, std::string *name)
{
    name->clear();

    for (;;) {
        float cf;
        if (!read_f32(fh, &cf))
            return false;
        unsigned c = (unsigned)cf;
        if (c == 0)
            break;
        name->push_back(c);
    }

    return true;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <fxb-file>\n", argv[0]);
        return 1;
    }

    FILE *fh = fopen(argv[1], "rb");
    if (!fh) {
        fprintf(stderr, "Cannot open file: %s\n", argv[1]);
        return 1;
    }

    struct Header {
        int32_t chunkMagic;
        int32_t byteSize;
        int32_t fxMagic;
        int32_t version;
        int32_t fxID;
        int32_t fxVersion;
        int32_t numPrograms;
        char future[128];
    };

    auto cconst = [](unsigned char a, unsigned char b, unsigned char c, unsigned char d) -> uint32_t
        { return ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | (uint32_t)d; };

    Header hdr;
    if (fread((char *)&hdr, sizeof(Header), 1, fh) != 1) {
        fprintf(stderr, "Cannot read file header\n");
        return 1;
    }

    if (be32toh(hdr.chunkMagic) != cconst('C', 'c', 'n', 'K')) {
        fprintf(stderr, "Chunk header is invalid\n");
        return 1;
    }

    if (be32toh(hdr.fxMagic) != cconst('F', 'B', 'C', 'h')) {
        fprintf(stderr, "Fx header is invalid\n");
        return 1;
    }

    uint32_t numPrograms = be32toh(hdr.numPrograms);
    fprintf(stderr, "Num Programs: %u\n", numPrograms);

    uint32_t bankSize;
    if (!read_u32_be(fh, &bankSize)) {
        fprintf(stderr, "Cannot read bank size\n");
        return 1;
    }

    ///
    float tag;
    if (!read_f32(fh, &tag)) {
        fprintf(stderr, "Cannot read tag\n");
        return 1;
    }

    if (tag != str_tag("DATA")) {
        fprintf(stderr, "Expected start tag 'DATA'\n");
        return 1;
    }

    unsigned pgm = 0;
    std::string *name = new std::string[numPrograms];
    std::array<float, Parameter_Count> *values = new std::array<float, Parameter_Count>[numPrograms];

    Parameter param[Parameter_Count];
    for (unsigned p = 0; p < Parameter_Count; ++p)
        InitParameter(p, param[p]);

    for (;;) {
        if (!read_f32(fh, &tag)) {
            fprintf(stderr, "Cannot read tag\n");
            return 1;
        }

        if (tag == str_tag("DONE")) {
            fprintf(stderr, "Done\n");
            break;
        }

        if (tag == str_tag("NAME")) {
            if (!read_name(fh, &name[pgm])) {
                fprintf(stderr, "Cannot read program name\n");
                return 1;
            }
            fprintf(stderr, "Name: %s\n", name[pgm].c_str());
        }
        else if (tag == str_tag("PROG")) {
            float value;
            if (!read_f32(fh, &value)) {
                fprintf(stderr, "Cannot read program number\n");
                return 1;
            }
            pgm = value;
            if (pgm >= numPrograms) {
                fprintf(stderr, "Invalid program number\n");
                return 1;
            }
            fprintf(stderr, "Program: %u\n", pgm);
        }
        else {
            float value;
            if (!read_f32(fh, &value)) {
                fprintf(stderr, "Cannot read parameter value\n");
                return 1;
            }

            int id = -1;

            if (tag == str_tag("OAWF")) id = pIdOscAWave;
            else if (tag == str_tag("OADU")) id = pIdOscADuty;
            else if (tag == str_tag("OAOV")) id = pIdOscAOver;
            else if (tag == str_tag("OAMU")) id = pIdOscAMultiple;
            else if (tag == str_tag("OASD")) id = pIdOscASeed;
            else if (tag == str_tag("OACT")) id = pIdOscACut;

            else if (tag == str_tag("OBWF")) id = pIdOscBWave;
            else if (tag == str_tag("OBDU")) id = pIdOscBDuty;
            else if (tag == str_tag("OBOV")) id = pIdOscBOver;
            else if (tag == str_tag("OBDE")) id = pIdOscBDetune;
            else if (tag == str_tag("OBMU")) id = pIdOscBMultiple;
            else if (tag == str_tag("OBSD")) id = pIdOscBSeed;
            else if (tag == str_tag("OBCT")) id = pIdOscBCut;

            else if (tag == str_tag("OBAL")) id = pIdOscBalance;
            else if (tag == str_tag("OMIX")) id = pIdOscMixMode;

            else if (tag == str_tag("FLCU")) id = pIdFltCutoff;
            else if (tag == str_tag("FLRE")) id = pIdFltReso;

            else if (tag == str_tag("SLDE")) id = pIdSlideDelay;
            else if (tag == str_tag("SLSP")) id = pIdSlideSpeed;
            else if (tag == str_tag("SLRO")) id = pIdSlideRoute;

            else if (tag == str_tag("ENAT")) id = pIdEnvAttack;
            else if (tag == str_tag("ENDC")) id = pIdEnvDecay;
            else if (tag == str_tag("ENSU")) id = pIdEnvSustain;
            else if (tag == str_tag("ENRE")) id = pIdEnvRelease;

            else if (tag == str_tag("ENOA")) id = pIdEnvOscADepth;
            else if (tag == str_tag("ENOB")) id = pIdEnvOscBDepth;
            else if (tag == str_tag("ENOD")) id = pIdEnvOscBDetuneDepth;
            else if (tag == str_tag("ENMX")) id = pIdEnvOscMixDepth;
            else if (tag == str_tag("ENFL")) id = pIdEnvFltDepth;
            else if (tag == str_tag("ENLF")) id = pIdEnvLfoDepth;

            else if (tag == str_tag("LFSP")) id = pIdLfoSpeed;
            else if (tag == str_tag("LFPI")) id = pIdLfoPitchDepth;
            else if (tag == str_tag("LFOA")) id = pIdLfoOscADepth;
            else if (tag == str_tag("LFOB")) id = pIdLfoOscBDepth;
            else if (tag == str_tag("LFMX")) id = pIdLfoOscMixDepth;
            else if (tag == str_tag("LFFL")) id = pIdLfoFltDepth;

            else if (tag == str_tag("VOAT")) id = pIdAmpAttack;
            else if (tag == str_tag("VODE")) id = pIdAmpDecay;
            else if (tag == str_tag("VOSU")) id = pIdAmpSustain;
            else if (tag == str_tag("VORE")) id = pIdAmpRelease;

            else if (tag == str_tag("VLAM")) id = pIdVelAmp;
            else if (tag == str_tag("VLOA")) id = pIdVelOscADepth;
            else if (tag == str_tag("VLOB")) id = pIdVelOscBDepth;
            else if (tag == str_tag("VLMX")) id = pIdVelOscMixDepth;
            else if (tag == str_tag("VLFC")) id = pIdVelFltCutoff;
            else if (tag == str_tag("VLFR")) id = pIdVelFltReso;

            else if (tag == str_tag("POLY")) id = pIdPolyphony;
            else if (tag == str_tag("POSP")) id = pIdPortaSpeed;
            else if (tag == str_tag("GAIN")) id = pIdOutputGain;

            if (id == -1) {
                fprintf(stderr, "Unrecognized parameter ID\n");
                return 1;
            }

            value = value * (param[id].ranges.max - param[id].ranges.min) + param[id].ranges.min;
            float valueNotFixed = value;
            param[id].ranges.fixValue(value);
            if (value != valueNotFixed) {
                fprintf(stderr, "Preset value is not in range.\n");
                return 1;
            }

            fprintf(stderr, "Parameter: %d/%d Value: %f\n", id+1, Parameter_Count, value);

            values[pgm][id] = value;
        }
    }

    unsigned pgm_count = 0;
    for (unsigned pgm = 0; pgm < numPrograms; ++pgm) {
        if (name[pgm] != "default")
            ++pgm_count;
    }

    printf("static const std::array<Preset, %u> PresetData = {\n", pgm_count);
    for (unsigned pgm = 0; pgm < numPrograms; ++pgm) {
        if (name[pgm] == "default")
            continue;

        printf("    /* %3u */ Preset{\"%s\",\n     {", pgm, name[pgm].c_str());
        for (unsigned i = 0; i < Parameter_Count; ++i)
            printf("%s%f", (i == 0) ? "" : ", ", values[pgm][i]);
        printf("}},\n");
    }
    printf("};\n");

    return 0;
}
