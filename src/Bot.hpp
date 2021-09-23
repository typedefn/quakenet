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

#define MAX_GENOMES 127
enum HandShakeState {
	None,
	Info,
	Prespawn,
	Spawn,
	Begin,
	JoinTeam,
	SelectClass,
	DisableChat,
	Waiting,
	Connected,
	Done,
};
class Bot {
public:
	Bot();
	virtual ~Bot();

	void mainLoop();

private:
	HandShakeState currentState;

	PlayerInfo *me;
	PlayerInfo players[MAX_CLIENTS];

	float blood;
	float armor;
	int challenge;
	int frame;
	int targetSlot;
	bool targetSelected;
	float elapsedTime;
	float totalTime;
	int newCount;
	bool modelDone;

	Command cmd;
	Command nullcmd;
	Command cmds[UPDATE_BACKUP];

	Connection connection;

	TsQueue<Message> outputQueue;
	TsQueue<Message> inputQueue;

	vector<Genome> genomes;
	vector<glm::vec3> waypoints;

	int generation;
	float crossoverRate;
	float mutationRate;
//  Genome * workingGenome;
	int genomeIndex;

	NeuralNet brain;

	vector<vector<double>> memory;
	std::thread thinker;
	Message lastMessage;
 int delay;
 double duration;
 void nullCommand(Command * cmd);
public:

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
	void createCommand(Message * s);
	void think();
	void createStartPopulation();
	void epoch();
	Genome wheelSelection();
	Genome randomSelection();
	//  void crossover(const vector<Gene> & mom, const vector<Gene> &dad, vector<Gene> &baby1, vector<Gene> &baby2);
	void crossoverAtSplits(const vector<double> &mom, const vector<double> &dad,
			vector<double> &baby1, vector<double> &baby2);
	void mutate(vector<double> &genes);

	void patrol();
	bool isTargetClose();
	void attackTarget();
	void requestStringCommand(string value);
	void requestStringCommand(string value, double delay);
	void parseStatic(Message * msg);
	void sendJunk(Message * s);
	void requestMoveCommand();
};

#endif /* BOT_HPP */

