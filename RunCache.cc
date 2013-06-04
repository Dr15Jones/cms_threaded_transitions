// -*- C++ -*-
//
// Package:     Package
// Class  :     RunCache
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
#include "RunCache.h"
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


//
// constructors and destructor
//
RunCache::RunCache(unsigned int iMaxNRuns):
m_nAvailableRuns(iMaxNRuns),
m_waitingForAvailableRun(false),
m_runAvailable(iMaxNRuns)
{
   //presize everything
   m_runs.reserve(iMaxNRuns);
   for(unsigned int i = 0; i<iMaxNRuns; ++i) {
      m_runs.emplace_back(i);
      m_runAvailable[i].store(true);
   }
}

// RunCache::RunCache(const RunCache& rhs)
// {
//    // do actual copying here;
// }

//RunCache::~RunCache()
//{
//}
Run*
RunCache::getARun()
{
   //It is possible that last time newRun was called, doneWithRun was called
   // at the same time and it just so happened
   //  1) newRun called '--m_nAvailableRun' and set value to 0
   //  2) doneWithRun called '++m_nAvailableRun' and set value to 1
   //  3) doneWithRun checked m_waitingForAvailableRun and found it to be false so stopped
   //  4) newRun did m_tasksWaitingForAvailableRun.reset() and then set m_waitingForAvailableRun to true
   //This would leave us with an available run (m_nAvailableRun>0) but m_waitingForAvailableRun == true
   

   unsigned int nAvailable = m_nAvailableRuns.load();
   if(m_waitingForAvailableRun and 0 == nAvailable ) {
      return nullptr;
   }
   if(m_waitingForAvailableRun) {
      m_tasksWaitingForAvailableRun.doneWaiting();
      m_waitingForAvailableRun.store(false);
   }
   //do we have an available run slot?
   if(0!=m_nAvailableRuns) {
      unsigned int openRun=0;

      for(auto& available: m_runAvailable) {
         if(available.load()){
            available.store(false);
            break;
         }
         ++openRun;
      }
      assert(openRun != m_runAvailable.size());
      
      if( 0 == --m_nAvailableRuns) {
         //THIS ISN'T SAFE: we could be in the middle of calling 'doneWaiting' when the reset is called
         //this needs to be synchronized.
         
         m_tasksWaitingForAvailableRun.reset();
         m_waitingForAvailableRun = true;
         writeLock([&]() {
               std::cout <<"newRun: waiting"<<std::endl;
               });
      }
      return &m_runs[openRun];
   }
   return nullptr;
}

void 
RunCache::waitForAnAvailableRun(tbb::task* iTask) {
   writeLock([&]() {
         std::cout <<" task waiting for available run"<<std::endl;
         });
   m_tasksWaitingForAvailableRun.add(iTask);
}

void 
RunCache::doneWithRun(unsigned int iCacheID) {
   //NOTE: THe order of atomic operations is critical.
   // m_waitingForAvailableRun must only be set after everything else
   // since it is a synchronization barrier to keep m_tasksWaitingForAvailableRun.reset
   // from being called while m_tasksWaitingForAvailableRun.doneWaiting is running
   // m_nAvilableRuns must be updated after changing m_runAvailable 
   
   m_runAvailable[iCacheID].store(true);
   unsigned int nAvailable = ++m_nAvailableRuns;
   writeLock([&]() {
         std::cout <<"doneWithRun: # now available "<<nAvailable<<" waiting "<<m_waitingForAvailableRun.load()<<std::endl;
         });
   if(true == m_waitingForAvailableRun.load()) {
      //NOTE: it is safe to call doneWaiting from multiple threads
      m_tasksWaitingForAvailableRun.doneWaiting();
      m_waitingForAvailableRun.store(false);
   }
}
