#include "context.h"

void Context::app_dag_build(std::vector<std::vector<uint16_t>>* dag, Json::Value root){
    std::string key;
    Json::Value::Members members = root.getMemberNames();
    for (Json::Value::Members::iterator iter = members.begin(); iter != members.end(); iter++)   // 遍历每个key
    {
        key = *iter;
        for(int i = 0; i < root[key].size(); i++)
            (*dag)[atoi(key.c_str())].push_back(root[key][i].asInt());
    }
    (*(dag))[1].push_back(0);
    (*(dag))[atoi(key.c_str())+1].push_back(atoi(key.c_str()));
}   

void Context::head_taskInfo_build(App* app, TaskInfo* taskInfo){
    taskInfo->task_info_id = 0;
    taskInfo->demond_cpus_num = 1;
    taskInfo->demond_gpus_num = 0;
    taskInfo->demond_deploy_memory = 0;
    taskInfo->demond_deploy_gpu_memory = 0;
    taskInfo->demond_run_memory = 0;
    taskInfo->demond_run_gpu_memory = 0;
    taskInfo->input_data_size = 0;
    taskInfo->task_type = START_OR_END_TASK;
    taskInfo->code = nullptr;
    taskInfo->tasks_entry = new std::unordered_map<uint16_t, Task*>();
    taskInfo->predict_deploy_time = nullptr;
    taskInfo->predict_run_time = nullptr;
    taskInfo->request_has_gen = 0;
    taskInfo->task_struct = nullptr;
    taskInfo->app_ptr = app;
    taskInfo->next = nullptr;
}

void Context::tail_taskInfo_build(App* app, TaskInfo* taskInfo, uint16_t n){
    taskInfo->task_info_id = n;
    taskInfo->demond_cpus_num = 1;
    taskInfo->demond_gpus_num = 0;
    taskInfo->demond_deploy_memory = 0;
    taskInfo->demond_deploy_gpu_memory = 0;
    taskInfo->demond_run_memory = 0;
    taskInfo->demond_run_gpu_memory = 0;
    taskInfo->input_data_size = 0;
    taskInfo->task_type = START_OR_END_TASK;
    taskInfo->code = nullptr;
    taskInfo->tasks_entry = new std::unordered_map<uint16_t, Task*>();
    taskInfo->predict_deploy_time = nullptr;
    taskInfo->predict_run_time = nullptr;
    taskInfo->request_has_gen = 0;
    taskInfo->task_struct = nullptr;
    taskInfo->app_ptr = app;
    taskInfo->next = nullptr;
}

