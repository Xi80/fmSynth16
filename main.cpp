//From Luna Tsukinashi
#include<mbed.h>

#include "YMF825/ymf825.h"
#include "MIDI/midi.h"
#include "tables.h"
#include "structs.h"

midi MIDI(PG_14,PG_9,31250);
ymf825 YMF825(PC_12,PC_11,PC_10,PC_9,PC_8);
Serial debugPort(USBTX,USBRX,921600);

struct fmStatus _fmStatus[16];
struct midiStatus _midiStatus[16];
uint8_t prevCh = 0;
uint8_t prevMax     = 0;
uint8_t offChannel  = 0;
uint8_t rpnMsb = 0x7f;
uint8_t rpnLsb = 0x7f;
uint8_t mode = 0;
uint8_t pitchBendSensitivity = 2;

void showInterFace(void){
    debugPort.puts("\e[1;1H\e[1;1f");
    debugPort.puts("\e[2J");
    debugPort.puts("Sakura767 Status Viewer Ver 1.0.0\n");
    debugPort.puts("Status:Active\n");
    debugPort.puts("vono|00|01|02|03|04|05|06|07|08|09|0A|0B|0C|0D|0E|0F|\n");
    debugPort.puts("chno|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    debugPort.puts("part|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    debugPort.puts("expr|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    debugPort.puts("hold|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    debugPort.puts("tone|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    debugPort.puts("pitb|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    debugPort.puts("Voices:--\n");
    debugPort.puts("Flags:--\n");
    debugPort.puts("\n");
}

void showExpression(uint8_t ch,bool status);
void showPartLevel(uint8_t ch,bool status);
void showTone(uint8_t ch,bool status);
void showHold(uint8_t ch,bool status);
void showPitchBend(uint8_t ch,bool status);
void showVoiceCount(void);

void showChannelNumber(uint8_t ch,bool status){
    char buffer[20] = "";
    sprintf(buffer,"\e[4;%dH\e[4;%df|",(5 +(3 * ch)),(5 +(3 * ch)));
    debugPort.puts(buffer);
    if(status == true){
        sprintf(buffer,"%02x|",_fmStatus[ch].midiChannel);
        debugPort.puts(buffer);
        showExpression(ch,true);
        showPartLevel(ch,true);
        showTone(ch,true);
        showHold(ch,true);
        showPitchBend(ch,true);
    } else {
        debugPort.puts("--|");
        showExpression(ch,false);
        showPartLevel(ch,false);
        showTone(ch,false);
        showHold(ch,false);
        showPitchBend(ch,false);
    }
    showVoiceCount();
    return;
}

void showPartLevel(uint8_t ch,bool status){
    char buffer[20] = "";
    uint8_t pl = _midiStatus[_fmStatus[ch].midiChannel].partLevel;
    sprintf(buffer,"\e[5;%dH\e[5;%df|",(5 +(3 * ch)),(5 +(3 * ch)));
    debugPort.puts(buffer);
    if(status == true){
        sprintf(buffer,"%02x\e[0m|",pl);
        if(pl < 40){
            debugPort.puts("\e[31m");
        } else if(pl < 80){
            debugPort.puts("\e[33m");
        } else {
            debugPort.puts("\e[32m");
        }
        debugPort.puts(buffer);
    } else {
        debugPort.puts("--|");
    }
}

void showExpression(uint8_t ch,bool status){
    char buffer[20] = "";
    uint8_t exp = _midiStatus[_fmStatus[ch].midiChannel].expression;
    sprintf(buffer,"\e[6;%dH\e[6;%df|",(5 +(3 * ch)),(5 +(3 * ch)));
    debugPort.puts(buffer);
    if(status == true){
        sprintf(buffer,"%02x\e[0m|",exp);
        if(exp < 40){
            debugPort.puts("\e[31m");
        } else if(exp < 80){
            debugPort.puts("\e[33m");
        } else {
            debugPort.puts("\e[32m");
        }
        debugPort.puts(buffer);
    } else {
        debugPort.puts("--|");
    }
}

void showHold(uint8_t ch,bool status){
    char buffer[20] = "";
    sprintf(buffer,"\e[7;%dH\e[7;%df|",(5 +(3 * ch)),(5 +(3 * ch)));
    debugPort.puts(buffer);
    if(status == true){
        if(_midiStatus[_fmStatus[ch].midiChannel].hold){
            debugPort.puts("\e[34mOO\e[0m|");
        } else {
            debugPort.puts("\e[35mXX\e[0m|");
        }
    } else {
        debugPort.puts("--|");
    }
}

void showTone(uint8_t ch,bool status){
    char buffer[20] = "";
    sprintf(buffer,"\e[8;%dH\e[8;%df|",(5 +(3 * ch)),(5 +(3 * ch)));
    debugPort.puts(buffer);
    if(status == true){
        sprintf(buffer,"%02x",_midiStatus[_fmStatus[ch].midiChannel].programNumber);
        debugPort.puts(buffer);
    } else {
        debugPort.puts("--|");
    }
}

void showPitchBend(uint8_t ch,bool status){
    char buffer[20] = "";
    sprintf(buffer,"\e[9;%dH\e[9;%df|",(5 +(3 * ch)),(5 +(3 * ch)));
    debugPort.puts(buffer);
    if(status == true){
        sprintf(buffer,"%02x\e[0m|",_midiStatus[_fmStatus[ch].midiChannel].pitchBend >> 6);
        if((_midiStatus[_fmStatus[ch].midiChannel].pitchBend >> 6)> 128){
            debugPort.puts("\e[32m");
        } else if((_midiStatus[_fmStatus[ch].midiChannel].pitchBend >> 6)< 128){
            debugPort.puts("\e[36m");
        }
        debugPort.puts(buffer);
    } else {
        debugPort.puts("--|");
    }
}

void showVoiceCount(void){
    uint8_t cnt = 0;
    char text[20] = "";
    for(int i = 0;i < 16;i++){
        if(_fmStatus[i].isUsed)cnt++;
    }
    debugPort.puts("\e[10;8H\e[10;8f");
    sprintf(text,"%02d",cnt);
    debugPort.puts(text);
    return;
}

void showStatus(uint8_t status){
    debugPort.puts("\e[2;8H\e[2;8f");
    switch(status){
        case 0:
            debugPort.puts("\e[31mInitialising  \e[0m");
            break;
        case 1:
            debugPort.puts("\e[32mNormal(Active)\e[0m");
            break;
        case 2:
            debugPort.puts("\e[33mDebug Mode    \e[0m");
            break;
    }
}

bool ofRecent = false;
void showOverFlag(bool status){
    if(ofRecent == status)return;
    debugPort.puts("\e[11;7H\e[11;7f");
    if(status){
        debugPort.puts("OVR");
    } else {
        debugPort.puts("---");
    }
    ofRecent = status;
}

void noteOn(uint8_t channel,uint8_t note,uint8_t velocity){
    if(prevCh > 15){
        prevCh = 0;
    }
    prevMax = 0;
    if(channel == 9)return;
    for(int i = prevCh;i < 16;i++){
        if(_fmStatus[i].priority != 0){
            _fmStatus[i].priority++;
            if(_fmStatus[i].priority > 511)_fmStatus[i].priority=511;
        }
        if(_fmStatus[i].isUsed == false){
            _fmStatus[i].isUsed = true;
            _fmStatus[i].noteNumber = note;
            _fmStatus[i].midiChannel = channel;
            YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel)][(_midiStatus[channel].expression)]);
            YMF825.setModulation(i,_midiStatus[channel].modulation);
            YMF825.pitchWheelChange(i,_midiStatus[channel].pitchBend,pitchBendSensitivity);
            YMF825.noteOn(i,note,velocity >> 1,channel);
            showChannelNumber(i,true);
            showOverFlag(false);
            prevCh++;
            return;
        }
    }
    for(int i = 0;i < prevCh;i++){
        if(_fmStatus[i].priority != 0){
            _fmStatus[i].priority++;
            if(_fmStatus[i].priority > 511)_fmStatus[i].priority=511;
        }
        if(_fmStatus[i].isUsed == false){
            _fmStatus[i].isUsed = true;
            _fmStatus[i].noteNumber = note;
            _fmStatus[i].midiChannel = channel;
            YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel)][(_midiStatus[channel].expression)]);
            YMF825.setModulation(i,_midiStatus[channel].modulation);
            YMF825.pitchWheelChange(i,_midiStatus[channel].pitchBend,pitchBendSensitivity);
            YMF825.noteOn(i,note,velocity >> 1,channel);
            showChannelNumber(i,true);
            showOverFlag(false);
            prevCh++;
            return;
        }
    }
    showOverFlag(true);
    for(int i = 0;i < 16;i++){
        if(prevMax <= _fmStatus[i].priority + (_fmStatus[i].midiChannel << 1)&& _fmStatus[i].isUsed != false){
            prevMax = _fmStatus[i].priority + (_fmStatus[i].midiChannel << 1);
            offChannel = i;
        }
    }
    _midiStatus[offChannel].hold = false;
    _fmStatus[offChannel].priority = 0;
    YMF825.noteOff(offChannel);
    YMF825.noteOn(offChannel,note,velocity >> 1,channel);
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
                showChannelNumber(i,false);
                return;
            }

        }
    }
}

