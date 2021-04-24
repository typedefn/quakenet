
/* 
 * File:   Genome.cpp
 * Author: dervish
 * 
 * Created on March 6, 2021, 7:42 PM
 */

#include "Genome.hpp"
#include "Utility.hpp"

Genome::Genome(int max):fitness(0.0) {
    for(int i = 0; i < max; ++i) {
        double r = Utility::randomFloat();
        genes.push_back(r);
    }
}

Genome::~Genome() {
}

