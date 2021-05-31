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

#define MAX_GENOMES 127

struct PlayerInfo {
	long userId;
	char name[32];
	int frags;
	int ping;
	byte pl;
	float entertime;
	int slot;
	float coords[3];
	int frame;
	short flags;
};

class Bot {
public:
	Bot();
	virtual ~Bot();

	void mainLoop();

private:

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

	Command cmd;
	Command nullcmd;
	Command cmds[UPDATE_BACKUP];

	Connection connection;

	queue<Message> outputQueue;
	queue<Message> inputQueue;

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

	double getTime();
	void getChallenge();
	void parseServerMessage(Message *message);
	void sendIp(const string &realIp);
	void sendExtensions();
	void sendNew();
	void join();
	void setInfo();
	void requestPrespawn();
	void requestSpawn();
	void sendBegin();
	void sendDisableChat();
	void createCommand();
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
};

#endif /* BOT_HPP */

