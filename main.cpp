//From Luna Tsukinashi
#include<mbed.h>

#include "YMF825/ymf825.h"
#include "MIDI/midi.h"
#include "tables.h"
#include "structs.h"

midi MIDI(PG_14,PG_9,31250);
ymf825 YMF825(PC_12,PC_11,PC_10,PC_9,PC_8);
DigitalOut LEDs[3] = {LED1,LED2,LED3};
struct bridgeStatus _bridgeStatus;
struct fmStatus _fmStatus[16];
struct midiStatus _midiStatus[16];

void noteOn(uint8_t channel,uint8_t note,uint8_t velocity){
    LEDs[0] = !LEDs[0];
    if(_bridgeStatus.prevOnCh > 15){
        _bridgeStatus.prevOnCh = 0;
    }

    if(channel == 9)return;
    for(int i = _bridgeStatus.prevOnCh;i < 16;i++){
        if(_fmStatus[i].priority != 0){
            _fmStatus[i].priority++;
            if(_fmStatus[i].priority > 511)_fmStatus[i].priority=511;
        }
        if(_fmStatus[i].isUsed == false){
            _fmStatus[i].isUsed = true;
            _fmStatus[i].noteNumber = note;
            _fmStatus[i].midiChannel = channel;
            YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 3)][(_midiStatus[channel].expression >> 3)]);
            YMF825.setModulation(i,_midiStatus[channel].modulation);
            YMF825.pitchWheelChange(i,_midiStatus[channel].pitchBend,_midiStatus[channel].pitchBendSensitivity);
            YMF825.noteOn(i,note,velocity >> 1,channel);
            _bridgeStatus.prevOnCh++;
            return;
        }
    }
    for(int i = 0;i < _bridgeStatus.prevOnCh + 1;i++){
        if(_fmStatus[i].priority != 0){
            _fmStatus[i].priority++;
            if(_fmStatus[i].priority > 511)_fmStatus[i].priority=511;
        }
        if(_fmStatus[i].isUsed == false){
            _fmStatus[i].isUsed = true;
            _fmStatus[i].noteNumber = note;
            _fmStatus[i].midiChannel = channel;
            YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 3)][(_midiStatus[channel].expression >> 3)]);
            YMF825.setModulation(i,_midiStatus[channel].modulation);
            YMF825.pitchWheelChange(i,_midiStatus[channel].pitchBend,_midiStatus[channel].pitchBendSensitivity);
            YMF825.noteOn(i,note,velocity >> 1,channel);
            _bridgeStatus.prevOnCh++;
            return;
        }
    }
    _bridgeStatus.maxPriorityVal = 0;
    for(int i = 0;i < 16;i++){
        if( _bridgeStatus.maxPriorityVal <= _fmStatus[i].priority + (_fmStatus[i].midiChannel << 1)&& _fmStatus[i].isUsed != false){
             _bridgeStatus.maxPriorityVal = _fmStatus[i].priority + (_fmStatus[i].midiChannel << 1);
             _bridgeStatus.maxPriorityCh = i;
        }
    }
    _midiStatus[_bridgeStatus.maxPriorityCh].hold = false;
    _fmStatus[_bridgeStatus.maxPriorityCh].priority = 0;
    YMF825.noteOff(_bridgeStatus.maxPriorityCh);
    YMF825.noteOn(_bridgeStatus.maxPriorityCh,note,velocity >> 1,channel);

    return;
}

void noteOff(uint8_t channel,uint8_t note,uint8_t velocity){
    if(channel == 9)return;
    for(int i = 0;i < 16;i++){
        if(_fmStatus[i].isUsed == true && _fmStatus[i].noteNumber == note && _fmStatus[i].midiChannel == channel){
            if(_midiStatus[channel].hold == false){
                _fmStatus[i].isUsed = false;
                YMF825.noteOff(i);
                _fmStatus[i].priority = 0;
                return;
            }

        }
    }
}

void pitchBend(uint8_t channel,uint16_t pitchBend){
    _midiStatus[channel].pitchBend = pitchBend;
    for(int i = 0;i < 16;i++){
        if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
            YMF825.pitchWheelChange(i,pitchBend,_midiStatus[channel].pitchBendSensitivity);
        }
    }
}

