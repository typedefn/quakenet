/*
 * DownloadMessage.cpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#include "DownloadMessage.hpp"
#include <Bot.hpp>

DownloadMessage::DownloadMessage(Bot *bot) :
    ServerMessage(bot) {

}

DownloadMessage::~DownloadMessage() {
  // TODO Auto-generated destructor stub
}

void DownloadMessage::read(Message *message) {
  // TODO: fix this later, just read the junk...
  int s = 0;
  int size = message->readShort();
  int percent = message->readByte();

}
