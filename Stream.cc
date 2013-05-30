// -*- C++ -*-
//
// Package:     Package
// Class  :     Stream
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Chris Jones
//         Created:  Thu, 14 Mar 2013 19:25:35 GMT
// $Id$
//

// system include files
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include "tbb/task.h"

// user include files
#include "Stream.h"
#include "Run.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//
static std::atomic<unsigned int> s_simultaneousEvents{0};
static std::atomic<unsigned int> s_simultaneousBeginRuns{0};
static std::atomic<unsigned int> s_simultaneousEndRuns{0};

//
// constructors and destructor
//
Stream::Stream(unsigned int iID):
m_id(iID),m_state(kInitialized),m_runTransitionID(0),m_run(0)
{
}

// Stream::Stream(const Stream& rhs)
// {
//    // do actual copying here;
// }

//Stream::~Stream()
//{
//}

void 
Stream::processBeginRun(Run const* iRun) {
   unsigned int n = ++s_simultaneousBeginRuns;
   printf("#streamBeginRuns %u\n",n);
   m_runTransitionID = iRun->transitionID();
   m_run = iRun;
   m_state = kBeginRun;
   usleep(100);
   --s_simultaneousBeginRuns;
}
void 
Stream::processEvent() {
   unsigned int n = ++s_simultaneousEvents;
   printf("#events %u\n",n);
   m_state = kEvent;
   usleep(1000);
   --s_simultaneousEvents;
}
//will decrement iDoneTask and if reach 0 will spawn it
// this allows global end run to be called once all streams
// are done with a 
void 
Stream::processEndRun(tbb::task* iDoneTask) {
   unsigned int n = ++s_simultaneousEndRuns;
   printf("#streamEndRuns %u\n",n);
   m_state=kEndRun;
   usleep(100);
   if(0==iDoneTask->decrement_ref_count()) {
      tbb::task::spawn(*iDoneTask);
   }
   --s_simultaneousEndRuns;
}

void 
Stream::processEndStream(tbb::task* iDoneTask) {
   m_state=kFinished;
   if(0==iDoneTask->decrement_ref_count()) {
      tbb::task::spawn(*iDoneTask);
   }
}
