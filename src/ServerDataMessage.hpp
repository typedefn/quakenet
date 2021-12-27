/*
 * ServerDataMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_SERVERDATAMESSAGE_HPP_
#define SRC_SERVERDATAMESSAGE_HPP_

#include <ServerMessage.hpp>

class ServerDataMessage : public ServerMessage {
public:
  ServerDataMessage(Bot * bot);
  virtual ~ServerDataMessage();

  void read(Message * message);
};

#endif /* SRC_SERVERDATAMESSAGE_HPP_ */
