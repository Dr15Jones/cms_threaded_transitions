#ifndef Subsystem_Package_Stream_h
#define Subsystem_Package_Stream_h
// -*- C++ -*-
//
// Package:     Package
// Class  :     Stream
// 
/**\class Stream Stream.h "Stream.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Thu, 14 Mar 2013 19:03:52 GMT
// $Id$
//

// system include files

// user include files
#include "Event.h"

// forward declarations
namespace tbb {
   class task;
}

class Run;
class GlobalWatcher;

 class Stream {
 public:
    Stream(unsigned int iID, GlobalWatcher*);

    enum States {kInitialized,kBeginRun,kEvent,kEndRun,kFinished};

    void processBeginRun(Run const* iRun);
    void processEvent();
    //will decrement iDoneTask and if reach 0 will spawn it
    // this allows global end run to be called once all streams
    // are done with a 
    void processEndRun(tbb::task* iDoneTask);
    void processEndStream(tbb::task* iDoneTask);

    States state() const { return m_state;}
    unsigned int id() const {return m_id;}

    //Coordinator looks at this and compares it to next transition
    // to see if processEndRun must be called
    unsigned int runTransitionID() const {return m_runTransitionID;}

    const Run* run() const {return m_run;}

    Event& event() {return m_event;}
 private:
    unsigned int m_id;
    States m_state;
    unsigned int m_runTransitionID;
    Run const* m_run;
    Event m_event;
    GlobalWatcher* m_watcher;
 };


#endif
