/*
 * UpdateStatMessage.cpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#include "UpdateStatMessage.hpp"
#include <Bot.hpp>

UpdateStatMessage::UpdateStatMessage(Bot *bot) :
    ServerMessage(bot) {
  // TODO Auto-generated constructor stub

}

UpdateStatMessage::~UpdateStatMessage() {
  // TODO Auto-generated destructor stub
}

void UpdateStatMessage::read(Message *message) {
  int i = message->readByte();
  int j = message->readByte();

  if (i >= 0 && i < MAX_CL_STATS) {
//    LOG << "updateStat: " << i << " " << j;
    bot->setStat(i, j);
  }
}
