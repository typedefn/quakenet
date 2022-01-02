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

#define MAX_GENOMES 127

enum HandShakeState {
  None, New, Info, Prespawn, Spawn, Begin, JoinTeam, SelectClass, DisableChat, Waiting, Connected, Done
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

  int challenge;
  int frame;

  long spawnCount;

  string mapName;
  string gameDir;

  Command nullcmd;
  Command cmds[UPDATE_BACKUP];

  Connection connection;

  TsQueue<Message> outputQueue;
  TsQueue<Message> inputQueue;

  map<string, vector<glm::vec3>> waypoints;

  std::thread thinker;

  unique_ptr<BotMemory> botMemory;
  unique_ptr<TargetingSystem> targetingSystem;

  map<string, string> mapChecksums;
  vector<unique_ptr<Goal>> goals;

  int delay;

  double timeChallengeSent;

  int stats[MAX_CL_STATS];


  mutex infoLock;
  mutex statLock;

  map<byte, unique_ptr<ServerMessage>> serverMessages;
  unsigned protoVer;
  int mySlot;
  bool gotChallenge;

  int validSequence;
  Goal *goal;
  int ahead;

  map<string, double> timers;

  double currentTime;
  double previousTime;
  int primeCounter;
public:
  void nullCommand(Command *cmd);

  string getMapCheckSum(string key) {
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

  double getTime();
  void getChallenge();
  void parseServerMessage(Message *message);
  void sendIp(const string &realIp);
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
  void requestStringCommand(string value);
  void requestStringCommand(string value, double delay);
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

  map<string, vector<glm::vec3>> getWaypoints() const {
    return waypoints;
  }

  void setSpawnCount(long v) {
    this->spawnCount = v;
  }

  void setGameDir(const string &v) {
    this->gameDir = v;
  }

  string getGameDir() {
    return this->gameDir;
  }

  void setMapName(const string &v) {
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

