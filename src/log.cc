#include "log.h"

namespace yela {

std::string filename;
std::ofstream out_file;
std::string id;

void InitLog(const std::string &i) {
  id = i;
  filename = id + "_log.txt";
  out_file.open(filename);
  if (out_file.is_open() == false) {
    std::cerr << "Failed to init log file." << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Log(const std::string &msg) {
  out_file << id << ": " << msg << std::endl;
}

void CloseLog() {
  out_file.close();
}

}
