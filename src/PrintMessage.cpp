/*
 * PrintMessage.cpp
 *
 *  Created on: Dec 21, 2021
 *      Author: mastakillah
 */

#include "PrintMessage.hpp"
#include <Bot.hpp>

PrintMessage::PrintMessage(Bot *bot) :
    ServerMessage(bot) {
  // TODO Auto-generated constructor stub

}

PrintMessage::~PrintMessage() {
  // TODO Auto-generated destructor stub
}

void PrintMessage::read(Message *message) {
  int level = message->readByte();
//  vector<string> lines;
//

  std::string line = message->readString();

//  lines.push_back(line1);
//  LOG << "Level " << level << " LINE " << line;

  if (lastLine.find("krupt_drv") != std::string::npos && line.find("has joined Team No") != std::string::npos) {
    bot->setState(SelectClass);
  } else if (lastLine.find("krupt_drv") != std::string::npos && line.find("joined!") != std::string::npos) {
    bot->setState(JoinTeam);
  }

  lastLine = line;
}

