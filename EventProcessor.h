#ifndef Subsystem_Package_EventProcessor_h
#define Subsystem_Package_EventProcessor_h
// -*- C++ -*-
//
// Package:     Package
// Class  :     EventProcessor
// 
/**\class EventProcessor EventProcessor.h "EventProcessor.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Thu, 14 Mar 2013 19:22:33 GMT
// $Id$
//

// system include files

// user include files

// forward declarations
class Source;
class GlobalWatcher;

class EventProcessor {
   public:
      EventProcessor(Source* iSource, GlobalWatcher* iWatcher, unsigned int iNStreams, unsigned int iNRuns);

      void processAll();
   private:
      Source* m_source;
      GlobalWatcher* m_watcher;
      unsigned int m_nStreams;
      unsigned int m_nRuns;
};


#endif
