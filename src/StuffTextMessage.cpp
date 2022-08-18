/*
 * StuffTextMessage.cpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#include "StuffTextMessage.hpp"
#include <Bot.hpp>

StuffTextMessage::StuffTextMessage(Bot *bot) :
    ServerMessage(bot) {
  // TODO Auto-generated constructor stub

}

StuffTextMessage::~StuffTextMessage() {
  // TODO Auto-generated destructor stub
}

void StuffTextMessage::read(Message *message) {

  std::string msg(message->readString());
  std::vector<std::string> lines = Utility::split(msg, '\n');

  for (const std::string &line : lines) {
    std::vector<std::string> tokens = Utility::split(line, ' ');

    if(tokens.empty()) {
      continue;
    }

    std::string token = tokens.at(0);

    if (token == "packet") {
      std::string realIp = tokens.at(4);
      realIp.pop_back();
      bot->sendIp(realIp);
    } else if (token == "cmd") {

      std::string type = tokens.at(1);

      if (type == "pext") {
        bot->sendExtensions();
      } else if (type == "new") {
        bot->sendNew();
      } else if (type == "spawn" || type == "prespawn") {
        std::stringstream ss;
        ss << type << " " << tokens.at(2) << " " << tokens.at(3);
        bot->requestStringCommand(ss.str());
      }
    } else if (token == "fullserverinfo") {
      mapName = Utility::findValue("map", line);
      LOG << "MAP NAME = " << mapName;
      bot->setMapName(mapName);
      bot->setState(Info);
    } else if (token == "skins") {
      bot->setState(Begin);
    } else if (token == "team" && tokens.at(1) == "blue") {
//      bot->requestStringCommand("setinfo \"team\" \"blue\"", 2);
    } else if (token == "skin" && tokens.at(1) == "tf_sold") {
      bot->setState(Done);
    }

  }
}
