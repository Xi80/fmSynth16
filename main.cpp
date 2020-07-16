//From Luna Tsukinashi
#include<mbed.h>

#include "YMF825/ymf825.h"
#include "MIDI/midi.h"
#include "tables.h"
#include "structs.h"

midi MIDI(D1,D0,31250);
ymf825 YMF825(D11,D12,D13,D10,D8);

struct fmStatus _fmStatus[16];
struct midiStatus _midiStatus[16];
uint8_t prevMax     = 0;
uint8_t offChannel  = 0;
uint8_t rpnMsb = 0x7f;
uint8_t rpnLsb = 0x7f;
uint8_t mode = 0;
uint8_t pitchBendSensitivity = 2;
DigitalOut LEDs[] = {A0,A1,A2,A3};
InterruptIn buttons[] = {D4,D5,D6,D7};

void outputLED(uint8_t data){
    LEDs[0] = (data & 0b0001)? 1 : 0;
    LEDs[1] = (data & 0b0010)? 1 : 0;
    LEDs[2] = (data & 0b0100)? 1 : 0;
    LEDs[3] = (data & 0b1000)? 1 : 0;
}

void noteOn(uint8_t channel,uint8_t note,uint8_t velocity){
    if(mode == 0)outputLED(channel);
    prevMax = 0;
    if(channel == 9)return;
    for(int i = 0;i < 16;i++){
        if(_fmStatus[i].priority != 0){
            _fmStatus[i].priority++;
            if(_fmStatus[i].priority > 511)_fmStatus[i].priority=511;
        }
        if(_fmStatus[i].isUsed == false){
            if(mode == 1)outputLED(i);
            _fmStatus[i].isUsed = true;
            _fmStatus[i].noteNumber = note;
            _fmStatus[i].midiChannel = channel;
            YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 2)][(_midiStatus[channel].expression >> 2)]);
            YMF825.setModulation(i,_midiStatus[channel].modulation);
            YMF825.pitchWheelChange(i,_midiStatus[channel].pitchBend,pitchBendSensitivity);
            YMF825.noteOn(i,note,velocity >> 1,channel);
            return;
        }
    }
    for(int i = 0;i < 16;i++){
        if(prevMax <= _fmStatus[i].priority + (_fmStatus[i].midiChannel << 1)&& _fmStatus[i].isUsed != false){
            prevMax = _fmStatus[i].priority + (_fmStatus[i].midiChannel << 1);
            offChannel = i;
        }
    }
    _midiStatus[offChannel].hold = false;
    _fmStatus[offChannel].priority = 0;
    YMF825.noteOff(offChannel);

    _fmStatus[offChannel].isUsed = false;
    noteOn(channel,note,velocity);
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
            YMF825.pitchWheelChange(i,pitchBend,pitchBendSensitivity);
        }
    }
}

uint8_t status = 0x00;
uint16_t rpn = 0x7f7f;
void controlChange(uint8_t channel,uint8_t number,uint8_t value){
    switch(number){
        case 0:
            if(value != 64)return;
            _midiStatus[channel].bankMsb = value;
            break;
        case 1:
            status = (status & 0b100)|0b0001;
            _midiStatus[channel].modulation = value;
            for(int i = 0;i < 16;i++){
                if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                    YMF825.setModulation(i,value);
                }
            }
            break;
        case 6:
        case 38:
            rpn =  (rpnMsb << 7) | rpnLsb;
            if(rpn == 0x7f7f)break;
            switch(rpn){
                case 0x0000:
                    if(value > 24)return;
                    pitchBendSensitivity = value;
                    break;
            }
            break;
        case 7:
            status = (status & 0b100)|0b0010;
            _midiStatus[channel].partLevel = value;
            for(int i = 0;i < 16;i++){
                if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                    YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 2)][(_midiStatus[channel].expression >> 2)]);
                }
            }
            break;
        case 11:
            status = (status & 0b100)|0b0100;
            _midiStatus[channel].expression = value;
            for(int i = 0;i < 16;i++){
                if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                    YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 2)][(_midiStatus[channel].expression >> 2)]);
                }
            }
            break;
        case 64:
            if(value > 63){
                status &= ~(0b1000);
                _midiStatus[channel].hold = true;
            } else {
                status |= 0b1000;
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
            if(value != 0)break;
            rpnLsb = value;
            break;
        case 101:
            if(value != 0)break;
            rpnMsb = value;
            break;
    }
    if(mode == 2)outputLED(status);
}

void programChange(uint8_t channel,uint8_t number){
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
        pitchBendSensitivity = 2;
    }
    YMF825.allKeyOff();
    MIDI.resetStatus();
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

void sysEx(uint8_t *data,uint8_t length){
    if(length != 35)return;
    uint8_t tmp[30];
    for(int i = 0;i < 4;i++){
        if(data[i] != toneHeader[i]){
            return;
        }
    }
    for(int i = 5;i < 35;i++){
        tmp[i - 5]=data[i];
    }
    YMF825.setOriginalTone(data[4],tmp);
}

void int1(void){
    mode = 0;
}

void int2(void){
    mode = 1;
}

void int3(void){
    mode = 2;
}

void int4(void){

}

int main(void){
    for(int i = 0;i < 4;i++){
        buttons[i].mode(PullUp);
    }
    MIDI.setCallbacKReceiveControlChange(controlChange);
    MIDI.setCallbackReceiveNoteOn(noteOn);
    MIDI.setCallbackReceiveNoteOff(noteOff);
    MIDI.setCallbackReceivePitchWheelChange(pitchBend);
    MIDI.setCallbackReceiveProgramChange(programChange);
    MIDI.setCallbackReceiveReset(reset);
    MIDI.setCallbackReceiveAllNotesOff(notesOff);
    MIDI.setCallbackReceiveAllSoundsOff(soundsOff);
    MIDI.setCallbackReceiveSystemExclusive(sysEx);
    reset();

    buttons[0].fall(int1);
    buttons[1].fall(int2);
    buttons[2].fall(int3);
    buttons[3].fall(int4);
    while(1){
        MIDI.midiParse();
    }
}