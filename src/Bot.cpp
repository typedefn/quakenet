/* 
 * File:   Bot.cpp
 * Author: dervish
 * 
 * Created on February 21, 2021, 8:35 AM
 */

#include "Bot.hpp"
#include "Gene.hpp"

Bot::Bot() {
	challenge = 0;
	blood = 0;
	armor = 0;
	frame = 0;
	targetSlot = 0;
	targetSelected = false;
	elapsedTime = 0;
	me = nullptr;
	crossoverRate = 0.7;
	mutationRate = 0.15;
	generation = 0;
	genomeIndex = 0;
	totalTime = 0;
	currentState = None;
	srand(time(nullptr));
	newCount = 0;
	delay = 0;
	duration = 0;
	modelDone = false;
	ipRecv = false;
	beginSent = false;
	this->botMemory = make_unique<BotMemory>(this, 4.0);

	for(int i = 0; i < MAX_CLIENTS; i++) {
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
		players[i].userId = 0;
	}
}

Bot::~Bot() {
}

void Bot::mainLoop() {
	connection.connect();
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

	char junk;

	while (!fs.eof()) {
		stringstream ss;
		string type;
		char line[256] = { 0 };
		fs.getline(line, 256);
		ss << line;
		if (line[0] == 'w' && line[1] == 'p') {
			glm::vec3 waypoint;
			ss >> junk >> junk >> waypoint.x >> waypoint.y >> waypoint.z;
			waypoints.push_back(waypoint);
		}
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
				std::cout << " sent delayed msg of size " << s << endl;
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

		timePassed = getTime();
	}
}

PlayerInfo * Bot::getPlayerById(int id) {
	return &players[id];
}

PlayerInfo * Bot::getMe() {
	return me;
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
				challenge = atoi(msg.readString());

				for (;;) {
					c = msg.readLong();
					cout << "Proto version " << c << endl;
					if (msg.isBadRead()) {
						break;
					}

					// Read proto version info...
					msg.readLong();
				}
				strcpy(userInfo,
						"\\rate\\25000\\name\\krupt_drv\\msg\\1\\noaim\\1\\*client\\dumbo 1234\\*z_ext\\511");
				snprintf(data, sizeof(data),
						"\xff\xff\xff\xff" "connect %i %i %i \"%s\"\n",
						PROTOCOL_VERSION, connection.getQport(), challenge,
						userInfo);
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

	msg.clear();
	msg.writeByte(3);
	msg.writeByte(18);
	msg.writeByte(0);
	msg.writeByte(0);
	msg.writeByte(0);
	msg.writeByte(0);
	msg.writeByte('!');
	msg.writeByte(0);
	msg.writeByte(14);
	outputQueue.push(msg);

}

