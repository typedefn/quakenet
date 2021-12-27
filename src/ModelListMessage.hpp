/*
 * ModelListMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_MODELLISTMESSAGE_HPP_
#define SRC_MODELLISTMESSAGE_HPP_

#include "ServerMessage.hpp"

class ModelListMessage: public ServerMessage {
public:
  ModelListMessage(Bot * bot);
  virtual ~ModelListMessage();
  void read(Message * message);
};

#endif /* SRC_MODELLISTMESSAGE_HPP_ */
