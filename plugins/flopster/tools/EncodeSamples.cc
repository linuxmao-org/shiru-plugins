#include <string>
#include <vector>
#include <cstdio>
#include <cstdarg>
#include <cstring>

std::string vastrprintf(const char *fmt, va_list ap)
{
    char *cstr = nullptr;
    if (vasprintf(&cstr, fmt, ap) == -1)
        return std::string();
    std::string str(cstr);
    free(cstr);
    return str;
}

std::string astrprintf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    std::string str = vastrprintf(fmt, ap);
    va_end(ap);
    return str;
}

static constexpr unsigned HEAD_BUZZ_RANGE = 12*3;
static constexpr unsigned HEAD_SEEK_RANGE = 12*3;
static constexpr unsigned STEP_SAMPLES_ALL = 80;

struct sampleStruct
{
    unsigned char *src;
    signed short int *wave;
    int loop_start;
    int loop_end;
    int length;
};

void SampleFree(sampleStruct *sample)
{
    if(sample&&sample->src)
    {
        delete[] sample->src;

        memset(sample,0,sizeof(sampleStruct));
    }
}

void SampleLoad(sampleStruct *sample, const std::string &filename)
{
    int ptr,size,bytes,align,filesize;
    FILE *file;

    file=fopen(filename.c_str(),"rb");

    if(!file) return;

    fseek(file,0,SEEK_END);
    filesize=ftell(file);
    fseek(file,0,SEEK_SET);

    sample->src=new unsigned char[filesize+8];

    if (fread(sample->src,filesize,1,file) != 1) {
        fclose(file);
        SampleFree(sample);
        return;
    }

    fclose(file);

    if(memcmp(sample->src,"RIFF",4)||memcmp(sample->src+8,"WAVEfmt ",8))
    {
        SampleFree(sample);
        return;
    }

    size=sample->src[4]+(sample->src[5]<<8)+(sample->src[6]<<16)+(sample->src[7]<<24);
    align=sample->src[32]+(sample->src[33]<<8);
    bytes=sample->src[40]+(sample->src[41]<<8)+(sample->src[42]<<16)+(sample->src[43]<<24);

    sample->wave=(signed short int*)(sample->src+44);
    sample->length=bytes/align;
    sample->loop_start=0;
    sample->loop_end=0;

    ptr=44+bytes;

    while(ptr<size)
    {
        if(!memcmp(&sample->src[ptr],"smpl",4))
        {
            if(sample->src[ptr+0x24]+sample->src[ptr+0x25]+sample->src[ptr+0x26]+sample->src[ptr+0x27])
            {
                sample->loop_start=sample->src[ptr+0x34]+(sample->src[ptr+0x35]<<8)+(sample->src[ptr+0x36]<<16)+(sample->src[ptr+0x37]<<24);
                sample->loop_end  =sample->src[ptr+0x38]+(sample->src[ptr+0x39]<<8)+(sample->src[ptr+0x3a]<<16)+(sample->src[ptr+0x3b]<<24);
            }

            break;
        }

        ++ptr;
    }
}

struct SampleDescription {
    SampleDescription(std::string filename, bool group = false, std::string name = {}, size_t index = 0)
        : filename(std::move(filename)), group(group), name(std::move(name)), index(index) {}

    std::string filename;
    bool group = false;
    std::string name;
    size_t index = 0;
};

std::vector<SampleDescription> DescribeAllSamples(const std::string &directory)
{
    std::vector<SampleDescription> v;
    for(unsigned i=0;i<STEP_SAMPLES_ALL;++i)
    {
        v.emplace_back(astrprintf("%s/step_%2.2i.wav",directory.c_str(),i), true, "step", i);
    }
    for(unsigned i=0;i<HEAD_BUZZ_RANGE;++i)
    {
        v.emplace_back(astrprintf("%s/buzz_%2.2i.wav",directory.c_str(),i), true, "buzz", i);
    }
    for(unsigned i=0;i<HEAD_SEEK_RANGE;++i)
    {
        v.emplace_back(astrprintf("%s/seek_%2.2i.wav",directory.c_str(),i), true, "seek", i);
    }
    v.emplace_back(astrprintf("%s/push.wav",directory.c_str()), false, "push");
    v.emplace_back(astrprintf("%s/insert.wav",directory.c_str()), false, "insert");
    v.emplace_back(astrprintf("%s/eject.wav",directory.c_str()), false, "eject");
    v.emplace_back(astrprintf("%s/pull.wav",directory.c_str()), false, "pull");
    v.emplace_back(astrprintf("%s/spindle.wav",directory.c_str()), false, "spindle");
    return v;
}

void LoadAllSamples(const std::string &directory)
{
    std::vector<SampleDescription> desc = DescribeAllSamples(directory);
    std::vector<sampleStruct> smpl(desc.size());

    printf("#include \"FlopsterSamples.hpp\"\n\n");

    for (size_t i = 0, n = desc.size(); i < n; ++i)
        SampleLoad(&smpl[i], desc[i].filename);

    for (size_t i = 0, n = smpl.size(); i < n; ++i) {
        std::string name = desc[i].name;
        if (desc[i].group) name += astrprintf("_%2.2u", desc[i].index);
        printf("static const short Wave_%s[] = {", name.c_str());
        for (size_t j = 0, m = smpl[i].length; j < m; ++j)
            printf("%s%d", j ? "," : "", smpl[i].wave[j]);
        printf("};\n");
    }

    printf("const sampleObject SampleSet[%u] = {\n", (unsigned)desc.size());
    for (size_t i = 0, n = desc.size(); i < n; ++i) {
        std::string name = desc[i].name;
        if (desc[i].group) name += astrprintf("_%2.2u", desc[i].index);
        printf("    {Wave_%s, %u, %u, %u},\n", name.c_str(), smpl[i].loop_start, smpl[i].loop_end, smpl[i].length);
    }
    printf("};\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <sample-directory>\n", argv[0]);
        return 1;
    }

    LoadAllSamples(argv[1]);

    return 0;
}
