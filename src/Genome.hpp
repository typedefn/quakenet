
/* 
 * File:   Genome.hpp
 * Author: dervish
 *
 * Created on March 6, 2021, 7:42 PM
 */

#ifndef GENOME_HPP
#define GENOME_HPP
#include "Common.hpp"

#define MAX_GENES 50

class Genome {
public:
  Genome() { fitness = 0.0;};
  Genome(int max);  

  virtual ~Genome();
  
  bool operator <(const Genome &other) const {
    return (this->fitness < other.fitness);
  }
  
  void setFitness(double f) {
    fitness = f;
  }
  
  const double getFitness() const {
    return fitness;
  }
  
  vector<double> & getGenes() {
    return genes;
  }
  
  void setGenes(vector<double> genes) {
    this->genes = genes;
  }
  
private:  
  vector<double> genes;
  double fitness;
  
};

#endif /* GENOME_HPP */

