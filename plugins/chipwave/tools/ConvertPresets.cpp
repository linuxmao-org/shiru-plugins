#include "../DistrhoPluginInfo.h"
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

            if (tag == str_tag("OAWF")) id = Parameter_OscAWave;
            else if (tag == str_tag("OADU")) id = Parameter_OscADuty;
            else if (tag == str_tag("OAOV")) id = Parameter_OscAOver;
            else if (tag == str_tag("OAMU")) id = Parameter_OscAMultiple;
            else if (tag == str_tag("OASD")) id = Parameter_OscASeed;
            else if (tag == str_tag("OACT")) id = Parameter_OscACut;

            else if (tag == str_tag("OBWF")) id = Parameter_OscBWave;
            else if (tag == str_tag("OBDU")) id = Parameter_OscBDuty;
            else if (tag == str_tag("OBOV")) id = Parameter_OscBOver;
            else if (tag == str_tag("OBDE")) id = Parameter_OscBDetune;
            else if (tag == str_tag("OBMU")) id = Parameter_OscBMultiple;
            else if (tag == str_tag("OBSD")) id = Parameter_OscBSeed;
            else if (tag == str_tag("OBCT")) id = Parameter_OscBCut;

            else if (tag == str_tag("OBAL")) id = Parameter_OscBalance;
            else if (tag == str_tag("OMIX")) id = Parameter_OscMixMode;

            else if (tag == str_tag("FLCU")) id = Parameter_FltCutoff;
            else if (tag == str_tag("FLRE")) id = Parameter_FltReso;

            else if (tag == str_tag("SLDE")) id = Parameter_SlideDelay;
            else if (tag == str_tag("SLSP")) id = Parameter_SlideSpeed;
            else if (tag == str_tag("SLRO")) id = Parameter_SlideRoute;

            else if (tag == str_tag("ENAT")) id = Parameter_EnvAttack;
            else if (tag == str_tag("ENDC")) id = Parameter_EnvDecay;
            else if (tag == str_tag("ENSU")) id = Parameter_EnvSustain;
            else if (tag == str_tag("ENRE")) id = Parameter_EnvRelease;

            else if (tag == str_tag("ENOA")) id = Parameter_EnvOscADepth;
            else if (tag == str_tag("ENOB")) id = Parameter_EnvOscBDepth;
            else if (tag == str_tag("ENOD")) id = Parameter_EnvOscBDetuneDepth;
            else if (tag == str_tag("ENMX")) id = Parameter_EnvOscMixDepth;
            else if (tag == str_tag("ENFL")) id = Parameter_EnvFltDepth;
            else if (tag == str_tag("ENLF")) id = Parameter_EnvLfoDepth;

            else if (tag == str_tag("LFSP")) id = Parameter_LfoSpeed;
            else if (tag == str_tag("LFPI")) id = Parameter_LfoPitchDepth;
            else if (tag == str_tag("LFOA")) id = Parameter_LfoOscADepth;
            else if (tag == str_tag("LFOB")) id = Parameter_LfoOscBDepth;
            else if (tag == str_tag("LFMX")) id = Parameter_LfoOscMixDepth;
            else if (tag == str_tag("LFFL")) id = Parameter_LfoFltDepth;

            else if (tag == str_tag("VOAT")) id = Parameter_AmpAttack;
            else if (tag == str_tag("VODE")) id = Parameter_AmpDecay;
            else if (tag == str_tag("VOSU")) id = Parameter_AmpSustain;
            else if (tag == str_tag("VORE")) id = Parameter_AmpRelease;

            else if (tag == str_tag("VLAM")) id = Parameter_VelAmp;
            else if (tag == str_tag("VLOA")) id = Parameter_VelOscADepth;
            else if (tag == str_tag("VLOB")) id = Parameter_VelOscBDepth;
            else if (tag == str_tag("VLMX")) id = Parameter_VelOscMixDepth;
            else if (tag == str_tag("VLFC")) id = Parameter_VelFltCutoff;
            else if (tag == str_tag("VLFR")) id = Parameter_VelFltReso;

            else if (tag == str_tag("POLY")) id = Parameter_Polyphony;
            else if (tag == str_tag("POSP")) id = Parameter_PortaSpeed;
            else if (tag == str_tag("GAIN")) id = Parameter_OutputGain;

            if (id == -1) {
                fprintf(stderr, "Unrecognized parameter ID\n");
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
