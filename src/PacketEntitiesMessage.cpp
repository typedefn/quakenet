/*
 * PacketEntitiesMessage.cpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#include <PacketEntitiesMessage.hpp>
#include <Bot.hpp>

PacketEntitiesMessage::PacketEntitiesMessage(Bot *bot) :
    ServerMessage(bot) {
}

PacketEntitiesMessage::~PacketEntitiesMessage() {
}

void PacketEntitiesMessage::read(Message *message) {
  bot->parsePacketEntities(message, false);
}
