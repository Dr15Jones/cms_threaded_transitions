// -*- C++ -*-
//
// Package:     Package
// Class  :     Source
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Chris Jones
//         Created:  Fri, 24 May 2013 16:05:37 GMT
// $Id$
//

// system include files
#include <iostream>

// user include files
#include "Source.h"
#include "Event.h"
#include "Run.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
Source::Source(unsigned int iNRuns, unsigned int iNEventsPerRun, unsigned int iNEvents):
m_nextTransition{iNEvents==0? kStop: kRun},
m_nextRunNumber{1},
m_nextEventNumber{0},
m_nEventsSeen{0},
m_nRuns{iNRuns},
m_nEventsPerRun{iNEventsPerRun},
m_nEvents{iNEvents}
{
}

// Source::Source(const Source& rhs)
// {
//    // do actual copying here;
// }

//Source::~Source()
//{
//}

//
// assignment operators
//
// const Source& Source::operator=(const Source& rhs)
// {
//   //An exception safe implementation is
//   Source temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void 
Source::finishTransition()
{
   if(kRun == m_nextTransition) {
      m_nextTransition = kEvent;
      ++m_nextEventNumber;
      return;
   }
   if(kEvent == m_nextTransition) {
      ++m_nEventsSeen;
      if(m_nEventsSeen == m_nEvents) {
         m_nextTransition = kStop;
      }
      else if(m_nEventsSeen % m_nEventsPerRun == 0) {
         m_nextTransition = kRun;
         ++m_nextRunNumber;
      } else {
         ++m_nextEventNumber;
      }
   }
}


void Source::gotoNextEvent(Event& iEvent) {
   std::cout <<"Event "<<nextRunsNumber()<<" "<<nextEventsNumber()<<std::endl;
 iEvent.setNumber( {nextRunsNumber(),nextEventsNumber()});
 finishTransition();
}

void Source::gotoNextRun(Run& iRun) {
   std::cout <<"Run "<<nextRunsNumber()<<std::endl;
   
 iRun.set(nextRunsNumber(),0);
 finishTransition();
}

//
// const member functions
//

//
// static member functions
//
