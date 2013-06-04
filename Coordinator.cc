// -*- C++ -*-
//
// Package:     Package
// Class  :     Coordinator
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Chris Jones
//         Created:  Thu, 14 Mar 2013 19:15:21 GMT
// $Id$
//

// system include files
#include <cassert>
#include <iostream>
#include "tbb/task.h"

// user include files
#include "Coordinator.h"
#include "RunCache.h"
#include "GlobalWatcher.h"
#include "Stream.h"
#include "Source.h"

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
      GlobalBeginRunTask(GlobalWatcher* iWatcher, Coordinator* iCoordinator, Run* iRun) :
      m_watcher(iWatcher),
      m_coordinator(iCoordinator),
      m_run(iRun) {}
      
      tbb::task* execute() {
         //Call to globalBeginRun would be done here
         m_watcher->globalBeginRun(*m_run);
         m_coordinator->beginRunHasFinished(m_run);
      }
   private:
      GlobalWatcher* m_watcher;
      Coordinator* m_coordinator;
      Run* m_run;
   };
   
   class GlobalEndRunTask : public tbb::task {
   public:
      //NOTE: in reality, this would need access to the list of modules that want global transitions
      GlobalEndRunTask(GlobalWatcher* iWatcher, Coordinator* iCoordinator, Run* iRun,tbb::task* iDoneWithRunTask):
      m_watcher(iWatcher),
      m_coordinator(iCoordinator),
      m_doneTask(iDoneWithRunTask),
      m_run(iRun) {
         assert(0!=iWatcher);
         assert(0!=iCoordinator);
         assert(0!=m_doneTask);
      }

      tbb::task* execute() {
         //Call go globalEndRun would be done here
         m_watcher->globalEndRun(*m_run);
         m_coordinator->doneWithRun(m_run);
         tbb::task* returnValue = nullptr;
         if( 0 == m_doneTask->decrement_ref_count()) {
            returnValue = m_doneTask;
         }
         return returnValue;
      }
   private:
      GlobalWatcher* m_watcher;
      Coordinator* m_coordinator;
      tbb::task* m_doneTask;
      Run* m_run;
   };
   
   class TryLaterTask : public tbb::task {
   public:
      TryLaterTask(Stream* iStream, Coordinator* iCoordinator):
      m_stream(iStream),
      m_coordinator(iCoordinator) {}
      
      tbb::task* execute() {
         return m_coordinator->assignWorkTo(m_stream);
      }
   private:
      Stream* m_stream;
      Coordinator* m_coordinator;
   };
}

//
// constructors and destructor
//
Coordinator::Coordinator(tbb::task* iEndTask, Source* iSource, GlobalWatcher* iWatcher, RunCache& iRunCache):
m_waitingTask(iEndTask),
m_source(iSource),
m_watcher(iWatcher),
m_runHandler(iRunCache),
m_activeRun(nullptr),
m_waitingForBeginRunToFinish(),
m_endRunTasks(iRunCache.maxNumberOfConcurrentRuns()),
m_runSumQueues(iRunCache.maxNumberOfConcurrentRuns()),
m_presentRunTransitionID(0),
m_presentRunNumber(0)
{
   //presize everything
   unsigned int maxRuns = iRunCache.maxNumberOfConcurrentRuns();
   m_waitingForBeginRunToFinish.reserve(maxRuns);
   for(unsigned int i = 0; i<maxRuns; ++i) {
      m_endRunTasks[i].store(nullptr);
      m_waitingForBeginRunToFinish.push_back(std::shared_ptr<edm::WaitingTaskList>(new edm::WaitingTaskList));
   }
}

// Coordinator::Coordinator(const Coordinator& rhs)
// {
//    // do actual copying here;
// }

//Coordinator::~Coordinator()
//{
//}

tbb::task* 
Coordinator::assignWorkTo(Stream* iStream) {
   return m_queue.pushAndGetNextTaskToRun([iStream,this](){ 
      auto p = this->doAssignWorkTo_(iStream);
      if(p) {
         tbb::task::spawn(*p);
         }
      }
      );
}

class StreamEndStreamTask : public tbb::task {
  public:
     StreamEndStreamTask(Stream* iStream, tbb::task* iDoneTask):
     m_stream(iStream), m_doneTask(iDoneTask) {}
     
     tbb::task* execute() {
        m_stream->processEndStream(m_doneTask);
        return 0;
     }
  private:
     Stream* m_stream;
     tbb::task* m_doneTask;
};

class StreamEventTask : public tbb::task {
  public:
     StreamEventTask(Stream* iStream, Coordinator* iCoord):
     m_stream(iStream), m_coord(iCoord) {}
     
     tbb::task* execute() {
        m_stream->processEvent();
        return m_coord->assignWorkTo(m_stream);
     }
  private:
     Stream* m_stream;
     Coordinator* m_coord;
     unsigned int m_number;
};

