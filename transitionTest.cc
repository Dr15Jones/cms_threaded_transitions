#include "EventProcessor.h"
#include "Source.h"


int main() {
   Source s();
   
   EventProcessor ep(&s, 4);
   ep.processAll();
   
   return 0;
}










