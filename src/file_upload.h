#ifndef YELA_FILE_UPLOAD_H_
#define YELA_FILE_UPLOAD_H_

#include <iomanip>

#include "global.h"
#include "log.h"
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
    size(0) {}
};

class UploadManager {
 public:
  UploadManager();
  ~UploadManager();

  int Upload(const std::string &input);
  int SearchFile(ustring meta_hash);
  void RegisterNetwork(std::shared_ptr<Network> network);
 private:
  const int kBlockSize = 8192;

  void GetSha1(const void *content, size_t size, unsigned char *md);
  std::string Sha1ToString(const ustring &sha1);

  std::vector<FileInfo> files_;

  std::shared_ptr<Network> network_;
};

}

#endif
