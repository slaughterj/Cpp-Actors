#include "Process.h"
#include <cstdio>
#include <utility>

typedef std::pair<proc::ProcessId, proc::ProcessId> idPair_t;

void pingFunc(Variant& var)
{
    // This is Possible because of the converison operator.
    // If you would like to use 'static_cast<ProcessId>(var)'
    // then uncomment 'explicit' in the conversion
    // operator overload in 'Variant.h'.
    idPair_t id = var;
    auto self = id.first;
    auto receiver = id.second;

    static unsigned int numOfPings = 0;
    if(numOfPings < 10)
    {
        numOfPings++;
        proc::sendMsg(receiver, std::make_pair(receiver, self));
        return;
    }
    else
    {
        printf("I've gotten enough pings. I'm finished.\n");
        proc::sendMsg(receiver, proc::PROC_CONTROL::KILL);
        proc::sendMsg(self, proc::PROC_CONTROL::KILL);

        return;
    }
}

void pongFunc(Variant& var)
{
    idPair_t id = var;
    auto self = id.first;
    auto receiver = id.second;

    proc::MSG_STAT status = sendMsg(receiver, std::make_pair(receiver, self));
    if(status == proc::MSG_STAT::SUCCESS)
    {
        return;
    }
    else
    {
        printf("The Process didn't receive the message successfully.\n");
        return;
    }
}

int main()
{   /*
     * The spawn function uses initializer_list syntax and takes an unordered_map
     * of Type<T>::Id as a key, where T is any type, and a function with the signature of
     * 'void (*)(Variant&)' as the value. Any number of key-value pairs can be used. 
     * Whenever the process is sent a message of the type as the key the function will be
     * called.
     *
     * It is overloaded so you can add a function of type 'void (*)()'. This will be
     * called once the process is spawned. A lambda
     * conforming to that type may be used also, as is done in the example below.
     */
    
    proc::ProcessId ping = spawn({{Type<idPair_t>::Id, pingFunc}});
    proc::ProcessId pong = spawn([] { printf("From the init function.\n"); },
            {{Type<idPair_t>::Id, pongFunc}});
    
    sendMsg(ping, std::make_pair(ping, pong));

    // This must be called so that the main thread waits for all processes to finish. No join necessary.
    proc::processWait();
    printf("Finished.\n");
    return 0;
}
