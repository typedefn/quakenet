/* 
 * File:   Bot.cpp
 * Author: dervish
 * 
 * Created on February 21, 2021, 8:35 AM
 */

#include "Bot.hpp"

Bot::Bot(char **argv) {
  ahead = 1;
  protoVer = 0;
  delay = 0;
  challenge = 0;
  frame = 0;
  mySlot = -1;
  previousState = currentState = None;
  this->botMemory = std::make_unique<BotMemory>(this, 8);
  this->targetingSystem = std::make_unique<TargetingSystem>(this);

  std::string configFilename(argv[3]);
  this->config = std::make_unique<Config>(configFilename);
  this->impulseConfig = std::make_unique<Config>("../resources/impulses.ini");
  
  std::string botSection = this->config->getString("main", "bot0");
  botConfig.name = this->config->getString(botSection, "name");
  botConfig.skin = this->config->getString(botSection, "skin");
  botConfig.team = this->config->getString(botSection, "team");
  botConfig.bottomColor = this->config->getString(botSection, "bottomcolor");
  botConfig.targetDistance = this->config->getInt(botSection, "target_distance");
  botConfig.fov = this->config->getInt(botSection, "fov");
  botConfig.seekDistance = this->config->getInt(botSection, "seek_distance");

  goals.push_back(std::make_unique<PatrolGoal>(this));
  goals.push_back(std::make_unique<AttackGoal>(this));
//  goals.push_back(std::make_unique<RoamGoal>(this));

  for (int i = 0; i < MAX_CLIENTS; i++) {
    players[i].coords[0] = 0;
    players[i].coords[1] = 0;
    players[i].coords[2] = 0;
    players[i].entertime = 0.0;
    players[i].flags = 0;
    players[i].frags = 0;
    players[i].frame = 0;
    players[i].name, "";
    players[i].ping = 0;
    players[i].pl = 0;
    players[i].slot = -1;
    players[i].active = false;
    players[i].angles[0] = 0;
    players[i].angles[1] = 0;
    players[i].angles[2] = 0;
    players[i].velocity = glm::vec3(0, 0, 0);
    players[i].position = glm::vec3(0, 0, 0);
    players[i].speed = 0;
    players[i].direction = glm::vec3(0, 0, 0);
  }
  this->argv = argv;

  waypoints[""] = std::vector<glm::vec3>();
  waypoints["patrol"] = std::vector<glm::vec3>();
  waypoints["start"] = std::vector<glm::vec3>();

  for (size_t i = 0; i < MAX_CL_STATS; i++) {
    setStat(i, 0);
  }

  spawnCount = 0;
  timeChallengeSent = getTime();

  serverMessages[svc_serverdata] = std::make_unique<ServerDataMessage>(this);
  serverMessages[svc_packetentities] = std::make_unique<PacketEntitiesMessage>(this);
  serverMessages[svc_modellist] = std::make_unique<ModelListMessage>(this);
  serverMessages[svc_download] = std::make_unique<DownloadMessage>(this);
  serverMessages[svc_spawnstaticsound] = std::make_unique<SpawnStaticSoundMessage>(this);
  serverMessages[svc_updatestat] = std::make_unique<UpdateStatMessage>(this);
  serverMessages[svc_soundlist] = std::make_unique<SoundListMessage>(this);
  serverMessages[svc_setinfo] = std::make_unique<SetInfoMessage>(this);
  serverMessages[svc_stufftext] = std::make_unique<StuffTextMessage>(this);
  serverMessages[svc_updateuserinfo] = std::make_unique<UpdateUserInfoMessage>(this);
  serverMessages[svc_playerinfo] = std::make_unique<PlayerInfoMessage>(this);
  serverMessages[svc_print] = std::make_unique<PrintMessage>(this);
  gotChallenge = false;
  validSequence = 0;
  goal = nullptr;

  timers["spawn"] = 0;
  timers["think"] = 0;
  timers["command"] = 0;
  timers["state"] = 0;
  timers["forward"] = 0;
  timers["button"] = 0;
  timers["prime"] = 0;
  primeCounter = 0;
  timers["stuck"] = MAX_TIMEOUT_IN_SECONDS;
}