void Bot::parseServerMessage(Message *message) {
	int msgSvcStart = 0;
	int cmd = 0;
	int spawnCount = 0;
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

		cout << "cmd = " << cmd << endl;

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
			if (modelDone) {
				return;
			}

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
					stringstream ss;
//					ss << "modellist 1 " << n;
//					requestStringCommand(ss.str());
					cout << "MODEL LIST = " << n << "out of " << numModels
							<< endl;
					return;
				}

			}

			break;
		}
		case svc_download: {
			int size, percent;
			int s = 0;
			size = message->readShort();
			cout << "SIZE = " << size << endl;
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
			cout << "svc_updatestat: " << i << " " << j << endl;

			break;
		}
		case svc_soundlist: {
			if (modelDone) {
				return;
			}

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
						cout << "Error send to many sound_precache" << endl;
					}
					str = message->readString();
				} while (*str);
			}

			// Now request model list.
			if (!modelDone) {
//				stringstream ss;
//				requestStringCommand("modellist 1 0");
			}
			break;
		}
		case svc_intermission: {
			break;
		}
		case svc_muzzleflash: {
			int i = message->readShort();
			cout << "svc_muzzleflash: " << i << endl;
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
				std::cout << "setinfo > MAX_CLIENTS" << endl;
				break;
			}

			string key(message->readString());
			string value(message->readString());

			if (key == "team" && value == "blue") {
				currentState = SelectClass;
				cout << "JOINED TEAM" << endl;
			} else if (key == "skin" && value == "tf_sold") {
				currentState = Done;
				cout << "SELECTED CLASS!" << endl;
			}

			cout << "svc_setinfo(" << slot << "):" << key << "\\" << value
					<< endl;

			break;
		}
		case svc_updatefrags: {
			message->readByte();
			message->readShort();
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
				cout << "proto version " << protover << endl;
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
			message->readLong();

			// Gamedir
			message->readString();

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
				float maxspeed = message->readFloat();
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

//			cout << "svc_stufftext: [" << line << "]" << endl;
			if (tokens.size() > 1 && tokens[0] == "packet"
					&& tokens[2] == "\"ip") {
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
					cout << "SPAWN CMD = " << spawnCmd << endl;
					if (beginSent && me == nullptr) {
						break;
					}
				}

				requestStringCommand(ss.str());
			} else if (tokens.size() > 0 && tokens[0] == "fullserverinfo") {
				currentState = Info;
				ipRecv = true;
			} else if (tokens.size() == 1 && tokens[0] == "skins\n") {
				modelDone = true;
			} else if (tokens.size() == 2 && tokens[0] == "exec"
					&& tokens[1] == "1on1r.cfg\n") {
			}
			break;
		}
		case svc_updateuserinfo: {
			int slot = message->readByte();
			if (slot >= MAX_CLIENTS) {
				cout << "slot(" << slot << ") is greater than " << MAX_CLIENTS
						<< endl;
				break;
			}

			long userId = message->readLong();
			string value(message->readString());
			string name = Utility::findValue("name", value);
			cout << "name is " << name << endl;
			// TODO: fix hardcoded name of bot later.
			if (name == "krupt_drv") {
				cout << "ME AM IN SLOT " << slot << " id = " << userId << endl;
				me = &players[slot];
				me->userId = userId;
				me->slot = slot;
				if (currentState == None && beginSent) {
					currentState = JoinTeam;
				}
			}
			cout << "svc_updateuserinfo:(" << slot << "): " << value << endl;

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

			if (me != nullptr && num != me->slot) {
				targetSlot = num;
				cout << "TARGET = " << num << endl;
			}

			short flags = message->readShort();
			players[num].flags = flags;
			players[num].active = true;

			for (int i = 0; i < 3; i++) {
				float a = message->readFloat();
				players[num].coords[i] = a;
				cout << "client = " << num << "  i = " << i << " coord = " << a
						<< endl;
			}

			byte frame = message->readByte();

			if (flags & PF_MSEC) {
				byte msec = message->readByte();
			}

			if (flags & PF_COMMAND) {
				int bits = message->readByte();

				// Need to be vary of version for protoVersion
				// if (client.protoVersion <= 26) ...
				if (bits & CM_ANGLE1) {
					cout << "CM_ANGLE1(" << num << ")" << "  = "
							<< message->readFloat() << endl;
				}
				if (bits & CM_ANGLE3) {
					cout << "CM_ANGLE3(" << num << ")" << "  = "
							<< message->readFloat() << endl;
				}

				if (bits & CM_FORWARD) {
					cout << "CM_FORWARD(" << num << ")" << " = "
							<< (message->readByte() << 3) << endl;
				}
				if (bits & CM_SIDE) {
					cout << "CM_SIDE(" << num << ")" << "   = "
							<< (message->readByte() << 3) << endl;
				}
				if (bits & CM_UP) {
					cout << "CM_UP(" << num << ")" << "      = "
							<< (message->readByte() << 3) << endl;
				}

				if (bits & CM_BUTTONS) {
					cout << "CM_BUTTONS(" << num << ")" << " = "
							<< message->readByte() << endl;
				}

				if (bits & CM_IMPULSE) {
					cout << "CM_IMPULSE(" << num << ")" << " = "
							<< message->readByte() << endl;
				}

				cout << "MSEC(" << num << ")" << "       = "
						<< message->readByte() << endl;
			}

			for (int i = 0; i < 3; i++) {
				if (flags & (PF_VELOCITY1 << i)) {
					cout << "PF_VELOCITY" << i << "(" << num << ")" << "     = "
							<< message->readShort() << endl;
				}
			}

			if (flags & PF_MODEL) {
				cout << "PF_MODEL(" << num << ")" << "         =  "
						<< message->readByte() << endl;
			}

			if (flags & PF_SKINNUM) {
				cout << "PF_SKINNUM(" << num << ")" << "       = "
						<< message->readByte() << endl;
			}

			if (flags & PF_EFFECTS) {
				cout << "PF_EFFECTS(" << num << ")" << "       = "
						<< message->readByte() << endl;
			}

			if (flags & PF_WEAPONFRAME) {
				cout << "PF_WEAPONFRAME(" << num << ")" << " = "
						<< message->readByte() << endl;
			}

			cout << "---end---" << endl;

			break;
		}
		case svc_lightstyle: {
			message->readByte();
			message->readString();
			break;
		}
		case svc_updatestatlong: {
			message->readByte();
			message->readLong();
			break;
		}
		case svc_setangle: {
			float x = message->readFloat();
			float y = message->readFloat();
			float z = message->readFloat();

			cout << "svc_setangle x = " << x << " y = " << y << " z = " << z
					<< endl;
			break;
		}
		case svc_damage: {
			armor = message->readByte();
			blood = message->readByte();
			float coords[3];
			float dotP = 0;

			for (int i = 0; i < 3; i++) {
				coords[i] = message->readCoord();
				dotP += coords[i] * coords[i];
			}

			// For some reason when dotP is equal to 0, that means that the bot got hit by rocket, armor
			// is the amount taken off of armor and blood is the amount taken out of health.
			if (dotP <= 0) {

				cout << "svc_damage: ARMOR " << armor << " BLOOD = " << blood
						<< " " << coords[0] << " " << coords[1] << " "
						<< coords[2] << endl;

			}
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
//			cout << "in default" << std::endl;
			break;
		}
		}
	}
}

