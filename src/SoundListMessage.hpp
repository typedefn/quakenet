/*
 * SoundListMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_SOUNDLISTMESSAGE_HPP_
#define SRC_SOUNDLISTMESSAGE_HPP_

#include "ServerMessage.hpp"

class SoundListMessage: public ServerMessage {
public:
  SoundListMessage(Bot * bot);
  virtual ~SoundListMessage();
  void read(Message * message);
};

#endif /* SRC_SOUNDLISTMESSAGE_HPP_ */
