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
#include <tbb/task.h>
#include <cassert>
#include <iostream>

// user include files
#include "RunHandler.h"
#include "Coordinator.h"
#include "Stream.h"
#include "Source.h"
#include "WaitingTaskList.h"
#include "GlobalWatcher.h"
#include "writeLock.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//
namespace {
   class StreamBeginRunTask : public tbb::task {
     public:
        StreamBeginRunTask(Stream* iStream, Coordinator* iCoord, const Run* iRun):
        m_stream(iStream), m_coord(iCoord), m_run(iRun) {}

        tbb::task* execute() {
           m_stream->processBeginRun(m_run);
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
           m_stream->processBeginRun(m_run);
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
           m_stream->processEndRun();
           auto newStreamTask = m_coord->assignWorkTo(m_stream);
           auto v = m_doneTask->decrement_ref_count() ;
           /*writeLock([&]() {
              std::cout <<" decrementing "<<m_doneTask<<" "<<v<<std::endl;
              });*/
            
           if( 0 == v ) {
              //we want the end of run task to happen before the stream
              // task starts
              if(nullptr!=newStreamTask) {
                 tbb::task::spawn(*newStreamTask);
              }
              return m_doneTask;
           }
           return newStreamTask;
        }
     private:
        Stream* m_stream;
        Coordinator* m_coord;
        tbb::task* m_doneTask;
   };
   class GlobalBeginRunTask : public tbb::task {
   public:
      GlobalBeginRunTask(RunHandler* iHandler, unsigned int iCacheID) :
      m_handler(iHandler),
      m_cacheID(iCacheID) {}
      
      tbb::task* execute() {
         //Call to globalBeginRun would be done here
         m_handler->doBeginRunProcessing(m_cacheID);
         m_handler->beginHasFinished(m_cacheID);
      }
   private:
      RunHandler* m_handler;
      unsigned int m_cacheID;
   };
   
   class GlobalEndRunTask : public tbb::task {
   public:
      //NOTE: in reality, this would need access to the list of modules that want global transitions
      GlobalEndRunTask(RunHandler* iHandler, unsigned int iCacheID,tbb::task* iDoneWithRunTask):
      m_handler(iHandler),
      m_doneTask(iDoneWithRunTask),
      m_cacheID(iCacheID) {
         assert(0!=iHandler);
         assert(0!=m_doneTask);
      }

      tbb::task* execute() {
         //Call go globalEndRun would be done here
         m_handler->doEndRunProcessing(m_cacheID);
         m_handler->doneWithRun(m_cacheID);
         tbb::task* returnValue = nullptr;
         if( 0 == m_doneTask->decrement_ref_count()) {
            returnValue = m_doneTask;
         }
         return returnValue;
      }
   private:
      RunHandler* m_handler;
      tbb::task* m_doneTask;
      unsigned int m_cacheID;
   };
   
   class StartNewRunTask : public tbb::task {
   public:
      StartNewRunTask(RunHandler*,unsigned int iNewRunsNumber,Source*, tbb::task* iDoneWithRunTask) {}
      tbb::task* execute() {}
      
   };
}


//
// constructors and destructor
//
RunHandler::RunHandler(unsigned int iMaxNRuns, GlobalWatcher* iWatcher):
m_nAvailableRuns(iMaxNRuns),
m_runSumQueues(iMaxNRuns),
m_presentRunTransitionID(1),
m_presentCacheID(std::numeric_limits<unsigned int>::max()),
m_presentRunNumber(0),
m_waitingForAvailableRun(false),
m_watcher(iWatcher),
m_coordinator(nullptr),
m_endRunTasks(iMaxNRuns)
{
   //presize everything
   m_runs.reserve(iMaxNRuns);
   m_waitingForBeginToFinish.reserve(iMaxNRuns);
   for(unsigned int i = 0; i<iMaxNRuns; ++i) {
      m_runs.emplace_back(i);
      m_endRunTasks[i].store(nullptr);
      m_waitingForBeginToFinish.push_back(std::shared_ptr<edm::WaitingTaskList>(new edm::WaitingTaskList));
   }
}

// RunHandler::RunHandler(const RunHandler& rhs)
// {
//    // do actual copying here;
// }

//RunHandler::~RunHandler()
//{
//}

void 
RunHandler::setCoordinator(Coordinator* iCoord) {
   m_coordinator = iCoord;
}