static
tbb::task* doRunTaskFirstIfNotNull(tbb::task* iOne, tbb::task* iTwo) {
   if (nullptr == iOne) {
      return iTwo;
   }
   if(nullptr!=iTwo) {
      tbb::task::spawn(*iTwo);
   }
   return iOne;
}

tbb::task* 
Coordinator::doAssignWorkTo_(Stream* iStream) {
   Source::Transitions nextTran = m_source->nextTransition();
   if(nextTran == Source::kStop) {
      doneProcessing();
      //tell Stream to do end processing
      if(Stream::kEvent==iStream->state() or Stream::kBeginRun == iStream->state()) {
         //need to do endRun
         return prepareToRemoveFromRun(iStream);
      }
      if(Stream::kEndRun == iStream->state() or Stream::kInitialized == iStream->state()) {
         //Launch end stream task
         tbb::task* task{new (tbb::task::allocate_root()) StreamEndStreamTask(iStream,m_waitingTask)};
         return task;
      }
   }
   
   //If we have a task to finish up for the run (e.g. summing run products), we want that to run before starting
   // a task for the Stream. This can be done by pushing the Stream task onto the 'spawn' queue first
   tbb::task* runTask = nullptr;
   if(nextTran == Source::kRun) {
      //RunCache knows if this run has already been 'pulled' or if we are waiting
      // for a resource to become available
      // When a resource becomes available RunCache puts a 'pull' task onto the Coordinator's queue
      //If we get the same Run we just 'pulled' then we pull this
      runTask = newRun(m_source->nextRunsNumber());

      if(nullptr == runTask) {
         if(Stream::kEndRun==iStream->state() or Stream::kInitialized ==iStream->state()) {
            writeLock([&]() {
               std::cout <<"no available run for stream "<<iStream->id()<<std::endl;
               });
            
            m_runHandler.waitForAnAvailableRun(new (tbb::task::allocate_root()) TryLaterTask(iStream,this));
            return nullptr;
         }
         if( m_presentRunTransitionID != iStream->runTransitionID()) {
            //need to do endRun
            return prepareToRemoveFromRun(iStream);
         }
      }
      
      //newRun is allowed to 'pull' the transition from the source
      nextTran = m_source->nextTransition();
   }

   if(Stream::kEndRun==iStream->state() or Stream::kInitialized ==iStream->state()) {
      if(nextTran == Source::kRun or nextTran == Source::kStop) {
         return doRunTaskFirstIfNotNull(runTask,assignToARun(iStream));
      }
      if(nextTran == Source::kEvent) {
         //pull the event
         m_source->gotoNextEvent(iStream->event());
         return doRunTaskFirstIfNotNull(runTask,assignToRunThenDoEvent(iStream));
      }
   }
   
   if( m_presentRunTransitionID != iStream->runTransitionID()) {
      //need to do endRun
      return doRunTaskFirstIfNotNull(runTask,prepareToRemoveFromRun(iStream));
   }

   
   if(nextTran == Source::kEvent) {
      //process the event
      m_source->gotoNextEvent(iStream->event());
      tbb::task* task{new (tbb::task::allocate_root()) StreamEventTask(iStream,this)};
      return doRunTaskFirstIfNotNull(runTask,task);
   }
   
   assert(false);
   return runTask;
}

void 
Coordinator::beginRunHasFinished(Run* iRun)
{
   auto cacheID = iRun->cacheID();
   assert(cacheID < m_waitingForBeginRunToFinish.size());
   assert(nullptr != m_waitingForBeginRunToFinish[cacheID].get());
   
   m_waitingForBeginRunToFinish[cacheID]->doneWaiting();
}

tbb::task* 
Coordinator::assignToARun(Stream* iStream) {
   writeLock([&](){
      std::cout<<"assignToARun "<<iStream->id()<<std::endl;
      });
   unsigned int cacheID = m_activeRun->cacheID();
   assert(cacheID < m_endRunTasks.size());
   /*writeLock([&]() {
      std::cout <<" incrementing "<<m_endRunTasks[m_presentCacheID].load()<<" "<<m_presentCacheID<<std::endl;
      });*/
   m_endRunTasks[cacheID].load()->increment_ref_count();
   tbb::task* task{new (tbb::task::allocate_root()) StreamBeginRunTask{iStream,this,m_activeRun}};
   
   //NOTE: would be nice to know if global beginrun was already called so we could just return the new task
   assert(cacheID < m_waitingForBeginRunToFinish.size());
   m_waitingForBeginRunToFinish[cacheID]->add(task);
   
   return nullptr;
}

