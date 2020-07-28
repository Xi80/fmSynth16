//From Luna Tsukinashi
#include "ymf825.h"

ymf825::ymf825(PinName mosi, PinName miso, PinName sck, PinName slaveSelect, PinName reset) : _spi(mosi, miso, sck),
                                                                                              _slaveSelect(slaveSelect),_reset(reset){
    _spi.frequency(10000000UL);
    initialise(false);
}


void ymf825::initialise(bool softReset = false) {
    if (softReset == false) {
        for (int i = 0l; i < 16; i++) {
            toneNumbers[i] = 0;
        }
    }

    _reset = 0;
    wait_us(100);
    _reset = 1;

    singleWrite(0x1D, 0x01);
    singleWrite(0x02, 0x0E);
    wait_us(1);
    singleWrite(0x00, 0x01);
    singleWrite(0x01, 0x00);
    singleWrite(0x1A, 0xA3);
    wait_us(1);
    singleWrite(0x1A, 0x00);
    thread_sleep_for(30);
    singleWrite(0x02, 0x04);
    wait_us(1);
    singleWrite(0x02, 0x00);
    singleWrite(0x19, 0x95);
    singleWrite(0x1B, 0x3F);
    singleWrite(0x14, 0x00);
    singleWrite(0x03, 0x01);

    singleWrite(0x08, 0xF6);
    wait_us(21);
    singleWrite(0x08, 0x00);
    singleWrite(0x09, 0xF8);
    singleWrite(0x0A, 0x00);

    singleWrite(0x17, 0x40);
    singleWrite(0x18, 0x00);

    for (int i = 0; i < 16; i++) {
        singleWrite(0x0B, i);
        singleWrite(0x0F, 0x30);
        singleWrite(0x10, 0x71);
        singleWrite(0x11, 0x00);
        singleWrite(0x12, 0x08);
        singleWrite(0x13, 0x00);
    }
    sendAllToneList();
    return;
}

void ymf825::sendToneList(uint8_t number) {
    singleWrite(0x08, 0x16);
    wait_us(1);
    singleWrite(0x08, 0x00);
    _slaveSelect = 0;
    _spi.write(0x07);
    _spi.write(0x81 + number);
    for(int i = 0;i <= number;i++){
        for(int j = 0;j < 30;j++){
            _spi.write(gmTable[toneNumbers[i]][j]);
        }
    }
    _spi.write(0x80);
    _spi.write(0x03);
    _spi.write(0x81);
    _spi.write(0x80);
    _slaveSelect = 1;
    return;
}

void ymf825::sendAllToneList(void) {
    singleWrite(0x08, 0x16);
    wait_us(1);
    singleWrite(0x08, 0x00);
    _slaveSelect = 0;
    _spi.write(0x07);
    _spi.write(0x81 + 0x0F);
    for(int i = 0;i <= 0x0F;i++){
        for(int j = 0;j < 30;j++){
            _spi.write(gmTable[toneNumbers[i]][j]);
        }
    }
    _spi.write(0x80);
    _spi.write(0x03);
    _spi.write(0x81);
    _spi.write(0x80);
    _slaveSelect = 1;
    return;
}

void ymf825::setToneListFromGM(uint8_t channel, uint8_t number) {
    toneNumbers[channel] = number;
    return;
}

void ymf825::noteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint8_t inst) {
    singleWrite(0x0B, channel);
    singleWrite(0x0D, fNumberTableHigh[note]);
    singleWrite(0x0E, fNumberTableLow[note]);
    singleWrite(0x0C, velocity);
    singleWrite(0x0F, 0x40 | (inst & 0x0F));
    return;
}

void ymf825::noteOff(uint8_t channel) {
    singleWrite(0x0B, channel);
    singleWrite(0x0F, 0x00);
    return;
}

void ymf825::pitchWheelChange(uint8_t channel, uint16_t pitch, uint8_t sensitivity) {
    uint8_t pit = pitch >> 6;
    uint16_t INT = (pitchBendTable[sensitivity][pit] >> 9) & 0x03;
    uint16_t FRAC = pitchBendTable[sensitivity][pit] & 0x1FF;
    singleWrite(0x0B, channel);
    singleWrite(0x12, (INT << 3) | ((FRAC >> 6) & 0x07));
    singleWrite(0x13, (FRAC & 0x3F) << 1);
    return;
}

void ymf825::setChannelVolume(uint8_t channel, uint8_t volume) {
    singleWrite(0x0B, channel);
    singleWrite(0x10, volume);
    return;
}

void ymf825::setModulation(uint8_t channel, uint8_t modulation) {
    modulation = modulation >> 4;
    singleWrite(0x0B, channel);
    singleWrite(0x11, modulation);
    return;
}


void ymf825::singleWrite(uint8_t address, uint8_t data) {
    _slaveSelect = 0;
    _spi.write(address);
    _spi.write(data);
    _slaveSelect = 1;
}

void ymf825::allKeyOff(void) {
    singleWrite(0x08, 0x80);
    return;
}

void ymf825::allMute(void) {
    singleWrite(0x08, 0x40);
    return;
}
