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

  string line = message->readString();

//  lines.push_back(line1);
//  LOG << "Level " << level << " LINE " << line;

  if (lastLine.find("krupt_drv") != string::npos && line.find("has joined Team No") != string::npos) {
    bot->setState(SelectClass);
  } else if (lastLine.find("krupt_drv") != string::npos && line.find("joined!") != string::npos) {
    bot->setState(JoinTeam);
  }

  lastLine = line;
}

