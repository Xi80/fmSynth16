//From Luna Tsukinashi
#pragma once

#include<mbed.h>

struct channel {
    uint8_t toneNumber = 0;
};

struct fmStatus {
    bool isUsed = false;
    uint8_t noteNumber = 0;
    uint8_t midiChannel = 0;
    uint16_t priority = 0;
};

struct midiStatus {
    uint8_t bankMsb = 0x00;
    uint8_t partLevel = 100;
    uint8_t expression = 127;
    uint8_t modulation = 0;
    uint16_t pitchBend = 8192;
    bool hold = false;
    uint8_t programNumber = 0;
};

namespace midiMessages {
    const uint8_t noteOff = 0x80;
    const uint8_t noteOn = 0x90;
    const uint8_t controlChange = 0xB0;
    const uint8_t programChange = 0xC0;
    const uint8_t pitchWheelChange = 0xE0;

    const uint8_t allSoundsOff = 0x78;
    const uint8_t allNotesOff = 0x7B;

    const uint8_t beginSystemExclusive = 0xF0;
    const uint8_t endSystemExclusive = 0xF7;
    const uint8_t activeSensing = 0xFE;
}
