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

//  LOG << "setInfo: " << key << " " << value;
  // TODO: Fix all this hardcoded gibberish later...
  if (key == "team") {
      PlayerInfo * pi = bot->getPlayerBySlot(slot);
      pi->team = value;
      LOG << pi->name << " join team " << pi->team; 
  } else if (key == "skin" && value == "tf_sold") {
//    bot->setState(Done);
//    LOG << "SELECTED CLASS 'SOLDIER'!";
  }
}
