/*
 * UpdateUserInfoMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_UPDATEUSERINFOMESSAGE_HPP_
#define SRC_UPDATEUSERINFOMESSAGE_HPP_

#include "ServerMessage.hpp"

class UpdateUserInfoMessage: public ServerMessage {
public:
  UpdateUserInfoMessage(Bot * bot);
  virtual ~UpdateUserInfoMessage();
  void read(Message * message);
};

#endif /* SRC_UPDATEUSERINFOMESSAGE_HPP_ */
