/*
 * DownloadMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_DOWNLOADMESSAGE_HPP_
#define SRC_DOWNLOADMESSAGE_HPP_

#include "ServerMessage.hpp"

class DownloadMessage: public ServerMessage {
public:
  DownloadMessage(Bot * bot);
  virtual ~DownloadMessage();
  void read(Message * message);
};

#endif /* SRC_DOWNLOADMESSAGE_HPP_ */
