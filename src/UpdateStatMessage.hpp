/*
 * UpdateStatMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_UPDATESTATMESSAGE_HPP_
#define SRC_UPDATESTATMESSAGE_HPP_

#include "ServerMessage.hpp"

class UpdateStatMessage: public ServerMessage {
public:
  UpdateStatMessage(Bot * bot);
  virtual ~UpdateStatMessage();
  void read(Message * message);
};

#endif /* SRC_UPDATESTATMESSAGE_HPP_ */
