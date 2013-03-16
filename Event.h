#ifndef Subsystem_Package_Event_h
#define Subsystem_Package_Event_h
// -*- C++ -*-
//
// Package:     Package
// Class  :     Event
// 
/**\class Event Event.h "Event.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Thu, 14 Mar 2013 19:02:23 GMT
// $Id$
//

// system include files
#include <utility>
// user include files

// forward declarations

    class Event {
    public:
       Event():m_number(0,0) {}

       typedef  std::pair<unsigned int, unsigned int> EventNumber;

       EventNumber number() const { return m_number;}

       void setNumber(EventNumber const& iNumber) {
          m_number = iNumber;
       }
    private:
       EventNumber m_number;
    };

#endif
