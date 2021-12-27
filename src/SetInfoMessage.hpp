/*
 * SetInfoMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_SETINFOMESSAGE_HPP_
#define SRC_SETINFOMESSAGE_HPP_

#include "ServerMessage.hpp"

class SetInfoMessage: public ServerMessage {
public:
  SetInfoMessage(Bot * bot);
  virtual ~SetInfoMessage();
  void read(Message * message);
};

#endif /* SRC_SETINFOMESSAGE_HPP_ */
