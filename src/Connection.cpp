/* 
 * File:   Connection.cpp
 * Author: dervish
 * 
 * Created on February 21, 2021, 8:37 AM
 */

#include "Connection.hpp"

Connection::Connection() {
	handshake = false;
	outgoingSequence = 0;
	sendReliable = false;
	incomingAck = 0;
	incomingReliableAck = 0;
	incomingReliableSequence = 0;
	incomingSequence = 0;
	qport = 0;
	sockfd = -1;
	deltaSequence = 0;
}

Connection::Connection(const Connection &orig) {
}

Connection::~Connection() {
}

void Connection::connect() {
	// Creating socket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));

	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = INADDR_ANY; //inet_addr(argv[1]); ////INADDR_ANY;

	qport = 48984;
	srand((unsigned) time(nullptr));
	qport = (rand() % 10000) + 48984;

	struct sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(struct sockaddr_in));
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(qport);
	clientAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr*) &clientAddr, sizeof(clientAddr)) < 0) {
		LOG << "failed to bind to port " << qport;
	}

	socklen_t len;
	fcntl(sockfd, F_SETFL, O_NONBLOCK);
}

int Connection::sendConnectionless(Message msg) {
	return sendInner(&msg);
}

int Connection::send(Message msg) {
	Message zend;

	writeHeader(&zend);
	msg.copyMessage(&zend);
	return sendInner(&zend);
}

int Connection::sendInner(Message *msg) {

//	cout << "Sending[";
	for (int i = 0; i < msg->data.size(); i++) {
		int c = msg->data.data()[i];
		if (c < 32 || c > 127) {
//			printf("[%i]", c);
		} else {
//			printf("%c", c);
		}
	}

//	cout << "] length[" << msg->data.size() << "]" << endl;

	return sendto(sockfd, (const char*) &msg->data[0], msg->data.size(),
	MSG_CONFIRM, (const struct sockaddr*) &servaddr, sizeof(servaddr));
}

bool Connection::recv(Message *msg, bool block) {
	FD_ZERO(&readfds);
	socklen_t len;
	FD_SET(sockfd, &readfds);
	char buffer[MAXLINE];
	int n;
	int rv = 0;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	if (!block) {
		rv = select(sockfd + 1, &readfds, nullptr, nullptr, &tv);
	} else {
		rv = 1;
	}

	if (rv == 1) {
		n = recvfrom(sockfd, (char*) buffer, MAXLINE, MSG_WAITALL,
				(struct sockaddr*) &servaddr, &len);
		msg->setCurrentSize(n);
		msg->setMaxSize(MAXLINE);

		buffer[n] = '\0';

		if (n >= MAXLINE) {
			cout << "buffer is huge truncating " << n << endl;
			n = MAXLINE - 1;
		} else if (n < 0) {
			return false;
		}
		stringstream ss;
//		cout << "Recv: ";

		for (int i = 0; i < n; i++) {
			int c = buffer[i];
			if (c >= 32 && c <= 126) {
//				cout << (char) c;
			} else {
//				cout << "[" << (short) c << "]";
			}

			msg->writeByte(c);
		}
//		cout << endl;

		return true;
	}

	return false;
}

bool Connection::process(Message *msg) {
	unsigned sequence, sequenceAck;
	unsigned reliableAck, reliableMessage;

	sequence = msg->readLong();
	sequenceAck = msg->readLong();

	reliableMessage = sequence >> 31;
	reliableAck = sequenceAck >> 31;

	sequence &= ~(1 << 31);
	sequenceAck &= ~(1 << 31);

	incomingSequence = sequence;
	incomingAck = sequenceAck;
	incomingReliableAck = reliableAck;

	if (reliableMessage) {
		incomingReliableSequence ^= 1;
	}

	return true;
}

Message::Message() {
	connectionless = false;
	delay = 0;
	clear();
}

Message::~Message() {

}

void Message::beginRead() {
	msgReadCount = 0;
	msgBadRead = false;
}

unsigned Message::readLong() {
	unsigned c;

	if (msgReadCount + 4 > curSize) {
		msgBadRead = true;
		return -1;
	}

	c = data[msgReadCount] + (data[msgReadCount + 1] << 8)
			+ (data[msgReadCount + 2] << 16) + (data[msgReadCount + 3] << 24);
	msgReadCount += 4;
	return c;
}

int Message::readByte() {
	int c;

	if (msgReadCount + 1 > curSize) {
		msgBadRead = true;
		return -1;
	}
	c = (unsigned char) data[msgReadCount];
	msgReadCount++;
	return c;
}

