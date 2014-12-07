/*
 * Process.h
 *
 * A very lightweight actor model implementation inspired by erlang.
 *
 * Created by Jason Slaughter 10 October 2014
 */

#ifndef PROCESS_H_
#define PROCESS_H_ 

#include "Variant.h"
#include <thread>
#include <atomic>
#include <unordered_map>

namespace proc
{

struct ProcessId
{
    unsigned int m_index;
    uintptr_t m_pointer;
    uint64_t m_id;
};

typedef void (*messageHandler_t)(const Variant& var, ProcessId self);
typedef void(*initFunc_t)();
typedef std::unordered_map<unsigned int, messageHandler_t> messageMap_t;

enum class MSG_STAT : unsigned int
{
    SUCCESS = 0, // Message was delivered successfully.
    QUEUE_FULL = 1, // The process' message queue is full.
    NO_MATCH = 2, // The process has no matching function for the type and won't respond.
    NO_PROCESS = 3 // The process has been killed or never existed.
};

enum class PROC_CONTROL : unsigned int
{
    KILL = 0 // Kill Process.
};

} // namespace proc

namespace
{
using namespace proc;

class ProcessManager
{
    class Process;
    enum { MAX_PROCESSES = 256 };
public:
    std::atomic<uint64_t> m_processCounter;
    std::atomic_uint m_numProcesses;

    std::atomic_uint m_queueFront;
    std::atomic_bool m_isAvailable[MAX_PROCESSES];
    std::atomic_uintptr_t m_processes[MAX_PROCESSES];
    std::atomic_uint m_nextFreeQueue[MAX_PROCESSES];

    ProcessManager()
    {
        m_processCounter.store(0);
        m_queueFront.store(255);

        for(int i = 0, j = 255; i < 256; i++, j--)
        {
            m_nextFreeQueue[i].store(j);
            m_isAvailable[i].store(false);
            m_processes[i].store(0);
        }
    }

}g_manager;

class Process
{ 
public:
    uint8_t m_current;
    Variant m_messageQueue[256];
    std::atomic<uint8_t> m_queueBack;
    std::atomic_uint m_numMessages;
    ProcessId m_Pid;
    initFunc_t m_initFunc; // NEW: process init function, called upon spawn.
    messageMap_t m_map;

    Process(messageMap_t msgmap) : m_map(msgmap), m_initFunc(nullptr)
    {
        m_current = 0;
        Variant v(nullptr);

        for(int i = 0; i < 256; i++)
        {
            m_messageQueue[i] = v;
        }

        m_queueBack.store(0);
        m_Pid.m_pointer = (std::uintptr_t)this;
    }
};

void insertProcess(Process* process)
{
    auto index = g_manager.m_nextFreeQueue[g_manager.m_queueFront--].load();
    g_manager.m_processes[index].store((uintptr_t)process);
    process->m_Pid.m_index = index;
    process->m_Pid.m_id = g_manager.m_processCounter++;
    g_manager.m_isAvailable[index].store(true);
}

void removeProcess(Process* process)
{
    g_manager.m_queueFront++;
    g_manager.m_nextFreeQueue[g_manager.m_queueFront.load()].store(process->m_Pid.m_index);
    g_manager.m_processes[process->m_Pid.m_index].store(0);
    delete process;
    g_manager.m_numProcesses--;
}

void run(Process* process, ProcessManager* manager, unsigned int index)
{
	Variant var;
    if(process->m_initFunc)
    {
        process->m_initFunc();
    }
	
	while(true)
	{
			if(process->m_messageQueue[process->m_current].getId() != Type<std::nullptr_t>::Id)
			{
				var = process->m_messageQueue[process->m_current];
				if(Type<PROC_CONTROL>::Id == var.getId())
				{
                   PROC_CONTROL control = var.get<PROC_CONTROL>();
                   int a = var.getId();
                    // TODO: switch(control)
                    removeProcess(process);
                    return;
				}
                else
                {
                    var = process->m_messageQueue[process->m_current];
		            if(process->m_map.find(var.getId()) == process->m_map.end())
                    {
                        process->m_messageQueue[process->m_current] = Variant(nullptr);
                        process->m_current++;
                        process->m_numMessages--;
                    }
                    else
                    {
                        auto func = process->m_map[var.getId()];
                        func(var, process->m_Pid);
                        process->m_messageQueue[process->m_current] = Variant(nullptr);
                        process->m_current++;
                        process->m_numMessages--;
                    }
                }
			}
			else
			{
				;
			}
	}
}

} // unnamed namespace


/*************************************************************************
 *
 *                          Exported Functions
 *
 *************************************************************************/

namespace proc
{
	template <typename T>
	MSG_STAT sendMsg(ProcessId pid, const T& t)
	{
        Variant var(t);
        bool available = g_manager.m_isAvailable[pid.m_index].load();
        std::uintptr_t pointer = g_manager.m_processes[pid.m_index].load();
        Process* process = (Process*)pointer;

        /*
         * If the process is still accepting messages AND the pointer is still valid AND
         * the 'm_id' is the same as the one of the ProcessId passed into the function.
         */
        if(available && (pointer != 0) && (process->m_Pid.m_id == pid.m_id))
        {
            // If the message queue is full.
            if(process->m_numMessages.load() == 256) 
            {
                return MSG_STAT::QUEUE_FULL;
            }
            // If the map has no value to match the key.
            if(process->m_map.find(var.getId()) == process->m_map.end()) 
            {
                return MSG_STAT::NO_MATCH;
            }
            
            process->m_numMessages++;
            int index = process->m_queueBack++;
            process->m_messageQueue[index] = t;
            return MSG_STAT::SUCCESS;
        }
        else
        {
            return MSG_STAT::NO_PROCESS;
        }
    }

MSG_STAT sendMsg(ProcessId pid, PROC_CONTROL control)
{
    bool available = g_manager.m_isAvailable[pid.m_index].load();
    std::uintptr_t pointer = g_manager.m_processes[pid.m_index].load();
    Process* process = (Process*)pointer;

        if(available && (pointer != 0) && (process->m_Pid.m_id == pid.m_id))
        {
            if(process->m_numMessages.load() == 256) 
            {
                return MSG_STAT::QUEUE_FULL;
            }
            // TODO: switch(control).
            g_manager.m_isAvailable[pid.m_index].store(false);
            process->m_numMessages++;
            int index = process->m_queueBack++;
            process->m_messageQueue[index] = Variant(control); 
            return MSG_STAT::SUCCESS;
        }
        else
        {
            return MSG_STAT::NO_PROCESS;
        }
}

// Call only once in main to wait for other processes to finish.
void processWait()
{
    while (g_manager.m_numProcesses.load() != 0)
    {
        ;
    }
}
// TODO: init overload. Add member to Process to hold init function.
ProcessId& spawn(messageMap_t msgmap)
{
    Process* process = new Process(msgmap);
	insertProcess(process);
    g_manager.m_numProcesses++;
	
    std::thread thr(run, process, &g_manager, process->m_Pid.m_index);
	thr.detach();
    return process->m_Pid;
}

ProcessId& spawn(initFunc_t initfunc, messageMap_t msgmap)
{
    Process* process = new Process(msgmap);
    process->m_initFunc = initfunc;
	insertProcess(process);
    g_manager.m_numProcesses++;
	
    std::thread thr(run, process, &g_manager, process->m_Pid.m_index);
	thr.detach();
    return process->m_Pid;

}

} // namespace proc

#endif // PROCESS_H_