Bot::~Bot() {
}

void Bot::mainLoop() {
  nullCommand(&nullcmd);
  for (int i = 0; i < UPDATE_BACKUP; i++) {
    nullCommand(&cmds[i]);
  }

  connection.connect(this->argv);
  getChallenge();
  currentTime = getTime();
  previousTime = getTime();

  bool received = false;
  running = true;
  bool connectionLess = true;

  while (running) {
    int s = 0;
    previousTime = currentTime;
    currentTime = getTime();

    Message inMessage;


    if (connection.recv(&inMessage)) {
      if (connection.process(&inMessage)) {
        parseServerMessage(&inMessage);
      }
    }

    frame = (connection.getOutgoingSequence() & UPDATE_MASK);

    if (!outputQueue.empty()) {
      Message outMessage = outputQueue.front();
      if (outMessage.isConnectionless()) {
        s = connection.sendConnectionless(outMessage);
      } else {
        connectionLess = false;
        s = connection.send(outMessage);
      }
      outputQueue.pop();
    } else if (!connectionLess) {
      Message msg;
      createCommand(&msg);
      outputQueue.push(msg);
    }

    updateState();
  }

  requestStringCommand("drop");
  Message dropMsg = outputQueue.front();
  connection.send(dropMsg);
}

PlayerInfo* Bot::getPlayerBySlot(size_t id) {
  PlayerInfo *pi = nullptr;

//  infoLock.lock();
  pi = &players[id];
//  infoLock.unlock();

  return pi;
}

PlayerInfo* Bot::getMe() {
  return getPlayerBySlot(mySlot);
}

double Bot::getTime() {
  struct timeval tp;
  struct timezone tzp;
  static int secbase;

  gettimeofday(&tp, &tzp);

  if (!secbase) {
    secbase = tp.tv_sec;
    return tp.tv_usec / 1000000.0;
  }

  return (tp.tv_sec - secbase) + tp.tv_usec / 1000000.0;
}

void Bot::getChallenge() {
  Message msg;
  msg.writeString("\xff\xff\xff\xff");
  msg.writeString("getchallenge");
  msg.writeByte(10);
  connection.sendConnectionless(msg);
  gotChallenge = false;

  timeChallengeSent = getTime();

  int c = 0;

  while (!gotChallenge) {
    msg.clear();
    if (connection.recv(&msg)) {

      msg.beginRead();
      msg.readLong();
      c = msg.readByte();

      switch (c) {
        case S2C_CHALLENGE: {
          char data[MAXLINE] = { 0 };
          char userInfo[MAX_INFO_STRING + 32] = { 0 };
          challenge = atoi(msg.readString());

          for (;;) {
            c = msg.readLong();
            if (msg.isBadRead()) {
              break;
            }

            // Read proto version info...
            msg.readLong();
          }
          std::stringstream ss;
          ss << "\\rate\\25000\\name\\";
          ss << botConfig.name;
          ss << "\\chat\\2\\msg\\1\\noaim\\1\\*client\\QuakeBotClient\\spectator\\0\\pmodel\\13845\\emodel\\6967\\*z_ext\\511";
          strcpy(userInfo, ss.str().c_str());
          snprintf(data, sizeof(data), "\xff\xff\xff\xff" "connect %d %d %i \"%s\"\n",
          PROTOCOL_VERSION, connection.getQport(), challenge, userInfo);
          Message s;
          s.writeString(data);
          s.writeString("0x58455446 0x2140f000");
          s.writeByte(10);
          s.writeString("0x32455446 0x2");
          s.writeByte(10);
          s.writeString("0x3144564d 0x1");
          s.writeByte(10);
          connection.sendConnectionless(s);
          break;
        }
        case S2C_CONNECTION: {
          sendNew();
          sendImpulse(0, 0);
          sendImpulse(0, 0);
          sendNew();
          gotChallenge = true;
          break;
        }
        default: {
          break;
        }
      }
    }
  }

}

