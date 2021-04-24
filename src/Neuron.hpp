
/* 
 * File:   Neuron.hpp
 * Author: dervish
 *
 * Created on March 6, 2021, 11:11 PM
 */

#ifndef NEURON_HPP
#define NEURON_HPP
#include "Common.hpp"

class Neuron {
public:
  Neuron(int i);
  virtual ~Neuron();
  
  int getNumberOfInputs() const {
    return numberOfInputs;
  }
  
  vector<double> & getWeights() {
    return weights;
  }
  
private:
  vector<double> weights;
  int numberOfInputs;
};

#endif /* NEURON_HPP */

