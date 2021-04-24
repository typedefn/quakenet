
/* 
 * File:   Utility.cpp
 * Author: dervish
 * 
 * Created on February 21, 2021, 8:55 AM
 */

#include "Utility.hpp"

Utility::Utility() {
}

Utility::Utility(const Utility& orig) {
}

Utility::~Utility() {
}

void Utility::CRC_Init(unsigned short *crcvalue) {
    *crcvalue = CRC_INIT_VALUE;
}

unsigned short Utility::CRC_Block(byte *start, unsigned int count) {
    unsigned short crc;

    Utility::CRC_Init(&crc);
    while (count--)
        crc = (crc << 8) ^ crctable[(crc >> 8) ^ *start++];

    return crc;
}

byte Utility::crcByte(byte *base, int length, int sequence) {
    unsigned short crc;
    byte * p;
    byte chkb[60 + 4];

    p = chktbl + ((unsigned int) sequence % (sizeof (chktbl) - 4));

    if (length > 60) {
        length = 60;
    }

    for (int i = 0; i < length; i++) {
        chkb[i] = base[i];
    }

    chkb[length] = (sequence & 0xff) ^ p[0];
    chkb[length + 1] = p[1];
    chkb[length + 2] = ((sequence >> 8) & 0xff) ^ p[2];
    chkb[length + 3] = p[3];
    length += 4;

    crc = Utility::CRC_Block(chkb, length);
    crc &= 0xff;
    return crc;
}

string Utility::findValue(const string &key, const string &map) {
    size_t found = map.find(key);
    string value;

    if (found == string::npos) {
        return value;
    }

    size_t endPosition = map.find("\\", found);

    value = map.substr(found + 1 + key.size(), endPosition);
    return value;
}

int Utility::randomRanged(int a, int b) {
    return (a + (std::rand() % (b - a + 1)));
}

double Utility::randomFloat() {
    return (Utility::randomRanged(1, 100) / 102.0);
}

