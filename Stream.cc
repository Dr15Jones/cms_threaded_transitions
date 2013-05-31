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
#include "tbb/task.h"

// user include files
#include "Stream.h"
#include "Run.h"
#include "GlobalWatcher.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
Stream::Stream(unsigned int iID, GlobalWatcher* iWatcher):
m_id(iID),m_state(kInitialized),m_runTransitionID(0),m_run(0),
m_watcher(iWatcher)
{
   m_watcher->beginStream(m_id);
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
   m_runTransitionID = iRun->transitionID();
   m_run = iRun;
   m_state = kBeginRun;
   m_watcher->streamBeginRun(m_id,*iRun);
}
void 
Stream::processEvent() {
   m_state = kEvent;
   m_watcher->event(m_id,m_event);
}
//will decrement iDoneTask and if reach 0 will spawn it
// this allows global end run to be called once all streams
// are done with a 
void 
Stream::processEndRun() {
   m_state=kEndRun;
   m_watcher->streamEndRun(m_id,*m_run);
}

void 
Stream::processEndStream(tbb::task* iDoneTask) {
   m_watcher->endStream(m_id);
   
   m_state=kFinished;
   if(0==iDoneTask->decrement_ref_count()) {
      tbb::task::spawn(*iDoneTask);
   }
}
