#include "log.h"

namespace yela {

std::string filename;
std::ofstream out_file;
int log_port;

void InitLog(int port) {
  log_port = port;
  filename = std::to_string(port) + "_log.txt";
  out_file.open(filename);
  if (out_file.is_open() == false) {
    std::cerr << "Failed to init log file." << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Log(const std::string &msg) {
  out_file << log_port << ": " << msg << std::endl;
}

void CloseLog() {
  out_file.close();
}

}