void pitchBend(uint8_t channel,uint16_t pitchBend){
    _midiStatus[channel].pitchBend = pitchBend;
    for(int i = 0;i < 16;i++){
        if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
            showPitchBend(i,true);
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
                    YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel)][(_midiStatus[channel].expression)]);
                    showPartLevel(i,true);
                }
            }
            break;
        case 11:
            status = (status & 0b100)|0b0100;
            _midiStatus[channel].expression = value;
            for(int i = 0;i < 16;i++){
                if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                    YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel)][(_midiStatus[channel].expression)]);
                    showExpression(i,true);
                }
            }
            break;
        case 64:
            if(value > 63){
                status &= ~(0b1000);
                for(int i = 0;i < 16;i++){
                    if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                        showHold(i,true);
                    }
                }
                _midiStatus[channel].hold = true;
            } else {
                status |= 0b1000;
                _midiStatus[channel].hold = false;
                for(int i = 0;i < 16;i++){
                    if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                        YMF825.noteOff(i);
                        _fmStatus[i].priority = 0;
                        _fmStatus[i].isUsed = false;
                        showChannelNumber(i,false);
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

int main(void){
    wait_us(500);
    showInterFace();
    showStatus(0);
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
    wait(1.0);
    showStatus(2);
    while(1){
        MIDI.midiParse();
    }
}