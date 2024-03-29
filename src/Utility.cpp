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

std::string Utility::findValue(const std::string &key, const std::string &map) {
  std::stringstream ss;
  ss << "\\" << key;

  size_t found = map.find(ss.str());
  std::string value;

  if (found == std::string::npos) {
    return value;
  }

  size_t startPosition = found + 2 + key.size();
  size_t endPosition = map.find("\\", startPosition);
  endPosition -= startPosition;

  value = map.substr(startPosition, endPosition);

  // Reached end of string.
  if (value.size() > 2 && value.at(value.size() - 1) == '"') {
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

int Utility::getRandomNormal() {
  int sign = 0;

  int r = ((rand() % 3));

  switch (r) {
  case 0:
    sign = 1;
    break;
  case 1:
    sign = -1;
    break;
  default:
    sign = 0;
  }

  return sign;
}
void Utility::loadMap(const std::string &path, int *mapchecksum, int *mapchecksum2) {
  unsigned int i = 0;
  byte *cmodBase = nullptr;
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  FILE *f = fopen(path.c_str(), "r");

  if (f == nullptr) {
    LOG << "Unable to find " << path;
    throw std::runtime_error("Cannot open file");
  }

  size_t filesize = file.tellg();
  Header *header = nullptr;
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

  i = littleLong(header->version);

  // swap all the lumps
  for (i = 0; i < sizeof(Header) / 4; i++) {
    ((int*) header)[i] = littleLong(((int*) header)[i]);
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
    header = (Header*) paddedBuffer;
    position += sizeof(Header);

    // Copy lumps: align on 4-byte boundary
    for (i = 0; i < HEADER_LUMPS; ++i) {
      if (position % 4) {
        position += 4 - (position % 4);
      }
      if (position + header->lumps[i].filelen > requiredSize) {
        LOG << "CM_LoadMap: " << path << " caused error while aligning lumps";
      }
      memcpy((byte*) paddedBuffer + position, ((byte*) buf) + header->lumps[i].fileofs, header->lumps[i].filelen);
      header->lumps[i].fileofs = position;

      position += header->lumps[i].filelen;
    }

    // Use the new buffer
    delete[] paddedBuffer;
  }

  cmodBase = (byte*) header;

  unsigned int checksum, checksum2 = 0;

  // checksum all of the map, except for entities
  for (i = 0; i < HEADER_LUMPS; i++) {
    if (i == LUMP_ENTITIES)
      continue;
    checksum ^= littleLong(Com_BlockChecksum(cmodBase + header->lumps[i].fileofs, header->lumps[i].filelen));

    if (i == LUMP_VISIBILITY || i == LUMP_LEAFS || i == LUMP_NODES)
      continue;
    checksum2 ^= littleLong(Com_BlockChecksum(cmodBase + header->lumps[i].fileofs, header->lumps[i].filelen));
  }

  delete[] buf;

  LOG << "Checksum for " << path << " is " << checksum << "/" << (int) checksum2;
  *mapchecksum = checksum;
  *mapchecksum2 = checksum2;
}

short Utility::shortSwap (short s)
{
        union
        {
                short   s;
                byte    b[2];
        } dat1, dat2;
        dat1.s = s;
        dat2.b[0] = dat1.b[1];
        dat2.b[1] = dat1.b[0];
        return dat2.s;
}

int Utility::longSwap (int l)
{
        union
        {
                int             l;
                byte    b[4];
        } dat1, dat2;
        dat1.l = l;
        dat2.b[0] = dat1.b[3];
        dat2.b[1] = dat1.b[2];
        dat2.b[2] = dat1.b[1];
        dat2.b[3] = dat1.b[0];
        return dat2.l;
}

float Utility::floatSwap (float f)
{
        union
        {
                float   f;
                byte    b[4];
        } dat1, dat2;
        dat1.f = f;
        dat2.b[0] = dat1.b[3];
        dat2.b[1] = dat1.b[2];
        dat2.b[2] = dat1.b[1];
        dat2.b[3] = dat1.b[0];
        return dat2.f;
}

int Utility::makeChar(int i)
{
        i &= ~3;
        if (i < -127 * 4)
                i = -127 * 4;
        if (i > 127 * 4)
                i = 127 * 4;
        return i;
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



std::string Utility::readFile(const std::string &filename) {
  std::ifstream file;
  std::string contents;
  std::string line;
  file.open(filename);
  if (file.is_open()) {
    while (getline(file, line)) {
      contents += line + "\n";
    }
    file.close();
  }
  return contents;
}

std::vector<std::string> Utility::split(std::string str, char delimiter = ' ') {
  std::vector<std::string> strings;
  std::stringstream ss(str);
  std::string s;
  while (getline(ss, s, delimiter)) {
    strings.push_back(s);
  }
  return strings;
}

void Utility::swap(float & a, float & b) {
  float c = a;
  a = b;
  b = c;
}

bool Utility::fileExists(const std::string & name) {
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}