void Bot::parseServerMessage(Message *message) {
  int msgSvcStart = 0;
  int cmd = 0;

  while (1) {

    if (message->isBadRead()) {
      break;
    }

    cmd = message->readByte();

    if (cmd <= 0) {
      continue;
    }

    auto pos = serverMessages.find(cmd);

    if (pos != serverMessages.end()) {
      serverMessages.at(cmd)->read(message);
      // Keep continue here until all the switch messages types have been converted.
      continue;
    }
//    LOG << cmd;
    // TODO old switch statement, needs to be wrapped into classes.
    switch (cmd) {
      case svc_fte_spawnstatic2: {
        // Static entities are non-interactive world objects like torches.
        parseStatic(message, true);
        break;
      }
     case nq_svc_time: {
        message->readFloat();
        break;
      }
      case nq_svc_updatename: {
        message->readByte();
        message->readString();
        break;
      }
      case svc_intermission: {
        float orig[3] = { };
        float angles[3] = { };

        for (int i = 0; i < 3; i++) {
          orig[i] = message->readCoord();
        }

        for (int i = 0; i < 3; i++) {
          angles[i] = message->readFloat();
        }

        break;
      }
      case svc_muzzleflash: {
        int i = message->readShort();
        break;
      }
      case svc_finale: {
        message->readString();
        break;
      }
      case nq_svc_version: {
        message->readLong();
        break;
      }
      case svc_centerprint: {
        message->readString();
        break;
      }
      case svc_setpause: {
        message->readByte();
        break;
      }
      case svc_updatefrags: {
        unsigned slot = message->readByte();
        if (slot >= MAX_CLIENTS) {
          break;
        }

        PlayerInfo *pi = getPlayerBySlot(slot);
        pi->frags = message->readShort();

        break;
      }
      case svc_updateping: {
        int slot = message->readByte();
        int ping = message->readShort();
        break;
      }
      case svc_updatepl: {
        message->readByte();
        message->readByte();
        break;
      }
      case svc_updateentertime: {
        message->readByte();
        message->readFloat();
        break;
      }
      case svc_maxspeed: {
        message->readFloat();
        break;
      }
      case svc_entgravity: {
        message->readFloat();
        break;
      }
      case svc_serverinfo: {
        std::string key = message->readString();
        std::string value = message->readString();
        break;
      }
      case svc_cdtrack: {
        byte cdTrack = message->readByte();
        break;
      }
      case svc_lightstyle: {
        message->readByte();
        message->readString();
        break;
      }
      case svc_updatestatlong: {
        byte i = message->readByte();
        long j = message->readLong();
//        LOG << "svc updatestat long = " << (int)i << " value = " << (int)j;
        if (i >= 0 && i < MAX_CL_STATS) {
          setStat(i, j);
        }
        break;
      }
//      case svc_setangle: {
//        float x = message->readChar() * (360.0 / 256);
//        float y = message->readChar() * (360.0 / 256);
//        float z = message->readChar() * (360.0 / 256);
//        break;
//      }
//      case svc_damage: {
//        int armor_ = message->readByte();
//        int blood_ = message->readByte();
//        float coords[3];
//        float distanceToMe;
//        float distanceToTarget;
//
//        for (int i = 0; i < 3; i++) {
//          coords[i] = message->readCoord();
//        }
//
//        glm::vec3 from(coords[0], coords[2], coords[1]);
//        break;
//      }
//      case svc_sound: {
//        float pos[3] = { };
//        byte channel = message->readByte();
//        byte soundNumber = message->readByte();
//
//        for (int i = 0; i < 3; i++) {
//          pos[i] = message->readCoord();
//        }
//
//        break;
//      }
//      case svc_stopsound: {
//        message->readShort();
//        break;
//      }
//
//      case svc_spawnbaseline: {
//        message->readShort();
//        parseBaseline(message);
//        break;
//      }
      case svc_spawnstatic: {
        parseStatic(message, false);
        break;
      }
//    case svc_temp_entity: {
//      float pos[3] = { };
//      bool parsed = false;
//      byte type = message->readByte();
//
//      switch (type) {
//      case TE_LIGHTNING1:
//        parseBeam(message, 1);
//        parsed = true;
//        break;
//
//      case TE_LIGHTNING2:
//        parseBeam(message, 2);
//        parsed = true;
//        break;
//
//      case TE_LIGHTNING3:
//        parseBeam(message, 3);
//        parsed = true;
//        break;
//      case TE_GUNSHOT: {
//        int count = message->readByte();
//        pos[0] = message->readCoord();
//        pos[1] = message->readCoord();
//        pos[2] = message->readCoord();
//        parsed = true;
//        break;
//      }
//      case TE_BLOOD: {
//        int count = message->readByte();
//        pos[0] = message->readCoord();
//        pos[1] = message->readCoord();
//        pos[2] = message->readCoord();
//        parsed = true;
//        break;
//      }
//
//      case TE_LIGHTNINGBLOOD: {
//        pos[0] = message->readCoord();
//        pos[1] = message->readCoord();
//        pos[2] = message->readCoord();
//        parsed = true;
//        break;
//      }
//      }
//
//      if (!parsed) {
//        pos[0] = message->readCoord();
//        pos[1] = message->readCoord();
//        pos[2] = message->readCoord();
//      }
//      break;
//    }
      case svc_chokecount: {
        int count = message->readByte();
        break;
      }
      case svc_deltapacketentities: {
        parsePacketEntities(message, true);
        break;
      }
      default: {
        break;
      }
    }
  }
}

