/* 
 * File:   Bot.cpp
 * Author: dervish
 * 
 * Created on February 21, 2021, 8:35 AM
 */

#include "Bot.hpp"
#include "Gene.hpp"

Bot::Bot(char **argv) {
  challenge = 0;
  blood = 100;
  armor = 200;
  frame = 0;
  targetSlot = 0;
  elapsedTime = 0;
  me = nullptr;
  totalTime = 0;
  previousState = currentState = None;
  srand(time(nullptr));
  newCount = 0;
  delay = 0;
  duration = 0;
  ipRecv = false;
  requestChallenge = true;
  this->botMemory = make_unique<BotMemory>(this, 4.0);
  this->targetingSystem = make_unique<TargetingSystem>(this);

  goals.push_back(make_unique<PatrolGoal>(this));
  goals.push_back(make_unique<SeekGoal>(this));
  goals.push_back(make_unique<AttackGoal>(this));

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
    players[i].slot = 0;
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

  frameTime = 0.05;

  for (size_t i = 0; i < MAX_CL_STATS; i++) {
    setStat(i, 0);
  }
  respawned = false;
}

Bot::~Bot() {
}

void Bot::mainLoop() {
  connection.connect(this->argv);
  getChallenge();
  nullCommand(&nullcmd);

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

  double lastSent = 0;
  double lastReceived = 0;
  double timePassed = 0;
  int counter = 0;
  double updateDuration = 0;

  while (1) {
    int s = 0;
    if (!outputQueue.empty()) {
      Message message = outputQueue.front();
      double deltaTime = (timePassed - lastSent);
      if (deltaTime >= message.delay) {
        if (message.isConnectionless()) {
          s = connection.sendConnectionless(message);
        } else {
          s = connection.send(message);
        }
        lastSent = getTime();
        lastMessage.clear();
        message.copyMessage(&lastMessage);
        outputQueue.pop();
      }
    }

    Message message;
    while (connection.recv(&message, false)) {
      connection.process(&message);
      parseServerMessage(&message);
      lastReceived = getTime();
    }

    if ((timePassed - updateDuration) > 1.5) {
      updateState();
      updateDuration = getTime();
    }

    if (requestChallenge) {
      if ((getTime() - timeChallengeSent) > 10.0) {
        LOG << "Resending challenge due to timeout";
        timeChallengeSent = getTime();
        getChallenge();
      }
    }

    timePassed = getTime();
  }
}

PlayerInfo* Bot::getPlayerById(size_t id) {
  PlayerInfo *pi = nullptr;

  infoLock.lock();
  pi = &players[id];
  infoLock.unlock();

  return pi;
}

