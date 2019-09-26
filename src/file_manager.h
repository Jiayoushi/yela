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
#include "base/stoppable.h"

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

class FileManager: public Stoppable {
 public:
  FileManager();
  ~FileManager();

  // Register certain components
  void RegisterNetwork(std::shared_ptr<Network> network);

  // Requests from local user
  void Download(const std::string &filename);
  void Search(const std::string &filename);
  int Upload(const std::string &filename);

  // Handle requests from remote user
  void HandleBlockRequest(const Message &msg);
  void HandleBlockReply(const Message &msg);
  void HandleSearchRequest(const Message &msg);
  void HandleSearchReply(const Message &msg);

  void Run();
 private:
  const int kTotalBudgetPerFile = 100;
  const int kBudgetPerMessage = 2;
  const int kBlockSize = 8192;

  void GetSha1(const void *content, size_t size, unsigned char *md);
  std::string Sha1ToString(const ustring &sha1);
  
  // All messages that needs to be sent
  struct Task {
    Message msg;
    int budget; // Only used for file searching

    Task(const Message &m):
      msg(m), budget(0) {

    }
    Task(const Message &m, int b):
      msg(m), budget(b) {}
  };
  void PushTask(const Task &msg);
  std::list<Task> tasks_;
  std::mutex tasks_mutex_;
  std::condition_variable tasks_cond_;
  const int kMessageSendingGapInMs = 1000;


  // File Downloading
  const int kDownloadFileHopLimit = 10;

  // Files in memory, TODO: probably a bad idea
  std::vector<FileInfo> files_;
  std::shared_ptr<Network> network_;
};

}

#endif
