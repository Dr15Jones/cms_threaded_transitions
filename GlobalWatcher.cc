// -*- C++ -*-
//
// Package:     Package
// Class  :     GlobalWatcher
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Chris Jones
//         Created:  Thu, 30 May 2013 22:09:21 GMT
// $Id$
//

// system include files
#include <unistd.h>
#include <atomic>
#include <cstdio>

// user include files
#include "GlobalWatcher.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
GlobalWatcher::GlobalWatcher()
{
}

// GlobalWatcher::GlobalWatcher(const GlobalWatcher& rhs)
// {
//    // do actual copying here;
// }

//GlobalWatcher::~GlobalWatcher()
//{
//}

//
// assignment operators
//
// const GlobalWatcher& GlobalWatcher::operator=(const GlobalWatcher& rhs)
// {
//   //An exception safe implementation is
//   GlobalWatcher temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//

//
// const member functions
//
void 
GlobalWatcher::globalBeginRun(Run const&) const
{}
void 
GlobalWatcher::globalEndRun(Run const&) const
{}

void 
GlobalWatcher::beginStream(unsigned int) const
{}
void 
GlobalWatcher::streamBeginRun(unsigned int, Run const&) const
{
   unsigned int n = ++m_simultaneousBeginRuns;
   printf("#streamBeginRuns %u\n",n);
   usleep(100);
   --m_simultaneousBeginRuns;
}
void 
GlobalWatcher::event(unsigned int, Event const&) const
{
   ++m_nEventsSeen;
   unsigned int n = ++m_simultaneousEvents;
   printf("#events %u\n",n);
   usleep(1000);
   --m_simultaneousEvents;
}
void 
GlobalWatcher::streamEndRun(unsigned int, Run const&) const
{
   unsigned int n = ++m_simultaneousEndRuns;
   printf("#streamEndRuns %u\n",n);
   usleep(100);
   --m_simultaneousEndRuns;
}
void 
GlobalWatcher::endStream(unsigned int) const
{}

//
// static member functions
//
