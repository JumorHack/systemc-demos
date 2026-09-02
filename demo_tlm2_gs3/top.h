#ifndef TOP_H
#define TOP_H

#include <cstdio>

#include "initiator.h"
#include "target.h"
#include "router.h"

SC_MODULE(Top)
{
    Initiator* initiator;
    Router<4>* router;
    Memory*    memory[4];

    SC_CTOR(Top)
    {
        // Instantiate components
        initiator = new Initiator("initiator");
        router    = new Router<4>("router");
        for (int i = 0; i < 4; i++)
        {
            char txt[20];
            snprintf(txt, sizeof(txt), "memory_%d", i);
            memory[i] = new Memory(txt);
        }

        // Bind: initiator -> router target socket
        initiator->socket.bind( router->target_socket );

        // Bind: each router initiator socket -> one memory
        for (int i = 0; i < 4; i++)
            router->initiator_socket[i]->bind( memory[i]->socket );
    }
};

#endif // TOP_H
