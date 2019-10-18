#ifndef YELA_FILE_DOWNLOAD_H_
#define YELA_FILE_DOWNLOAD_H_

#include "../base/stoppable.h"
#include "file_upload.h"

namespace yela {

class DownloadManager: public Stoppable {
 public:
  DownloadManager();
  ~DownloadManager();

  void Run();
  void Download(const std::string &input);
  void HandleBlockRequest(const Message &msg);
  void HandleBlockReply(const Message &msg);
  void RegisterNetwork(std::shared_ptr<Network> network);
  void RegisterUploadManager(std::shared_ptr<UploadManager> upload);
 private:
  void RelayMessage(const Message &msg);
  void WriteToFile(const std::string &file_content, const std::string &filename);

  const int kBlockRequestHopLimit = 10;
  const int kBlockReplyHopLimit = 10;

  std::shared_ptr<Network> network_;
  std::shared_ptr<UploadManager> upload_manager_;
};

}

#endif
