#ifndef YELA_FILE_MANAGER_H_
#define YELA_FILE_MANAGER_H_

#include <vector>
#include <string>
#include <iostream>

#include "../network.h"
#include "../message.h"
#include "../base/stoppable.h"
#include "../interface.h"
#include "file_search.h"
#include "file_upload.h"
#include "file_download.h"

namespace yela {

class FileManager: public Stoppable {
 public:
  FileManager();
  ~FileManager();

  // Register certain components
  void RegisterNetwork(std::shared_ptr<Network> network);
  void RegisterInterface(std::shared_ptr<Interface> interface);

  // Requests from local user
  void Upload(const std::string &input);
  void Download(const std::string &input);
  void Search(const std::string &input);

  void HandleRemoteMessage(const Message &msg);
  void HandleLocalRequest(const Input &input);

  void Run();
 private:
  // Handle requests from remote user
  void HandleBlockRequest(const Message &msg);
  void HandleBlockReply(const Message &msg);
  void HandleSearchRequest(const Message &msg);
  void HandleSearchReply(const Message &msg);

  // File Uploading
  std::shared_ptr<UploadManager> upload_manager_;

  // File Downloading
  std::shared_ptr<DownloadManager> download_manager_;

  // File Searching
  std::shared_ptr<SearchManager> search_manager_;

  // Other components
  std::shared_ptr<Network> network_;
  std::shared_ptr<Interface> interface_;
};

}

#endif
