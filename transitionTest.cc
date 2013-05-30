#include "EventProcessor.h"
#include "TestSource.h"
#include <memory>
#include <iostream>

#include "tbb/task_scheduler_init.h"


int main() {
   //const unsigned int nThreads = 1;
   const unsigned int nThreads = 4;
   //const unsigned int nSimultaneousRuns = 1;
   const unsigned int nSimultaneousRuns = 2;
   std::unique_ptr<tbb::task_scheduler_init> tsi{new tbb::task_scheduler_init(nThreads)};
   
   TestSource s{1,10,20};
   
   EventProcessor ep(&s, nThreads, nSimultaneousRuns);
   ep.processAll();
   
   std::cerr <<"Process finished\n";
   
   return 0;
}










