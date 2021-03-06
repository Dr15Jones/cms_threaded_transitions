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
#include "TestSource.h"
#include "Event.h"
#include "Run.h"
#include "writeLock.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
TestSource::TestSource(unsigned int iNRuns, unsigned int iNEventsPerRun, unsigned int iNTransitions):
m_nextTransition{iNTransitions==0? kStop: kRun},
m_nextRunNumber{1},
m_nextEventNumber{0},
m_nTransitionsSeen{0},
m_nRuns{iNRuns},
m_nEventsPerRun{iNEventsPerRun},
m_nEventsSeen{0},
m_nTransitions{iNTransitions}
{
}

// TestSource::TestSource(const TestSource& rhs)
// {
//    // do actual copying here;
// }

//TestSource::~TestSource()
//{
//}

//
// assignment operators
//
// const TestSource& TestSource::operator=(const TestSource& rhs)
// {
//   //An exception safe implementation is
//   TestSource temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void 
TestSource::finishTransition()
{
   ++m_nTransitionsSeen;
   if(m_nTransitionsSeen == m_nTransitions) {
      m_nextTransition = kStop;
      return;
   }
   if(kRun == m_nextTransition) {
      if(m_nEventsPerRun !=0) {
         m_nextTransition = kEvent;
         ++m_nextEventNumber;
      } else {
         ++m_nextRunNumber;
      }
      return;
   }
   if(kEvent == m_nextTransition) {
      ++m_nEventsSeen;
      if(m_nEventsSeen % m_nEventsPerRun == 0) {
         m_nextTransition = kRun;
         ++m_nextRunNumber;
      } else {
         ++m_nextEventNumber;
      }
   }
}


void TestSource::gotoNextEvent(Event& iEvent) {
   writeLock([&](){
      std::cout <<"Event "<<nextRunsNumber()<<" "<<nextEventsNumber()<<std::endl;
      });
   iEvent.setNumber( {nextRunsNumber(),nextEventsNumber()});
   finishTransition();
}

void TestSource::gotoNextRun(Run& iRun) {
   writeLock([&](){
      std::cout <<"Run "<<nextRunsNumber()<<std::endl;
      });
   finishTransition();
}

//
// const member functions
//

//
// static member functions
//
