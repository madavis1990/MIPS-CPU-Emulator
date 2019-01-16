/******************************
 * Michael Davis
 * CS 3339 - spring 2017
 * Section 252
 * Project 4
 ******************************/
#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  bubbles = 0;
  hazards = 0;

  memops = 0;
  branches = 0;
  taken = 0;

  for(int i = IF1; i < PIPESTAGES; i++) {
    resultReg[i] = -1;
    validReg[i] = -1;
    hazardCount[i] = 0;
  }
}

void Stats::clock() {
  cycles++;
  // run all pipeline flops
  for(int i = WB; i > IF1; i--) {
    resultReg[i] = resultReg[i-1];
    validReg[i] = validReg[i-1];
    if(validReg[i] == i+1){
       validReg[i] = -1;
    }
  }
  // inject no-op into IF1
  resultReg[IF1] = -1;
  validReg[IF1] = -1;
}

void Stats::registerSrc(int r, int v){
   for(int i = ID; i < WB; i++){
      if(resultReg[i] == r){
        if(v < validReg[i]){
            hazardCount[i]++;
            hazards++;
            bubble();
         }
      }
   }
}

void Stats::registerDest(int r, int v) {
    if(r != 0){
       resultReg[ID] = r;
       validReg[ID] = v;
    }
}

void Stats::flush(int count) { // count == how many ops to flush
   flushes = flushes + count;
   cycles = cycles + count;
}

void Stats::bubble() {
    clock();
    bubbles++;
}

long Stats::getHazards(int i) {
   return hazardCount[i];
}