tbb::task* 
Coordinator::assignToRunThenDoEvent(Stream* iStream) {
   unsigned int cacheID = m_activeRun->cacheID();
   assert(cacheID < m_endRunTasks.size());
   /*writeLock([&]() {
      std::cout <<" incrementing "<<m_endRunTasks[m_presentCacheID].load()<<" "<<m_presentCacheID<<std::endl;
      });*/
   m_endRunTasks[cacheID].load()->increment_ref_count();
   tbb::task* task{new (tbb::task::allocate_root()) StreamBeginRunThenEventTask(iStream,this,m_activeRun)};
   
   //NOTE: would be nice to know if global beginrun was already called so we could just return the new task
   assert(cacheID < m_waitingForBeginRunToFinish.size());
   m_waitingForBeginRunToFinish[cacheID]->add(task);
   
   return nullptr;   
}
tbb::task* 
Coordinator::prepareToRemoveFromRun(Stream* iStream) {
   auto cacheID = iStream->run()->cacheID();
   assert(cacheID < m_endRunTasks.size());
   tbb::task* endTask = m_endRunTasks[cacheID];
   /*writeLock([&]() {
      std::cout <<"prepareToRemoveFromRun "<<cacheID<<" "<<iStream->id()<<std::endl;
      });*/
   assert(nullptr != endTask);
   
   return (new (tbb::task::allocate_root()) StreamEndRunTask(iStream,this,endTask));
}

void 
Coordinator::doneWithRun(Run* iRun) {
   unsigned int cacheID = iRun->cacheID();
   assert(cacheID < m_endRunTasks.size());
   m_endRunTasks[cacheID].store(nullptr);
   m_runHandler.doneWithRun(cacheID);
}

void
Coordinator::doneProcessing() {
   if(m_activeRun) {
      unsigned int cacheID = m_activeRun->cacheID();
      assert(cacheID < m_endRunTasks.size());
      auto v = m_endRunTasks[cacheID].load()->decrement_ref_count();
      /*writeLock([&]() {
         std::cout <<"decrementing "<<m_presentCacheID<<" "<<m_endRunTasks[m_presentCacheID].load()<<" count "<<v<<std::endl;
         });*/
      m_activeRun = nullptr;
   }
}



tbb::task* 
Coordinator::newRun(unsigned int iNewRunsNumber) {
   if(iNewRunsNumber == m_presentRunNumber) {
      //NOTE: should check to see if this is the same run and if we've already pulled this run 
      // then this indicates a 'reduction' step should be done
      bool keepSumming=true;
      tbb::task* returnValue = nullptr;
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
         assert(m_activeRun != nullptr);
         auto id = m_source->nextRunFragmentIdentifier();
         m_source->gotoNextRun(*m_activeRun);
         unsigned int cacheID  = m_activeRun->cacheID();
         tbb::task* doneTask = m_endRunTasks[cacheID];
         doneTask->increment_ref_count();
         
         auto activeRun = m_activeRun;
         auto source = m_source;
         auto t = [activeRun,source,id,doneTask] () {
            source->sumRunInfo(*activeRun,id);
            doneTask->decrement_ref_count();
         };
         if( nullptr == returnValue ) {
            returnValue = m_runSumQueues[cacheID].pushAndGetNextTaskToRun(t);
         } else {
            m_runSumQueues[cacheID].push(t);
         }
         auto trans = m_source->nextTransition();
         keepSumming = ((trans == Source::kRun) and (iNewRunsNumber==m_source->nextRunsNumber()));
      }while(keepSumming);
      return returnValue;
   }
   
   //previous run is now allowed to handle endRun
   if(m_activeRun) {
      unsigned int cacheID = m_activeRun->cacheID();
      assert(cacheID < m_endRunTasks.size());
      auto v = m_endRunTasks[cacheID].load()->decrement_ref_count();
      writeLock([&]() {
         std::cout <<"decrementing "<<cacheID<<" "<<m_endRunTasks[cacheID].load()<<" count "<<v<<std::endl;
         });
      m_activeRun = nullptr;
   }
   
   //Id moves here because after this call we might check to see if a waiting Stream uses this ID
   ++m_presentRunTransitionID;
   
   Run* newRun = m_runHandler.getARun();
   
   //do we have an available run slot?
   if(0!=newRun) {
      return startNewRun(iNewRunsNumber,newRun);
   }
   return nullptr;
}

//NOTE: startNewRun method must only be called from Coordinator's queue
tbb::task* 
Coordinator::startNewRun(unsigned int iRunNumber, Run* iRun) {
   unsigned int cacheID = iRun->cacheID();
   assert(cacheID < m_endRunTasks.size());
   
   assert(m_endRunTasks[cacheID].load()==nullptr);
   m_waitingTask->increment_ref_count();
   tbb::task* task{new (tbb::task::allocate_root()) GlobalEndRunTask(m_watcher,this,iRun,m_waitingTask)};
   /*writeLock([&]() {
      std::cout <<" made and incremented "<<task<<" id "<<iCacheID<<std::endl;
      });*/
   task->increment_ref_count();
   m_endRunTasks[cacheID].store(task);
   m_activeRun = iRun;
   m_presentRunNumber=iRunNumber;
   iRun->set(iRunNumber,m_presentRunTransitionID);
   m_source->gotoNextRun(*iRun);

   m_waitingForBeginRunToFinish[cacheID]->reset();
   tbb::task* t{new (tbb::task::allocate_root()) GlobalBeginRunTask(m_watcher,this,iRun)};
   return t;
}


