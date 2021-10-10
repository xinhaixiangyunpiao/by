#include "common.h"

namespace util{

    uint8_t convert(std::string s){
        if(s == "I5_8260"){
            return 0x01;
        }else if(s == "I7_6500"){
            return 0x02;
        }else if(s == "GTX_2080Ti"){
            return 0x01;
        }else if(s == "Tesla_V100"){
            return 0x02;
        }else if(s == "normal"){
            return 0x00;
        }else if(s == "long_load_time"){
            return 0x04;
        }else if(s == "stateful"){
            return 0x08;
        }
        return 0x00;
    }

    std::string to_string(uint8_t type, uint8_t value){
        switch(type){
            case TASK_STATUS:
                if(value == 0x00)
                    return "UNDEPLOYED";
                else if(value == 0x01)
                    return "WAITING";
                else if(value == 0x02)
                    return "READY";
                else if(value == 0x03)
                    return "DEPLOYING";
                else if(value == 0x04)
                    return "DEPLOYED";
                else if(value == 0x05)
                    return "RUNNING";
                else if(value == 0x06)
                    return "FINISHED";
                else
                    return "UNKNOW";
                break;
        }
    }
}