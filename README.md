# quakenet
QuakeWorld Bot Client

This is a quake bot that connects via UDP to a TF QW server, still under development and needs lots of work.

Only has waypoints setup for 2fort5r.bsp in the ./resources/2fort5r.ini file.
There are some bot profiles, like what position the bot should play defense vs offense.

# Areas that need work

- Needs better AI system.
- Currently only compiles in Linux, maybe make a Windoze port of it.
- Currently only have 2 defense points setup in the 2fort5r map, more way points can be added.

# How to compile on Linux

- Install cmake
- mkdir ./build
- cd ./build
- cmake ../
- make

# How to run

./quakenet IP port <file.ini>

For example, to connect to a server hosted on 127.0.0.1 on port 27500 you will execute:

./quakenet 127.0.0.1 27500 botconfig.ini
  
