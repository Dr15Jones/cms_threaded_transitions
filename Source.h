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
   typedef unsigned int RunFragmentIdentifier;
   
   Source(unsigned int iNRuns, unsigned int iNEventsPerRun, unsigned int iNEvents);
   enum Transitions {kStop,kRun,kEvent};

   Transitions nextTransition() const {return m_nextTransition;}

   unsigned int nextRunsNumber() const {return m_nextRunNumber;}
   unsigned int nextEventsNumber() const {return m_nextEventNumber;}

   //Since parts of a run can be broken up this value uniquely
   // identifies a particular fragment of a Run to be summed
   // this value is Source implementation defined and will be
   // passed back to the Source duing the 'sumRunInfo' call
   RunFragmentIdentifier nextRunFragmentIdentifier() const {return 0;}

   void gotoNextEvent(Event&);
   void gotoNextRun(Run&);
   void sumRunInfo(Run&, RunFragmentIdentifier) {}
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
