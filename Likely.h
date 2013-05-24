#ifndef FWCore_Utilities_Likely_h
#define FWCore_Utilities_Likely_h
//#include "FWCore/Utilities/interface/GCCPrerequisite.h"

/* Convenience macro to test the version of gcc.
   Use it like this:
   #if GCC_PREREQUISITE(3,4,4)
   ... code requiring gcc 3.4.4 or later ...
   #endif
   Note - it won't work for gcc1 since the _MINOR macros
   were not defined then.  */
#if defined __GNUC__ && defined __GNUC_MINOR__ && defined __GNUC_PATCHLEVEL__
# define GCC_PREREQUISITE(maj, min, patch) \
        ((__GNUC__ << 16) + (__GNUC_MINOR__ << 8) + __GNUC_PATCHLEVEL__ >= ((maj) << 16) + ((min) << 8) + (patch))
#elif defined __GNUC__ && defined __GNUC_MINOR__ 
# define GCC_PREREQUISITE(maj, min, patch) \
        ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define GCC_PREREQUISITE(maj, min, patch) 0
#endif


#if GCC_PREREQUISITE(3,0,0)

#if defined(NO_LIKELY)
#define likely(x) (x)
#define unlikely(x) (x)   
#elif defined(REVERSE_LIKELY)
#define unlikely(x) (__builtin_expect(x, true))
#define likely(x) (__builtin_expect(x, false))
#else
#define likely(x) (__builtin_expect(x, true))
#define unlikely(x) (__builtin_expect(x, false))
#endif

#else
#define NO_LIKELY
#define likely(x) (x)  
#define unlikely(x) (x)
#endif

#endif
