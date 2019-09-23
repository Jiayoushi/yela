#ifndef YELA_CMD_H_
#define YELA_CMD_H_

#include <string>

#include "global.h"

namespace yela {

struct Arguments {
  std::string settings_file;
};

Arguments GetCommandLine(int argc, char *argv[]);

}

#endif
