#if !defined(writeLock_h)
#define writeLock_h
#include <mutex>

extern  std::mutex s_writeLock;

template<typename T>
void writeLock(T iFunctor) {
   std::lock_guard<std::mutex> lock(s_writeLock);
   iFunctor();
}

#endif