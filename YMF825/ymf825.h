//From Luna Tsukinashi
#pragma once

#include<mbed.h>
#include"../tables.h"
#include"../structs.h"

class ymf825 {
public:
    ymf825(PinName, PinName, PinName, PinName, PinName);

    void initialise(bool);

    void sendAllToneList(void);

    void sendToneList(uint8_t);

    void setToneListFromGM(uint8_t, uint8_t);

    void noteOn(uint8_t, uint8_t, uint8_t, uint8_t);

    void noteOff(uint8_t);

    void pitchWheelChange(uint8_t, uint16_t, uint8_t);

    void pitchChange(uint8_t, uint8_t, uint8_t);

    void setChannelVolume(uint8_t, uint8_t);

    void setModulation(uint8_t, uint8_t);

    void allKeyOff(void);

    void allMute(void);

    SPI _spi;
private:
    DigitalOut _slaveSelect, _reset;
    uint8_t toneNumbers[16];
    void singleWrite(uint8_t, uint8_t);
};