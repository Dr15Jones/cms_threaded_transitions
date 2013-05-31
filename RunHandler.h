#ifndef Subsystem_Package_RunHandler_h
#define Subsystem_Package_RunHandler_h
// -*- C++ -*-
//
// Package:     Package
// Class  :     RunHandler
// 
/**\class RunHandler RunHandler.h "RunHandler.h"

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
// user include files
#include "Run.h"
#include "WaitingTaskList.h"
#include "SerialTaskQueue.h"

// forward declarations
class Coordinator;
class Stream;
class Source;
class GlobalWatcher;

class RunHandler{
   public:
      explicit RunHandler(unsigned int iNRuns, GlobalWatcher* iWatcher);

      void setCoordinator(Coordinator*);

      tbb::task* assignToARun(Stream*);
      tbb::task* assignToRunThenDoEvent(Stream*);
      tbb::task* prepareToRemoveFromRun(Stream*);

      tbb::task* newRun(unsigned int, Source*, tbb::task* iDoneWithRunTask);

      unsigned int presentRunTransitionID() const;

      void doneWithRun(unsigned int iCacheID);
      void beginHasFinished(unsigned int iCacheID);
      
      void doneProcessing();

      //these can be called asynchronously since they just 
      // pass the appropriate Run the the GlobalWatcher
      void doBeginRunProcessing(unsigned int iCacheID) const;
      void doEndRunProcessing(unsigned int iCacheID) const;
   private:
      void startNewRun(unsigned int iRunNumber, unsigned int iCacheID, Source* iSource, tbb::task* iDoneWithRunTask);

      std::vector<Run> m_runs;
      std::vector<std::atomic<tbb::task*>> m_endRunTasks;
      std::vector<std::shared_ptr<edm::WaitingTaskList>> m_waitingForBeginToFinish;
      std::vector<edm::SerialTaskQueue> m_runSumQueues;
      std::atomic<unsigned int> m_nAvailableRuns;
      std::atomic<bool> m_waitingForAvailableRun;
      edm::WaitingTaskList m_tasksWaitingForAvailableRun;
      GlobalWatcher* m_watcher;
      Coordinator* m_coordinator;

      unsigned int m_presentRunTransitionID;
      unsigned int m_presentCacheID;
      unsigned int m_presentRunNumber;
};


#endif
