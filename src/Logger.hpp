/*
 * Logger.hpp
 *
 *  Created on: Nov 11, 2021
 *      Author: mastakillah
 */

#ifndef COMMON_SRC_LOGGER_HPP_
#define COMMON_SRC_LOGGER_HPP_
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>
#include <ctime>
#include <iomanip>

#define LOG Logger()

class Logger {
public:
  Logger() {
  }
  virtual ~Logger() {
    print();
  }

  template<typename T>
  Logger& operator <<(const T &data) {
    o << data;
    return *this;
  }

  void print() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::cout << "[" << std::put_time(&tm, "%a %b %d %H:%M:%S %Y") << "] " << o.str() << std::endl;
    o.str("");
    o.clear();
  }

private:
  std::stringstream o;

};

#endif /* COMMON_SRC_LOGGER_HPP_ */
