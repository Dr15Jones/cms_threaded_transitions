#ifndef Subsystem_Package_Coordinator_h
#define Subsystem_Package_Coordinator_h
// -*- C++ -*-
//
// Package:     Package
// Class  :     Coordinator
// 
/**\class Coordinator Coordinator.h "Coordinator.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Thu, 14 Mar 2013 19:15:21 GMT
// $Id$
//

// system include files
#include <memory>
#include <vector>

// user include files
#include "SerialTaskQueue.h"
#include "WaitingTaskList.h"


// forward declarations
class Stream;
class RunCache;
class Source;
class GlobalWatcher;
class Run;

namespace tbb {
   class task;
}

class Coordinator {
   public:
      Coordinator(tbb::task* iWaitingTask, Source*, GlobalWatcher*, RunCache&);
      tbb::task* assignWorkTo(Stream*);

      void beginRunHasFinished(Run*);
      
      void doneWithRun(Run*);
   private:
      tbb::task* doAssignWorkTo_(Stream* iStream);
      void doneProcessing();
      tbb::task* prepareToRemoveFromRun(Stream* iStream);
      tbb::task* newRun(unsigned int iNewRunsNumber);
      tbb::task* startNewRun(unsigned int iNewRunsNumber, Run* iRun);
      tbb::task* assignToRunThenDoEvent(Stream* iStream);
      tbb::task* assignToARun(Stream* iStream);
      
      edm::SerialTaskQueue m_queue;
      tbb::task* m_waitingTask;
      Source* m_source;
      GlobalWatcher* m_watcher;
      RunCache& m_runHandler;
      Run* m_activeRun;
      std::vector<std::shared_ptr<edm::WaitingTaskList>> m_waitingForBeginRunToFinish;
      std::vector<std::atomic<tbb::task*>> m_endRunTasks;
      std::vector<edm::SerialTaskQueue> m_runSumQueues;
      
      unsigned int m_presentRunTransitionID;
      unsigned int m_presentRunNumber;
};


#endif