void Bot::sendIp(const std::string &realIp) {
  char data[MAXLINE];
  snprintf(data, sizeof(data), "\xff\xff\xff\xff" "ip 0 %s", realIp.c_str());

  Message s;
  s.setConnectionless(true);
  s.writeString(data);
  outputQueue.push(s);
}

void Bot::sendExtensions() {
  LOG << "Sending extension";
  Message s;
  s.writeByte(clc_stringcmd);
  s.writeString("pext 0x58455446 0x2140f000 0x32455446 0x2 0x3144564d 0x1");
  s.writeByte(0);
  outputQueue.push(s);
}

void Bot::sendNew() {
  Message s;
  s.delay = 0;
  s.writeByte(clc_stringcmd);
  s.writeString("new");
  s.writeByte(0);
  outputQueue.push(s);
}

void Bot::updateState() {
  if (connection.hasJoinedGame()) {
    currentState = None;
    return;
  }

  if (previousState != currentState) {
    LOG << "State changed from " << previousState << " to " << currentState;
  }

  previousState = currentState;

  switch (currentState) {
    case New:
      sendNew();
      currentState = None;
      break;
    case Info:
      setInfo();
      currentState = None;
      break;
    case Prespawn:
      currentState = None;
      break;
    case Begin: {
      std::stringstream ss;
      ss << "begin " << spawnCount;
      requestStringCommand(ss.str().c_str());
      currentState = None;
      break;
    }
    case JoinTeam: {
      // Assuming it is fortress gamedir now.
      int team = impulseConfig->getInt("main", botConfig.team);
      sendImpulse(team, 2);

      {
        std::stringstream ss;
        ss << "setinfo \"bottomcolor\" \"";
        ss << botConfig.bottomColor << "\"";
        requestStringCommand(ss.str(), 2);
      }
      {
        std::stringstream ss;
        ss << "setinfo \"team\" \""; 
        ss << botConfig.team << "\"";
        requestStringCommand(ss.str(), 0);
      }

      currentState = None;
      break;
    }
    case SelectClass: {
      int skin = impulseConfig->getInt("main", botConfig.skin);
      sendImpulse(skin, 2);

      {
        std::stringstream ss;
        ss << "setinfo \"skin\" \"";
        ss << botConfig.skin << "\"";
        requestStringCommand(ss.str(), 2);
      }

      currentState = None;
      break;
    }
    case DisableChat:
      break;
    case Done: {
      requestStringCommand("say yo check ma style!");
      requestStringCommand("setinfo \"chat\" \"\"");
      connection.handshakeComplete();
      currentState = None;
      for (int i = 0; i < UPDATE_BACKUP; i++) {
        nullCommand(&cmds[i]);
      }

      pthread_t tid;
      pthread_create(&tid, nullptr, &Bot::thinkLoop, this); 
      break;
    }
    default:
      break;
  }

}

