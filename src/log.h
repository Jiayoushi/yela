#ifndef YELA_LOG_H_
#define YELA_LOG_H_

#include <string>
#include <iostream>
#include <fstream>

namespace yela {

void InitLog(const std::string &id);
void Log(const std::string &msg);
void CloseLog();

}

#endif
