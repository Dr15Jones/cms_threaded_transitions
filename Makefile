#Change this to point to your boost installation
BOOST_INCLUDES := -I/Users/cdj/src/cms/software/FWLiteVersions/osx108_amd64_gcc472/external/boost/1.50.0-cms4/include
BOOST_LIB_PATH := -L/Users/cdj/src/cms/software/FWLiteVersions/osx108_amd64_gcc472/external/boost/1.50.0-cms4/lib
#BOOST_INCLUDES := -I$(BOOST_INC)
CXXOTHERFLAGS :=
UNAME:=$(shell uname -s)
#UNAME:=Linux
TBB_INCLUDES :=-I/Users/cdj/src/cms/software/FWLiteVersions/osx108_amd64_gcc472/external/tbb/41_20120718oss/include
TBB_LIB_PATH :=-L/Users/cdj/src/cms/software/FWLiteVersions/osx108_amd64_gcc472/external/tbb/41_20120718oss/lib
#TBB_LIB := $(TBB_LIB_PATH) -ltbb_debug
ifeq ($(UNAME), Linux)
BOOST_INCLUDES := -I/uscms_data/d2/cdj/build/multicore/dispatch/install/include
#change the following to to point to where you installed tbb
TBB_INCLUDES := -I/uscms_data/d2/cdj/build/multicore/tbb/tbb40_20120408oss/include
#TBB_LIB_PATH := -L/uscms_data/d2/cdj/build/multicore/tbb/tbb40_20120408oss//build/linux_intel64_gcc_cc4.7.0_libc2.5_kernel2.6.18_debug
TBB_LIB_PATH := -L/uscms_data/d2/cdj/build/multicore/tbb/tbb40_20120408oss//build/linux_intel64_gcc_cc4.7.0_libc2.5_kernel2.6.18_release
CXXOTHERFLAGS := -D__USE_XOPEN2K8
endif
TBB_LIB := $(TBB_LIB_PATH) -ltbb
BOOST_LIB := $(BOOST_LIB_PATH) -lboost_thread
CXX=g++
CPPFLAGS=$(TBB_INCLUDES) $(BOOST_INCLUDES) $(CPPUNIT_INCLUDES) 
#CXXFLAGS=-O3 -g -std=c++0x -fPIC -pthread $(CXXOTHERFLAGS)
CXXFLAGS=-O0 -g -std=c++0x -fPIC -pthread $(CXXOTHERFLAGS)
LDFLAGS= -std=c++0x -pthread
LINKER=g++

all: transitionTest

objects := $(patsubst %.cc,%.o,$(wildcard *.cc))


transitionTest: $(objects)
	$(LINKER) $+ $(LDFLAGS) $(TBB_LIB) -o $@

clean:
	rm -f *.o 
	rm -f transitionTest
	rm -f $(objects)
