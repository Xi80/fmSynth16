//From Luna Tsukinashi
#include<mbed.h>

#include "YMF825/ymf825.h"
#include "MIDI/midi.h"
#include "tables.h"
#include "structs.h"

midi MIDI(PG_14,PG_9,31250);
ymf825 YMF825(PC_12,PC_11,PC_10,PC_9,PC_8);
RawSerial debugPort(USBTX,USBRX,921600);
DigitalOut LEDs[2] = {LED1,LED2};

struct bridgeStatus _bridgeStatus;
struct fmStatus _fmStatus[16];
struct midiStatus _midiStatus[16];

/*
Sakura767 Status Viewer Ver1.2.3
Status:Active

FM Tone Generator Status
VoNo|00|01|02|03|04|05|06|07|08|09|0A|0B|0C|0D|0E|0F|
Used|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|

FM Inst Map
InNo|00|01|02|03|04|05|06|07|08|09|0A|0B|0C|0D|0E|0F|
Inst|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|


MIDI Status
Part|00|01|02|03|04|05|06|07|08|09|0A|0B|0C|0D|0E|0F|
VoCo|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|
Inst|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|
BndS|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|
Bend|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|
Hold|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|
PaLv|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|
Expr|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|
*/

void showInterface(void){
    debugPort.puts("\e[1;1H\e[1;1f");
    debugPort.puts("\e[2J");
    debugPort.puts("\e[36m\e[4m\e[1m\e[4mSakura767 Status Viewer Ver1.2.3\e[0m\n");
    debugPort.puts("Status:Active\n");
    debugPort.puts("\n");
    debugPort.puts("\e[35mFM Tone Generator Status\e[0m\n");
    debugPort.puts("\e[47m\e[30mVoNo|00|01|02|03|04|05|06|07|08|09|0A|0B|0C|0D|0E|0F|\e[0m\n");
    debugPort.puts("Used|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    debugPort.puts("\n");
    debugPort.puts("\e[35mMIDI Status\e[0m\n");
    debugPort.puts("\e[47m\e[30mPart|00|01|02|03|04|05|06|07|08|09|0A|0B|0C|0D|0E|0F|\e[0m\n");
    debugPort.puts("VoCo|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|\n");
    debugPort.puts("Inst|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|\n");
    debugPort.puts("BndS|02|02|02|02|02|02|02|02|02|02|02|02|02|02|02|02|\n");
    debugPort.puts("Bend|80|80|80|80|80|80|80|80|80|80|80|80|80|80|80|80|\n");
    debugPort.puts("Hold|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    debugPort.puts("PaLv|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|\n");
    debugPort.puts("Expr|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|\n");
}

void checkUsed(uint8_t);

void checkVoiceCount(uint8_t);
void checkMidiInst(uint8_t);
void checkBendS(uint8_t);
void checkBend(uint8_t);
void checkHold(uint8_t);
void checkPaLv(uint8_t);
void checkExpr(uint8_t);

void checkUsed(uint8_t number){
    char buffer[40];
    uint8_t pos = 6 + 3* number;
    sprintf(buffer,"\e[6;%dH\e[6;%df",pos,pos);
    debugPort.puts(buffer);
    if(_fmStatus[number].isUsed){
        sprintf(buffer,"%02x|",_fmStatus[number].midiChannel);
        debugPort.puts(buffer);
    } else {
        debugPort.puts("--|");
    }
}

void checkVoiceCount(uint8_t number){
    char buffer[40];
    uint8_t count = 0;
    uint8_t pos = 6 + 3* number;
    for(int i = 0;i < 16;i++){
        if(_fmStatus[i].isUsed && _fmStatus[i].midiChannel == number){
            count++;
        }
    }
    sprintf(buffer,"\e[10;%dH\e[10;%df",pos,pos);
    debugPort.puts(buffer);
    sprintf(buffer,"%02x|",count);
    debugPort.puts(buffer);
}

void checkMidiInst(uint8_t number){
    char buffer[40];
    uint8_t pos = 6 + 3* number;
    sprintf(buffer,"\e[11;%dH\e[11;%df",pos,pos);
    debugPort.puts(buffer);
    sprintf(buffer,"%02x|",_midiStatus[number].programNumber);
    debugPort.puts(buffer);
}

void checkBendS(uint8_t number){
    char buffer[40];
    uint8_t pos = 6 + 3* number;
    sprintf(buffer,"\e[12;%dH\e[12;%df",pos,pos);
    debugPort.puts(buffer);
    sprintf(buffer,"%02x|",_midiStatus[number].pitchBendSensitivity);
    debugPort.puts(buffer);
}

void checkBend(uint8_t number ){
    char buffer[40];
    uint8_t pos = 6 + 3* number;
    sprintf(buffer,"\e[13;%dH\e[13;%df",pos,pos);
    debugPort.puts(buffer);
    if((_midiStatus[number].pitchBend >> 6) < 128){
        debugPort.puts("\e[32m");
    } else if((_midiStatus[number].pitchBend >> 6) > 128){
        debugPort.puts("\e[33m");
    }
    sprintf(buffer,"%02x\e[0m|",_midiStatus[number].pitchBend >> 6);
    debugPort.puts(buffer);
}

void checkHold(uint8_t number){
    char buffer[40];
    uint8_t pos = 6 + 3* number;
    sprintf(buffer,"\e[14;%dH\e[14;%df",pos,pos);
    debugPort.puts(buffer);
    if(_midiStatus[number].hold){
        debugPort.puts("\e[31mOO\e[0m|");
    } else {
        debugPort.puts("--|");
    }
}

void checkPaLv(uint8_t number){
    char buffer[40];
    uint8_t pos = 6 + 3* number;
    sprintf(buffer,"\e[15;%dH\e[15;%df",pos,pos);
    debugPort.puts(buffer);
    if(_midiStatus[number].partLevel < 40){
        debugPort.puts("\e[31m");
    } else if(_midiStatus[number].partLevel < 80){
        debugPort.puts("\e[33m");
    } else {
        debugPort.puts("\e[32m");
    }
    sprintf(buffer,"%02x\e[0m|",_midiStatus[number].partLevel);
    debugPort.puts(buffer);
}

void checkExpr(uint8_t number){
    char buffer[40];
    uint8_t pos = 6 + 3* number;
    sprintf(buffer,"\e[16;%dH\e[16;%df",pos,pos);
    debugPort.puts(buffer);
    if(_midiStatus[number].expression < 40){
        debugPort.puts("\e[31m");
    } else if(_midiStatus[number].expression < 80){
        debugPort.puts("\e[33m");
    } else {
        debugPort.puts("\e[32m");
    }
    sprintf(buffer,"%02x\e[0m|",_midiStatus[number].expression);
    debugPort.puts(buffer);
}

void noteOn(uint8_t channel,uint8_t note,uint8_t velocity){
    LEDs[0] = !LEDs[0];

    if(_bridgeStatus.prevOnCh > 15){
        _bridgeStatus.prevOnCh = 0;
    }

    for(int i = 0;i < 16;i++){
        if(_fmStatus[i].priority != 0){
            _fmStatus[i].priority++;
            if(_fmStatus[i].priority > 511)_fmStatus[i].priority=511;
        }
    }

    if(channel == 9)return;

    for(int i = _bridgeStatus.prevOnCh;i < 16;i++){

        if(_fmStatus[i].isUsed == false){
            _fmStatus[i].isUsed = true;
            _fmStatus[i].noteNumber = note;
            _fmStatus[i].midiChannel = channel;
            YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 3)][(_midiStatus[channel].expression >> 3)]);
            YMF825.setModulation(i,_midiStatus[channel].modulation);
            YMF825.pitchWheelChange(i,_midiStatus[channel].pitchBend,_midiStatus[channel].pitchBendSensitivity);
            YMF825.noteOn(i,note,velocity >> 1,channel);
            _bridgeStatus.prevOnCh++;
            checkUsed(i);
            checkVoiceCount(channel);
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
            checkUsed(i);
            checkVoiceCount(channel);
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
    checkUsed(_bridgeStatus.maxPriorityCh);
    checkVoiceCount(channel);
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
                checkUsed(i);
                checkVoiceCount(channel);
                return;
            }
        }
    }
}

