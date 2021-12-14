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

  PlayerInfo* getPlayerById(int id);
  PlayerInfo* getMe();

private:
  HandShakeState currentState;
  HandShakeState previousState;

  PlayerInfo *me;
  PlayerInfo players[MAX_CLIENTS];
  char **argv;

  float blood;
  float armor;
  int challenge;
  int frame;
  int targetSlot;
  float elapsedTime;
  float totalTime;
  int newCount;
  bool ipRecv;
  string spawnCmd;
  Command cmd;
  Command nullcmd;
  Command cmds[UPDATE_BACKUP];

  Connection connection;

  TsQueue<Message> outputQueue;
  TsQueue<Message> inputQueue;

  vector<glm::vec3> waypoints;

  vector<vector<double>> memory;
  std::thread thinker;
  Message lastMessage;
  unique_ptr<BotMemory> botMemory;
  unique_ptr<TargetingSystem> targetingSystem;

  double respawnTimer;

  vector<unique_ptr<Goal>> goals;
  int delay;
  double duration;
  void nullCommand(Command *cmd);
public:

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
    return &cmds[frame];
  }

  vector<glm::vec3> getWaypoints() const {
    return waypoints;
  }
  int getBlood() {
    return blood;
  }
};

#endif /* BOT_HPP */

