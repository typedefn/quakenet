/* 
 * File:   Utility.hpp
 * Author: dervish
 *
 * Created on February 21, 2021, 8:55 AM
 */

#ifndef UTILITY_HPP
#define UTILITY_HPP
#include "Protocol.hpp"
#include "Common.hpp"
#define LUMP_ENTITIES 0
#define LUMP_PLANES   1
#define LUMP_TEXTURES 2
#define LUMP_VERTEXES 3
#define LUMP_VISIBILITY 4
#define LUMP_NODES    5
#define LUMP_TEXINFO  6
#define LUMP_FACES    7
#define LUMP_LIGHTING 8
#define LUMP_CLIPNODES  9
#define LUMP_LEAFS    10
#define LUMP_MARKSURFACES 11
#define LUMP_EDGES    12
#define LUMP_SURFEDGES  13
#define LUMP_MODELS   14
#define HEADER_LUMPS  15

struct Lump {
  int fileofs, filelen;
};

struct Header {
  int version;
  Lump lumps[HEADER_LUMPS];
};

class Utility {
public:
  Utility();
  Utility(const Utility &orig);
  virtual ~Utility();

  static void CRC_Init(unsigned short *crcvalue);
  static unsigned short CRC_Block(byte *start, unsigned int count);
  static byte crcByte(byte *base, int length, int sequence);

  static string findValue(const string &key, const string &map);

  static int randomRanged(int a, int b);
  static double randomFloat();

  static void loadMap(const string & path, int *mapchecksum, int *mapchecksum2);

  static int littleLong(int l);
  static float littleFloat(float l);


private:

};

#endif /* UTILITY_HPP */

