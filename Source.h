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
   Source(unsigned int iNRuns, unsigned int iNEventsPerRun, unsigned int iNEvents);
   enum Transitions {kStop,kRun,kEvent};

   Transitions nextTransition() const {return m_nextTransition;}

   unsigned int nextRunsNumber() const {return m_nextRunNumber;}
   unsigned int nextEventsNumber() const {return m_nextEventNumber;}

   void gotoNextEvent(Event&);
   void gotoNextRun(Run&);
   void sumRunInfo(Run&) {}
private:
   void finishTransition();
   
   Transitions m_nextTransition;
   unsigned int m_nextRunNumber;
   unsigned int m_nextEventNumber;
   unsigned int m_nEventsSeen;
   
   const unsigned int m_nRuns;
   const unsigned int m_nEventsPerRun;
   const unsigned int m_nEvents;
};

#endif
