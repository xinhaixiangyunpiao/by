#ifndef CONTEXT_H
#define CONTEXT_H

#include "common.h"
#include "strategy.h"
#include "webServer.h"
#include <jsoncpp/json/json.h>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <string.h>

/**
 *  Context
 *  Node -> Node -> Node
 *  Node -> Cpu - Cpu - Cpu
 *       -> Gpu - Gpu - Gpu
 *       -> Memory
 *  App -> App -> App
 *  App -> TaskInfo
 *            |
 *      -> TaskInfo(管理Code)
 *            |
 *      -> TaskInfo
 *  TaskInfo -> Task
 *               |
 *           -> Task(管理Input)
 *               |
 *           -> Task
 * **/

// 提前声明，防止编译不通过
struct Code;
struct Data;
struct CpuInfo;
struct GpuInfo;
struct MemoryInfo;
struct Node;
struct Task;
struct TaskInfo;
struct App;
struct Record;
struct pair_hash;

class Strategy;

typedef struct Code{
    char *context;        // 存储代码体，最长不超过MAX_CODE_BYTES字节
    uint32_t code_len;    // 数据长度，字节数
} Code; // 代码体

typedef struct Data{
    byte* context; // 数据首地址
    uint32_t data_len;  // 数据长度，字节数
} Data; // 输入数据

typedef struct CpuInfo{
    uint8_t type;          // CPU type
    uint16_t cores_num;    // 逻辑核数
    uint16_t cores_avail;  // 当前可用逻辑核数
    CpuInfo* next;         // 下一个CPU
} CpuInfo;

typedef struct MemoryInfo{
    uint16_t mem_total;            // 总的内存大小
    uint16_t mem_for_deploy;       // 部署空间的内存大小
    uint16_t mem_for_run;          // 运行空间的内存大小
    uint16_t mem_avail;            // 当前可用内存大小
    uint16_t mem_for_deploy_avail; // 部署空间当前可用内存大小
    uint16_t mem_for_run_avail;    // 运行空间当前可用内存大小
} MemoryInfo;

typedef struct GpuInfo{
    uint8_t type;          // GPU type
    MemoryInfo memory;     // GPU memory
    GpuInfo* next;         // 下一块GPU
} GpuInfo;

typedef struct Node{
    uint16_t node_id;                  // 节点的id
    uint16_t cpus_num;                 // 总的cpu数量
    uint16_t gpus_num;                 // 总的gpu数量
    MemoryInfo memory;                 // 内存情况
    CpuInfo* cpu_struct;               // CPU首节点
    GpuInfo* gpu_struct;               // GPU首节点
    uint16_t tasks_num;                // 在node上部署或运行的task的总数
    std::unordered_map<std::pair<uint16_t, uint16_t>, std::vector<Task*>, pair_hash>* tasks_entry; // Task描述符表,使用app_id和task_info_id做索引
    Node* next;                        // 下一个Node节点
} Node;

typedef struct Task{
    uint16_t task_id;                    // Task的id,实例化序号,0,1,2,3...
    uint32_t input_data_size;            // input data的大小
    uint8_t task_status;                 // task的状态
    uint16_t node_id;                    // 部署或运行在了哪个node上，如果还未部署或运行，此值表示上报data的节点
    uint64_t timestamp_deplay_start;     // task开始部署的时间戳，还未开始时此值为0
    uint64_t timestamp_run_start;        // task开始执行的时间戳，还未开始时此值为0
    uint64_t timestamp_deploy_end;       // task结束部署的时间戳，还未结束时此值为0
    uint64_t timestamp_run_end;          // task结束运行的时间戳，还未结束时此值为0
    TaskInfo* task_info_ptr;             // 指向task_info
    Data* input;                         // 指向输入数据
    Task* previous;                      // 上一个Task节点
    Task* next;                          // 下一个Task节点
} Task;

