/*
 * ServerDataMessage.cpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#include "ServerDataMessage.hpp"
#include <Bot.hpp>

ServerDataMessage::ServerDataMessage(Bot *bot) :
    ServerMessage(bot) {

}

ServerDataMessage::~ServerDataMessage() {

}

void ServerDataMessage::read(Message *message) {
  unsigned protoVer = 0;

  if (bot->getProtoVer() != 0){
    return;
  }

  for (;;) {
    protoVer = message->readLong();
    if (message->isBadRead()) {
      break;
    }

    bot->setProtoVer(protoVer);
    LOG << "Protocol version set to " << protoVer;

    if (protoVer == PROTOCOL_VERSION_FTE) {
      long fteextensions = message->readLong();
      continue;
    }

    if (protoVer == PROTOCOL_VERSION_FTE2) {
      long fteext = message->readLong();
      continue;
    }

    if (protoVer == PROTOCOL_VERSION_MVD1) {
      long ext = message->readLong();
      continue;
    }

    if (protoVer == PROTOCOL_VERSION) {
      break;
    }
  }

  bot->setSpawnCount(message->readLong());
  bot->setGameDir(message->readString());

  unsigned playerNum = message->readByte();

  if (playerNum & 128) {
    //spectator = true;
    playerNum &= ~128;
  }

  // Get level description
  string levelDescription1 = message->readString();
//  LOG << "Level description " << levelDescription1;


  if (protoVer >= 25) {
    float gravity = message->readFloat();
    float stopspeed = message->readFloat();
    float maxSpeed = message->readFloat();
    float specMaxSpeed = message->readFloat();
    float accelerate = message->readFloat();
    float airAccelerate = message->readFloat();
    float waterAccelerate = message->readFloat();
    float friction = message->readFloat();
    float waterFriction = message->readFloat();
    float entGravity = message->readFloat();
  }
}
