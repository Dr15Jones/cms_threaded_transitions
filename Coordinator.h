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
namespace tbb {
   class task;
}

class Coordinator {
   public:
      tbb::task* assignWorkTo(Stream*);
      void start(tbb::task* iWaitingTask);
   private:
      SerialTaskQueue m_queue;
      tbb::task* m_waitingTask;
      bool m_lastTransitionGotten;
};


#endif