Context::Context(){  // 新建taskInfo和Node结构体，建立索引。
    // 由node.json建立Node数据结构
    std::ifstream fin("../config/nodes.json", std::ios::binary);
    if(!fin.is_open()){
        std::cout << "open nodes json file failed!" << std::endl;
        return ;
    }
    Json::Reader reader;
    Json::Value root;
    std::cout << "Building Nodes Information." << std::endl;
    if(reader.parse(fin, root)){
        int n = root["number"].asInt();
        Json::Value nodes = root["context"];
        Node* head = new Node();
        Node* pre = head;
        for(int i = 0; i < n; i++){
            Node* node = new Node();
            float mem_for_cpu_deploy_percent = nodes[i]["mem_for_cpu_deploy_percent"].asFloat();
            float mem_for_gpu_deploy_percent = nodes[i]["mem_for_gpu_deploy_percent"].asFloat();
            node->node_id = nodes[i]["id"].asInt();
            node->cpus_num = 1;
            node->gpus_num = nodes[i]["gpu_num"].asInt();
            (node->memory).mem_total = nodes[i]["mem_total"].asInt();
            (node->memory).mem_for_deploy = (node->memory).mem_total*mem_for_cpu_deploy_percent;
            (node->memory).mem_for_run = (node->memory).mem_total - (node->memory).mem_for_deploy;
            (node->memory).mem_avail = (node->memory).mem_total;
            (node->memory).mem_for_deploy_avail = (node->memory).mem_for_deploy;
            (node->memory).mem_for_run_avail = (node->memory).mem_for_run_avail;
            CpuInfo* cpu = new CpuInfo();
            cpu->type = util::convert(nodes[i]["cpu_type"].asString());
            cpu->cores_num = nodes[i]["cpu_core_num"].asInt();
            cpu->cores_avail = cpu->cores_num;
            cpu->next = nullptr;
            node->cpu_struct = cpu;
            GpuInfo* gpu_dummy = new GpuInfo();
            GpuInfo* gpu_cur = gpu_dummy;
            for(int i = 0; i < node->gpus_num; i++){
                GpuInfo* gpu = new GpuInfo();
                gpu->type = util::convert(nodes[i]["gpu_type"].asString());
                (gpu->memory).mem_total = nodes[i]["gpu_mem_every"].asInt();
                (gpu->memory).mem_for_deploy = (gpu->memory).mem_total*mem_for_gpu_deploy_percent;
                (gpu->memory).mem_for_run = (gpu->memory).mem_total - (gpu->memory).mem_for_deploy;
                (gpu->memory).mem_avail = (gpu->memory).mem_total;
                (gpu->memory).mem_for_deploy_avail = (gpu->memory).mem_for_deploy;
                (gpu->memory).mem_for_run_avail = (gpu->memory).mem_for_run_avail;
                gpu_cur->next = gpu;
                gpu_cur = gpu_cur->next;
            }
            gpu_cur->next = nullptr;
            node->gpu_struct = gpu_dummy->next;
            node->tasks_num = 0;
            node->tasks_entry = nullptr;
            pre->next = node;
            pre = pre->next;
            nodes_entry[node->node_id] = node;
        }
        node_struct = head->next;
    }else{
        std::cout << "parse node file failed. " << std::endl;
        return ;
    }
    fin.close();
    // 由app.json建立App数据结构
    fin.open("../config/apps.json", std::ios::binary);
    if(!fin.is_open()){
        std::cout << "open apps json file failed!" << std::endl;
        return ;
    }
    std::cout << "Building Apps Information." << std::endl;
    if(reader.parse(fin, root)){
        int n = root["number"].asInt();
        Json::Value apps = root["context"];
        App* head = new App();
        App* pre = head;
        for(int i = 0; i < n; i++){
            App* app = new App();
            app->app_id = i;
            app->n = apps[i]["task_number"].asInt()+2;
            app->request_total = apps[i]["request_total"].asInt();
            std::vector<std::vector<uint16_t>>* dag = new std::vector<std::vector<uint16_t>>(app->n, std::vector<uint16_t>());
            app->dag = dag;
            // 构造app的DAG
            app_dag_build(dag, apps[i]["dag"]);
            TaskInfo* head_taskInfo = new TaskInfo();
            // 构造头结点的TaskInfo
            head_taskInfo_build(app, head_taskInfo);
            app->task_infos_entry = new std::unordered_map<uint16_t, TaskInfo*>();
            (*(app->task_infos_entry))[head_taskInfo->task_info_id] = head_taskInfo;
            // 构造中间结点的TaskInfo
            Json::Value tasks = apps[i]["tasks"];
            TaskInfo* cur = head_taskInfo;
            uint16_t taskInfo_id = 0;
            for(int i = 0; i < tasks.size(); i++){
                TaskInfo* taskInfo = new TaskInfo();
                taskInfo_id = tasks[i]["id"].asInt();
                taskInfo->task_info_id = taskInfo_id;
                taskInfo->demond_cpus_num = tasks[i]["cpu_cores_num"].asInt();
                taskInfo->demond_gpus_num = tasks[i]["gpus_num"].asInt();
                taskInfo->demond_deploy_memory = tasks[i]["deploy_peak_mem"].asInt();
                taskInfo->demond_deploy_gpu_memory = tasks[i]["deploy_peak_gpu_mem"].asInt();
                taskInfo->demond_run_memory = tasks[i]["run_peak_mem"].asInt();
                taskInfo->demond_run_gpu_memory = tasks[i]["run_peak_gpu_mem"].asInt();
                taskInfo->task_type = util::convert(tasks[i]["task_type"].asString());
                Code* code = new Code();
                code->context = (char*)tasks[i]["code"].asString().data();
                code->code_len = strlen(code->context);
                taskInfo->code = code;
                taskInfo->tasks_entry = new std::unordered_map<uint16_t, Task*>();
                taskInfo->predict_deploy_time = new std::unordered_map<uint16_t, double>();
                taskInfo->predict_run_time = new std::unordered_map<uint16_t, double>();
                taskInfo->request_has_gen = 0;
                taskInfo->task_struct = nullptr;
                taskInfo->app_ptr = app;
                (*(app->task_infos_entry))[taskInfo->task_info_id] = taskInfo;
                cur->next = taskInfo;
                cur = cur->next;
            }
            // 构造尾结点的TaskInfo
            TaskInfo* tail_taskInfo = new TaskInfo();
            tail_taskInfo_build(app, tail_taskInfo, taskInfo_id+1);
            cur->next = tail_taskInfo;
            (*(app->task_infos_entry))[tail_taskInfo->task_info_id] = tail_taskInfo;
            app->task_info_struct = head_taskInfo;
            // 链接App链表
            pre->next = app;
            pre = pre->next;
            apps_entry[app->app_id] = app;
        }
        app_struct = head->next;
    }else{
        std::cout << "parse app file failed. " << std::endl;
    }
    fin.close();
    // 由bw.json初始化带宽情况
    fin.open("../config/bindwidth.mat", std::ios::in);
    if(!fin.is_open()){
        std::cout << "open bindwidth file failed." << std::endl;
        return;
    }
    std::cout << "Building Bindwidth Information." << std::endl;
    int n;
    fin >> n;
    for(int i = 0; i < n; i++){
        std::vector<double> line;
        for(int j = 0; j < n; j++){
            double value;
            fin >> value;
            line.push_back(value);
        }
        bindWidth.push_back(line);
    }
    fin.close();
}