void pitchBend(uint8_t channel,uint16_t pitchBend){
    _midiStatus[channel].pitchBend = pitchBend;
    checkBend(channel);
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
                    checkBendS(channel);
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
            checkPaLv(channel);
            for(int i = 0;i < 16;i++){
                if(_fmStatus[i].isUsed   == true && _fmStatus[i].midiChannel == channel){
                    YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 3)][(_midiStatus[channel].expression >> 3)]);
                }
            }
            break;
        case 11:
            _midiStatus[channel].expression = value;
            checkExpr(channel);
            for(int i = 0;i < 16;i++){
                if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                    YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 3)][(_midiStatus[channel].expression >> 3)]);
                }
            }
            break;
        case 64:
            if(value > 63){
                _midiStatus[channel].hold = true;
                checkHold(channel);
            } else {
                _midiStatus[channel].hold = false;
                checkHold(channel);
                for(int i = 0;i < 16;i++){
                    if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                        YMF825.noteOff(i);
                        _fmStatus[i].priority = 0;
                        _fmStatus[i].isUsed = false;
                        checkUsed(i);
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
    }
}

void programChange(uint8_t channel,uint8_t number){
    _midiStatus[channel].programNumber = number;
    checkMidiInst(channel);
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
    showInterface();
    YMF825.initialise(false);
    YMF825.sendAllToneList();
    for(int i = 0;i < 16;i++){
        checkUsed(i);
        checkVoiceCount(i);
        checkMidiInst(i);
        checkBendS(i);
        checkBend(i);
        checkHold(i);
        checkPaLv(i);
        checkExpr(i);
    }
}

void soundsOff(uint8_t channel){
    YMF825.allMute();
    for(int i = 0;i < 16;i++){
        _fmStatus[i].isUsed = false;
    }
    MIDI.resetStatus();
    reset();
}

void notesOff(uint8_t channel){
    YMF825.allKeyOff();
    for(int i = 0;i < 16;i++){
        _fmStatus[i].isUsed = false;
    }
    MIDI.resetStatus();
    reset();
}

int main(void){
    wait_us(500);
    showInterface();
    MIDI.setCallbacKReceiveControlChange(controlChange);
    MIDI.setCallbackReceiveNoteOn(noteOn);
    MIDI.setCallbackReceiveNoteOff(noteOff);
    MIDI.setCallbackReceivePitchWheelChange(pitchBend);
    MIDI.setCallbackReceiveProgramChange(programChange);
    MIDI.setCallbackReceiveReset(reset);
    MIDI.setCallbackReceiveAllNotesOff(notesOff);
    MIDI.setCallbackReceiveAllSoundsOff(soundsOff);
    reset();
    for(int i = 0;i < 16;i++){
        checkUsed(i);
        checkVoiceCount(i);
        checkMidiInst(i);
        checkBendS(i);
        checkBend(i);
        checkHold(i);
        checkPaLv(i);
        checkExpr(i);
    }
    while(1){
        MIDI.midiParse();
    }
}