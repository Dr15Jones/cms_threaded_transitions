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
      
      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------

   private:
      GlobalWatcher(const GlobalWatcher&) = delete; // stop default

      const GlobalWatcher& operator=(const GlobalWatcher&) = delete; // stop default

      // ---------- member data --------------------------------

};


#endif