Context::~Context(){ // 释放内存

}

void Context::bind(Strategy* strategy){
    this->strategy = strategy;
}

void Context::updateContext(Event* event){
    // 在相应的app和taskinfo下链入task结构体，并加入unordered_map索引，在task中链入输入数据
    uint16_t app_id = event->app_id;
    uint16_t node_id = event->node_id;
    uint32_t data_len = event->data_len;
    // 遍历所有的taskInfo
    TaskInfo* taskInfo = (apps_entry[app_id])->task_info_struct;
    while(taskInfo){
        // 新建task并链入
        Task* task = new Task();
        task->task_id = taskInfo->request_has_gen;
        taskInfo->request_has_gen += 1;
        if(taskInfo->task_info_id == 0)
            task->input_data_size = data_len;
        else
            task->input_data_size = taskInfo->input_data_size;
        task->node_id = node_id;
        task->task_status = UNDEPLOYED;
        task->timestamp_deplay_start = 0;
        task->timestamp_run_start = 0;
        task->timestamp_deploy_end = 0;
        task->timestamp_run_end = 0;
        task->task_info_ptr = taskInfo;
        if(taskInfo->task_info_id == 0){
            Data* data = new Data();
            data->context = event->data;
            data->data_len = data_len;
            task->input = data;
        }else
            task->input = nullptr;
        task->next = nullptr;
        if(taskInfo->task_struct == nullptr){
            task->previous = nullptr;
            taskInfo->task_struct = task;
        }else{
            task->previous = taskInfo->task_struct;
            taskInfo->task_struct->next = task;
            taskInfo->task_struct = task;
        }
        (*(taskInfo->tasks_entry))[task->task_id] = task;
        taskInfo = taskInfo->next;
    }
}

