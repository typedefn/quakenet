
/* 
 * File:   Connection.hpp
 * Author: dervish
 *
 * Created on February 21, 2021, 8:37 AM
 */

#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include <Common.hpp>
#include <Protocol.hpp>
#include <Utility.hpp>

//#define LOG_RECV_TRAFFIC 1
//#define LOG_SEND_TRAFFIC 1

class Message {
public:

  Message();
  virtual ~Message();
  
  void beginRead();
  void clear();

  unsigned readLong();
  int readByte();
  int readChar();
  int readShort();
  char *readString();
  float readFloat();
  float readFloatCoord();
  float readCoord();
  float readAngle();
  void readData(void * data, int len);
  float readAngle16();

  void writeAngle16(float f);
  void writeChar(int c);
  void writeByte(int c);
  void writeString(const char *s);
  void writeLong(int c);
  void writeShort(int c);

  void writeDeltaUserCommand(Command *from, Command *cmd);
  
  void copyMessage(Message *copyTo);
  
  void setCurrentSize(int sz) {
    this->curSize = sz;
  }
  
  int getCurrentSize() const {
    return curSize;
  }
  
  void setMaxSize(int max) {
    this->maxSize = max;
  }
  
  bool isBadRead() const {
    return msgBadRead;
  }
  
  int getReadCount() const {
    return msgReadCount;    
  }
  
  void incReadCount() {
    msgReadCount++;
  }
  
  size_t getSize() const {
    return data.size();
  }
  
  const std::vector<byte> & getData() const {
    return data;
  }
  
  void pushData(byte b) {
    data.push_back(b);
  }
 
  void setConnectionless(bool cl) {
    connectionless = cl;
  }
  
  bool isConnectionless() const {
    return connectionless;
  }
  
  bool hasMore() {
    return msgReadCount <  curSize;
  }

  void setCommand(bool value) {
    command = value;
  }

  bool isCommand() {
    return command;
  }
  double delay;
private:
  int msgReadCount;
  bool msgBadRead;
  bool allowOverflow;
  bool overflowed;
  std::vector<byte> data;
  int maxSize;
  int curSize;
  bool connectionless;
  friend class Connection;
  bool command;
};

class Connection {
public:
  Connection();
  virtual ~Connection();

  void connect(char **argv);
  int send(Message msg);
  int sendConnectionless(Message msg);
  void close();
  bool recv(Message * msg);
  bool process(Message *msg);
  int getQport() const {
    return qport;
  }
  
  bool hasJoinedGame() const {
    return handshake;
  }
  
  void handshakeComplete() {
    handshake = true;
  }
  
  int getOutgoingSequence() const {
    return outgoingSequence;
  }

  void writeHeader(Message *msg);

  int getIncomingSequence() const {
    return incomingSequence;
  }

private:

  int incomingSequence;
  int incomingAck;
  int incomingReliableAck;
  int incomingReliableSequence;
  int outgoingSequence;
  bool sendReliable;
  int deltaSequence;
  bool handshake;
  int sockfd;

  fd_set readfds;
  struct timeval tv;
  struct sockaddr_in servaddr;
  int qport;
  

  int sendInner(Message * msg);
};

#endif /* CONNECTION_HPP */

