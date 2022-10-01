# quakenet
QuakeWorld Bot Client

This is a quake bot that connects via UDP to a TF QW server, still under development and needs lots of work.

Only has waypoints setup for 2fort5r.bsp and 1on1r.bsp in the ./resources/ .ini files.
There are some bot profiles, like what position the bot should play defense vs offense.

# Areas that need work

- Needs better AI system.
- Currently only compiles in Linux, maybe make a Windoze port of it.

# How to compile on Linux

- Install cmake
- mkdir ./build
- cd ./build
- cmake ../
- make

# How to run

./quakenet IP port <file.ini>

For example, to connect to a server hosted on 127.0.0.1 on port 27500 with the dervish.ini bot profile you will execute:

./quakenet 127.0.0.1 27500 dervish.ini
  