void * Bot::thinkLoop(void *ptr) {
  Bot * bot = (Bot*)(ptr);
  while(true) {
    bot->think();
    usleep(10);
  }

  return nullptr;
}


void Bot::setInfo() {
  Message s;
  s.delay = 0;

  s.writeByte(clc_stringcmd);
  s.writeString("setinfo pmodel 13845");
  s.writeByte(0);
  s.writeByte(clc_stringcmd);
  s.writeString("setinfo emodel 6967");
  s.writeByte(0);

  std::stringstream ss;
  std::stringstream mapChecksum2;
  int dummy = 0;
  int checksum2 = 0;
  std::stringstream mapss;
  mapss << "../resources/maps/" << mapName << ".bsp";
  Utility::loadMap(mapss.str(), &dummy, &checksum2);
  mapChecksum2 << checksum2;

  ss << "prespawn " << spawnCount << " 0 " << mapChecksum2.str();
  s.writeByte(clc_stringcmd);
  s.writeString(ss.str().c_str());
  s.writeByte(0);
  outputQueue.push(s);
 
  BspParser bspParser;
  Model bspModel = bspParser.loadModel(mapss.str());
  targetingSystem->setBspModel(bspModel); 
  initConfiguration();
}

void Bot::sendImpulse(byte impulse, long delay) {
  this->impulse(impulse);
}

void Bot::createCommand(Message *s) {
  Command *oldcmd, *cmd;

  s->writeByte(clc_move);
  int crcIndex = s->getSize();
  s->writeByte(0);
  // packet loss
  s->writeByte(0);

  int i = (connection.getOutgoingSequence() - 2) & UPDATE_MASK;
  cmd = &cmds[i];

  s->writeDeltaUserCommand(&nullcmd, cmd);
  oldcmd = cmd;

  i = (connection.getOutgoingSequence() - 1) & UPDATE_MASK;
  cmd = &cmds[i];
  s->writeDeltaUserCommand(oldcmd, cmd);
  oldcmd = cmd;

  i = (connection.getOutgoingSequence()) & UPDATE_MASK;
  cmd = &cmds[i];
  s->writeDeltaUserCommand(oldcmd, cmd);

  size_t size = s->getSize();
  byte maxBuffer[size] = { 0 };
  copy(s->getData().begin(), s->getData().end(), maxBuffer);

  s->clear();

  maxBuffer[crcIndex] = Utility::crcByte(maxBuffer + crcIndex + 1, size - crcIndex - 1, connection.getOutgoingSequence());

  for (int i = 0; i < size; i++) {
    s->pushData(maxBuffer[i]);
  }

  s->setCurrentSize(size);
}

void Bot::think() {
  static std::string previousDescription;
  PlayerInfo *me = getPlayerBySlot(mySlot);

  if (me == nullptr) {
    LOG << " me == nullptr";
    return;
  }

  botMemory->updateVision();
  targetingSystem->update();

  if (getHealth() > 0) {
    double maxScore = -1.0;
    for (const auto &g : goals) {
       if (g->isFinished()) {
        continue;
      }

      double desire = g->calculateDesirability();
      if (desire > maxScore) {
        maxScore = desire;
        goal = g.get();
      }
    }

    if (goal != nullptr) {
      goal->update();
      std::string description = goal->description();
      if (previousDescription != description) {
        LOG << "Bot is " << description << " maxScore " << maxScore;
      }

      previousDescription = description;
    }
  } else if (getHealth() <= 0) {
    for (const auto &g : goals) {
      g->reset();
    }

    targetingSystem->clearTarget();

    timers["button"] += (currentTime - previousTime);
    if (timers["button"] > 1) {
      clickButton(0);
      timers["button"] = 0;
    } else if (timers["button"] > 0.2) {
      clickButton(1);
    }
  }
}

void Bot::requestStringCommand(std::string value) {
  requestStringCommand(value, 1);
}

