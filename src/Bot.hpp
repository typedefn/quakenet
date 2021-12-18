/* 
 * File:   Bot.hpp
 * Author: dervish
 *
 * Created on February 21, 2021, 8:35 AM
 */

#ifndef BOT_HPP
#define BOT_HPP
#include "Protocol.hpp"
#include "Common.hpp"
#include "Connection.hpp"
#include "Utility.hpp"
#include "Genome.hpp"
#include "NeuralNet.hpp"
#include "Entity.hpp"
#include "TsQueue.hpp"
#include "BotMemory.hpp"
#include "TargetingSystem.hpp"
#include "Logger.hpp"
#include "AttackGoal.hpp"
#include "PatrolGoal.hpp"
#include "Goal.hpp"
#include "SeekGoal.hpp"

#define MAX_GENOMES 127

enum HandShakeState {
  None, Info, Prespawn, Spawn, Begin, JoinTeam, SelectClass, DisableChat, Waiting, Connected, Done
};
class Bot {
public:
  Bot(char **argv);
  virtual ~Bot();

  void mainLoop();

  PlayerInfo* getPlayerById(size_t id);
  PlayerInfo* getMe();

private:
  HandShakeState currentState;
  HandShakeState previousState;

  PlayerInfo *me;
  PlayerInfo players[MAX_CLIENTS];
  char **argv;

  float blood;
  float armor;
  unsigned long challenge;
  int frame;
  int targetSlot;
  float elapsedTime;
  float totalTime;
  int newCount;
  bool ipRecv;
  long spawnCount;
  string spawnCmd;
  string mapName;
  string gameDir;

  Command cmd;
  Command nullcmd;
  Command cmds[UPDATE_BACKUP];

  Connection connection;

  TsQueue<Message> outputQueue;
  TsQueue<Message> inputQueue;

  map<string, vector<glm::vec3>> waypoints;

  vector<vector<double>> memory;
  std::thread thinker;
  Message lastMessage;
  unique_ptr<BotMemory> botMemory;
  unique_ptr<TargetingSystem> targetingSystem;

  map<string, string> mapChecksums;

  double respawnTimer;

  vector<unique_ptr<Goal>> goals;
  int delay;
  double duration;
  double timeChallengeSent;
  bool requestChallenge;
  float frameTime;

  int   stats[MAX_CL_STATS];

  void nullCommand(Command *cmd);

  mutex infoLock;
  mutex statLock;
  bool respawned;
public:

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

  int getTargetId() {
    if (targetSlot > MAX_CLIENTS) {
      LOG << "getTargetId error " << targetSlot << " > " << MAX_CLIENTS;
      return 0;
    }
    return targetSlot;
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
  void sendBegin();
  void sendDisableChat();
  void createCommand(Message *s);
  void think();
  void requestStringCommand(string value);
  void requestStringCommand(string value, double delay);
  void parseStatic(Message *msg);

  TargetingSystem* getTargetingSystem() {
    return targetingSystem.get();
  }

  Command* getCommand() {
    Command * cmd = &cmds[frame];
    return cmd;
  }

  map<string, vector<glm::vec3>> getWaypoints() const {
    return waypoints;
  }
  bool getRespawned() {
    return respawned;
  }

  void setRespawned(bool value) {
    respawned = value;
  }
};

#endif /* BOT_HPP */

