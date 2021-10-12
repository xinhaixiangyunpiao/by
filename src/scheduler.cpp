#include "scheduler.h"

Scheduler::Scheduler(){

}

void Scheduler::bind(Context* context, Strategy* strategy){
    this->context = context;
    this->strategy = strategy;
}

bool Scheduler::isReady(TaskNode& taskNode){
    // 如果是首节点，直接返回true
    Task* task = context->getTask(taskNode.app_id, taskNode.task_info_id, taskNode.task_id);
    if(task->task_id == 0)
        return true;
    // 如果是中间节点，需要有依赖关系的前驱节点执行完毕后返回true，否则返回false
    App* app = task->task_info_ptr->app_ptr;
    for(auto i : *(app->dag)){
        for(auto j : i){
            if(context->getTask(taskNode.app_id, j, taskNode.task_id)->task_status != FINISHED)
                return false;
        }
    }
    return true;
}

bool Scheduler::isSatified(Node* node, Task* task){
    // CPU核数，GPU个数，内存足够
    TaskInfo* taskInfo = task->task_info_ptr;
    if(taskInfo->cpu_occ_num > node->cpu_core_avail)
        return false;
    if(taskInfo->gpu_occ_num > node->gpu_num_avail)
        return false;
    if(taskInfo->peak_mem > node->mem_avail)
        return false;
    return true;
}

double Scheduler::calEST(Node* node, Task* task){
    double deployTime, createEnvTime, ModelTransTime;
    // 部署时间（执行环境开辟时间 + 模型传输时间）
    if(context->hasExecuteEnv(node, task)){
        deployTime = 0;
        createEnvTime = 0;
        ModelTransTime = 0;
    }
    // 数据就位时间（上一节点的输入数据以及状态表的传输时间）
    
    // 取最小值
    
}

double Scheduler::calEFT(Node* node, Task* task){
    double EST = calEST(node, task);
    // 计算理论运行时间

    // 获取预测运行时间

    // 加权求得实际预测运行时间

    // 与EST相加返回

}

void Scheduler::dispatch(){
    // 将等待队列组中所有准备好的队头取出，放入ready队列，同时将相应任务的状态置为READY
    for(auto & item : strategy->getWaitingQueue()){
        std::deque<TaskNode>& q = item.second;
        TaskNode currentTask = q.front();
        if(isReady(currentTask)){
            readyQ.push(currentTask);
            Task* task = context->getTask(currentTask.app_id, currentTask.task_info_id, currentTask.task_id);
            task->task_status = READY;
            q.pop_front();
        }
    }
    // 将ready队列队顶元素取出，
    if(readyQ.size() != 0){
        TaskNode targetTask = readyQ.top();
        // 生成放在每个Node上的执行计划判断时间，选取。
        Node* node = context->getNodeHead();
        Task* task = context->getTask(targetTask);
        int targetNodeId = -1;
        double minEFT = DBL_MAX;
        while(node){
            if(isSatified(node, task)){
                double EFT = calEFT(node, task);
                if(EFT < minEFT){
                    minEFT = EFT;
                    targetNodeId = node->node_id;
                }
            }
            node = node->next;
        }
        // 修改task的状态，分发任务到节点。
        if(targetNodeId == -1)
            return ;
        
        // 将节点从队列移除。
        
    }
}

int Scheduler::getReadyQueueSize(){
    return readyQ.size();
}

std::string Scheduler::getReadyQueueTop(){
    std::string res = "";
    TaskNode taskNode = readyQ.top();
    res += "&nbsp;&nbsp;&nbsp;";
    res += "app_id: " + std::to_string(taskNode.app_id);
    res += " task_info_id: " + std::to_string(taskNode.task_info_id);
    res += " task_id: " + std::to_string(taskNode.task_id);
    res += " priority: " + std::to_string(taskNode.priority);
    res += "<br>";
    return res;
}

Scheduler::~Scheduler(){

}