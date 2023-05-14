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
  std::string value(message->readString());
  std::string name = Utility::findValue("name", value);

  PlayerInfo *pi = bot->getPlayerBySlot(slot);
  // TODO: fix hardcoded name of bot later.
  if (name == bot->getBotConfig().name) {
//    LOG << "In slot " << slot << " with id " << userId;
    bot->setMySlot(slot);
    pi->team = bot->getBotConfig().team;
    pi->name = name;
    // Assuming it is fortress
  } else if (pi != nullptr) {
    pi->team = Utility::findValue("team", value);
    pi->name = name;
  }

  LOG << slot << " name = " << name << " pi->team = " << pi->team;

  if (pi != nullptr && userId != 0 && !name.empty()) {
    pi->active = true;
    pi->slot = slot;
    LOG << "updateUserInfo: " << " slot = " << slot << " user id " << userId << " team " << pi->team << " name " << name << " value = " << value;
  }

  std::string spectator = Utility::findValue("*spectator", value);

  if (spectator == "1") {
    pi->active = false;
    LOG << "is spectator = " << value;
  }

//  LOG << "updateUserInfo: " << " slot = " << slot << " user id " << userId << " " << value << " " << name;
  // updateUserInfo:  slot = 1 user id 2 \*client\ezQuake 6923\chat\1\bottomcolor\4\topcolor\4\skin\tf_sold\team\red\name\mastakillah mastakillah
}

