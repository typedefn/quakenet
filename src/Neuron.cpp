
/* 
 * File:   Neuron.cpp
 * Author: dervish
 * 
 * Created on March 6, 2021, 11:11 PM
 */

#include "Neuron.hpp"
#include "Utility.hpp"

Neuron::Neuron(int i) {
    numberOfInputs = i;
    
    for(int i = 0; i < numberOfInputs; ++i) {
        double r = Utility::randomFloat();
        weights.push_back(r);
    }
}

Neuron::~Neuron() {
}

