#include "EventProcessor.h"
#include "TestSource.h"
#include "GlobalWatcher.h"
#include <memory>
#include <iostream>
#include <cassert>

#include "tbb/task_scheduler_init.h"

#include <boost/property_tree/json_parser.hpp>

int main(int argc, char * const argv[]) {
   assert(argc==2);
   boost::property_tree::ptree pConfig; 
   try {
     read_json(argv[1],pConfig);
   } catch(const std::exception& iE) {
     std::cout <<iE.what()<<std::endl;
     exit(1);
   }
   
   const unsigned int nThreads = pConfig.get<unsigned int>("process.options.nThreads",tbb::task_scheduler_init::default_num_threads());
   std::unique_ptr<tbb::task_scheduler_init> tsi{new tbb::task_scheduler_init(nThreads)};

   const unsigned int nSimultaneousRuns = pConfig.get<unsigned int>("process.options.nSimultaneousRuns",1);
   const unsigned int nStreams = pConfig.get<unsigned int>("process.options.nStreams",1);
   
   auto const& sourceParams = pConfig.get_child("process.source");
   std::string const sourceType = sourceParams.get<std::string>("@type");
   std::unique_ptr<Source> pSource;
   if(sourceType == "TestSource") {
      pSource.reset( new TestSource(sourceParams.get<unsigned int>("nRuns"),
                                    sourceParams.get<unsigned int>("nEventsPerRun"),
                                    sourceParams.get<unsigned int>("nEvents")));
   }
   assert(0!=pSource.get());

   GlobalWatcher gw;
   EventProcessor ep(pSource.get(), &gw,nStreams, nSimultaneousRuns);
   ep.processAll();
   
   assert(gw.nEventsSeen() == sourceParams.get<unsigned int>("nEvents"));
   
   assert(gw.nBeginRunsSeen() == gw.nEndRunsSeen());
   assert(gw.nBeginRunsSeen() == sourceParams.get<unsigned int>("nRuns"));
   assert(gw.nBeginStreamsSeen() == gw.nEndStreamsSeen());
   assert(gw.nBeginStreamsSeen() == nStreams);
   
   std::cerr <<"Process finished\n";

}
