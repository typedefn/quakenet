#include "Bot.hpp"

using namespace std;

int main(int argc, char **argv) {

  if (argc != 3) {
    LOG << "To connect to a QW server type:";
    LOG << "./quakenet ip port";
    return 0;
  }
    Bot b(argv);
    b.mainLoop();
    
    return 0;
}
