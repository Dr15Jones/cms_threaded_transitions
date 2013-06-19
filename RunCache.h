#ifndef Subsystem_Package_RunCache_h
#define Subsystem_Package_RunCache_h
// -*- C++ -*-
//
// Package:     Package
// Class  :     RunCache
// 
/**\class RunCache RunCache.h "RunCache.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Thu, 14 Mar 2013 19:13:54 GMT
// $Id$
//

// system include files
#include <atomic>
#include <vector>
#include <memory>
#include <functional>
// user include files
#include "Run.h"
#include "SerialTaskQueue.h"
#include "WaitingTaskList.h"

// forward declarations
class Coordinator;
class Stream;
class Source;
class GlobalWatcher;

class RunCache{
   public:
      explicit RunCache(unsigned int iNRuns);

      unsigned int maxNumberOfConcurrentRuns() const {return m_runs.size();}

      ///Returns nullptr if no available run slot
      /// This must be called serially
      Run* getARun();
      void waitForAnAvailableRun(tbb::task*);

      //this can be called asynchronously
      void doneWithRun(unsigned int iCacheID);

   private:
      std::vector<Run> m_runs;
      std::vector<std::atomic<bool>> m_runAvailable;
      std::atomic<unsigned int> m_nAvailableRuns;
      std::atomic<bool> m_waitingForAvailableRun;
      edm::WaitingTaskList m_tasksWaitingForAvailableRun;
};


#endif
