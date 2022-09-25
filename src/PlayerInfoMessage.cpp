/*
 * PlayerInfoMessage.cpp
 *
 *  Created on: Dec 20, 2021
 *      Author: mastakillah
 */

#include "PlayerInfoMessage.hpp"
#include <Bot.hpp>

PlayerInfoMessage::PlayerInfoMessage(Bot *bot) :
    ServerMessage(bot) {
  // TODO Auto-generated constructor stub

}

PlayerInfoMessage::~PlayerInfoMessage() {
  // TODO Auto-generated destructor stub
}

void PlayerInfoMessage::read(Message *message) {
  float frameTime = 0.05;

  unsigned num = message->readByte();
  if (num >= MAX_CLIENTS) {
    return;
  }

  PlayerInfo *pi = bot->getPlayerBySlot(num);

  if (pi->active && pi->slot != bot->getMe()->slot) {
    bot->getTargetingSystem()->setTarget(num);
//    LOG << "SLOT " << num << " is set as target";
  }

  int flags = message->readShort();
  pi->flags = flags;

  for (int i = 0; i < 3; i++) {
    pi->coords[i] = message->readFloatCoord();
  }

  pi->frame = message->readByte();

  glm::vec3 position = glm::vec3(pi->coords[0], pi->coords[2], pi->coords[1]);
  pi->position = position;

//  if (num == bot->getTargetId()) {
  //LOG << "[" << num << "]" << " x = " << position.x << " y = " << position.y << " z = " << position.z;
//  }

  pi->time = bot->getTime();

  if (flags & PF_MSEC) {
    byte msec = message->readByte();
    if (num == bot->getTargetingSystem()->getTarget()) {
      frameTime = msec * 0.001;
    }

    if (frameTime > 0.1) {
      frameTime = 0.1;
    }
  }

  if (flags & PF_COMMAND) {
    int forward = 0;
    int bits = message->readByte();

    if (bot->getProtoVer() <= 26) {
      // read current angles
      if (bits & CM_ANGLE1)
        pi->angles[0] = message->readAngle16();
      if (bits & CM_ANGLE2)
        pi->angles[1] = message->readAngle16();
      if (bits & CM_ANGLE3)
        pi->angles[2] = message->readAngle16();

      // read movement
      if (bits & CM_FORWARD)
        int t = message->readChar() << 3;
      if (bits & CM_SIDE)
        int t = message->readChar() << 3;
      if (bits & CM_UP)
        int t = message->readChar() << 3;

    } else {
      // read current angles
      if (bits & CM_ANGLE1)
        pi->angles[0] = message->readAngle16();
      if (bits & CM_ANGLE2)
        pi->angles[1] = message->readAngle16();
      if (bits & CM_ANGLE3)
        pi->angles[2] = message->readAngle16();

//
//      pi->direction.x = cos(pi->angles[0]*PI/180);
//      pi->direction.z = sin(pi->angles[1]*PI/180);
//
//
//        LOG << " num " << num << " is facing x = " << pi->direction.x << "  z = " << pi->direction.z;
//

      // read movement
      if (bits & CM_FORWARD) {
        forward = message->readShort();
      }
      if (bits & CM_SIDE) {
       int s = message->readShort();
      }
      if (bits & CM_UP) {
        int s = message->readShort();
      }
    }
    // read buttons
    if (bits & CM_BUTTONS) {
      int s = message->readByte();
    }

    if (bits & CM_IMPULSE) {
     int s = message->readByte();
    }
    int s = message->readByte();
  }

  for (int i = 0; i < 3; i++) {
    if (flags & (PF_VELOCITY1 << i)) {
      short v = message->readShort();
      pi->velocity[i] = v;
      if (abs(v) > 0) {
        pi->direction[i] = v;
      }
    } else {
      pi->velocity[i] = 0;
    }
  }

  float t = pi->velocity.z;
  pi->velocity.z = pi->velocity.y;
  pi->velocity.y = t;

  t = pi->direction.z;
  pi->direction.z = pi->direction.y;
  pi->direction.y = t;

  if (flags & PF_MODEL) {
    int b = message->readByte();
  }

  if (flags & PF_SKINNUM) {
    int b = message->readByte();
  }

  if (flags & PF_EFFECTS) {
    int b = message->readByte();
  }

  if (flags & PF_WEAPONFRAME) {
    int b = message->readByte();
  }

  for (int i = 0; i < 3; i++) {
    pi->velocity[i] *= frameTime;
  }

  pi->speed = sqrt(pi->velocity[0] * pi->velocity[0] + pi->velocity[1] * pi->velocity[1] + pi->velocity[2] * pi->velocity[2]);
}

