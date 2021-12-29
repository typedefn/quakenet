/* 
 * File:   Bot.cpp
 * Author: dervish
 * 
 * Created on February 21, 2021, 8:35 AM
 */

#include "Bot.hpp"
#include "Gene.hpp"

Bot::Bot(char **argv) {

  protoVer = 0;
  delay = 5000;
  challenge = 0;
  frame = 0;
  mySlot = -1;
  previousState = currentState = None;
  this->botMemory = make_unique<BotMemory>(this, 8);
  this->targetingSystem = make_unique<TargetingSystem>(this);

  goals.push_back(make_unique<PatrolGoal>(this));
//  goals.push_back(make_unique<SeekGoal>(this));
//  goals.push_back(make_unique<AttackGoal>(this));

  for (int i = 0; i < MAX_CLIENTS; i++) {
    players[i].coords[0] = 0;
    players[i].coords[1] = 0;
    players[i].coords[2] = 0;
    players[i].entertime = 0.0;
    players[i].flags = 0;
    players[i].frags = 0;
    players[i].frame = 0;
    strcpy(players[i].name, "");
    players[i].ping = 0;
    players[i].pl = 0;
    players[i].slot = -1;
    players[i].active = false;
    players[i].angles[0] = 0;
    players[i].angles[1] = 0;
    players[i].angles[2] = 0;
    players[i].velocity = vec3(0, 0, 0);
    players[i].position = vec3(0, 0, 0);
    players[i].speed = 0;
    players[i].direction = vec3(0, 0, 0);
  }
  this->argv = argv;

  // TODO: Put these checksums in a map checksum file.
  // Already have a loadMap utility method that gets the checksums of files.
  mapChecksums["ultrav"] = "360735597";
  mapChecksums["1on1r"] = "-756178370";
  mapChecksums["skull666"] = "-1518401826";
  mapChecksums["areowalk"] = "-638279197";
  mapChecksums["ztndm3"] = "-1723650232";
  mapChecksums["bravado"] = "-1859843008";

  waypoints[""] = vector<vec3>();
  waypoints["patrol"] = vector<vec3>();
  waypoints["start"] = vector<vec3>();

  for (size_t i = 0; i < MAX_CL_STATS; i++) {
    setStat(i, 0);
  }

  spawnCount = 0;
  timeChallengeSent = getTime();

  serverMessages[svc_serverdata] = make_unique<ServerDataMessage>(this);
  serverMessages[svc_packetentities] = make_unique<PacketEntitiesMessage>(this);
  serverMessages[svc_modellist] = make_unique<ModelListMessage>(this);
  serverMessages[svc_download] = make_unique<DownloadMessage>(this);
  serverMessages[svc_spawnstaticsound] = make_unique<SpawnStaticSoundMessage>(this);
  serverMessages[svc_updatestat] = make_unique<UpdateStatMessage>(this);
  serverMessages[svc_soundlist] = make_unique<SoundListMessage>(this);
  serverMessages[svc_setinfo] = make_unique<SetInfoMessage>(this);
  serverMessages[svc_stufftext] = make_unique<StuffTextMessage>(this);
  serverMessages[svc_updateuserinfo] = make_unique<UpdateUserInfoMessage>(this);
  serverMessages[svc_playerinfo] = make_unique<PlayerInfoMessage>(this);
  serverMessages[svc_print] = make_unique<PrintMessage>(this);
  gotChallenge = false;
  validSequence = 0;
  goal = nullptr;
}

Bot::~Bot() {
}

