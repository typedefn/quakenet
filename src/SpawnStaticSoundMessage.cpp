/*
 * SpawnStaticSoundMessage.cpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#include "SpawnStaticSoundMessage.hpp"
#include <Bot.hpp>

SpawnStaticSoundMessage::SpawnStaticSoundMessage(Bot *bot) :
    ServerMessage(bot) {
  // TODO Auto-generated constructor stub

}

SpawnStaticSoundMessage::~SpawnStaticSoundMessage() {
  // TODO Auto-generated destructor stub
}

void SpawnStaticSoundMessage::read(Message *message) {
  //read junk
  for (int i = 0; i < 3; i++) {
    message->readCoord();
  }
  for (int i = 0; i < 3; i++) {
    message->readByte();
  }
}
