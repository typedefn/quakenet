/*
 * SetInfoMessage.cpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#include "SetInfoMessage.hpp"
#include <Bot.hpp>

SetInfoMessage::SetInfoMessage(Bot *bot) :
    ServerMessage(bot) {
  // TODO Auto-generated constructor stub

}

SetInfoMessage::~SetInfoMessage() {
  // TODO Auto-generated destructor stub
}

void SetInfoMessage::read(Message *message) {
  int slot = message->readByte();

  if (slot >= MAX_CLIENTS) {
    return;
  }

  std::string key(message->readString());
  std::string value(message->readString());

  PlayerInfo * pi = bot->getPlayerBySlot(slot);
  BotConfig botConfig = bot->getBotConfig();

  if (key == "team" && pi->name == botConfig.name) {
      pi->team = value;
      LOG << pi->name << " join team " << pi->team; 
      bot->setState(SelectClass);
  } else if (key == "skin" && value == botConfig.skin) {
     //bot->setState(Done);
     LOG << "SELECTED CLASS " << botConfig.skin; 
  }
}
