#include "strategy.h"

Strategy::Strategy(){
    
}

void Strategy::bind(Context* context, Scheduler* scheduler){
    this->context = context;
    this->scheduler = scheduler;
}

void Strategy::decide(){
    if(context == nullptr)
        return;
    // 将tasks中的task加入就绪队列
    App* app = context->getAppHead();
    while(app){
        TaskInfo* taskInfo = app->task_info_struct;
        while(taskInfo){
            Task* task = taskInfo->task_struct;
            while(task && task->task_status == UNDEPLOYED){
                uint16_t app_id = app->app_id;
                uint16_t task_info_id = taskInfo->task_info_id;
                uint16_t task_id = task->task_id;
                std::pair<uint16_t, uint16_t> index = std::make_pair(app_id, task_id);
                TaskNode taskNode(app_id, task_info_id, task_id);
                waitingQs[index].push_back(taskNode);
                task->task_status = WAITING;
                task = task->previous;
            }
            taskInfo = taskInfo->next;
        }
        app = app->next;
    }

    // 触发一次调度
    if(scheduler == nullptr)
        return;
    scheduler->dispatch();
}

Strategy::~Strategy(){

}

bool isReady(TaskNode& taskNode){

}

int Strategy::getWaitingQueuesNum(){
    return waitingQs.size();
}

std::unordered_map<std::pair<uint16_t,uint16_t>, std::deque<TaskNode>, pair_hash>& Strategy::getWaitingQueue(){
    return waitingQs;
}

std::string Strategy::getWaitingQueuesDetail(){
    std::string res = "";
    for(auto item : waitingQs){
        res += "&nbsp;&nbsp;&nbsp;";
        res += "app_id: " + std::to_string(item.first.first);
        res += " task_id: " + std::to_string(item.first.second);
        res += ":&nbsp;&nbsp;&nbsp;";
        res += "0----";
        for(std::deque<TaskNode>::iterator iter = item.second.begin(); iter != item.second.end(); iter++){
            res += std::to_string(iter->task_info_id);
            res += "----";
        }
        res += std::to_string(item.second.size()+1);
        res += "<br>";
    }
    return res;
}
