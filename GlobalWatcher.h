#ifndef Subsystem_Package_GlobalWatcher_h
#define Subsystem_Package_GlobalWatcher_h
// -*- C++ -*-
//
// Package:     Package
// Class  :     GlobalWatcher
// 
/**\class GlobalWatcher GlobalWatcher.h "GlobalWatcher.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Thu, 30 May 2013 22:09:19 GMT
// $Id$
//

// system include files
#include <atomic>

// user include files

// forward declarations
class Run;
class Event;

class GlobalWatcher
{

   public:
      GlobalWatcher();
      //virtual ~GlobalWatcher();

      // ---------- const member functions ---------------------
      void globalBeginRun(Run const&) const;
      void globalEndRun(Run const&) const;

      void beginStream(unsigned int) const;
      void streamBeginRun(unsigned int, Run const&) const;
      void event(unsigned int, Event const&) const;
      void streamEndRun(unsigned int, Run const&) const;
      void endStream(unsigned int) const;
      
      unsigned int nEventsSeen() const { return m_nEventsSeen.load();}
      unsigned int nBeginRunsSeen() const { return m_nBeginRunsSeen.load();}
      unsigned int nEndRunsSeen() const { return m_nEndRunsSeen.load();}
      unsigned int nBeginStreamsSeen() const { return m_nBeginStreamsSeen.load();}
      unsigned int nEndStreamsSeen() const { return m_nEndStreamsSeen.load();}
      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------

   private:
      GlobalWatcher(const GlobalWatcher&) = delete; // stop default

      const GlobalWatcher& operator=(const GlobalWatcher&) = delete; // stop default

      // ---------- member data --------------------------------
      mutable std::atomic<unsigned int> m_simultaneousEvents{0};
      mutable std::atomic<unsigned int> m_simultaneousBeginRuns{0};
      mutable std::atomic<unsigned int> m_simultaneousEndRuns{0};
      mutable std::atomic<unsigned int> m_nEventsSeen{0};
      mutable std::atomic<unsigned int> m_nBeginRunsSeen{0};
      mutable std::atomic<unsigned int> m_nEndRunsSeen{0};
      mutable std::atomic<unsigned int> m_nBeginStreamsSeen{0};
      mutable std::atomic<unsigned int> m_nEndStreamsSeen{0};

};


#endif
