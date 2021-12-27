/*
 * PrintMessage.hpp
 *
 *  Created on: Dec 21, 2021
 *      Author: mastakillah
 */

#ifndef SRC_PRINTMESSAGE_HPP_
#define SRC_PRINTMESSAGE_HPP_

#include "ServerMessage.hpp"

class PrintMessage: public ServerMessage {
public:
  PrintMessage(Bot * bot);
  virtual ~PrintMessage();
  void read(Message * message);

private:
  string lastLine;
};

#endif /* SRC_PRINTMESSAGE_HPP_ */
