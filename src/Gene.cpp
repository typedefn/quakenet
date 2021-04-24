
/* 
 * File:   Gene.cpp
 * Author: dervish
 * 
 * Created on March 6, 2021, 7:34 PM
 */

#include "Gene.hpp"

Gene::Gene() {
    action = (ActionType)Utility::randomRanged(0, 4);
    duration = Utility::randomRanged(0, MAX_ACTION_DURATION_MS) / 1000.0f;
}

Gene::Gene(ActionType a, float d):action(a), duration(d) {
    
}

Gene::~Gene() {
}

