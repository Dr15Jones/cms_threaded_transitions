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
   
   Source() {}
   virtual ~Source() {}
   enum Transitions {kStop,kRun,kEvent};

   virtual Transitions nextTransition() const = 0;

   virtual unsigned int nextRunsNumber() const = 0;
   virtual unsigned int nextEventsNumber() const  = 0;

   //Since parts of a run can be broken up this value uniquely
   // identifies a particular fragment of a Run to be summed
   // this value is Source implementation defined and will be
   // passed back to the Source duing the 'sumRunInfo' call
   virtual RunFragmentIdentifier nextRunFragmentIdentifier() const = 0;

   virtual void gotoNextEvent(Event&) = 0;
   virtual void gotoNextRun(Run&) = 0;
   virtual void sumRunInfo(Run&, RunFragmentIdentifier) = 0;
};

#endif
