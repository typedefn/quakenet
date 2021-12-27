/*
 * ServerMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_SERVERMESSAGE_HPP_
#define SRC_SERVERMESSAGE_HPP_
#include <Protocol.hpp>
#include <Common.hpp>
#include <Connection.hpp>

class Bot;

class ServerMessage {
public:
  ServerMessage(Bot * bot);
  virtual ~ServerMessage();

  virtual void read(Message * message) = 0;

protected:
  Bot * bot;
};

#endif /* SRC_SERVERMESSAGE_HPP_ */