int Message::readShort() {
	int c;

	if (msgReadCount + 2 > curSize) {
		msgBadRead = true;
		return -1;
	}

	c = (short) (data[msgReadCount] + (data[msgReadCount + 1] << 8));

	msgReadCount += 2;

	return c;
}

float Message::readCoord() {
	return readShort() * (1.0 / 8);
}

void Message::clear() {
	curSize = 0;
	overflowed = false;
	msgBadRead = false;
	msgReadCount = 0;
	data.clear();
}

void Message::writeAngle16(float f) {
	writeShort(Q_rint(f * 65536.0 / 360.0) & 65535);
}

void Message::writeChar(int c) {
	data.push_back(c);
}

void Message::writeByte(int c) {
	data.push_back(c);
}

void Message::writeString(const char *s) {
	size_t len = strlen(s);
	for (size_t i = 0; i < len; i++) {
		byte b = s[i];
		data.push_back(b);
	}
}

void Message::writeLong(int c) {
	byte buf[4] = { 0 };

	buf[0] = c & 0xff;
	buf[1] = (c >> 8) & 0xff;
	buf[2] = (c >> 16) & 0xff;
	buf[3] = (c >> 24);

	data.push_back(buf[0]);
	data.push_back(buf[1]);
	data.push_back(buf[2]);
	data.push_back(buf[3]);
}

void Message::writeShort(int c) {
	byte buf[2] = { 0 };
	buf[0] = c & 0xff;
	buf[1] = c >> 8;
	data.push_back(buf[0]);
	data.push_back(buf[1]);
}

char* Message::readString() {
	static char string[2048];
	unsigned int l;
	int c;
	l = 0;

	do {
		c = readByte();
		if (c == 255) // skipp security problems
			continue;

		if (c == -1 || c == 0) // msg badread or end of string
			break;

		string[l] = c;
		l++;
	} while (l < sizeof(string) - 1);

	string[l] = 0;
	return string;

}

float Message::readFloat() {

	union {
		byte b[4];
		float f;
		int l;
	} dat;

	dat.b[0] = data[msgReadCount];
	dat.b[1] = data[msgReadCount + 1];
	dat.b[2] = data[msgReadCount + 2];
	dat.b[3] = data[msgReadCount + 3];

	msgReadCount += 4;

	return dat.f;
}

void Connection::writeHeader(Message *msg) {
	unsigned w1 = outgoingSequence | (sendReliable << 31);
	unsigned w2 = incomingSequence | (incomingReliableSequence << 31);

	outgoingSequence++;
	deltaSequence = w2 + 1;
	msg->writeLong(w1);
	msg->writeLong(w2);
	msg->writeShort(qport);
}

void Message::copyMessage(Message *copyTo) {
	for (int i = 0; i < data.size(); i++) {
		copyTo->data.push_back(data[i]);
	}
}

void Message::writeDeltaUserCommand(Command *from, Command *cmd) {
	int bits = 0;

	if (cmd->angles[0] != from->angles[0]) {
		bits |= CM_ANGLE1;
	}
	if (cmd->angles[1] != from->angles[1]) {
		bits |= CM_ANGLE2;
	}
	if (cmd->angles[2] != from->angles[2]) {
		bits |= CM_ANGLE3;
	}
	if (cmd->forwardMove != from->forwardMove) {
		bits |= CM_FORWARD;
	}
	if (cmd->sideMove != from->sideMove) {
		bits |= CM_SIDE;
	}
	if (cmd->upMove != from->upMove) {
		bits |= CM_UP;
	}
	if (cmd->buttons != from->buttons) {
		bits |= CM_BUTTONS;
	}
	if (cmd->impulse != from->impulse) {
		bits |= CM_IMPULSE;
	}

	writeByte(bits);

	if (bits & CM_ANGLE1) {
		writeAngle16(cmd->angles[0]);
	}
	if (bits & CM_ANGLE2) {
		writeAngle16(cmd->angles[1]);
	}
	if (bits & CM_ANGLE3) {
		writeAngle16(cmd->angles[2]);
	}

	if (bits & CM_FORWARD) {
		writeShort(cmd->forwardMove);
	}

	if (bits & CM_SIDE) {
		writeShort(cmd->sideMove);
	}

	if (bits & CM_UP) {
		writeShort(cmd->upMove);
	}

	if (bits & CM_BUTTONS) {
		writeByte(cmd->buttons);
	}

	if (bits & CM_IMPULSE) {
		writeByte(cmd->impulse);
	}

	//msec
	writeByte(cmd->msec);
}
