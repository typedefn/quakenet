/*
 * SoundListMessage.cpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#include "SoundListMessage.hpp"
#include <Bot.hpp>

SoundListMessage::SoundListMessage(Bot *bot) :
    ServerMessage(bot) {
  // TODO Auto-generated constructor stub

}

SoundListMessage::~SoundListMessage() {
  // TODO Auto-generated destructor stub
}

void SoundListMessage::read(Message *message) {
  char *str;
  unsigned protoVer = bot->getProtoVer();
  int n;
  byte numSounds;
  if (protoVer >= 26) {
    numSounds = message->readByte();

    while (1) {
      str = message->readString();
      if (!str[0]) {
        break;
      }

      numSounds++;

      if (str[0] == '/') {
        str++;
      }
    }

    n = message->readByte();

    if (n) {
      stringstream ss;
      ss << "soundlist" << " " << 1 << " " << n;
      bot->requestStringCommand(ss.str());
      return;
    }
  } else {

    numSounds = 0;

    do {
      if (++numSounds > 255) {
        LOG << "Error send to many sound_precache";
      }
      str = message->readString();
    } while (*str);
  }
}
