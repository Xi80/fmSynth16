//From Luna Tsukinashi
#include<mbed.h>

#include "YMF825/ymf825.h"
#include "MIDI/midi.h"
#include "tables.h"
#include "structs.h"

midi MIDI(D1,D0,31250);
ymf825 YMF825(D11,D12,D13,D10,D9);
//BufferedSerial debugPort(USBTX,USBRX,921600);
//DigitalOut LEDs[2] = {LED1,LED2};

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
/*
void showInterface(void){
    printf("\e[1;1H\e[1;1f");
    printf("\e[2J");
    printf("\e[36m\e[4m\e[1m\e[4mSakura767 Status Viewer Ver1.2.3\e[0m\n");
    printf("Status:Active\n");
    printf("\n");
    printf("\e[35mFM Tone Generator Status\e[0m\n");
    printf("\e[47m\e[30mVoNo|00|01|02|03|04|05|06|07|08|09|0A|0B|0C|0D|0E|0F|\e[0m\n");
    printf("Used|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    printf("\n");
    printf("\e[35mMIDI Status\e[0m\n");
    printf("\e[47m\e[30mPart|00|01|02|03|04|05|06|07|08|09|0A|0B|0C|0D|0E|0F|\e[0m\n");
    printf("VoCo|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|\n");
    printf("Inst|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|\n");
    printf("BndS|02|02|02|02|02|02|02|02|02|02|02|02|02|02|02|02|\n");
    printf("Bend|80|80|80|80|80|80|80|80|80|80|80|80|80|80|80|80|\n");
    printf("Hold|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    printf("PaLv|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|\n");
    printf("Expr|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|00|\n");
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
    uint8_t pos = 6 + 3* number;
    printf("\e[6;%dH\e[6;%df",pos,pos);
    if(_fmStatus[number].isUsed){
        printf("%02x|",_fmStatus[number].midiChannel);
    } else {
        printf("--|");
    }
}

void checkVoiceCount(uint8_t number){
    uint8_t count = 0;
    uint8_t pos = 6 + 3* number;
    for(int i = 0;i < 16;i++){
        if(_fmStatus[i].isUsed && _fmStatus[i].midiChannel == number){
            count++;
        }
    }
    printf("\e[10;%dH\e[10;%df",pos,pos);
    printf("%02x|",count);
}

void checkMidiInst(uint8_t number){
    uint8_t pos = 6 + 3* number;
    printf("\e[11;%dH\e[11;%df",pos,pos);
    printf("%02x|",_midiStatus[number].programNumber);
}

void checkBendS(uint8_t number){
    uint8_t pos = 6 + 3* number;
    printf("\e[12;%dH\e[12;%df",pos,pos);
    printf("%02x|",_midiStatus[number].pitchBendSensitivity);
}

void checkBend(uint8_t number ){
    uint8_t pos = 6 + 3* number;
    printf("\e[13;%dH\e[13;%df",pos,pos);
    if((_midiStatus[number].pitchBend >> 6) < 128){
        printf("\e[32m");
    } else if((_midiStatus[number].pitchBend >> 6) > 128){
        printf("\e[33m");
    }
    printf("%02x\e[0m|",_midiStatus[number].pitchBend >> 6);
}

void checkHold(uint8_t number){
    uint8_t pos = 6 + 3* number;
    printf("\e[14;%dH\e[14;%df",pos,pos);
    if(_midiStatus[number].hold){
        printf("\e[31mOO\e[0m|");
    } else {
        printf("--|");
    }
}

void checkPaLv(uint8_t number){
    uint8_t pos = 6 + 3* number;
    printf("\e[15;%dH\e[15;%df",pos,pos);
    if(_midiStatus[number].partLevel < 40){
        printf("\e[31m");
    } else if(_midiStatus[number].partLevel < 80){
        printf("\e[33m");
    } else {
        printf("\e[32m");
    }
    printf("%02x\e[0m|",_midiStatus[number].partLevel);
}

void checkExpr(uint8_t number){
    uint8_t pos = 6 + 3* number;
    printf("\e[16;%dH\e[16;%df",pos,pos);
    if(_midiStatus[number].expression < 40){
        printf("\e[31m");
    } else if(_midiStatus[number].expression < 80){
        printf("\e[33m");
    } else {
        printf("\e[32m");
    }
    printf("%02x\e[0m|",_midiStatus[number].expression);
}
*/

