#pragma once
#include <utility>
#include <cstddef>

__attribute__((unused))
static const std::pair<const char *, const char *> PresetData[] = {
    {"Saw","P&255"},
    {"Pulse","P&128"},
    {"Pulse Velocity","(P&256)*V/128"},
    {"Saw Decay","(P&255)*((255-(T/500))&255)/256"},
    {"Phat","(P&255)+((P*257/256)&127)"},
    {"Ringy","(P&255)^((P*257/256)&127)"},
    {"Pulsating","((P/11)^((P/12)&4095))&(T&255)"},
    {"Radio","(P*P/200)&255"},
    {"Velocity","P&V"},
    {"Arp 1","(P*((T/4096)&3))&255"},
    {"Hover","P&((P*250/256)&255)"},
    {"Harsh","(P+(P>>1)^(P>>2))&255"},
    {"Noise hit","(P*P>>(T/100))&255"},
    {"Copter","R&255&((P&0XFF0)>>4)"},
    {"Wide bass","(P&255)-(P*256/257&128)"},
    {"Space","(P%256)^(P%255)^(P%254)"},
    {"Gliss Down","(T*100/(P/2000))&128"},
    {"Synth Key","(T*100/(P/2000))&128"},
    {"Metallic 1","(P*1000000/T)&255"},
    {"Metallic 2","(P*10000/(T/10))&128"},
    {"Distortion","((P+P^T)&P*257/256)&255"},
    {"Digi Pad","(P*256/257)&(P^(P*257/256))"},
    {"Arp 2","(T/1500%6)*P&256"},
};

enum {
    PresetDataLength = sizeof(PresetData) / sizeof(PresetData[0]),
};


enum {
    PresetPolyphony = 8, MinPolyphony = 1, MaxPolyphony = 8,
    PresetPortaSpeed = 100, MinPortaSpeed = 1, MaxPortaSpeed = 100,
    PresetOuputGain = 100, MinOutputGain = 1, MaxOutputGain = 200,
};

enum {
    NormalOutputGain = MaxOutputGain / 2,
};
