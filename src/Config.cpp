#include <Config.hpp>

Config::Config(const std::string &filename) {
  load(filename);
}

Config::~Config() {
}

void Config::load(const std::string &filename) {
  std::ifstream file;
  std::string contents;
  std::string line;
  file.open(filename);
  LOG << "Loading " << filename;
 
  std::string section;
  std::string key;
  std::string value;

  if (file.is_open()) {
    while (getline(file, line)) {
      auto sectionIt = std::find(line.begin(), line.end(), '[');
      auto it = std::find(line.begin(), line.end(), '=');

      if (sectionIt != line.end()) {
        section = std::string(line.begin() + 1, line.end() - 1);
      } else if (it != line.end()) { 
        key = std::string(line.begin(), it);
        value = std::string(it + 1, line.end());
        LOG << "[" << section << "] " << key << " = " << value;
        sectionKeyValues[section][key] = value;
      }
   }

    file.close();
  } else {
    LOG << "Unable to open " << filename;
  }
}

int Config::getInt(const std::string & section, const std::string & key) {
  return std::stoi(sectionKeyValues[section][key]);
}

std::string Config::getString(const std::string & section, const std::string & key) {
  return sectionKeyValues[section][key];
}
