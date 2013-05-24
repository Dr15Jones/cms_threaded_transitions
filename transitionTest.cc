#include "EventProcessor.h"
#include "Source.h"
#include <memory>

#include "tbb/task_scheduler_init.h"


int main() {
   const unsigned int nThreads = 4;
   const unsigned int nSimultaneousRuns = 1;
   std::unique_ptr<tbb::task_scheduler_init> tsi{new tbb::task_scheduler_init(nThreads)};
   
   Source s{1,100,10};
   
   EventProcessor ep(&s, nThreads, nSimultaneousRuns);
   ep.processAll();
   
   return 0;
}










