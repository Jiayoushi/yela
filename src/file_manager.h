#ifndef YELA_FILE_MANAGER_H_
#define YELA_FILE_MANAGER_H_

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <thread>
#include <condition_variable>

#include "network.h"
#include "message.h"
#include "log.h"

namespace yela {

typedef std::basic_string<unsigned char> ustring;

struct FileInfo {
  std::string name;

  ustring content_sha1;
  ustring meta_sha1;

  size_t size;
  std::vector<std::string> content;

  FileInfo():
    size(0) {

  }
};

class FileManager {
 public:
  FileManager(std::shared_ptr<Network> network);
  ~FileManager();

  // Requests from local user
  void Download(const std::string &filename);
  void Search(const std::string &filename);
  int Upload(const std::string &filename);

  // Handle requests from remote user
  void HandleBlockRequest(const Message &msg);
  void HandleBlockReply(const Message &msg);
  void HandleSearchRequest(const Message &msg);
  void HandleSearchReply(const Message &msg);
 private:
  const int kTotalBudgetPerFile = 100;
  const int kBudgetPerMessage = 2;
  const int kBlockSize = 8192;

  void GetSha1(const void *content, size_t size, unsigned char *md);
  std::string Sha1ToString(const ustring &sha1);
  
  // All messages that needs to be sent
  struct Task {
    Message msg;
    int budget;

    Task(const Message &m, int b):
      msg(m), budget(b) {}
  };
  std::list<Task> tasks_;
  std::mutex tasks_mutex_;
  std::condition_variable tasks_cond_;
  const int kMessageSendingGapInMs = 1000;

  // A thread that sends messages
  bool sending_;
  void Sending();
  std::thread sending_thread_;

  std::vector<FileInfo> files_;
  std::shared_ptr<Network> network_;
};

}

#endif
