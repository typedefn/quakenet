#include "Bot.hpp"

using namespace std;

int main(int argc, char **argv) {

  if (argc != 4) {
    LOG << "To connect to a QW server type:";
    LOG << "./quakenet <ip> <port> <bot.ini file>";
    return 0;
  }

  Bot b(argv);
  b.mainLoop();

  return 0;
}
