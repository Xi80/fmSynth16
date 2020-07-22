//From Luna Tsukinashi
#pragma once

#include <mbed.h>
#include "CircularBuffer.h"
#include "../tables.h"
#include "../structs.h"

const int receiveMessageBufferSize = 1024;

const int systemExclusiveBufferSize = 128;

class midi {
public:
    midi(PinName tx, PinName rx, uint16_t baud);
    //receive Functions

    void resetStatus(void);

    void setCallbackReceiveNoteOff(void (*func)(uint8_t, uint8_t, uint8_t));

    void setCallbackReceiveNoteOn(void (*func)(uint8_t, uint8_t, uint8_t));

    void setCallbacKReceiveControlChange(void (*func)(uint8_t, uint8_t, uint8_t));

    void setCallbackReceiveProgramChange(void (*func)(uint8_t, uint8_t));

    void setCallbackReceivePitchWheelChange(void (*func)(uint8_t, uint16_t));

    void setCallbackReceiveAllSoundsOff(void (*func)(uint8_t));

    void setCallbackReceiveAllNotesOff(void (*func)(uint8_t));

    void setCallbackReceiveReset(void (*func)(void));

    void setCallbackReceiveSystemExclusive(void (*func)(uint8_t *, uint8_t));

    void midiParse(void);

    void rxIrq(bool);

private:
    Serial _serial;
    lunaLib::CircularBuffer <uint8_t, receiveMessageBufferSize> messageBuffer;
    uint8_t systemExclusiveBuffer[systemExclusiveBufferSize];
    uint8_t systemExclusiveBufferPosition = 0;

    void (*callbackFunctionNoteOff)(uint8_t, uint8_t, uint8_t);

    void (*callbackFunctionNoteOn)(uint8_t, uint8_t, uint8_t);

    void (*callbackFunctionControlChange)(uint8_t, uint8_t, uint8_t);

    void (*callbackFunctionProgramChange)(uint8_t, uint8_t);

    void (*callbackFunctionPitchWheelChange)(uint8_t, uint16_t);

    void (*callbackFunctionAllSoundsOff)(uint8_t);

    void (*callbackFunctionAllNotesOff)(uint8_t);

    void (*callbackFunctionReset)(void);

    void (*callbackFunctionSystemExclusive)(uint8_t *, uint8_t);

    uint8_t runningStatusMessage = 0x00;
    uint8_t runningStatusChannel = 0x00;

    bool isSystemExclusiveMode = false;
    bool threeBytesFlag = false;

    uint8_t data[3];

    void receiveMessage(void);

    void decodeMessage(void);

    void decodeSystemExclusive(void);

    bool checkResetMessage(void);
};