tbb::task* 
RunHandler::assignToARun(Stream* iStream) {
   writeLock([&](){
      std::cout<<"assignToARun "<<iStream->id()<<std::endl;
      });
   assert(m_presentCacheID < m_endRunTasks.size());
   /*writeLock([&]() {
      std::cout <<" incrementing "<<m_endRunTasks[m_presentCacheID].load()<<" "<<m_presentCacheID<<std::endl;
      });*/
   m_endRunTasks[m_presentCacheID].load()->increment_ref_count();
   tbb::task* task{new (tbb::task::allocate_root()) StreamBeginRunTask(iStream,m_coordinator,&(m_runs[m_presentCacheID]))};
   
   //NOTE: would be nice to know if global beginrun was already called so we could just return the new task
   assert(m_presentCacheID < m_waitingForBeginToFinish.size());
   m_waitingForBeginToFinish[m_presentCacheID]->add(task);
   
   return nullptr;
}

tbb::task* 
RunHandler::assignToRunThenDoEvent(Stream* iStream) {
   assert(m_presentCacheID < m_endRunTasks.size());
   /*writeLock([&]() {
      std::cout <<" incrementing "<<m_endRunTasks[m_presentCacheID].load()<<" "<<m_presentCacheID<<std::endl;
      });*/
   m_endRunTasks[m_presentCacheID].load()->increment_ref_count();
   tbb::task* task{new (tbb::task::allocate_root()) StreamBeginRunThenEventTask(iStream,m_coordinator,&(m_runs[m_presentCacheID]))};
   
   //NOTE: would be nice to know if global beginrun was already called so we could just return the new task
   assert(m_presentCacheID < m_waitingForBeginToFinish.size());
   m_waitingForBeginToFinish[m_presentCacheID]->add(task);
   
   return nullptr;   
}
tbb::task* 
RunHandler::prepareToRemoveFromRun(Stream* iStream) {
   auto cacheID = iStream->run()->cacheID();
   assert(cacheID < m_endRunTasks.size());
   tbb::task* endTask = m_endRunTasks[cacheID];
   /*writeLock([&]() {
      std::cout <<"prepareToRemoveFromRun "<<cacheID<<" "<<iStream->id()<<std::endl;
      });*/
   assert(nullptr != endTask);
   
   return (new (tbb::task::allocate_root()) StreamEndRunTask(iStream,m_coordinator,endTask));
}

tbb::task* 
RunHandler::newRun(unsigned int iNewRunsNumber, Source* iSource, tbb::task* iDoneWithRunTask) {
   //this is an inadequate test. We need to know if the previous call to newRun has resulted in the
   // run to be 'pulled' or if we are just recalling the same value
   // Coordinator should keep track of that stuff!!
   if(m_waitingForAvailableRun) {
      return nullptr;
   }
   tbb::task* returnValue = nullptr;
   if(iNewRunsNumber == m_presentRunNumber) {
      //NOTE: should check to see if this is the same run and if we've already pulled this run 
      // then this indicates a 'reduction' step should be done
      bool keepSumming=true;
      do {
         writeLock([&]() {
            std::cout <<"doing summing"<<std::endl;
            });
         keepSumming=false;
         //the following should be put into a task and run asynchronously
         // We should have a 'summing' queue for each Run whis is where the 
         // summing tasks are put.
         // It should also be 'dependent' on the Run so we will not run the 
         // global end run until all summing has finished.
         assert(m_presentCacheID<m_runs.size());
         auto& run = m_runs[m_presentCacheID];
         auto id = iSource->nextRunFragmentIdentifier();
         iSource->gotoNextRun(run);
         tbb::task* doneTask = m_endRunTasks[m_presentCacheID];
         doneTask->increment_ref_count();
         
         auto t = [&run,iSource,id,doneTask] () {
            iSource->sumRunInfo(run,id);
            doneTask->decrement_ref_count();
         };
         if( nullptr == returnValue ) {
            returnValue = m_runSumQueues[m_presentCacheID].pushAndGetNextTaskToRun(t);
         } else {
            m_runSumQueues[m_presentCacheID].push(t);
         }
         auto trans = iSource->nextTransition();
         keepSumming = ((trans == Source::kRun) and (iNewRunsNumber==iSource->nextRunsNumber()));
      }while(keepSumming);
      return returnValue;
   }
   
   //previous run is now allowed to handle endRun
   if(m_presentCacheID != std::numeric_limits<unsigned int>::max()) {
      assert(m_presentCacheID < m_endRunTasks.size());
      auto v = m_endRunTasks[m_presentCacheID].load()->decrement_ref_count();
      writeLock([&]() {
         std::cout <<"decrementing "<<m_presentCacheID<<" "<<m_endRunTasks[m_presentCacheID].load()<<" count "<<v<<std::endl;
         });
      m_presentCacheID = std::numeric_limits<unsigned int>::max();
   }
   
   //Id moves here because after this call we might check to see if a waiting Stream uses this ID
   ++m_presentRunTransitionID;
   
   //do we have an available run slot?
   if(0!=m_nAvailableRuns) {
      unsigned int openRun=0;

      for(auto& endTask: m_endRunTasks) {
         if(nullptr==endTask.load()){
            break;
         }
         ++openRun;
      }
      assert(openRun != m_endRunTasks.size());
      return startNewRun(iNewRunsNumber,openRun,iSource, iDoneWithRunTask);
   } else {
      //The bool works as a synchronization barries since reset
      // can not be called simultaneously with any other method of WaitingTaskList
      m_tasksWaitingForAvailableRun.reset();
      m_waitingForAvailableRun.store(true);
      
      m_tasksWaitingForAvailableRun.add(new (tbb::task::allocate_root()) StartNewRunTask(this,iNewRunsNumber,iSource,iDoneWithRunTask));
   }
   return nullptr;
}

