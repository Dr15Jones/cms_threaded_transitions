// -*- C++ -*-
//
// Package:     Package
// Class  :     RunHandler
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Chris Jones
//         Created:  Thu, 14 Mar 2013 19:21:32 GMT
// $Id$
//

// system include files
#include <limits>

// user include files
#include "RunHandler.h"
#include "Stream.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
RunHandler::RunHandler(unsigned int iMaxNRuns, Coordinator* iCoord):
m_presentRunTransitionID(1),m_presentRunCacheID(std::numeric_limits<unsigned int>::max),m_presentRunNumber(0),m_coordinator(iCoord)
{
   //presize everything
}

// RunHandler::RunHandler(const RunHandler& rhs)
// {
//    // do actual copying here;
// }

RunHandler::~RunHandler()
{
}


tbb::task* 
RunHandler::assignToARun(Stream* iStream) {
   m_endTasks[m_presentRunCacheID].load()->increment_ref_count();
   auto* task{new (tbb::task::allocate_root()) StreamBeginRunTask(iStream,m_coordinator,m_runs[m_presentRunCacheID])};
   
   //NOTE: would be nice to know if global beginrun was already called so we could just return the new task
   m_waitingForBeginToFinish.add(task);
   
   return nullptr;
}
tbb::task* 
RunHandler::assignToRunThenDoEvent(Stream* iStream) {
   m_endTasks[m_presentRunCacheID].load()->increment_ref_count();
   auto* task{new (tbb::task::allocate_root()) StreamBeginRunThenEventTask(iStream,m_coordinator,m_runs[m_presentRunCacheID])};
   
   //NOTE: would be nice to know if global beginrun was already called so we could just return the new task
   m_waitingForBeginToFinish.add(task);
   
   return nullptr;   
}
tbb::task* 
RunHandler::prepareToRemoveFromRun(Stream* iStream) {
   auto cacheID = iStream->run().cacheID();
   tbb::task* endTask = m_endTasks[cacheID];
   assert(nullptr != endTask);
   
   return (new (tbb::task::allocate_root()) StreamEndRunTask(iStream,m_coordinator,endTask));
}

void 
RunHandler::newRun(unsigned int iNewRunsNumber, Source* iSource) {
   //this is an inadequate test. We need to know if the previous call to newRun has resulted in the
   // run to be 'pulled' or if we are just recalling the same value
   // Coordinator should keep track of that stuff!!
   if(m_waitingForAvailableRun) {
      return;
   }
   if(iNewRunsNumber == m_presentRunNumber) {
      //NOTE: should check to see if this is the same run and if we've already pulled this run 
      // then this indicates a 'reduction' step should be done
      bool keepSumming=true;
      do {
         keepSumming=false;
         //the following should be put into a task and run asynchronously
         iSource->sumRunInfo(m_runs[m_presentRunCacheID]);
         
         auto trans = iSource->nextTransition();
         keepSumming = ((trans == Source::kRun ) and (iNewRunsNumber==m_source->nextRunsNumber));
      }while(keepSumming);
      return;
   }
   
   //previous run is now allowed to handle endRun
   if(m_presentCacheID != std::numeric_limits<unsigned int>::max) {
      m_endTasks[m_presentCacheID].load()->decrement_ref_count();
      m_presentCacheID = std::numeric_limits<unsigned int>::max;
   }
   
   //Id moves here because after this call we might check to see if a waiting Stream uses this ID
   ++m_presentRunTransitionID;
   
   //do we have an available run slot?
   if(0!=m_m_nAvailableRuns) {
      unsigned int openRun=0;

      for(auto& endTask: m_endTasks) {
         if(nullptr==endTask.load()){
            break;
         }
         ++openRun;
      }
      assert(openRun != m_endTasks.size());
      startNewRun(iNumber,iSource);
   } else {
      //The bool works as a synchronization barries since reset
      // can not be called simultaneously with any other method of WaitingTaskList
      m_tasksWaitingForAvailableRun.reset();
      m_waitingForAvailableRun.store(true);
      
      m_tasksWaitingForAvailableRun.add(new (tbb::task::allocate_root()) StartNewRunTask(this,iNewRunsNumber,iSource));
   }
}

unsigned int 
RunHandler::presentRunTransitionID() const {
   return m_presentRunTransitionID;
}

//NOTE: startNewRun method must only be called from Coordinator's queue
void 
RunHandler::startNewRun(unsigned int iRunNumber, unsigned int iCacheID, Source* iSource) {
   assert(m_endTasks[iCacheID].load()==nullptr);
   auto* task{new (tbb::task::allocate_root()) GlobalEndRunTask(this,iCacheID)};
   task->increment_ref_count();
   m_endTasks[iCacheID].store(task);
   --m_m_nAvailableRuns;
   m_presentRunCacheID = iCacheID;
   m_presentRunNumber=iRunNumber;
   m_runs[iCacheID].set(iRunNumber,m_presentRunTransitionID);
   iSource->gotoNextRun(m_runs[iCacheID]);

   m_waitingForBeginToFinish[iCacheID].reset();
   auto* task{new (tbb::task::allocate_root()) GlobalBeginRunTask(this,iCacheID)};
   tbb::task::spawn(*task);
}

class GlobalEndRunTask : public tbb::task {
public:
   //NOTE: in reality, this would need access to the list of modules that want global transitions
   GlobalEndRunTask(RunHandler* iHandler, unsigned int iCacheID):
   m_handler(iHandler),
   m_cacheID(iCacheID) {}
   
   tbb::task* execute() {
      //Call go globalEndRun would be done here
      m_handler->doneWithRun(m_cacheID);
      return nullptr;
   }
private:
   RunHandler* m_handler;
   unsigned int m_cacheID;
};

void 
RunHandler::doneWithRun(unsigned int iCacheID) {
   m_endTasks[iCacheID].store(nullptr);
   ++m_m_nAvailableRuns;
   if(m_waitingForAvailableRun.compare_exchange_strong(true,false)) {
      m_tasksWaitingForAvailableRun.doneWaiting();
   }
}




class StreamBeginRunTask : public tbb::task {
  public:
     StreamBeginRunTask(Stream* iStream, Coordinator* iCoord, const Run* iRun):
     m_stream(iStream), m_coord(iCoord), m_run(iRun) {}
     
     tbb::task* execute() {
        m_stream->processBeginRun(*m_run);
        return m_coord->assignWorkTo(m_stream);
     } 
  private:
     Stream* m_stream;
     Coordinator* m_coord;
     const Run* m_run;
};

class StreamBeginRunThenEventTask : public tbb::task {
  public:
     StreamBeginRunThenEventTask(Stream* iStream, Coordinator* iCoord, const Run* iRun):
     m_stream(iStream), m_coord(iCoord), m_run(iRun) {}
     
     tbb::task* execute() {
        m_stream->processBeginRun(*m_run);
        m_stream->processEvent();
        return m_coord->assignWorkTo(m_stream);
     } 
  private:
     Stream* m_stream;
     Coordinator* m_coord;
     const Run* m_run;
};


class StreamEndRunTask : public tbb::task {
  public:
     StreamEndRunTask(Stream* iStream, Coordinator* iCoord, tbb::task* iDoneTask):
     m_stream(iStream), m_coord(iCoord), m_doneTask(iDoneTask) {}
     
     tbb::task* execute() {
        m_stream->processEndRun(m_doneTask);
        return m_coord->assignWorkTo(m_stream);
     }
  private:
     Stream* m_stream;
     Coordinator* m_coord;
     tbb::task* m_doneTask;
};
