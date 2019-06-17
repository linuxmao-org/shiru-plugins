#pragma once

struct sampleObject
{
    const signed short int *wave;

    int loop_start;
    int loop_end;

    int length;

    explicit operator bool() const noexcept { return wave != nullptr; }
};

extern const sampleObject SampleSet[];
