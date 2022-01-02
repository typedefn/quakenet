# quakenet
QuakeWorld Bot Client

This is a quake bot that connects via UDP to a QW or a TF QW server, still under development and needs lots of work.

# Areas that need work

- Probably need to implement configuration parser, so that the bot config is not hardcoded, such as bot name, team selection, class selection, etc...
- Needs better AI system.
- Currently only compiles in Linux, maybe make a Windoze port of it.


# How to compile on Linux

- Install cmake
- mkdir ./build
- cd ./build
- cmake ../
- make

# How to run

./quakenet IP port

For example, to connect to a server hosted on 127.0.0.1 on port 27500 you will execute:

./quakenet 127.0.0.1 27500
  