PlayerInfo* Bot::getMe() {
  size_t mySlot = me->slot;

  return getPlayerById(mySlot);
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
  bool established = false;

  int c = 0;

  while (!established) {
    msg.clear();
    if (connection.recv(&msg, false)) {
      msg.beginRead();
      msg.readLong();
      c = msg.readByte();

      switch (c) {
      case S2C_CHALLENGE: {
        char data[MAXLINE] = { 0 };
        char userInfo[MAX_INFO_STRING + 32] = { 0 };
        challenge = atol(msg.readString());

        for (;;) {
          c = msg.readLong();
          if (msg.isBadRead()) {
            break;
          }

          // Read proto version info...
          msg.readLong();
        }
        strcpy(userInfo, "\\rate\\25000\\name\\krupt_drv\\msg\\1\\noaim\\1\\*client\\dumbo 1234\\*z_ext\\511");
        snprintf(data, sizeof(data), "\xff\xff\xff\xff" "connect %d %d %lu \"%s\"\n",
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
        requestStringCommand("new");
        requestStringCommand("new", 2);
        established = true;
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

  unsigned protover;

  while (1) {

    if (message->isBadRead()) {
      break;
    }

    msgSvcStart = message->getReadCount();

    cmd = message->readByte();

    if (cmd == -1) {
      message->incReadCount();
      break;
    }

    switch (cmd) {
    case svc_nop: {
      break;
    }

    case svc_packetentities: {
      for (int i = 0; i < 7; i++) {
        message->readByte();
      }
      break;
    }
    case svc_modellist: {
      int numModels, n;
      char *str;
      if (protover >= 26) {
        numModels = message->readByte();

        while (1) {
          str = message->readString();
          if (!str[0]) {
            break;
          }

          numModels++;

          if (str[0] == '/') {
            str++;
          }
        }

        if ((n = message->readByte())) {
          return;
        }

      }

      break;
    }
    case svc_download: {
      int size, percent;
      int s = 0;
      size = message->readShort();
      for (s = 0; s < 24; s++) {
        message->readByte();
      }
      break;
    }
    case svc_spawnstaticsound: {
      //read junk
      for (int i = 0; i < 3; i++) {
        message->readCoord();
      }
      for (int i = 0; i < 3; i++) {
        message->readByte();
      }
      break;
    }
    case svc_fte_spawnstatic2: {
      // Static entities are non-interactive world objects like torches.
      parseStatic(message);
      for (int i = 0; i < 17; i++) {
        message->readByte();
      }
      break;
    }
    case svc_fte_spawnbaseline2: {
      parseStatic(message);
      break;
    }
    case nq_svc_time: {
      message->readFloat();
      break;
    }
    case svc_updatestat: {
      int i = message->readByte();
      int j = message->readByte();

      if (i >= 0 && i < MAX_CL_STATS) {
        setStat(i, j);
      }

      break;
    }
    case svc_soundlist: {
      char *str;
      int n;
      byte numSounds;
      if (protover >= 26) {
        numSounds = message->readByte();

        while (1) {
          str = message->readString();
          if (!str[0]) {
            break;
          }

          numSounds++;

          if (str[0] == '/') {
            str++;
          }
        }

        n = message->readByte();

        if (n) {
//					stringstream ss;
//					ss << "soundlist" << " " << 1 << " " << n;
//					requestStringCommand(ss.str());
          return;
        }
      } else {

        numSounds = 0;

        do {
          if (++numSounds > 255) {
            LOG << "Error send to many sound_precache";
          }
          str = message->readString();
        } while (*str);
      }

      break;
    }
    case svc_intermission: {
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
    case svc_print: {
      int id = message->readByte();
      byte *dbyte = (byte*) message->readString();
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
    case svc_setinfo: {
      int slot = message->readByte();

      if (slot >= MAX_CLIENTS) {
        break;
      }

      string key(message->readString());
      string value(message->readString());

      if (key == "team" && value == "blue") {
        currentState = SelectClass;
        LOG << "JOINED BLUE TEAM";
      } else if (key == "skin" && value == "tf_sold") {
        currentState = Done;
        LOG << "SELECTED CLASS 'SOLDIER'!";
      }

      break;
    }
    case svc_updatefrags: {
      int slot = message->readByte();
      if (slot >= MAX_CLIENTS) {
        break;
      }

      PlayerInfo *pi = getPlayerById(slot);
      pi->frags = message->readShort();

      break;
    }
    case svc_updateping: {
      message->readByte();
      message->readShort();
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

      break;
    }
    case svc_serverdata: {
      for (;;) {
        protover = message->readLong();
        if (message->isBadRead()) {
          break;
        }

        if (protover == PROTOCOL_VERSION_FTE) {
          long fteextensions = message->readLong();
          continue;
        }

        if (protover == PROTOCOL_VERSION_FTE2) {
          long fteext = message->readLong();
          continue;
        }

        if (protover == PROTOCOL_VERSION_MVD1) {
          long ext = message->readLong();
          continue;
        }

        if (protover == PROTOCOL_VERSION) {
          break;
        }
      }
      spawnCount = message->readLong();

      // Gamedir
      gameDir = message->readString();
      unsigned playerNum = message->readByte();

      if (playerNum & 128) {
        //spectator = true;
        playerNum &= ~128;
      }

      // Get the full level name
      message->readString();

      if (protover >= 25) {
        float gravity = message->readFloat();
        float stopspeed = message->readFloat();
        float maxSpeed = message->readFloat();
        float specMaxSpeed = message->readFloat();
        float accelerate = message->readFloat();
        float airAccelerate = message->readFloat();
        float waterAccelerate = message->readFloat();
        float friction = message->readFloat();
        float waterFriction = message->readFloat();
        float entGravity = message->readFloat();
      }

      //ask for sound list
//			requestStringCommand("soundlist 1 0");
      break;
    }
    case svc_stufftext: {
      string temp;
      string line(message->readString());
      vector<string> tokens;
      stringstream check(line);
      while (getline(check, temp, ' ')) {
        tokens.push_back(temp);
      }

      std::size_t pos = line.find("cmd");
//      stringstream cs;
//      for(size_t i = 0; i < line.size(); i++) {
//        char c = line.at(i);
//        if (c >= 32 && c <= 126) {
//               cs << (char) c;
//             } else {
//               cs << "[" << (short) c << "]";
//             }
//      }
//
//      LOG << "line = " << cs.str();
      if (tokens.size() > 1 && tokens[0] == "packet" && tokens[2] == "\"ip") {
        string realIpVal = tokens[4].substr(0, tokens[4].size() - 5);
        sendIp(realIpVal);
      } else if (tokens.size() > 1 && tokens[1] == "pext\n") {
        sendExtensions();
      } else if (tokens.size() >= 2 && tokens[0] == "cmd") {

        if (tokens.at(1) == "skin") {
          break;
        }

        stringstream ss;
        for (int i = 0; i < tokens.size() - 1; i++) {
          string t = tokens.at(i + 1);
          if (i == tokens.size() - 2) {
            t.pop_back();
          }
          ss << t;
          if (i < tokens.size() - 2) {
            ss << " ";
          }
        }

        if (tokens[1] == "spawn" && atoi(tokens[3].c_str()) > 0) {
          currentState = Begin;
          spawnCmd = ss.str();
        }

        requestStringCommand(ss.str());
      } else if (tokens.size() > 2 && tokens[0] == "fullserverinfo") {
        mapName = Utility::findValue("map", line);
        LOG << "MAP NAME = " << mapName;
        requestChallenge = false;
        currentState = Info;
      }
      break;
    }
    case svc_updateuserinfo: {
      int slot = message->readByte();
      if (slot >= MAX_CLIENTS) {
        break;
      }

      long userId = message->readLong();
      string value(message->readString());
      string name = Utility::findValue("name", value);
      // TODO: fix hardcoded name of bot later.
      if (name == "krupt_drv") {
        LOG << "I AM IN SLOT " << slot << " id = " << userId;
        me = getPlayerById(slot);
        me->slot = slot;
        if (currentState == Spawn) {
          if (gameDir == "qw") {
            currentState = Done;
          } else {
            currentState = JoinTeam;
          }
        }
      }
      break;
    }
    case svc_cdtrack: {
      byte cdTrack = message->readByte();
      break;
    }
    case svc_playerinfo: {
      unsigned num = message->readByte();
      if (num >= MAX_CLIENTS) {
        break;
      }

      PlayerInfo *pi = getPlayerById(num);

      if (me != nullptr && num != me->slot) {
        targetSlot = num;
        pi->slot = num;
      }

      short flags = message->readShort();
      pi->flags = flags;
      pi->active = true;

      for (int i = 0; i < 3; i++) {
        float a = message->readFloat();
        pi->coords[i] = a;

        if (num == targetSlot) {
//          LOG << i << " " << a;
        }
      }

      vec3 position = vec3(pi->coords[0], pi->coords[2], pi->coords[1]);
      pi->position = position;
      pi->time = getTime();

      byte frame = message->readByte();

      if (flags & PF_MSEC) {
        byte msec = message->readByte();
        if (num == targetSlot) {
          frameTime = msec * 0.001;
        }

        if (frameTime > 0.1) {
          frameTime = 0.1;
        }
      }

      if (flags & PF_COMMAND) {
        int bits = message->readByte();

        // Need to be vary of version for protoVersion
        // if (client.protoVersion <= 26) ...
        if (bits & CM_ANGLE1) {
          message->readFloat();
        }
        if (bits & CM_ANGLE3) {
          message->readFloat();
        }

        if (bits & CM_FORWARD) {
          byte z = (message->readByte() << 3);
        }
        if (bits & CM_SIDE) {
          byte z = (message->readByte() << 3);
        }
        if (bits & CM_UP) {
          byte z = (message->readByte() << 3);
        }

        if (bits & CM_BUTTONS) {
          message->readByte();
        }

        if (bits & CM_IMPULSE) {
          message->readByte();
        }

        //cout << "MSEC(" << num << ")" << "       = "
        message->readByte();
      }

      for (int i = 0; i < 3; i++) {
        if (flags & (PF_VELOCITY1 << i)) {
          short v = message->readShort();
          if (i == 0) {
            pi->velocity.x = v;
          } else if (i == 1) {
            pi->velocity.z = v;
          } else if (i == 2) {
            pi->velocity.y = v;
          }

//          LOG << "PF_VELOCITY" << i << "(" << num << ")" << "     = " << v * frameTime;
        } else {
          pi->velocity[i] = 0;
        }
      }

      float velLength = length(pi->velocity);
      if (velLength > 0.001) {
        pi->direction = normalize(pi->velocity);
      }

      for (int i = 0; i < 3; i++) {
        pi->velocity[i] *= frameTime;
      }

      pi->speed = sqrt(pi->velocity[0] * pi->velocity[0] + pi->velocity[1] * pi->velocity[1] + pi->velocity[2] * pi->velocity[2]);

      if (flags & PF_MODEL) {
//				cout << "PF_MODEL(" << num << ")" << "         =  "
        message->readByte();
      }

      if (flags & PF_SKINNUM) {
//				cout << "PF_SKINNUM(" << num << ")" << "       = "
        message->readByte();
      }

      if (flags & PF_EFFECTS) {
//				cout << "PF_EFFECTS(" << num << ")" << "       = "
        message->readByte();
      }

      if (flags & PF_WEAPONFRAME) {
        message->readByte();
      }
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
      float x = message->readFloat();
      float y = message->readFloat();
      float z = message->readFloat();
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
      break;
    }
    case svc_stopsound: {
      message->readShort();
      break;
    }
    case svc_spawnbaseline: {
      message->readShort();
      break;
    }
    default: {
      break;
    }
    }
  }
}

void Bot::sendIp(const string &realIp) {
  char data[MAXLINE];
  snprintf(data, sizeof(data), "\xff\xff\xff\xff" "ip 0 %s\n", realIp.c_str());

  timeChallengeSent = getTime();

  Message s;
  s.setConnectionless(true);
  s.writeString(data);
  outputQueue.push(s);
}

void Bot::sendExtensions() {
  LOG << "Sending extension";
  Message s;
  s.delay = 2.0f;
  s.writeChar(4);
  s.writeString("pext 0x58455446 0x2140f000 0x32455446 0x2 0x3144564d 0x1");
  s.writeChar(0);
  s.writeChar(3);
  s.writeChar(134);
  s.writeChar(0);
  s.writeChar(0);
  s.writeChar(14);
  s.writeChar(0);
  s.writeChar(14);
  s.writeChar(0);
  s.writeChar(13);
  outputQueue.push(s);
}

void Bot::sendNew() {
  Message s;
  s.writeChar(4);
  s.writeString("new");
  s.writeChar(0);
  s.writeChar(3);
  s.writeChar(134);
  s.writeChar(0);
  s.writeChar(0);
  s.writeChar(14);
  s.writeChar(0);
  s.writeChar(13);
  s.writeChar(0);
  s.writeChar(13);
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
  switch (currentState) {
  case Info:
    setInfo();
    currentState = None;
    break;
  case Prespawn:
    currentState = None;
    break;
  case Spawn:
    break;
  case Begin: {
    requestStringCommand(spawnCmd, 2);
    stringstream ss;
    ss << "begin " << spawnCount;
    requestStringCommand(ss.str().c_str(), 3);
    currentState = Spawn;
    break;
  }
  case JoinTeam: {
    // Assuming it is fortress gamedir now.
    sendImpulse(1, 0);
    requestStringCommand("setinfo \"topcolor\" \"13\"", 0);
    requestStringCommand("setinfo \"bottomcolor\" \"13\"", 0);
    requestStringCommand("setinfo \"team\" \"blue\"", 0);
    break;
  }
  case SelectClass:
    sendImpulse(3, 0);
    requestStringCommand("setinfo \"skin\" \"tf_sold\"", 0);
    requestStringCommand("setinfo \"chat\" \"\"", 0);
    break;
  case DisableChat:
    break;
  case Done:
    currentState = None;
    connection.handshakeComplete();
    cmds[frame].angles[1] = 90;
    thinker = std::thread(&Bot::think, this);
    break;
  default:
    break;
  }

  previousState = currentState;
}

void Bot::setInfo() {
  Message s;
  s.delay = 7;
  s.writeByte(4);
  s.writeString("setinfo pmodel 13845");
  s.writeByte(0);
  s.writeByte(4);
  s.writeString("setinfo emodel 6967");
  s.writeByte(0);
  s.writeByte(4);
  stringstream ss;
  string mapChecksum2 = "-756178370";

  if (mapChecksums.find(mapName) != mapChecksums.end()) {
    mapChecksum2 = mapChecksums.at(mapName);
  } else {
    LOG << "Failed to look up checksum for map " << mapName << ", using default for 1on1r";
  }

  ss << "prespawn " << spawnCount << " 0 " << mapChecksum2;
  s.writeString(ss.str().c_str());
  s.writeByte(0);
  outputQueue.push(s);
}

void Bot::sendImpulse(byte impulse, long delay) {
  cmds[frame].impulse = impulse;
  frame = (frame + 1) % UPDATE_BACKUP;
  Message s;
  s.delay = delay;
  createCommand(&s);
  outputQueue.push(s);
}

void Bot::createCommand(Message *s) {
  Command *oldcmd, *cmd;

  s->writeByte(clc_move);
  int crcIndex = s->getSize();
  s->writeByte(0);
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

  stringstream ss;
  vector<byte> bytes = s->getData();
  for (int i = 0; i < bytes.size(); i++) {
    ss << bytes.at(i);
  }

}

void Bot::think() {
  static double extramsec = 0;
  LOG << "Thinking thread launched!";
  string previousDescription;
  Goal *goal = nullptr;
  double previousTime = 0;
  double counter = 0;
  double currentTime = 0;
  for (int i = frame; i < UPDATE_BACKUP; i++) {
    nullCommand(&cmds[i]);
  }

  while (true) {
    usleep(5000);
    extramsec += 0.5 * 1000;
    int ms = extramsec;

    extramsec -= ms;

    if (ms > 250) {
      ms = 100;
    }

    nullCommand(&cmds[frame]);

    if (me != nullptr && getHealth() > 0) {

      extramsec += 0.01;

      botMemory->updateVision();
      targetingSystem->update();

      double maxScore = -1.0;

      // Search for new goal current goal is completed.
      if ((goal == nullptr) || (goal != nullptr && goal->isCompleted())) {
        for (const auto &g : goals) {
          double desire = g->calculateDesirability();
//          LOG << g->description() << " desire " << desire;
          if (desire > maxScore) {
            maxScore = desire;
            goal = g.get();
          }
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

    }

    if (getHealth() <= 0) {
      previousTime = currentTime;
      currentTime = getTime();

      counter += (currentTime - previousTime);

      getCommand()->buttons = 0;

      if (counter > 2) {
        getCommand()->buttons = 1;
        getCommand()->forwardMove = 0;
        targetingSystem->clearTarget();
        setRespawned(true);
        LOG << "Health is " << getHealth() << " armor is " << getArmor() << ", trying to respawn!";
        counter = 0;
      }
    }

    Message s;
    cmds[frame].msec = ms;
    frame = (frame + 1) % UPDATE_BACKUP;
    createCommand(&s);
    outputQueue.push(s);
  }
}

void Bot::requestStringCommand(string value) {
  requestStringCommand(value, 1);
}

void Bot::requestStringCommand(string value, double delay) {
  Message sendMsg;
  sendMsg.delay = delay;
  sendMsg.writeByte(clc_stringcmd);
  sendMsg.writeString(value.c_str());
  sendMsg.writeByte(0);
  outputQueue.push(sendMsg);
}

void Bot::parseStatic(Message *msg) {
  short bits = msg->readShort();

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
    msg->readFloat();
  }

  if (bits & U_ANGLE1) {
    msg->readFloat();
  }

  if (bits & U_ORIGIN2) {
    msg->readFloat();
  }

  if (bits & U_ANGLE2) {
    msg->readFloat();
  }

  if (bits & U_ORIGIN3) {
    msg->readFloat();
  }

  if (bits & U_ANGLE3) {
    msg->readFloat();
  }
}

void Bot::nullCommand(Command *cmd) {
  cmd->angles[0] = 0;
  cmd->angles[1] = 0;
  cmd->angles[2] = 0;
  cmd->buttons = 0;
  cmd->forwardMove = 0;
  cmd->impulse = 0;
  cmd->msec = 0;
  cmd->sideMove = 0;
  cmd->upMove = 0;
}

