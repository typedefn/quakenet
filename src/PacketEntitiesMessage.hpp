/*
 * PacketEntitiesMessage.hpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#ifndef SRC_PACKETENTITIESMESSAGE_HPP_
#define SRC_PACKETENTITIESMESSAGE_HPP_
#include <ServerMessage.hpp>

class PacketEntitiesMessage : public ServerMessage {
public:
  PacketEntitiesMessage(Bot * bot);
  virtual ~PacketEntitiesMessage();
  void read(Message * message);

};

#endif /* SRC_PACKETENTITIESMESSAGE_HPP_ */
