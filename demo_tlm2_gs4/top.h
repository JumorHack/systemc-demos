#ifndef TOP_H
#define TOP_H

#include "initiator.h"
#include "target.h"

SC_MODULE(Top)
{
  Initiator *initiator;
  Target    *target;

  SC_CTOR(Top)
  {
    // Instantiate components
    initiator = new Initiator("initiator");
    target    = new Target   ("target");

    // One initiator is bound directly to one target with no intervening bus
    initiator->socket.bind(target->socket);
  }
};

#endif // TOP_H
