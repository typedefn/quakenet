
/* 
 * File:   NeuralNet.hpp
 * Author: dervish
 *
 * Created on March 6, 2021, 11:17 PM
 */

#ifndef NEURALNET_HPP
#define NEURALNET_HPP

#include "NeuronLayer.hpp"

class NeuralNet {
public:
  NeuralNet();
  virtual ~NeuralNet();
  
  void createNet();
  
  vector<double> getWeights();
  
  int getNumberOfWeights() const {
    return numberOfTotalWeights;
  }
  
  void putWeights(const vector<double> &weights);
  
  inline double sigmoid(double activation, double response);
  
  vector<double> update(vector<double> &inputs);
  vector<int> calcSplitPoints();
  
  static int numOutputs;
private:
  int numInputs;
  
  int hiddenLayers;
  int neuronsPerHiddenLayer;
  double bias;
  int numberOfTotalWeights;
  
  vector<NeuronLayer> layers;

  
};

#endif /* NEURALNET_HPP */

