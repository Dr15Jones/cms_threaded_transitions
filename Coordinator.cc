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
#include "tbb/task.h"

// user include files
#include "Coordinator.h"
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
Coordinator::Coordinator():
m_lastTransitionGotten(false)
{
}

// Coordinator::Coordinator(const Coordinator& rhs)
// {
//    // do actual copying here;
// }

Coordinator::~Coordinator()
{
}

tbb::task* 
Coordinator::assignWorkTo(Stream* iStream) {
   return m_queue.pushAndGetNextTaskToRun([iStream,this](){ this->doAssignWorkTo_(iStream);});
}

class StreamEndStreamTask : public tbb::task {
  public:
     StreamEndRunTask(Stream* iStream, tbb::task* iDoneTask):
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


tbb::task* 
Coordinator::doAssignWorkTo_(Stream* iStream) {
   Source::Transitions nextTran = m_source->nextTransition();
   if(nextTran == Source::kStop) {
      //tell Stream to do end processing
      if(Stream::kEvent==iStream->state() or Stream::kBeginRun == iStream->state()) {
         //need to do endRun
         return m_runHandler.prepareToRemoveFromRun(iStream);
      }
      if(Stream::kEndRun == iStream->state()) {
         //Launch end stream task
         auto* task{new (tbb::task::allocate_root()) StreamEndStreamTask(iStream,m_doneTask)};
         return task;
      }
   }

   if(nextTran == Source::kRun) {
      //RunHandler knows if this run has already been 'pulled' or if we are waiting
      // for a resource to become available
      // When a resource becomes available RunHandler puts a 'pull' task onto the Coordinator's queue
      //If we get the same Run we just 'pulled' then we pull this
      m_runHandler.newRun(m_source->nextRunsNumber(),m_source);
   }

   //RunHandler is allowed to 'pull' the transition from the source
   nextTran = m_source->nextTransition();

   if(Stream::kEndRun==iStream->state() or Stream::kInitialized ==iStream->state()) {
      if(nextTran == Source::kRun) {
         return m_runHandler.assignToRun(iStream);
      }
      if(nextTran == Source::kEvent) {
         //pull the event
         m_source->getNextEvent(iStream->event());
         return m_runHandler.assignToRunThenDoEvent(iStream);
      }
   }
   
   if( m_runHandler.presentRunTransitionID() != iStream->runTransitionID()) {
      //need to do endRun
      return m_runHandler.prepareToRemoveFromRun(iStream);
   }

   
   if(nextTran == Source::kEvent) {
      //process the event
      m_source->getNextEvent(iStream->event());
      auto* task{new (tbb::task::allocate_root()) StreamEventTask(iStream,this)};
      return task;
   }
}
