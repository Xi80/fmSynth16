//From Luna Tsukinashi
#include "ymf825.h"

ymf825::ymf825(PinName mosi, PinName miso, PinName sck, PinName slaveSelect, PinName reset) : _spi(mosi, miso, sck),
                                                                                              _slaveSelect(slaveSelect),
                                                                                              _reset(reset) {
    for (int i = 0; i < 24; i++) {
        for (int j = 0; j < 30; j++) {
            originalTones[i][j] = 0x00;
        }
    }
    _spi.frequency(10000000UL);
    initialise(false);
}


void ymf825::initialise(bool softReset = false) {
    if (softReset == false) {
        for (int i = 0l; i < 16; i++) {
            channels[i].toneNumber = 0;
        }
    }
    for (int i = 0; i < 485; i++) {
        sendData[i] = 0x00;
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
    singleWrite(0x19, 0x33 << 1);
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
    sendData[0] = 0x81 + number;
    for (int j = 0; j < 30; j++) {
        if (channels[number].toneNumber > 127) {
            sendData[1 + number * 30 + j] = originalTones[channels[number].toneNumber - 128][j];
        } else {
            sendData[1 + number * 30 + j] = gmTable[channels[number].toneNumber][j];
        }
    }
    sendData[30 * (number + 1) + 1] = (0x80);
    sendData[30 * (number + 1) + 2] = (0x03);
    sendData[30 * (number + 1) + 3] = (0x81);
    sendData[30 * (number + 1) + 4] = (0x80);
    singleWrite(0x08, 0x16);
    wait_us(1);
    singleWrite(0x08, 0x00);
    _slaveSelect = 0;
    _spi.write(0x07);
    for (int i = 0; i < 30 * (number + 1) + 1; i++) {
        _spi.write(sendData[i]);
    }
    _slaveSelect = 1;
    return;
}

void ymf825::sendAllToneList(void) {
    sendData[0] = 0x90;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 30; j++) {
            if (channels[i].toneNumber > 127) {
                sendData[1 + i * 30 + j] = originalTones[channels[i].toneNumber - 128][j];
            } else {
                sendData[1 + i * 30 + j] = gmTable[channels[i].toneNumber][j];
            }
        }
    }
    sendData[481] = (0x80);
    sendData[482] = (0x03);
    sendData[483] = (0x81);
    sendData[484] = (0x80);
    singleWrite(0x08, 0xF6);
    wait_us(1);
    singleWrite(0x08, 0x00);
    _slaveSelect = 0;
    _spi.write(0x07);
    for (int i = 0; i < 485; i++) {
        _spi.write(sendData[i]);
    }
    wait_us(2);
    _slaveSelect = 1;
    return;
}

void ymf825::setToneListFromGM(uint8_t channel, uint8_t number) {
    channels[channel].toneNumber = number;
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

void ymf825::setOriginalTone(uint8_t number, uint8_t *data) {
    for (int i = 0; i < 30; i++) {
        originalTones[number][i] = data[i];
    }
    return;
}