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
        sectionKeyValues[section][key] = value;
      }
   }

    file.close();
  } else {
    std::stringstream ss;
    ss << "Unable to open " << filename;
    LOG << ss.str();
    throw std::runtime_error(ss.str());
  }
}

bool Config::sectionExist(const std::string & section) {
  if(sectionKeyValues.find(section) == sectionKeyValues.end()) {
    return false;
  }

  return true;
}

int Config::getInt(const std::string & section, const std::string & key) {
  if(sectionKeyValues.find(section) == sectionKeyValues.end()) {
    return 0;
  }

  auto keyMap = sectionKeyValues[section];
  
  if (keyMap.find(key) == keyMap.end()) {
    return 0;
  }
 
  return std::stoi(sectionKeyValues[section][key]);
}

std::string Config::getString(const std::string & section, const std::string & key) {

  if(sectionKeyValues.find(section) == sectionKeyValues.end()) {
    return "n/a";
  }

  auto keyMap = sectionKeyValues[section];
  
  if (keyMap.find(key) == keyMap.end()) {
    return "n/a";
  }
  

  return sectionKeyValues[section][key]; 
}

glm::vec3 Config::getVec3(const std::string & section, const std::string &key) {
  std::stringstream ss;
  glm::vec3 waypoint;

  std::string value = this->getString(section, key);
  if (value == "n/a") {
    return glm::vec3(0, 0, 0);
  }

  ss << value;
  ss >> waypoint.x >> waypoint.y >> waypoint.z;
  return waypoint;
}
