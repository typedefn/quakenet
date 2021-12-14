/* 
 * File:   Utility.cpp
 * Author: dervish
 * 
 * Created on February 21, 2021, 8:55 AM
 */

#include "Utility.hpp"
#include "md4.hpp"

Utility::Utility() {
}

Utility::Utility(const Utility &orig) {
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
  byte *p;
  byte chkb[60 + 4];

  p = chktbl + ((unsigned int) sequence % (sizeof(chktbl) - 4));

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
  stringstream ss;
  ss << "\\" << key << "\\";

  size_t found = map.find(ss.str());
  string value;

  if (found == string::npos) {
    return value;
  }

  size_t startPosition = found + 2 + key.size();
  size_t endPosition = map.find("\\", startPosition);
  endPosition -= startPosition;

  value = map.substr(startPosition, endPosition);

  // Reached end of string.
  if (value.size() > 2 && value.at(value.size() - 2) == '"') {
    value.pop_back();
    value.pop_back();
  }

  return value;
}

int Utility::randomRanged(int a, int b) {
  return (a + (std::rand() % (b - a + 1)));
}

double Utility::randomFloat() {
  return (Utility::randomRanged(1, 100) / 102.0);
}

void Utility::loadMap(const string &path, int *mapchecksum, int *mapchecksum2) {
  unsigned int i = 0;
  byte     *cmodBase = nullptr;
  ifstream file(path, ios::binary | ios::ate);
  FILE* f = fopen(path.c_str(), "r");
  size_t filesize = file.tellg();
  Header * header = nullptr;
  bool padLumps = false;
  int requiredLength = 0;
  unsigned int *paddedBuffer = nullptr;

  unsigned int *buf = new unsigned int[sizeof(unsigned int) * filesize];

  LOG << path << " size " << filesize;

  fread(buf, 1, filesize, f);
  file.close();
  fclose(f);

  int modType = (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
  LOG << "modType " << modType;

  header = (Header*) buf;

  i = littleLong (header->version);


  // swap all the lumps
    for (i = 0; i < sizeof(Header) / 4; i++) {
      ((int *)header)[i] = littleLong(((int *)header)[i]);
    }

    // Align the lumps
    for (i = 0; i < HEADER_LUMPS; ++i) {
      padLumps |= (header->lumps[i].fileofs % 4) != 0;

      if (header->lumps[i].fileofs < 0 || header->lumps[i].filelen < 0) {
        LOG << "CM_LoadMap: " << path << " has invalid lump definitions";
      }
      if (header->lumps[i].fileofs + header->lumps[i].filelen > filesize || header->lumps[i].fileofs + header->lumps[i].filelen < 0) {
        LOG << "CM_LoadMap: " << path << " has invalid lump definitions";
      }

      requiredLength += header->lumps[i].filelen;
    }



    if (padLumps) {
      int position = 0;
      int requiredSize = sizeof(Header) + requiredLength + HEADER_LUMPS * 4 + 1;

      paddedBuffer = new unsigned int[requiredSize];

      // Copy header
      memcpy(paddedBuffer, buf, sizeof(Header));
      header = (Header*)paddedBuffer;
      position += sizeof(Header);

      // Copy lumps: align on 4-byte boundary
      for (i = 0; i < HEADER_LUMPS; ++i) {
        if (position % 4) {
          position += 4 - (position % 4);
        }
        if (position + header->lumps[i].filelen > requiredSize) {
          LOG << "CM_LoadMap: " << path << " caused error while aligning lumps";
        }
        memcpy((byte*)paddedBuffer + position, ((byte*)buf) + header->lumps[i].fileofs, header->lumps[i].filelen);
        header->lumps[i].fileofs = position;

        position += header->lumps[i].filelen;
      }

      // Use the new buffer
      buf = paddedBuffer;
    }

    cmodBase = (byte *)header;

    unsigned int  checksum, checksum2 = 0;

    // checksum all of the map, except for entities
    for (i = 0; i < HEADER_LUMPS; i++) {
      if (i == LUMP_ENTITIES)
        continue;
      checksum ^= littleLong(Com_BlockChecksum(cmodBase + header->lumps[i].fileofs, header->lumps[i].filelen));

      if (i == LUMP_VISIBILITY || i == LUMP_LEAFS || i == LUMP_NODES)
        continue;
      checksum2 ^= littleLong(Com_BlockChecksum(cmodBase + header->lumps[i].fileofs, header->lumps[i].filelen));
    }

    delete[] paddedBuffer;
    delete[] buf;

    LOG << "Checksum for " << path << " is " << checksum << "/" << (int)checksum2;
}

int Utility::littleLong(int l) {
#if defined( __BIG_ENDIAN__ )
  std::reverse( reinterpret_cast<unsigned char*>( &l ), reinterpret_cast<unsigned char*>( &l ) + sizeof( int ) );
#endif
  return l;
}

float Utility::littleFloat(float l) {
#if defined( __BIG_ENDIAN__ )
  std::reverse( reinterpret_cast<unsigned char*>( &l ), reinterpret_cast<unsigned char*>( &l ) + sizeof( float ) );
#endif
  return l;
}


