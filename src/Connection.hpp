
/* 
 * File:   Connection.hpp
 * Author: dervish
 *
 * Created on February 21, 2021, 8:37 AM
 */

#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include "Common.hpp"
#include "Protocol.hpp"

class Message {
public:

  Message();
  virtual ~Message();
  
  void beginRead();
  void clear();

  unsigned readLong();
  int readByte();
  int readShort();
  char *readString();
  float readFloat();

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
  
  const vector<byte> & getData() const {
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
  
private:
  int msgReadCount;
  bool msgBadRead;
  bool allowOverflow;
  bool overflowed;
  vector<byte> data;
  int maxSize;
  int curSize;
  bool connectionless;
  friend class Connection;
};

class Connection {
public:
  Connection();
  Connection(const Connection& orig);
  virtual ~Connection();

  void connect();  
  void send(Message msg);
  void sendConnectionless(Message msg);
  
  bool recv(Message * msg, bool block);
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
  
  void writeHeader(Message *msg);
  void sendInner(Message * msg);
};

#endif /* CONNECTION_HPP */

