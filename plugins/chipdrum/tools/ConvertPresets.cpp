#include "../ChipDrumShared.hpp"
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
    unsigned note = 0;
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
        else if (tag == str_tag("NOTE")) {
            float value;
            if (!read_f32(fh, &value)) {
                fprintf(stderr, "Cannot read note number\n");
                return 1;
            }
            note = value;
            if (note >= SYNTH_NOTES) {
                fprintf(stderr, "Invalid note number\n");
                return 1;
            }
            fprintf(stderr, "Note: %u\n", note);
        }
        else {
            float value;
            if (!read_f32(fh, &value)) {
                fprintf(stderr, "Cannot read parameter value\n");
                return 1;
            }

            int id = -1;

{
    uint32_t tagi = tag;
    fprintf(stderr, "tag: %c %c %c %c\n", tagi & 0xff, (tagi >> 7) & 0xff, (tagi >> 14) & 0xff, (tagi >> 21) & 0xff);
}

            if (tag == str_tag("TLVL")) id = Parameter_ToneLevel1 + note;
            else if (tag == str_tag("TDC1")) id = Parameter_ToneDecay1 + note;
            else if (tag == str_tag("TDCL")) id = Parameter_ToneSustain1 + note;
            else if (tag == str_tag("TDC2")) id = Parameter_ToneRelease1 + note;
            else if (tag == str_tag("TNPI")) id = Parameter_TonePitch1 + note;
            else if (tag == str_tag("TNSL")) id = Parameter_ToneSlide1 + note;
            else if (tag == str_tag("TWAV")) id = Parameter_ToneWave1 + note;
            else if (tag == str_tag("TOVR")) id = Parameter_ToneOver1 + note;

            else if (tag == str_tag("LVLN")) id = Parameter_NoiseLevel1 + note;
            else if (tag == str_tag("DC1N")) id = Parameter_NoiseDecay1 + note;
            else if (tag == str_tag("DCLN")) id = Parameter_NoiseSustain1 + note;
            else if (tag == str_tag("DC2N")) id = Parameter_NoiseRelease1 + note;
            else if (tag == str_tag("PT1N")) id = Parameter_NoisePitch11 + note;
            else if (tag == str_tag("PT2N")) id = Parameter_NoisePitch21 + note;
            else if (tag == str_tag("2OFN")) id = Parameter_NoisePitch2Off1 + note;
            else if (tag == str_tag("2LNN")) id = Parameter_NoisePitch2Len1 + note;
            else if (tag == str_tag("PRDN")) id = Parameter_NoisePeriod1 + note;
            else if (tag == str_tag("PSEN")) id = Parameter_NoiseSeed1 + note;
            else if (tag == str_tag("TYPN")) id = Parameter_NoiseType1 + note;

            else if (tag == str_tag("RTME")) id = Parameter_RetrigTime1 + note;
            else if (tag == str_tag("RCNT")) id = Parameter_RetrigCount1 + note;
            else if (tag == str_tag("RRTE")) id = Parameter_RetrigRoute1 + note;

            else if (tag == str_tag("LLPF")) id = Parameter_FilterLP1 + note;
            else if (tag == str_tag("HHPF")) id = Parameter_FilterHP1 + note;
            else if (tag == str_tag("FLTR")) id = Parameter_FilterRoute1 + note;

            else if (tag == str_tag("GRPO")) id = Parameter_DrumGroup1 + note;
            else if (tag == str_tag("BDPT")) id = Parameter_DrumBitDepth1 + note;
            else if (tag == str_tag("UPDR")) id = Parameter_DrumUpdateRate1 + note;
            else if (tag == str_tag("VOLU")) id = Parameter_DrumVolume1 + note;
            else if (tag == str_tag("PANO")) id = Parameter_DrumPan1 + note;

            else if (tag == str_tag("VDVL")) id = Parameter_VelDrumVolume1 + note;
            else if (tag == str_tag("VTPL")) id = Parameter_VelTonePitch1 + note;
            else if (tag == str_tag("VNPL")) id = Parameter_VelNoisePitch1 + note;
            else if (tag == str_tag("VODL")) id = Parameter_VelToneOver1 + note;

            else if (tag == str_tag("1LNH")) id = Parameter_Hat1Length;
            else if (tag == str_tag("L2NH")) id = Parameter_Hat2Length;
            else if (tag == str_tag("LN3H")) id = Parameter_Hat3Length;
            else if (tag == str_tag("HPAN")) id = Parameter_HatPanWidth;

            else if (tag == str_tag("1PTT")) id = Parameter_Tom1Pitch;
            else if (tag == str_tag("P2TT")) id = Parameter_Tom2Pitch;
            else if (tag == str_tag("PT3T")) id = Parameter_Tom3Pitch;
            else if (tag == str_tag("TPAN")) id = Parameter_TomPanWidth;

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