void Bot::sendIp(const string &realIp) {
	char data[MAXLINE];
	snprintf(data, sizeof(data), "\xff\xff\xff\xff" "ip 0 %s\n",
			realIp.c_str());

	Message s;
	s.writeString(data);
	connection.sendConnectionless(s);
}

void Bot::sendExtensions() {
	cout << "Sending extension" << endl;
	Message s;
	s.writeChar(4);
	s.writeString("pext 0x58455446 0x2140f000 0x32455446 0x2 0x3144564d 0x1");
	s.writeChar(0);
	s.writeChar(3);
	s.writeChar('Z');
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

	std::cout << "CURRENT STATE " << currentState << endl;
	switch (currentState) {
	case Info:
		requestStringCommand("unmuteall");
		requestStringCommand("setinfo \"chat\" \"1\"");
		setInfo();
		currentState = None;
		break;
	case Prespawn:
		currentState = None;
		break;
	case Spawn:
		break;
	case Begin:
		if (!beginSent) {
			requestStringCommand(spawnCmd, 2);
			requestStringCommand("begin 1", 3);
		}
		beginSent = true;
		currentState = None;
		break;
	case JoinTeam: {
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
		if (!ipRecv) {
			requestStringCommand("new", 0);
		}
		break;
	}
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
	s.writeString("prespawn 1 0 -756178370");
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

	maxBuffer[crcIndex] = Utility::crcByte(maxBuffer + crcIndex + 1,
			size - crcIndex - 1, connection.getOutgoingSequence());

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
	cout << "Thinking!" << endl;

	for (int i = frame; i < UPDATE_BACKUP; i++) {
		nullCommand(&cmds[i]);
	}

	while (true) {
		usleep(10000);
		extramsec += 0.5 * 1000;
		int ms = extramsec;

		extramsec -= ms;

		if (ms > 250) {
			ms = 100;
		}

		nullCommand(&cmds[frame]);

		if (me != nullptr) {

			extramsec += 0.01;

			ActionType at = (ActionType) (rand() % 5);

			if (isTargetClose() && targetSlot != me->slot) {
				attackTarget();
			} else {
				patrol();
			}

			botMemory->updateVision();
		}

		Message s;
		cmds[frame].msec = ms;
		frame = (frame + 1) % UPDATE_BACKUP;
		createCommand(&s);
		outputQueue.push(s);
	}
}

bool Bot::isTargetClose() {
	const float maxDistance = 420.0;

	glm::vec3 targetPosition(players[targetSlot].coords[0],
			players[targetSlot].coords[2], players[targetSlot].coords[1]);
	glm::vec3 position(me->coords[0], me->coords[2], me->coords[1]);
	glm::vec3 facing = glm::normalize(
			glm::cross(glm::normalize(position), glm::vec3(0, 1, 0)));

	glm::vec3 directionToTarget = glm::normalize(targetPosition - position);

//	std::cout << "Target position = " << targetPosition.x << " "
//			<< targetPosition.y << " " << targetPosition.z << endl;

	float dist = glm::distance(targetPosition, position);
	float deltaAngle = glm::dot(directionToTarget, facing);

//	cout << getTime() << " STATE : DIST = " << dist << endl;

	if (dist <= maxDistance && deltaAngle >= 0.10) {
		return true;
	}

	return false;
}

void Bot::patrol() {
	static int wi = 0;
	const float maxDistance = 100.0;
	glm::vec3 position(me->coords[0], me->coords[2], me->coords[1]);
	glm::vec3 targetPosition(waypoints.at(wi).x, me->coords[2],
			waypoints.at(wi).z);

	float dist = glm::distance(targetPosition, position);

	if (dist <= maxDistance) {
		wi = (wi + 1) % waypoints.size();
	}

	glm::vec3 dir = targetPosition - position;

	cmds[frame].angles[1] = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));
	cmds[frame].forwardMove = 500;

}

void Bot::attackTarget() {
	glm::vec3 targetPosition(players[targetSlot].coords[0],
			players[targetSlot].coords[2], players[targetSlot].coords[1]);

	glm::vec3 position(me->coords[0], me->coords[2], me->coords[1]);

	glm::vec3 dir = targetPosition - position;

	cmds[frame].angles[1] = 90 + (atan2(-dir.x, dir.z) * (180.0 / PI));
	cmds[frame].forwardMove = 0;
	cmds[frame].buttons = 1;
}