typedef struct TaskInfo{
    uint16_t task_info_id;                         // TaskInfo的id,在DAG图中的序号
    uint16_t demond_cpus_num;                      // 要求cpu核数
    uint16_t demond_gpus_num;                      // 要求gpu个数
    uint16_t demond_deploy_memory;                 // 要求的部署内存
    uint16_t demond_deploy_gpu_memory;             // 要求的部署显存
    uint16_t demond_run_memory;                    // 要求的运行内存
    uint16_t demond_run_gpu_memory;                // 要求的运行显存
    uint16_t input_data_size;                      // 如果此值不为0，代表此task_info输入数据固定
    uint8_t task_type;                             // TASK的类型
    Code* code;                                    // 指向代码段
    std::unordered_map<uint16_t, Task*>* tasks_entry;            // 指向TaskInfo生成的所有task
    std::unordered_map<uint16_t, double>* predict_deploy_time;   // 以节点id为索引，保存当前任务在每个node上的预测部署时间
    std::unordered_map<uint16_t, double>* predict_run_time;      // 以节点id为索引，保存当前任务在每个node上的预测运行时间
    uint16_t request_has_gen;                      // 已经生成的task数目
    Task* task_struct;                             // 最新的task
    App* app_ptr;                                  // 指向的App
    TaskInfo* next;                                // 下一个TaskInfo节点
} TaskInfo; // 存储节点的动态消息

typedef struct App{
    uint16_t app_id;                     // App的id
    int n;                               // 节点数目
    uint32_t request_total;              // 总的request数目
    std::vector<std::vector<uint16_t>>* dag;                    // 邻接矩阵，存储每个节点的前驱节点
    std::unordered_map<uint16_t, TaskInfo*>* task_infos_entry;  // 与App相关联的task_info入口地址，使用task_info_id做索引，对应DAG中的编号。
    TaskInfo* task_info_struct;          // task_info的头指针
    App* next;                           // 下一个App
} App; // 存储App信息，包含DAG图信息，以及当前已经产生的request总数。

typedef struct Record{
    uint64_t timestamp;
    uint16_t input_data_size;
    uint64_t cost_time;
} Record;

class Context{
private:
    Node* node_struct;                                // 第一个Node节点，后续会以链表相连，总的节点情况
    App* app_struct;                                  // 总的App情况，链表相连
    std::unordered_map<uint16_t, Node*> nodes_entry;  // 建立node_id到Node入口地址的索引
    std::unordered_map<uint16_t, App*> apps_entry;    // app_id到App的映射
    std::vector<std::vector<double>> bindWidth;       // 第i号节点到第j号节点带宽
    std::map<std::pair<uint16_t,std::pair<uint16_t, uint16_t>>, std::vector<Record>> history_deploy_record; // 以app_id，task_info_id，node_id为索引的部署记录集合
    std::map<std::pair<uint16_t,std::pair<uint16_t, uint16_t>>, std::vector<Record>> history_run_record;    // 以app_id，task_info_id，node_id为索引的运行记录集合
    Strategy* strategy;

    void app_dag_build(std::vector<std::vector<uint16_t>>* dag, Json::Value root);
    void head_taskInfo_build(App* app, TaskInfo* taskInfo);
    void tail_taskInfo_build(App* app, TaskInfo* taskInfo, uint16_t n);

public:
    Context();
    ~Context();
    void bind(Strategy* strategy);
    void update(Event* event);
    void updateContext(Event* event);
    Node* getNodeHead();
    App* getAppHead();
    std::unordered_map<uint16_t, Node*>& getNodesMap();
    std::unordered_map<uint16_t, App*>& getAppsMap();
    Task* getTask(uint16_t app_id, uint16_t task_info_id, uint16_t task_id);
    Task* getTask(TaskNode& taskNode);
    bool hasExecuteEnv(Node* node, uint16_t app_id, uint16_t task_info_id);
    bool hasExecuteEnv(Node* node, Task* task);
    int getAppsNum();
    int getTaskInfosNum();
    int getNodesNum();
    int getTasksNum();
    std::string getTasksDetail();
};

#endif