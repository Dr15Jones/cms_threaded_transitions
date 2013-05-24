#ifndef Subsystem_Package_Coordinator_h
#define Subsystem_Package_Coordinator_h
// -*- C++ -*-
//
// Package:     Package
// Class  :     Coordinator
// 
/**\class Coordinator Coordinator.h "Coordinator.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Thu, 14 Mar 2013 19:15:21 GMT
// $Id$
//

// system include files

// user include files
#include "SerialTaskQueue.h"

// forward declarations
class Stream;
class RunHandler;
class Source;
namespace tbb {
   class task;
}

class Coordinator {
   public:
      Coordinator(tbb::task* iWaitingTask, Source*, RunHandler&);
      tbb::task* assignWorkTo(Stream*);
      void start(tbb::task* iWaitingTask);
   private:
      tbb::task* doAssignWorkTo_(Stream* iStream);
      edm::SerialTaskQueue m_queue;
      tbb::task* m_waitingTask;
      Source* m_source;
      RunHandler& m_runHandler;
      bool m_lastTransitionGotten;
};


#endif
