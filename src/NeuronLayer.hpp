
/* 
 * File:   NeuronLayer.hpp
 * Author: dervish
 *
 * Created on March 6, 2021, 11:14 PM
 */

#ifndef NEURONLAYER_HPP
#define NEURONLAYER_HPP
#include "Neuron.hpp"

class NeuronLayer {
public:
  NeuronLayer(int numNeurons, int numInputsPerNeuron);
  
  virtual ~NeuronLayer();
  
  int getNumInputsPerNeuron() const {
    return numInputsPerNeuron;
  }
  
  int getNumNeurons() const {
    return numNeurons;
  }
  
  vector<Neuron> & getNeurons() {
    return neurons;
  }
  
private:
  int numInputsPerNeuron;
  int numNeurons;
  vector<Neuron> neurons;
};

#endif /* NEURONLAYER_HPP */

