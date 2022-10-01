/* 
 * File:   Bot.hpp
 * Author: dervish
 *
 * Created on February 21, 2021, 8:35 AM
 */

#ifndef BOT_HPP
#define BOT_HPP
#include <Protocol.hpp>
#include <Common.hpp>
#include <Connection.hpp>
#include <Utility.hpp>
#include <Genome.hpp>
#include <NeuralNet.hpp>
#include <Entity.hpp>
#include <TsQueue.hpp>
#include <BotMemory.hpp>
#include <TargetingSystem.hpp>
#include <Logger.hpp>
#include <AttackGoal.hpp>
#include <PatrolGoal.hpp>
#include <Goal.hpp>
#include <SeekGoal.hpp>
#include <MessageTypes.hpp>
#include <Config.hpp>

#define MAX_GENOMES 127
#define MAX_TIMEOUT_IN_SECONDS 5

enum HandShakeState {
  None, New, Info, Prespawn, Spawn, Begin, JoinTeam, SelectClass, DisableChat, Waiting, Connected, Done
};

struct BotConfig {
  int id;
  std::string team;
  std::string skin; // class
  std::string name; 
  std::string bottomColor;
  std::map<std::string, std::vector<glm::vec3>> waypoints;
  std::string defend;
  int numRespawns;
};

class Bot {
public:
  Bot(char **argv);
  virtual ~Bot();

  void mainLoop();

  PlayerInfo* getPlayerBySlot(size_t id);
  PlayerInfo* getMe();

private:
  HandShakeState currentState;
  HandShakeState previousState;

  PlayerInfo players[MAX_CLIENTS];
  char **argv;
  glm::vec3 angle;

  int challenge;
  int frame;

  long spawnCount;

  bool running;

  std::string mapName;
  std::string gameDir;

  Command nullcmd;
  Command cmds[UPDATE_BACKUP];

  Connection connection;

  TsQueue<Message> outputQueue;
  TsQueue<Message> inputQueue;

  std::map<std::string, std::vector<glm::vec3>> waypoints;

  std::thread thinker;

  std::unique_ptr<BotMemory> botMemory;
  std::unique_ptr<TargetingSystem> targetingSystem;
  std::unique_ptr<Config> config;
  std::unique_ptr<Config> impulseConfig;
  std::unique_ptr<Config> mapConfig;

  std::map<std::string, std::string> mapChecksums;
  std::vector<std::unique_ptr<Goal>> goals;

  int delay;

  double timeChallengeSent;

  int stats[MAX_CL_STATS];


  std::mutex infoLock;
  std::mutex statLock;

  std::map<byte, std::unique_ptr<ServerMessage>> serverMessages;
  unsigned protoVer;
  int mySlot;
  bool gotChallenge;

  int validSequence;
  Goal *goal;
  int ahead;

  std::map<std::string, double> timers;
  BotConfig botConfig;

  double currentTime;
  double previousTime;
  int primeCounter;

  void initConfiguration();
  void initWaypoints(const std::string & section);

public:
  void nullCommand(Command *cmd);

  std::string getMapCheckSum(std::string key) {
    return mapChecksums[key];
  }

  int getStat(int stat) {
    int value = 0;

    if (stat >= MAX_CL_STATS) {
      return 0;
    }

    statLock.lock();
    value = stats[stat];
    statLock.unlock();
    return value;
  }

  void setStat(int stat, int value) {
    statLock.lock();
    stats[stat] = value;
    statLock.unlock();
  }

  int getHealth() {
    return getStat(STAT_HEALTH);
  }

  int getArmor() {
    return getStat(STAT_ARMOR);
  }

  int getActiveWeapon() {
    return getStat(STAT_ACTIVEWEAPON);
  }

  BotMemory* getBotMemory() {
    return botMemory.get();
  }

  BotConfig getBotConfig() {
    return botConfig;
  }

  

  double getTime();
  void getChallenge();
  void parseServerMessage(Message *message);
  void sendIp(const std::string &realIp);
  void sendExtensions();
  void sendNew();
  void updateState();
  void setInfo();
  void requestPrespawn(std::string prespawn);
  void requestSpawn();
  void requestSpawn2();
  void sendImpulse(byte impulse, long delay);
  void sendDisableChat();
  void createCommand(Message *s);
  void think();
  void requestStringCommand(std::string value);
  void requestStringCommand(std::string value, double delay);
  void parseStatic(Message *msg, bool extended);
  void parseBaseline(Message *msg);
  void parseBeam(Message *msg, int type);
  void parseProjectiles(Message *msg, bool indexed);
  void parsePacketEntities(Message *msg, bool delta);
  void parseDelta(Message *msg, byte bits);
  void parseBaseline2(Message *msg);

  void moveForward(short speed);
  void clickButton(int button);
  void moveSide(short speed);
  void moveUp(short speed);
  void rotateY(int angle);
  void rotateX(int angle);
  void impulse(int impulse);

  int getAngleY();
  int getAngleX();

  TargetingSystem* getTargetingSystem() {
    return targetingSystem.get();
  }

  Command* getCommand() {
    Command *cmd = &cmds[frame];
    return cmd;
  }

  Command * getCommands() {
    return &cmds[0];
  }

  std::map<std::string, std::vector<glm::vec3>> getWaypoints() const {
    return waypoints;
  }

  void setSpawnCount(long v) {
    this->spawnCount = v;
  }

  void setGameDir(const std::string &v) {
    this->gameDir = v;
  }

  std::string getGameDir() {
    return this->gameDir;
  }

  void setMapName(const std::string &v) {
    this->mapName = v;
  }

  void setProtoVer(unsigned v) {
    this->protoVer = v;
  }

  unsigned getProtoVer() {
    return this->protoVer;
  }

  void setState(HandShakeState state) {
    this->currentState = state;
  }

  HandShakeState getState() {
    return currentState;
  }

  void setMySlot(int mySlot) {
    if (this->mySlot == -1) {
      this->mySlot = mySlot;
      getMe()->slot = mySlot;
    } else {
      LOG << "Warning: something is trying to set myslot to " << mySlot << " current bot slot is " << this->mySlot;
    }
  }

  int getFrame() {
    return frame;
  }
};

#endif /* BOT_HPP */

