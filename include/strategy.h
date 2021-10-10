#ifndef STRATEGY_H
#define STRATEGY_H

#include "context.h"
#include "scheduler.h"
#include <queue>
#include <unordered_map>

class Context;
class Scheduler;

typedef struct TaskNode{
    uint16_t app_id;
    uint16_t task_info_id;
    uint16_t task_id;
    uint16_t priority;
    TaskNode(uint16_t app_id, uint16_t task_info_id, uint16_t task_id){
        this->app_id = app_id;
        this->task_info_id = task_info_id;
        this->task_id = task_id;
        this->priority = 0;
    }
    bool operator < (const TaskNode& other) const{
        return this->priority < other.priority;
}
} TaskNode;

struct pair_hash{
    template<class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& p) const{
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

class Strategy{
private:
    Context* context;
    Scheduler* scheduler;
    std::unordered_map<std::pair<uint16_t,uint16_t>, std::deque<TaskNode>, pair_hash> waitingQs; // 等待队列集合

public:
    Strategy();
    ~Strategy();
    void bind(Context* context, Scheduler* scheduler);
    void decide();
    int getWaitingQueuesNum();
    std::string getWaitingQueuesDetail();
    std::unordered_map<std::pair<uint16_t,uint16_t>, std::deque<TaskNode>, pair_hash>& getWaitingQueue();
};

#endif