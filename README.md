# quakenet
QuakeWorld Bot Client

This is a quake bot that connects via UDP to a QW server, still under development and needs lots of work.

# Areas that need work

- Probably need to implement configuration parser, so that the bot config is not hardcoded, such as bot name, team selection, class selection, etc...
- Needs better AI system.
- Sometimes the handshake fails, and the bot just sits as a ghosted client on the server until it is timed out.
- Currently only compiles in Linux, maybe make a Windoze port of it.


# How to compile on Linux

- Install cmake
- mkdir ./build
- cd ./build
- cmake ../
- make

# How to run

./quakenet IP port
  
