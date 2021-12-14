/*
 * md4.hpp
 *
 *  Created on: Dec 14, 2021
 *      Author: dervish
 */

#ifndef MD4_HPP_
#define MD4_HPP_

#include "Protocol.hpp"


unsigned Com_BlockChecksum (void *buffer, int length);
void Com_BlockFullChecksum (void *buffer, int len, unsigned char *outbuf);
byte COM_BlockSequenceCRCByte (byte *base, int length, int sequence);




#endif /* MD4_HPP_ */
