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

void Log(const std::string &target_ip, const std::string &target_id, const std::string &target_port,
         const Message &msg) {
  // Log
  if (msg["type"] == kTypes[kRumor]) {
    Log("Send rumor to " + target_id +
        " (" + target_ip + "," + target_port + ") " +
        " \"" + msg["data"] + "\"");
  } else if (msg["type"] == kTypes[kStatus]) {
    std::string log_msg = "Send table to " + target_id +
      " (" + target_ip + "," + target_port + ") " +
      " [";
    log_msg += msg["seqtable"];
    if (log_msg.back() != '[') {
      log_msg.pop_back();
    }
    log_msg += "]";
    Log(log_msg);
  } else if (msg["type"] == kTypes[kBlockRequest]) {
    std::string log_msg = "Send block request to " + target_id +
    " with sha1 " + msg["blockrequest"];
    Log(log_msg);
  } else if (msg["type"] == kTypes[kBlockReply]) {
    std::string log_msg = "Send block reply to " + target_id +
    " with sha1 " + msg["blockreply"];
    Log(log_msg);  
  } else {

  }
}

void Log(const std::string &msg) {
  out_file << id << ": " << msg << std::endl;
}

void CloseLog() {
  out_file.close();
}

}
