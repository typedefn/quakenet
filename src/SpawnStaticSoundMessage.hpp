/*
 * SpawnStaticSoundMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_SPAWNSTATICSOUNDMESSAGE_HPP_
#define SRC_SPAWNSTATICSOUNDMESSAGE_HPP_

#include "ServerMessage.hpp"

class SpawnStaticSoundMessage: public ServerMessage {
public:
  SpawnStaticSoundMessage(Bot * bot);
  virtual ~SpawnStaticSoundMessage();
  void read(Message * message);
};

#endif /* SRC_SPAWNSTATICSOUNDMESSAGE_HPP_ */