bool flag[4] = {false,false,false,false};

void SDOn(void){
    YMF825.noteOn(9,53,50,9);
    flag[0] = true;
}

void BDOn(void){
    YMF825.noteOn(13,24,50,13);
    flag[2] = true;
}

void CymOn(void){
    YMF825.noteOn(14,60,50,14);
    flag[2] = true;
}

void HiOn(void){
    YMF825.noteOn(15,60,50,15);
    flag[3] = true;
}

void rhythmOn(uint8_t note){
    switch(note){
        case 35:
        case 36:
            BDOn();
            break;
        case 38:
        case 40:
            SDOn();
            break;
        case 42:
        case 44:
        case 46:
            HiOn();
            break;
        case 49:
        case 51:
        case 52:
        case 53:
        case 55:
            CymOn();
            break;
    }
}

void noteOn(uint8_t channel,uint8_t note,uint8_t velocity){
    /*
    if(flag[0] != true){
        YMF825.noteOff(9);
    } else {
        flag[0] = false;
    }
    if(flag[1] != true){
        YMF825.noteOff(13);
    } else {
        flag[1] = false;
    }
    if(flag[2] != true){
        YMF825.noteOff(14);
    } else {
        flag[2] = false;
    }
    if(flag[3] != true){
        YMF825.noteOff(15);
    } else {
        flag[3] = false;
    }
    */
    //LEDs[0] = !LEDs[0];

    if(_bridgeStatus.prevOnCh > 15){
        _bridgeStatus.prevOnCh = 0;
    }

    for(int i = 0;i < 16;i++){
        if(_fmStatus[i].priority != 0){
            _fmStatus[i].priority++;
            if(_fmStatus[i].priority > 511)_fmStatus[i].priority=511;
        }
    }

    //if(channel == 13 || channel == 14 || channel == 15)return;
    if(channel == 9){
        //rhythmOn(note);
        return;
    }
    for(int i = _bridgeStatus.prevOnCh;i < 16;i++){
        //if(i == 13 || i == 14 || i == 15 || i == 9)continue;
        if(_fmStatus[i].isUsed == false){
            _fmStatus[i].isUsed = true;
            _fmStatus[i].noteNumber = note;
            _fmStatus[i].midiChannel = channel;
            YMF825.noteOff(i);
            YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 3)][(_midiStatus[channel].expression >> 3)]);
            YMF825.setModulation(i,_midiStatus[channel].modulation);
            YMF825.pitchWheelChange(i,_midiStatus[channel].pitchBend,_midiStatus[channel].pitchBendSensitivity);
            YMF825.noteOn(i,note,velocity >> 1,channel);
            _bridgeStatus.prevOnCh++;
            //checkUsed(i);
            //checkVoiceCount(channel);
            return;
        }
    }

    for(int i = 0;i < _bridgeStatus.prevOnCh + 1;i++){
        //if(i == 13 || i == 14 || i == 15 || i == 9)continue;
        if(_fmStatus[i].priority != 0){
            _fmStatus[i].priority++;
            if(_fmStatus[i].priority > 511)_fmStatus[i].priority=511;
        }
        if(_fmStatus[i].isUsed == false){
            _fmStatus[i].isUsed = true;
            _fmStatus[i].noteNumber = note;
            _fmStatus[i].midiChannel = channel;
            YMF825.noteOff(i);
            YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 3)][(_midiStatus[channel].expression >> 3)]);
            YMF825.setModulation(i,_midiStatus[channel].modulation);
            YMF825.pitchWheelChange(i,_midiStatus[channel].pitchBend,_midiStatus[channel].pitchBendSensitivity);
            YMF825.noteOn(i,note,velocity >> 1,channel);
            _bridgeStatus.prevOnCh++;
            //checkUsed(i);
            //checkVoiceCount(channel);
            return;
        }
    }
    _bridgeStatus.maxPriorityVal = 0;
    for(int i = 0;i < 16;i++){
        //if(i == 13 || i == 14 || i == 15 || i == 9)continue;
        if( _bridgeStatus.maxPriorityVal <= _fmStatus[i].priority + (_fmStatus[i].midiChannel << 1)&& _fmStatus[i].isUsed != false){
             _bridgeStatus.maxPriorityVal = _fmStatus[i].priority + (_fmStatus[i].midiChannel << 1);
             _bridgeStatus.maxPriorityCh = i;
        }
    }
    _midiStatus[_bridgeStatus.maxPriorityCh].hold = false;
    _fmStatus[_bridgeStatus.maxPriorityCh].priority = 0;
    YMF825.noteOff(_bridgeStatus.maxPriorityCh);
    YMF825.noteOn(_bridgeStatus.maxPriorityCh,note,velocity >> 1,channel);
    //checkUsed(_bridgeStatus.maxPriorityCh);
    //checkVoiceCount(channel);
    return;
}