void Bot::mainLoop() {
  connection.connect(this->argv);
  getChallenge();
  nullCommand(&nullcmd);
  for (int i = 0; i < UPDATE_BACKUP; i++) {
    nullCommand(&cmds[i]);
  }
  // Assuming this is a 1on1r.map so load 1on1r.bot
  fstream fs;
  string filename("../resources/1on1r.bot");

  fs.open(filename, fstream::in);
  if (fs.fail()) {
    stringstream ss;
    ss << "Failed to open " << filename;
    throw runtime_error(ss.str());
  }

  while (!fs.eof()) {
    stringstream ss;
    char line[256] = { 0 };
    fs.getline(line, 256);
    ss << line;
    string type;
    glm::vec3 waypoint;
    ss >> type >> waypoint.x >> waypoint.y >> waypoint.z;
    waypoints.at(type).push_back(waypoint);
  }

  fs.close();

  double currentTime = getTime();
  double previousTime = 0;
  double totalTime = 0;

  bool received = false;

  while (1) {
    int s = 0;
    previousTime = currentTime;
    currentTime = getTime();
    totalTime += (currentTime - previousTime);

    Message inMessage;

    if (connection.recv(&inMessage, false)) {
      if (connection.process(&inMessage)) {
        parseServerMessage(&inMessage);
      }
    }

    if (!outputQueue.empty()) {
      Message outMessage = outputQueue.front();
      if (outMessage.isConnectionless()) {
        s = connection.sendConnectionless(outMessage);
      } else {
        s = connection.send(outMessage);
      }
      outputQueue.pop();
      totalTime = 0;
    }

    updateState();

    // Resend msgs due to timeout or some weird issue.
    if (!connection.hasJoinedGame() && (getTime() - timeChallengeSent) > 10.0) {
      LOG << "Messaged timed out, resending";
      outputQueue.clear();
      connection.close();
      connection.connect(this->argv);
      getChallenge();
      currentState = None;
    }

    if (connection.hasJoinedGame()) {
      think();
    }

    // Prevent CPU hogging.
    usleep(delay);
  }
}

PlayerInfo* Bot::getPlayerBySlot(size_t id) {
  PlayerInfo *pi = nullptr;

  infoLock.lock();
  pi = &players[id];
  infoLock.unlock();

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
    if (connection.recv(&msg, false)) {

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
        strcpy(userInfo, "\\rate\\100000\\name\\krupt_drv\\*client\\dumbo1234\\*z_ext\\511");
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

    // TODO old switch statement, needs to be wrapped into classes.
    switch (cmd) {
    case svc_fte_spawnstatic2: {
      // Static entities are non-interactive world objects like torches.
      parseStatic(message, true);
      break;
    }
    case svc_fte_spawnbaseline2: {
      parseBaseline2(message);
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
      string key = message->readString();
      string value = message->readString();
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
      if (i >= 0 && i < MAX_CL_STATS) {
        setStat(i, j);
      }
      break;
    }
    case svc_setangle: {
      float x = message->readChar() * (360.0 / 256);
      float y = message->readChar() * (360.0 / 256);
      float z = message->readChar() * (360.0 / 256);
      break;
    }
    case svc_damage: {
      int armor_ = message->readByte();
      int blood_ = message->readByte();
      float coords[3];
      float distanceToMe;
      float distanceToTarget;

      for (int i = 0; i < 3; i++) {
        coords[i] = message->readCoord();
      }

      vec3 from(coords[0], coords[2], coords[1]);
      break;
    }
    case svc_sound: {
      float pos[3] = { };
      byte channel = message->readByte();
      byte soundNumber = message->readByte();

      for (int i = 0; i < 3; i++) {
        pos[i] = message->readCoord();
      }

      break;
    }
    case svc_stopsound: {
      message->readShort();
      break;
    }

    case svc_spawnbaseline: {
      message->readShort();
      parseBaseline(message);
      break;
    }
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
//    case svc_sellscreen: {
//      break;
//    }
//    case svc_smallkick: {
//      break;
//    }
//    case svc_bigkick: {
//      break;
//    }
//    case svc_nails: {
//      parseProjectiles(message, false);
//
//      break;
//    }
//    case svc_nails2: {
//      parseProjectiles(message, true);
//
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
//    case svc_qizmovoice: {
//      int i;
//      message->readByte();
//      message->readByte();
//
//      for (i = 0; i < 32; i++)
//        message->readByte();
//
//      break;
//    }

    default: {
      break;
    }
    }
  }
}