void
RunHandler::doneProcessing() {
   if(m_presentCacheID != std::numeric_limits<unsigned int>::max()) {
      assert(m_presentCacheID < m_endRunTasks.size());
      auto v = m_endRunTasks[m_presentCacheID].load()->decrement_ref_count();
      /*writeLock([&]() {
         std::cout <<"decrementing "<<m_presentCacheID<<" "<<m_endRunTasks[m_presentCacheID].load()<<" count "<<v<<std::endl;
         });*/
      m_presentCacheID = std::numeric_limits<unsigned int>::max();
   }
}

unsigned int 
RunHandler::presentRunTransitionID() const {
   return m_presentRunTransitionID;
}

void 
RunHandler::beginHasFinished(unsigned int iCacheID)
{
   assert(iCacheID < m_waitingForBeginToFinish.size());
   assert(nullptr != m_waitingForBeginToFinish[iCacheID].get());
   
   m_waitingForBeginToFinish[iCacheID]->doneWaiting();
}


//NOTE: startNewRun method must only be called from Coordinator's queue
tbb::task* 
RunHandler::startNewRun(unsigned int iRunNumber, unsigned int iCacheID, Source* iSource,tbb::task* iDoneWithRunTask) {
   assert(iCacheID < m_endRunTasks.size());
   
   assert(m_endRunTasks[iCacheID].load()==nullptr);
   iDoneWithRunTask->increment_ref_count();
   tbb::task* task{new (tbb::task::allocate_root()) GlobalEndRunTask(this,iCacheID,iDoneWithRunTask)};
   /*writeLock([&]() {
      std::cout <<" made and incremented "<<task<<" id "<<iCacheID<<std::endl;
      });*/
   task->increment_ref_count();
   m_endRunTasks[iCacheID].store(task);
   --m_nAvailableRuns;
   m_presentCacheID = iCacheID;
   m_presentRunNumber=iRunNumber;
   m_runs[iCacheID].set(iRunNumber,m_presentRunTransitionID);
   iSource->gotoNextRun(m_runs[iCacheID]);

   m_waitingForBeginToFinish[iCacheID]->reset();
   tbb::task* t{new (tbb::task::allocate_root()) GlobalBeginRunTask(this,iCacheID)};
   return t;
}

void 
RunHandler::doneWithRun(unsigned int iCacheID) {
   assert(iCacheID < m_endRunTasks.size());
   m_endRunTasks[iCacheID].store(nullptr);
   ++m_nAvailableRuns;
   bool expected = true;
   if(m_waitingForAvailableRun.compare_exchange_strong(expected,false)) {
      m_tasksWaitingForAvailableRun.doneWaiting();
   }
}

void
RunHandler::doBeginRunProcessing(unsigned int iCacheID) const{
   assert(iCacheID < m_runs.size());
   m_watcher->globalBeginRun(m_runs[iCacheID]);
}

void
RunHandler::doEndRunProcessing(unsigned int iCacheID) const{
   assert(iCacheID < m_runs.size());
   m_watcher->globalEndRun(m_runs[iCacheID]);
}


