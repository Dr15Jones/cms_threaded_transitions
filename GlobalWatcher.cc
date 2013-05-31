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
{
   ++m_nBeginRunsSeen;
}
void 
GlobalWatcher::globalEndRun(Run const& iRun) const
{
   writeLock([&](){
      printf("globalEndRun %u\n",iRun.number());
      });
   ++m_nEndRunsSeen;
}

void 
GlobalWatcher::beginStream(unsigned int) const
{
   ++m_nBeginStreamsSeen;
}
void 
GlobalWatcher::streamBeginRun(unsigned int iStreamID, Run const&) const
{
   unsigned int n = ++m_simultaneousBeginRuns;
   writeLock([&](){
      printf("#streamBeginRuns %u stream:%u\n",n,iStreamID);
      });
   usleep(100);
   --m_simultaneousBeginRuns;
}
void 
GlobalWatcher::event(unsigned int iStreamID, Event const&) const
{
   ++m_nEventsSeen;
   unsigned int n = ++m_simultaneousEvents;
   writeLock([&](){
      printf("#events %u stream:%u\n",n,iStreamID);
      });
   usleep(1000);
   --m_simultaneousEvents;
}
void 
GlobalWatcher::streamEndRun(unsigned int iStreamID, Run const&) const
{
   unsigned int n = ++m_simultaneousEndRuns;
   writeLock([&](){
      printf("#streamEndRuns %u stream:%u\n",n,iStreamID);
      });
   usleep(100);
   --m_simultaneousEndRuns;
}
void 
GlobalWatcher::endStream(unsigned int) const
{
   ++m_nEndStreamsSeen;
}

//
// static member functions
//
