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

// forward declarations
class Coordinator;
class Stream;
class Source;

class RunHandler{
   public:
      explicit RunHandler(unsigned int iNRuns);

      void setCoordinator(Coordinator*);

      tbb::task* assignToARun(Stream*);
      tbb::task* assignToRunThenDoEvent(Stream*);
      tbb::task* prepareToRemoveFromRun(Stream*);

      void newRun(unsigned int, Source*);

      unsigned int presentRunTransitionID() const;

      void doneWithRun(unsigned int iCacheID);

   private:
      void startNewRun(unsigned int iRunNumber, unsigned int iCacheID, Source* iSource);

      std::vector<Run> m_runs;
      std::vector<std::atomic<tbb::task*>> m_endRunTasks;
      std::vector<std::shared_ptr<edm::WaitingTaskList>> m_waitingForBeginToFinish;
      std::atomic<unsigned int> m_nAvailableRuns;
      std::atomic<bool> m_waitingForAvailableRun;
      edm::WaitingTaskList m_tasksWaitingForAvailableRun;
      Coordinator* m_coordinator;

      unsigned int m_presentRunTransitionID;
      unsigned int m_presentCacheID;
      unsigned int m_presentRunNumber;
};


#endif
