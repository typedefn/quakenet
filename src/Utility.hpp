
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

class Utility {
public:
  Utility();
  Utility(const Utility& orig);
  virtual ~Utility();
  
  static void CRC_Init(unsigned short *crcvalue);
  static unsigned short CRC_Block(byte *start, unsigned int count);
  static byte crcByte(byte *base, int length, int sequence);
  
  static string findValue(const string & key, const string & map);
  
  static int randomRanged(int a, int b);
  static double randomFloat();
private:

};

#endif /* UTILITY_HPP */

