//From Luna Tsukinashi
#include "midi.h"

midi::midi(PinName tx, PinName rx, uint16_t baud) : _serial(tx, rx) {
    _serial.baud(baud);

    callbackFunctionNoteOff = NULL;
    callbackFunctionNoteOn = NULL;
    callbackFunctionControlChange = NULL;
    callbackFunctionProgramChange = NULL;
    callbackFunctionPitchWheelChange = NULL;

    callbackFunctionAllSoundsOff = NULL;
    callbackFunctionAllNotesOff = NULL;
    callbackFunctionReset = NULL;
    resetStatus();
    _serial.attach(callback(this, &midi::receiveMessage), Serial::RxIrq);
}

void midi::resetStatus(void) {
    messageBuffer.clear();
    systemExclusiveBufferPosition = 0;
    return;
}

void midi::setCallbackReceiveNoteOff(void (*func)(uint8_t, uint8_t, uint8_t)) {
    callbackFunctionNoteOff = func;
    return;
}

void midi::setCallbackReceiveNoteOn(void (*func)(uint8_t, uint8_t, uint8_t)) {
    callbackFunctionNoteOn = func;
    return;
}

void midi::setCallbacKReceiveControlChange(void (*func)(uint8_t, uint8_t, uint8_t)) {
    callbackFunctionControlChange = func;
    return;
}

void midi::setCallbackReceiveProgramChange(void (*func)(uint8_t, uint8_t)) {
    callbackFunctionProgramChange = func;
    return;
}


void midi::setCallbackReceivePitchWheelChange(void (*func)(uint8_t, uint16_t)) {
    callbackFunctionPitchWheelChange = func;
    return;
}

void midi::setCallbackReceiveAllSoundsOff(void (*func)(uint8_t)) {
    callbackFunctionAllSoundsOff = func;
    return;
}


void midi::setCallbackReceiveAllNotesOff(void (*func)(uint8_t)) {
    callbackFunctionAllNotesOff = func;
    return;
}

void midi::setCallbackReceiveSystemExclusive(void (*func)(uint8_t *, uint8_t)) {
    callbackFunctionSystemExclusive = func;
    return;
}

void midi::setCallbackReceiveReset(void (*func)(void)) {
    callbackFunctionReset = func;
    return;
}

void midi::midiParse(void) {
    if (!messageBuffer.size())return;
    data[0] = messageBuffer.pull();

    if (isSystemExclusiveMode) {
        if (data[0] == midiMessages::endSystemExclusive) {
            isSystemExclusiveMode = false;
            decodeSystemExclusive();
            return;
        } else {
            if (systemExclusiveBufferPosition > systemExclusiveBufferSize)systemExclusiveBufferPosition = 0;
            if (data[0] == midiMessages::activeSensing)return;
            systemExclusiveBuffer[systemExclusiveBufferPosition++] = data[0];
            return;
        }
    } else {
        if (data[0] == 0xFE)return;
        if (data[0] == 0xF0) {
            isSystemExclusiveMode = true;
            systemExclusiveBufferPosition = 0;
            return;
        }
        if (data[0] & 0x80) {
            if (!(data[0] >= 0xF8)) {
                runningStatusMessage = data[0] & 0xF0;
                runningStatusChannel = data[0] & 0x0F;
                threeBytesFlag = false;
                return;
            }
        } else {
            if (threeBytesFlag) {
                threeBytesFlag = false;
                data[2] = data[0];
                decodeMessage();
                return;
            } else {
                if (runningStatusMessage < 0xC0) {
                    threeBytesFlag = true;
                    data[1] = data[0];
                    return;
                } else {
                    if (runningStatusMessage < 0xE0) {
                        data[1] = data[0];
                        decodeMessage();
                        return;
                    } else {
                        if (runningStatusMessage < 0xF0) {
                            threeBytesFlag = true;
                            data[1] = data[0];
                            return;
                        }
                    }
                }
            }
        }
    }
}


void midi::receiveMessage(void) {
    uint8_t data = _serial.getc();
    if (messageBuffer.size() > receiveMessageBufferSize - 1)messageBuffer.clear();
    messageBuffer.push_back(data);
    return;
}

Serial _debug(USBTX, USBRX, 38400);

void midi::decodeMessage(void) {
    switch (runningStatusMessage) {
        case midiMessages::noteOff:
            if (callbackFunctionNoteOff != NULL)callbackFunctionNoteOff(runningStatusChannel, data[1], data[2]);
            break;
        case midiMessages::noteOn:
            if (data[2] == 0) {
                if (callbackFunctionNoteOff != NULL)callbackFunctionNoteOff(runningStatusChannel, data[1], data[2]);
            } else {
                if (callbackFunctionNoteOn != NULL)callbackFunctionNoteOn(runningStatusChannel, data[1], data[2]);
            }
            break;
        case midiMessages::controlChange:
            switch (data[1]) {
                case midiMessages::allSoundsOff:
                    if (callbackFunctionAllSoundsOff != NULL)callbackFunctionAllSoundsOff(runningStatusChannel);
                    break;
                case midiMessages::allNotesOff:
                    if (callbackFunctionAllNotesOff != NULL)callbackFunctionAllNotesOff(runningStatusChannel);
                    break;
                default:
                    if (callbackFunctionControlChange != NULL)
                        callbackFunctionControlChange(runningStatusChannel, data[1], data[2]);
                    break;
            }
            break;
        case midiMessages::programChange:
            if (callbackFunctionProgramChange != NULL)callbackFunctionProgramChange(runningStatusChannel, data[1]);
            break;
        case midiMessages::pitchWheelChange:
            uint16_t bend = (data[2] << 7) | data[1];
            if (callbackFunctionPitchWheelChange != NULL)callbackFunctionPitchWheelChange(runningStatusChannel, bend);
            break;
    }
}

void midi::decodeSystemExclusive(void) {
    if (checkResetMessage()) {
        if (callbackFunctionReset != NULL)callbackFunctionReset();
    } else {
        if (callbackFunctionSystemExclusive != NULL)
            callbackFunctionSystemExclusive(systemExclusiveBuffer, systemExclusiveBufferPosition);
    }
    systemExclusiveBufferPosition = 0;
}


bool midi::checkResetMessage(void) {
    if (systemExclusiveBufferPosition == 4) {
        for (int i = 0; i < 4; i++) {
            if (systemExclusiveBuffer[i] != gmSystemOn[i]) {
                return false;
            }
        }
    } else if (systemExclusiveBufferPosition == 7) {
        for (int i = 0; i < 7; i++) {
            if (systemExclusiveBuffer[i] != xgSystemOn[i]) {
                return false;
            }
        }
    } else if (systemExclusiveBufferPosition == 9) {
        for (int i = 0; i < 9; i++) {
            if (systemExclusiveBuffer[i] != gsReset[i]) {
                return false;
            }
        }
    } else {
        return false;
    }
    return true;
}