void Bot::epoch() {
	Genome mom = wheelSelection();
	Genome dad = wheelSelection();

	Genome baby1, baby2;
	crossoverAtSplits(mom.getGenes(), dad.getGenes(), baby1.getGenes(),
			baby2.getGenes());
	mutate(baby1.getGenes());
	genomes.push_back(baby1);
	brain.putWeights(baby1.getGenes());

//    Genome * weakest = &genomes.back();
//    weakest->setGenes(baby1.getGenes());
//

	++generation;
}

Genome Bot::randomSelection() {
	int index = rand() % genomes.size();
	Genome selected = genomes.at(index);

	cout << "ANN selected random genome with fitness of "
			<< selected.getFitness() << endl;

	return selected;

}

Genome Bot::wheelSelection() {
	double slice = Utility::randomFloat(); // / 1000.0f;
	double total = 0;
	int selectedGenome = 0;

	for (int i = 0; i < genomes.size(); ++i) {
		total += genomes.at(i).getFitness();
		if (total > slice) {
			selectedGenome = i;
			break;
		}
	}

	Genome selected = genomes.at(selectedGenome);

	cout << "ANN selected genome with fitness of " << selected.getFitness()
			<< " slice = " << slice << endl;

	return selected;
}
//
//void Bot::crossover(const vector<double>& mom, const vector<double>& dad, vector<double>& baby1, vector<double>& baby2) {
//    if (Utility::randomFloat() > crossoverRate || mom == dad) {
//        baby1 = mom;
//        baby2 = dad;
//        return;
//    }
//
//    int cp = Utility::randomRanged(0, mom.size() - 1);
//
//    for (int i = 0; i < cp; ++i) {
//        baby1.push_back(mom.at(i));
//        baby2.push_back(dad.at(i));
//    }
//
//    for (int i = cp; i < mom.size(); ++i) {
//        baby1.push_back(dad.at(i));
//        baby2.push_back(mom.at(i));
//    }
//}

void Bot::crossoverAtSplits(const vector<double> &mom,
		const vector<double> &dad, vector<double> &baby1,
		vector<double> &baby2) {

	if (Utility::randomFloat() > crossoverRate || mom == dad) {
		baby1 = mom;
		baby2 = dad;

		return;
	}

	vector<int> splits = brain.calcSplitPoints();

	int index1 = Utility::randomRanged(0, splits.size() - 2);
	int index2 = Utility::randomRanged(index1, splits.size() - 1);

	int cp1 = splits.at(index1);
	int cp2 = splits.at(index2);

	for (int i = 0; i < mom.size(); ++i) {
		if (i < cp1 || i >= cp2) {
			baby1.push_back(mom.at(i));
			baby2.push_back(dad.at(i));
		} else {
			baby1.push_back(dad.at(i));
			baby2.push_back(mom.at(i));
		}
	}

}

void Bot::mutate(vector<double> &genes) {
	for (int gene = 0; gene < genes.size(); ++gene) {
		if (Utility::randomFloat() < mutationRate) {
			double z = genes[gene];
			z += Utility::randomFloat();
			z = glm::clamp(z, 0.0, 1.0);
			genes[gene] = z;
		}
	}

}

void Bot::createStartPopulation() {
	for (int i = 0; i < 2; ++i) {
		genomes.push_back(Genome(brain.getNumberOfWeights()));
	}

	Genome genome = genomes.at(genomeIndex);
	brain.putWeights(genome.getGenes());

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

void Bot::requestMoveCommand() {
	Message sendMsg;
	sendMsg.delay = 5;
	sendJunk(&sendMsg);
	sendMsg.writeByte(0);
	sendMsg.writeByte(13);
	sendMsg.writeByte(0);
	sendMsg.writeByte(13);
	sendMsg.writeByte(0);
	sendMsg.writeByte(13);
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

void Bot::sendJunk(Message *s) {
	Command *oldcmd, *cmd;

	s->writeByte(clc_move);
	int crcIndex = s->getSize();
	s->writeByte(0);
	s->writeByte(0);

	int i = (connection.getOutgoingSequence() - 2) & UPDATE_MASK;
	cmd = &cmds[i];

//	s->writeDeltaUserCommand(&nullcmd, cmd);
	oldcmd = cmd;
	size_t size = s->getSize();
	byte maxBuffer[size] = { 0 };
	copy(s->getData().begin(), s->getData().end(), maxBuffer);

	s->clear();

	maxBuffer[crcIndex] = Utility::crcByte(maxBuffer + crcIndex + 1,
			size - crcIndex - 1, connection.getOutgoingSequence());

	for (int i = 0; i < size; i++) {
		s->pushData(maxBuffer[i]);
	}

	s->setCurrentSize(size);
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

