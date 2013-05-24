// -*- C++ -*-
//
// Package:     Package
// Class  :     EventProcessor
// 
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Chris Jones
//         Created:  Thu, 23 May 2013 21:43:32 GMT
// $Id$
//

// system include files
#include <vector>
#include <tbb/task.h>

// user include files
#include "EventProcessor.h"
#include "Stream.h"
#include "Coordinator.h"
#include "RunHandler.h"

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
EventProcessor::EventProcessor(Source* iSource, unsigned int iNStreams, unsigned int iNRuns):
m_source(iSource),
m_nStreams(iNStreams),
m_nRuns(iNRuns)
{
}

// EventProcessor::EventProcessor(const EventProcessor& rhs)
// {
//    // do actual copying here;
// }

//EventProcessor::~EventProcessor()
//{
//}

//
// assignment operators
//
// const EventProcessor& EventProcessor::operator=(const EventProcessor& rhs)
// {
//   //An exception safe implementation is
//   EventProcessor temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void 
EventProcessor::processAll() 
{
   auto eventLoopWaitTask = new (tbb::task::allocate_root()) tbb::empty_task{};
   eventLoopWaitTask->increment_ref_count();

   std::vector<std::shared_ptr<Stream>> streams;
   streams.reserve(m_nStreams);
   
   RunHandler runHandler(m_nRuns);
   
   Coordinator coordinator(eventLoopWaitTask,m_source,runHandler);
   
   for(unsigned int i=0; i<m_nStreams;++i) {
      std::shared_ptr<Stream> p{ new Stream{i}};
      streams.push_back(p);
      eventLoopWaitTask->increment_ref_count();
      auto t = coordinator.assignWorkTo(p.get());
      if(nullptr != t) {
         tbb::task::enqueue(*t);
      }
   }
   eventLoopWaitTask->wait_for_all();
   tbb::task::destroy(*eventLoopWaitTask);
}

//
// const member functions
//

//
// static member functions
//
