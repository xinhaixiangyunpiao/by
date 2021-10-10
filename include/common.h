#ifndef COMMON_H
#define COMMON_H 

#include <string>

// 类型定义
typedef unsigned char byte;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

// 表示类型
#define TASK_STATUS 0x01

// task type
#define NORMAL_TASK 0x00                  // 普通task
#define STRONG_TIME_DEPENDDENPY_TASK 0x01 // 强时间依赖任务
#define START_OR_END_TASK 0x02            // 开始或结束任务
#define LONG_LOAD_TIME_TASK 0x04          // 长装载时间任务
#define STATEFUL_TASK 0x08                // 有状态任务

// CPU_TYPE
#define I5_8260 0x01
#define I7_6500 0x02

// GPU_TYPE
#define GTX_2080Ti 0x01
#define Tesla_V100 0x02

// TASK_STATUS
#define UNDEPLOYED 0x00
#define WAITING 0x01
#define READY 0x02
#define DEPLOYING 0x03
#define DEPLOYED 0x04
#define RUNNING 0x05
#define FINISHED 0x06

// EVENT_TYPE
#define DATA_SUBMIT_EVENT 0x01
#define NODE_INFO_EVENT 0x02
#define TASK_INFO_EVENT 0x03

typedef struct Event{
    uint8_t eventType;
    uint16_t app_id;
    uint16_t node_id;
    byte* data;
    uint32_t data_len;
    Event(uint8_t eventType, uint16_t app_id, uint16_t node_id, byte* data, uint32_t data_len){
        this->eventType = eventType;
        this->app_id = app_id;
        this->node_id = node_id;
        this->data = data;
        this->data_len = data_len;
    }
} Event;

namespace util{
    uint8_t convert(std::string s);
    std::string to_string(uint8_t type, uint8_t value);
}

#endif