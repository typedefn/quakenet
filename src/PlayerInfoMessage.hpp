/*
 * PlayerInfoMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_PLAYERINFOMESSAGE_HPP_
#define SRC_PLAYERINFOMESSAGE_HPP_

#include "ServerMessage.hpp"

class PlayerInfoMessage: public ServerMessage {
public:
  PlayerInfoMessage(Bot * bot);
  virtual ~PlayerInfoMessage();
  void read(Message * message);
};

#endif /* SRC_PLAYERINFOMESSAGE_HPP_ */