void Bot::sendIp(const string &realIp) {
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
    LOG << "State changed to " << currentState << " from " << previousState;
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
    stringstream ss;
    ss << "begin " << spawnCount;
    requestStringCommand(ss.str().c_str());
    requestStringCommand("setinfo \"chat\" \"\"", 2);
    currentState = None;
    break;
  }
  case JoinTeam: {
    // Assuming it is fortress gamedir now.
    sendImpulse(1, 2);
    currentState = None;
    break;
  }
  case SelectClass: {
    sendImpulse(3, 2);
    break;
  }
  case DisableChat:
    break;
  case Done:
    sendImpulse(7, 2);
    requestStringCommand("setinfo \"bottomcolor\" \"13\"", 2);
    requestStringCommand("setinfo \"team\" \"blue\"", 0);
    requestStringCommand("setinfo \"skin\" \"tf_sold\"", 2);
    delay = 20;
    currentState = None;
    connection.handshakeComplete();
    break;
  default:
    break;
  }

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

  stringstream ss;
  string mapChecksum2 = "-756178370";

  if (mapChecksums.find(mapName) != mapChecksums.end()) {
    mapChecksum2 = mapChecksums.at(mapName);
  } else {
    LOG << "Failed to look up checksum for map " << mapName << ", using default for 1on1r";
  }

  ss << "prespawn " << spawnCount << " 0 " << mapChecksum2;
  s.writeByte(clc_stringcmd);
  s.writeString(ss.str().c_str());
  s.writeByte(0);
  outputQueue.push(s);
}

void Bot::sendImpulse(byte impulse, long delay) {
  for (int i = 0; i < UPDATE_BACKUP; i++) {
    cmds[frame].impulse = impulse;
    frame = (frame + 1) % UPDATE_BACKUP;
  }
  Message s;
  s.setCommand(true);
  s.delay = delay;
  createCommand(&s);
  outputQueue.push(s);

  for (int i = 0; i < UPDATE_BACKUP; i++) {
    nullCommand(&cmds[frame]);
    frame = (frame + 1) % UPDATE_BACKUP;
  }
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

  if (connection.getOutgoingSequence() - validSequence >= UPDATE_BACKUP - 1) {
    validSequence = 0;
  }

  if (validSequence) {
    s->writeByte(clc_delta);
    s->writeByte(validSequence & 255);
  }
}

void Bot::think() {
  static string previousDescription;
  static double previousTime = 0;
  static double counter = 0;
  static double currentTime = 0;

  PlayerInfo *me = getPlayerBySlot(mySlot);

  if (me == nullptr) {
    LOG << " me == nullptr";
    return;
  }

  frame = (connection.getOutgoingSequence() & UPDATE_MASK);
  Command *command = &cmds[frame];

  nullCommand(command);

  previousTime = currentTime;
  currentTime = getTime();
  counter += (currentTime - previousTime);

  botMemory->updateVision();
  targetingSystem->update();

  if (getHealth() > 0) {
    double maxScore = -1.0;
    for (const auto &g : goals) {
      double desire = g->calculateDesirability();
      if (desire > maxScore) {
        maxScore = desire;
        goal = g.get();
      }
    }

    if (goal != nullptr) {
      goal->update();
      string description = goal->description();
      if (previousDescription != description) {
        LOG << "Bot is " << description << " maxScore " << maxScore;
      }

      previousDescription = description;
    }
  } else if (getHealth() <= 0) {
    targetingSystem->clearTarget();
    clickButton(1);
  }

  // attempt every msec.
  float msecs = (command->msec / 1000.0f);

  if (counter > msecs) {
    Message s;
    createCommand(&s);
    outputQueue.push(s);
    counter = 0;
  }
}

void Bot::requestStringCommand(string value) {
  requestStringCommand(value, 1);
}

void Bot::requestStringCommand(string value, double delay) {
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
  if (frame < 32) {
    cmds[frame].upMove = speed;
  }
}

void Bot::moveSide(short speed) {
  if (frame < 32) {
    cmds[frame].sideMove = speed;
  }
}

void Bot::clickButton(int button) {
  if (frame < 32) {
    cmds[frame].buttons = button;
  }
}

void Bot::rotateY(int angle) {
  cmds[frame].angles[1] = angle;
}

void Bot::rotateX(int angle) {
  cmds[frame].angles[0] = angle;
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
  cmd->msec = 10 + (rand() % 20);
  cmd->sideMove = 0;
  cmd->upMove = 0;
}
