#ifndef Subsystem_Package_Run_h
#define Subsystem_Package_Run_h
// -*- C++ -*-
//
// Package:     Package
// Class  :     Run
// 
/**\class Run Run.h "Run.h"

 Description: [one line class summary]

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Thu, 14 Mar 2013 19:01:32 GMT
// $Id$
//

// system include files

// user include files

// forward declarations

class Run {
public:
   Run(unsigned int iCacheID):m_number(0),m_transitionID(0),m_cacheID(iCacheID) {}
   m_number(iNumber),m_transitionID(iTransitionID) {}
   unsigned int number() const {return m_number;}
   unsigned int transitionID() const {return m_transitionID;}
   unsigned int cacheID() const {return m_cacheID;}


   void set(unsigned int iNumber, unsigned int iTransitionID) {
      m_number=iNumber;
      m_transitionID=iTransitionID;
   }
private:
   unsigned int m_number;
   unsigned int m_transitionID;
   const unsigned int m_cacheID;
};


#endif
