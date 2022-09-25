/*
 * Config.hpp
 *
 *  Created on: May 31, 2020
 *      Author: dervish
 */

#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include "Common.hpp" 

class Config {
public:
  Config(const std::string & filename);
  virtual ~Config();

  int getInt(const std::string & section, const std::string & key);
  std::string getString(const std::string & section, const std::string & key);

  void load(const std::string & filename);

private:
 std::map<std::string, std::map<std::string, std::string>> sectionKeyValues;
};
#endif /* CONFIG_HPP_*/
