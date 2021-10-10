#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "context.h"
#include "strategy.h"
#include <queue>
#include <algorithm>
#include <cfloat>

struct TaskNode;

class Context;
class Strategy;

class Scheduler{
private:
    Context* context;
    Strategy* strategy;
    std::priority_queue<TaskNode> readyQ; 

public:
    Scheduler();
    ~Scheduler();
    void bind(Context* context, Strategy* strategy);
    bool isReady(TaskNode& taskNode);
    bool isSatified(Node* node, Task* task);
    double calEST(Node* node, Task* task);
    double calEFT(Node* node, Task* task);
    void dispatch();
    std::string getReadyQueueTop();
    int getReadyQueueSize();
};

#endif