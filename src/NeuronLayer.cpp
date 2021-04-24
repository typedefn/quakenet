
/* 
 * File:   NeuronLayer.cpp
 * Author: dervish
 * 
 * Created on March 6, 2021, 11:14 PM
 */

#include "NeuronLayer.hpp"

NeuronLayer::NeuronLayer(int numNeurons, int numInputsPerNeuron) {
    this->numNeurons = numNeurons;
    this->numInputsPerNeuron = numInputsPerNeuron;
    
    for(int i = 0; i < numNeurons; i++) {
        neurons.push_back(Neuron(numInputsPerNeuron));
    }
}


NeuronLayer::~NeuronLayer() {
}

