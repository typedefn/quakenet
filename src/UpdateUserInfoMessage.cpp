/*
 * UpdateUserInfoMessage.cpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#include "UpdateUserInfoMessage.hpp"
#include <Bot.hpp>

UpdateUserInfoMessage::UpdateUserInfoMessage(Bot *bot) :
    ServerMessage(bot) {
  // TODO Auto-generated constructor stub

}

UpdateUserInfoMessage::~UpdateUserInfoMessage() {
  // TODO Auto-generated destructor stub
}

void UpdateUserInfoMessage::read(Message *message) {
  unsigned slot = message->readByte();
  if (slot >= MAX_CLIENTS) {
    return;
  }

  long userId = message->readLong();
  string value(message->readString());
  string name = Utility::findValue("name", value);
  // TODO: fix hardcoded name of bot later.
  if (name == "krupt_drv") {
    LOG << "In slot " << slot << " with id " << userId;
    bot->setMySlot(slot);
    // Assuming it is fortress
  }

  PlayerInfo *pi = bot->getPlayerBySlot(slot);
  if (pi != nullptr && userId != 0 && !name.empty()) {
    pi->active = true;
    bot->setTargetSlot(slot);
    LOG << "updateUserInfo: " << " slot = " << slot << " user id " << userId << " " << value << " " << name;
  }


//  LOG << "updateUserInfo: " << " slot = " << slot << " user id " << userId << " " << value << " " << name;
  // updateUserInfo:  slot = 1 user id 2 \*client\ezQuake 6923\chat\1\bottomcolor\4\topcolor\4\skin\tf_sold\team\red\name\mastakillah mastakillah
}