void Bot::requestStringCommand(std::string value, double delay) {
  Message sendMsg;
  sendMsg.delay = 0;
  sendMsg.writeByte(clc_stringcmd);
  sendMsg.writeString(value.c_str());
  sendMsg.writeByte(0);
  outputQueue.push(sendMsg);
}

void Bot::parseBeam(Message *msg, int type) {
  int ent, i;
  float start[3] = { };
  float end[3] = { };
  ent = msg->readShort();

  start[0] = msg->readCoord();
  start[1] = msg->readCoord();
  start[2] = msg->readCoord();

  end[0] = msg->readCoord();
  end[1] = msg->readCoord();
  end[2] = msg->readCoord();

}

void Bot::parseProjectiles(Message *msg, bool indexed) {
  int c = msg->readByte();
  int num = 0;
  byte bits[6];
  for (int i = 0; i < c; i++) {
    num = indexed ? msg->readByte() : 0;

    for (int j = 0; j < 6; j++) {
      bits[j] = msg->readByte();
    }
  }
}

void Bot::parseStatic(Message *msg, bool extended) {
  if (extended) {
    parseDelta(msg, msg->readShort());
  } else {
    parseBaseline(msg);
  }

}

void Bot::parseBaseline(Message *msg) {
  // Parse baseline
  int i;
  float origin[3] = { };
  float angles[3] = { };
  byte modelindex = msg->readByte();
  byte frame = msg->readByte();
  byte colormap = msg->readByte();
  byte skinnum = msg->readByte();

  for (i = 0; i < 3; i++) {
    CoordData c = { 0 };
    msg->readData(&c, 2);
    angles[i] = msg->readAngle();
  }
}

void Bot::parsePacketEntities(Message *msg, bool delta) {
  byte from = 0;
  int newNum;

  if (delta) {
    from = msg->readByte();
  }

  validSequence = connection.getIncomingSequence();

  int word = 0;
  while (1) {
    word = (unsigned short) msg->readShort();
    if (msg->isBadRead()) {
      // something didn't parse right...
//      LOG << "msg_badread in packetentities";
      return;
    }

    if (!word) {
      break;
    }

    newNum = word & 511;

    parseDelta(msg, word);
  }

}

void Bot::parseDelta(Message *msg, byte bits) {
  int i;
  int morebits;
  bits &= ~511;

  if (bits & U_MOREBITS) {
    i = msg->readByte();
    bits |= i;
  }

  morebits = 0;

  if (bits & U_MODEL) {
    msg->readByte();
  }

  if (bits & U_FRAME) {
    msg->readByte();
  }

  if (bits & U_COLORMAP) {
    msg->readByte();
  }

  if (bits & U_SKIN) {
    msg->readByte();
  }

  if (bits & U_EFFECTS) {
    msg->readByte();
  }

  if (bits & U_ORIGIN1) {
    CoordData c = { 0 };
    msg->readData(&c, 2);
  }

  if (bits & U_ANGLE1) {
    msg->readAngle();
  }

  if (bits & U_ORIGIN2) {
    CoordData c = { 0 };
    msg->readData(&c, 2);
  }

  if (bits & U_ANGLE2) {
    msg->readAngle();
  }

  if (bits & U_ORIGIN3) {
    CoordData c = { 0 };
    msg->readData(&c, 2);
  }

  if (bits & U_ANGLE3) {
    msg->readAngle();
  }
}

void Bot::parseBaseline2(Message *msg) {
  parseDelta(msg, msg->readShort());
}

void Bot::moveForward(short speed) {
  cmds[frame].forwardMove = speed;
}

void Bot::moveUp(short speed) {
  cmds[frame].upMove = speed;
}

void Bot::moveSide(short speed) {
  cmds[frame].sideMove = speed;
}

void Bot::clickButton(int button) {
   cmds[frame].buttons = button;
 // }
}

void Bot::rotateY(int angle) {
  cmds[frame].angles[1] = angle;
  this->angle.y = angle;
}

void Bot::rotateX(int angle) {
  cmds[frame].angles[0] = angle;
  this->angle.x = angle;
}

int Bot::getAngleX() {
  int a = this->angle.x;
  return a;
}

int Bot::getAngleY() {
  int a = this->angle.y;
  return a;
}