// 接收worker传输过来的数据，更新context，并进行一次decide
void Context::update(Event* event){
    if(event == nullptr)
        return;
    switch(event->eventType){
        case DATA_SUBMIT_EVENT:
            // 数据提交，触发调度
            updateContext(event);
            break;
        case NODE_INFO_EVENT:
            // 更新节点信息
            break;
        case TASK_INFO_EVENT:
            // TASK生命周期回传
            break;
    }
    // strategy做出决策
    strategy->decide();
}

Node* Context::getNodeHead(){
    return node_struct;
}

App* Context::getAppHead(){
    return app_struct;
}   

std::unordered_map<uint16_t, Node*>& Context::getNodesMap(){
    return nodes_entry;
}

std::unordered_map<uint16_t, App*>& Context::getAppsMap(){
    return apps_entry;
}

Task* Context::getTask(uint16_t app_id, uint16_t task_info_id, uint16_t task_id){
    return (*(((*((apps_entry[app_id])->task_infos_entry))[task_info_id])->tasks_entry))[task_id];
}

Task* Context::getTask(TaskNode& taskNode){
    return getTask(taskNode.app_id, taskNode.task_info_id, taskNode.task_id);
}

bool Context::hasExecuteEnv(Node* node, uint16_t app_id, uint16_t task_info_id){
    if((*(node->tasks_entry))[std::make_pair(app_id, task_info_id)].size() != 0)
        return true;
    else
        return false;
}

bool Context::hasExecuteEnv(Node* node, Task* task){
    return hasExecuteEnv(node, task->task_info_ptr->app_ptr->app_id, task->task_info_ptr->task_info_id);
}

int Context::getAppsNum(){
    return apps_entry.size();
}

int Context::getTaskInfosNum(){
    int res = 0;
    App* app = app_struct;
    while(app){
        res += app->n;
        app = app->next;
    }
    return res;
}

int Context::getNodesNum(){
    return nodes_entry.size();
}

int Context::getTasksNum(){
    int res = 0;
    App* app = app_struct;
    while(app){
        if((*(app->task_infos_entry)).size() == 0){
            app = app->next;
            continue;
        }
        TaskInfo* taskInfo = (*(app->task_infos_entry))[0];
        while(taskInfo){
            res += taskInfo->request_has_gen;
            taskInfo = taskInfo->next;
        }
        app = app->next;
    }
    return res;
}

std::string Context::getTasksDetail(){
    std::string res = "";
    App* app = app_struct;
    while(app){
        if((*(app->task_infos_entry)).size() == 0){
            app = app->next;
            continue;
        }
        TaskInfo* taskInfo = (*(app->task_infos_entry))[0];
        while(taskInfo){
            Task* task = taskInfo->task_struct;
            while(task){
                res += "&nbsp;&nbsp;&nbsp;app_id: " + std::to_string(task->task_info_ptr->app_ptr->app_id);
                res += " task_info_id: " + std::to_string(task->task_info_ptr->task_info_id);
                res += " task_id: " + std::to_string(task->task_id);
                res += " status: " + util::to_string(TASK_STATUS, task->task_status);
                res += "<br>";
                task = task->previous;
            }
            taskInfo = taskInfo->next;
        }
        app = app->next;
    }
    return res;
}

int main(){
    // 初始化server
    Context context;
    Strategy strategy;
    Scheduler scheduler;
    context.bind(&strategy);
    strategy.bind(&context, &scheduler);
    scheduler.bind(&context, &strategy);
    // 测试生成一个Event，context的处理
    byte bb[3] = {0x01,0x02,0x03};
    Event* event = new Event(DATA_SUBMIT_EVENT, 0, 1, bb, 3);
    context.update(event);
    // 开启web界面
    webServer webServer(&context, &strategy, &scheduler);
    return 0;
}