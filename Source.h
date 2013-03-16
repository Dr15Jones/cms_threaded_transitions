#ifndef Subsystem_Package_Source_h
#define Subsystem_Package_Source_h
// -*- C++ -*-
//
// Package:     Package
// Class  :     Source
// 
/**\class Source Source.h "Source.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Thu, 14 Mar 2013 19:06:26 GMT
// $Id$
//

// system include files
#include <utility>
// user include files

// forward declarations
class Event;
class Run;

class Source {
public:
   enum Transitions {kStop,kRun,kEvent};

   Transitions nextTransition() const;

   unsigned int nextRunsNumber() const;
   std::pair<unsigned int, unsigned int> nextEventsNumber() const;

   void gotoNextEvent(Event&);
   void gotoNextRun(Run&);
private:
   void finishTransition();
};

void Source::gotoNextEvent(Event& iEvent) {
 iEvent->setNumber(nextEventsNumber());
 finishTransition();
}

void Source::gotoNextRun(Run& iRun) {
 iRun->setNumber(nextRunsNumber());
 finishTransition();
}


#endif
