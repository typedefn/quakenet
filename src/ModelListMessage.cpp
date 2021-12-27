/*
 * ModelListMessage.cpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#include "ModelListMessage.hpp"
#include <Bot.hpp>

ModelListMessage::ModelListMessage(Bot *bot) :
    ServerMessage(bot) {
  // TODO Auto-generated constructor stub

}

ModelListMessage::~ModelListMessage() {
  // TODO Auto-generated destructor stub
}

void ModelListMessage::read(Message *message) {
  unsigned protoVer = bot->getProtoVer();
  int numModels, n;
  char *str;
  if (protoVer >= 26) {
    numModels = message->readByte();

    while (1) {
      str = message->readString();
      if (!str[0]) {
        break;
      }

      numModels++;

      if (str[0] == '/') {
        str++;
      }
    }

    if ((n = message->readByte())) {
      stringstream ss;
      ss << "modellist" << " " << 1 << " " << n;
      bot->requestStringCommand(ss.str());
      return;
    }
  }
}