void noteOff(uint8_t channel,uint8_t note,uint8_t velocity){
    //if(channel == 9)return;
    for(int i = 0;i < 16;i++){
        //if(i == 13 || i == 14 || i == 15 || i == 9)continue;
        if(_fmStatus[i].isUsed == true && _fmStatus[i].noteNumber == note && _fmStatus[i].midiChannel == channel){
            if(_midiStatus[channel].hold == false){
                _fmStatus[i].isUsed = false;
                YMF825.noteOff(i);
                _fmStatus[i].priority = 0;
               // checkUsed(i);
                //checkVoiceCount(channel);
                return;
            }
        }
    }
}

void pitchBend(uint8_t channel,uint16_t pitchBend){
    //if(channel == 13 || channel == 14 || channel == 15)return;
    _midiStatus[channel].pitchBend = pitchBend;
    //checkBend(channel);
    for(int i = 0;i < 16;i++){
        if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
            YMF825.pitchWheelChange(i,pitchBend,_midiStatus[channel].pitchBendSensitivity);
        }
    }
}

void controlChange(uint8_t channel,uint8_t number,uint8_t value){
    //if(channel == 13 || channel == 14 || channel == 15)return;
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
                    //checkBendS(channel);
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
            //checkPaLv(channel);
            for(int i = 0;i < 16;i++){
                if(_fmStatus[i].isUsed   == true && _fmStatus[i].midiChannel == channel){
                    YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 3)][(_midiStatus[channel].expression >> 3)]);
                }
            }
            break;
        case 11:
            _midiStatus[channel].expression = value;
            //checkExpr(channel);
            for(int i = 0;i < 16;i++){
                if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                    YMF825.setChannelVolume(i,expTable[(_midiStatus[channel].partLevel >> 3)][(_midiStatus[channel].expression >> 3)]);
                }
            }
            break;
        case 64:
            if(value > 63){
                _midiStatus[channel].hold = true;
                //checkHold(channel);
            } else {
                _midiStatus[channel].hold = false;
                //checkHold(channel);
                for(int i = 0;i < 16;i++){
                    if(_fmStatus[i].isUsed == true && _fmStatus[i].midiChannel == channel){
                        //YMF825.noteOff(i);
                        _fmStatus[i].priority = 0;
                        _fmStatus[i].isUsed = false;
                        //checkUsed(i);
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
    //if(channel == 9 || channel == 13 || channel == 14 || channel == 15)return;
    _midiStatus[channel].programNumber = number;
    //checkMidiInst(channel);
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
    //showInterface();
    YMF825.initialise(false);
    YMF825.sendAllToneList();
    /*
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
    */
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
    //showInterface();
    MIDI.setCallbacKReceiveControlChange(controlChange);
    MIDI.setCallbackReceiveNoteOn(noteOn);
    MIDI.setCallbackReceiveNoteOff(noteOff);
    MIDI.setCallbackReceivePitchWheelChange(pitchBend);
    MIDI.setCallbackReceiveProgramChange(programChange);
    MIDI.setCallbackReceiveReset(reset);
    MIDI.setCallbackReceiveAllNotesOff(notesOff);
    MIDI.setCallbackReceiveAllSoundsOff(soundsOff);
    reset();
    /*
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
    */
    while(1){
        MIDI.midiParse();
    }
}