void Bot::impulse(int impulse) {
  cmds[frame].impulse = impulse;
}

void Bot::nullCommand(Command *cmd) {
  cmd->angles[0] = 0;
  cmd->angles[1] = 0;
  cmd->angles[2] = 0;
  cmd->buttons = 0;
  cmd->forwardMove = 0;
  cmd->impulse = 0;
  cmd->msec = 2;
  cmd->sideMove = 0;
  cmd->upMove = 0;
}

void Bot::nullButton(Command *cmd) {
  cmd->buttons = 0;
}

void Bot::nullButtons() {
  for (int i = 0; i < UPDATE_BACKUP; i++) {
    nullButton(&cmds[i]);
  }
}

void Bot::nullCommands() {
  for (int i = 0; i < UPDATE_BACKUP; i++) {
    nullCommand(&cmds[i]);
  }
}

void Bot::initConfiguration() {

  std::stringstream configSs;
  configSs << "../resources/" << mapName << ".ini";
  botConfig.defend = this->config->getString("main", "defend");
  
  this->mapConfig = std::make_unique<Config>(configSs.str());

  const int maxRespawns = 10;
  const int maxPositions = 20;

  // Initialize respawn positions.
  int i = 0;
  for (; i < maxRespawns; i++) {
    std::stringstream ssi;
    ssi << "respawn" << i;
    if(!mapConfig->sectionExist(ssi.str())) {
      break;
    } 
    initPosition(ssi.str());
    LOG << "initRespawn " << ssi.str();
  }

  botConfig.numRespawns = i;

  // Initialize defend/attack positions.
  int j = 0;
  for (; j < maxPositions; j++) {
    std::stringstream ssi;
    ssi << "position" << j;
    if(!mapConfig->sectionExist(ssi.str())) {
      break;
    }
    initPosition(ssi.str());
    LOG << "initPosition " << ssi.str();
  }

  // Initialize waypoints from respawn to position.
  for (int ii = 0; ii < i; ii++) {
    for (int jj = 0; jj < j; jj++) {
      std::stringstream ssi;
      ssi << "respawn" << ii << "-" << "position" << jj;
      initWaypoints(ssi.str());
      LOG << "init way points " << ssi.str();
    }
  }

  for (int k = 0; k < 10; ++k) {
    std::stringstream ss;
    ss << "no_firezone" << k;
    if (!mapConfig->sectionExist(ss.str())) {
      break;
    }
   
    std::pair<float, glm::vec3> p = { mapConfig->getInt(ss.str(), "radius"), mapConfig->getVec3(ss.str(), "origin") };
    botConfig.noFirezones.push_back(p);
    
  }
}

void Bot::initWaypoints(const std::string & section) {
  std::string wpDir = this->mapConfig->getString("main", "waypoints_dir");
  std::stringstream waypointsFilename;
  waypointsFilename << "../resources/" << wpDir << "/" << section << ".dat";

  std::fstream fs;
  std::string filename(waypointsFilename.str());

  fs.open(filename, std::fstream::in);
  if (fs.fail()) {
    std::stringstream ss;
    LOG << "Failed to open " << filename;
    return;
  }

  while (!fs.eof()) {
    std::stringstream ss;
    char line[256] = { 0 };
    fs.getline(line, 256);    
    glm::vec3 waypoint;
    ss << line;
    ss >> waypoint.x >> waypoint.y >> waypoint.z;
    botConfig.waypoints[section].push_back(waypoint);
  } 

  fs.close();
}

void Bot::initPosition(const std::string & section) {
  for(int i = 0; ;i++) { 
    std::stringstream ssp;
    ssp << "p" << i;

    if (this->mapConfig->getString(section, ssp.str()) == "n/a") {
      return;
    }

    glm::vec3 pt = this->mapConfig->getVec3(section, ssp.str());
    float dist = glm::dot(pt, glm::vec3(1, 1, 1));

    if (dist < 0.001 && dist > -0.001) {
      return;
    }

    LOG << section << " with " << ssp.str();
    botConfig.waypoints[section].push_back(pt);
  }
}
