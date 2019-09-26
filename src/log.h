#ifndef YELA_LOG_H_
#define YELA_LOG_H_

#include <string>
#include <iostream>
#include <fstream>

#include "message.h"

namespace yela {

void InitLog(const std::string &id);
void Log(const std::string &target_ip, const std::string &target_id, const std::string &target_port, 
         const Message &msg);
void Log(const std::string &msg);
void CloseLog();

}

#endif
