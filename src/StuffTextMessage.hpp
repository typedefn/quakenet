/*
 * StuffTextMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_STUFFTEXTMESSAGE_HPP_
#define SRC_STUFFTEXTMESSAGE_HPP_

#include "ServerMessage.hpp"

class StuffTextMessage: public ServerMessage {
public:
  StuffTextMessage(Bot * bot);
  virtual ~StuffTextMessage();
  void read(Message *message);

private:
  string mapName;
};

#endif /* SRC_STUFFTEXTMESSAGE_HPP_ */