void controlChange(uint8_t channel,uint8_t number,uint8_t value){
    switch(number){
        case 0:
            if(value != 64)return;
            _midiStatus[channel].bankMsb = value;
            break;
        case 1:
            _midiStatus[channel].modulation = value;
            for(int i = 0;i < 16;i++){
                if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                    YMF825.setModulation(i,value);
                }
            }
            break;
        case 6:
        case 38:
            _bridgeStatus.rpn =  (_bridgeStatus.rpnMsb << 8) | _bridgeStatus.rpnLsb;
            if(_bridgeStatus.rpn == 0x7f7f)break;
            switch(_bridgeStatus.rpn){
                case 0x0000:
                    if(value > 24)return;
                    _midiStatus[channel].pitchBendSensitivity = value;
                    _bridgeStatus.rpnLsb = 0x7f;
                    _bridgeStatus.rpnMsb = 0x7f;
                    _bridgeStatus.rpn = 0x7F7F;
                    break;
                default:
                    _bridgeStatus.rpnLsb = 0x7f;
                    _bridgeStatus.rpnMsb = 0x7f;
                    _bridgeStatus.rpn = 0x7F7F;
                    break;
            }
            break;
        case 7:
            _midiStatus[channel].partLevel = value;
            for(int i = 0;i < 16;i++){
                if(_fmStatus[i].isUsed   == true && _fmStatus[i].midiChannel == channel){
                    YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 3)][(_midiStatus[channel].expression >> 3)]);
                }
            }
            break;
        case 11:
            _midiStatus[channel].expression = value;
            for(int i = 0;i < 16;i++){
                if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                    YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 3)][(_midiStatus[channel].expression >> 3)]);
                }
            }
            break;
        case 64:
            if(value > 63){
                for(int i = 0;i < 16;i++){
                    if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                    }
                }
                _midiStatus[channel].hold = true;
            } else {
                _midiStatus[channel].hold = false;
                for(int i = 0;i < 16;i++){
                    if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                        YMF825.noteOff(i);
                        _fmStatus[i].priority = 0;
                        _fmStatus[i].isUsed = false;
                    }
                }
            }
            break;
        case 100:
            _bridgeStatus.rpnLsb = value;
            break;
        case 101:
            _bridgeStatus.rpnMsb = value;
            break;
        case 121:
            _midiStatus[channel].partLevel = 100;
            _midiStatus[channel].expression = 127;
            _midiStatus[channel].hold = false;
            _midiStatus[channel].pitchBend = 8192;
            _midiStatus[channel].pitchBendSensitivity = 2;
            break;
    }
}

void programChange(uint8_t channel,uint8_t number){
    _midiStatus[channel].programNumber = number;
    if(_midiStatus[channel].bankMsb == 64){
        YMF825.setToneListFromGM(channel,number + 128);
    } else {
        YMF825.setToneListFromGM(channel,number);
    }
    YMF825.sendToneList(channel);
    return;
}

void reset(void){
    for(int i = 0;i < 16;i++){
        _fmStatus[i].isUsed = false;
        _fmStatus[i].midiChannel = 0;
        _fmStatus[i].noteNumber = 0;
        _fmStatus[i].priority = 0;
        _midiStatus[i].bankMsb = 0x00;
        _midiStatus[i].hold = false;
        _midiStatus[i].modulation = 0;
        _midiStatus[i].partLevel = 100;
        _midiStatus[i].expression = 127;
        _midiStatus[i].pitchBend = 8192;
        _midiStatus[i].pitchBendSensitivity = 2;
    }
    YMF825.initialise(false);
    YMF825.sendAllToneList();
}

void soundsOff(uint8_t channel){
    YMF825.allMute();
    for(int i = 0;i < 16;i++){
        _fmStatus[i].isUsed = false;
    }
    MIDI.resetStatus();
}

void notesOff(uint8_t channel){
    YMF825.allKeyOff();
    for(int i = 0;i < 16;i++){
        _fmStatus[i].isUsed = false;
    }
    MIDI.resetStatus();
}

int main(void){
    MIDI.setCallbacKReceiveControlChange(controlChange);
    MIDI.setCallbackReceiveNoteOn(noteOn);
    MIDI.setCallbackReceiveNoteOff(noteOff);
    MIDI.setCallbackReceivePitchWheelChange(pitchBend);
    MIDI.setCallbackReceiveProgramChange(programChange);
    MIDI.setCallbackReceiveReset(reset);
    MIDI.setCallbackReceiveAllNotesOff(notesOff);
    MIDI.setCallbackReceiveAllSoundsOff(soundsOff);
    reset();
    while(1){
        MIDI.midiParse();
    }
}