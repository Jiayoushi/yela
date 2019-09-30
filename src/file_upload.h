#ifndef YELA_FILE_UPLOAD_H_
#define YELA_FILE_UPLOAD_H_

#include <iomanip>

#include "global.h"
#include "log.h"
#include "base/stoppable.h"

namespace yela {

struct FileInfo {
  std::string name;

  // These are sha1 hash in hex format
  std::string metafile; // concatenated sha1 of the metafile
  std::string metafile_sha1; // sha1 of the metafile

  size_t size;
  // TODO: should not be in the program's memory
  std::vector<std::string> content; // The file is in multiple blocks

  FileInfo():
    size(0) {}
};

class UploadManager {
 public:
  UploadManager();
  ~UploadManager();

  // Upload a file and returns the sha1 of the file's metafile
  std::string Upload(const std::string &input);
  int SearchFile(std::string meta_hash);
  void RegisterNetwork(std::shared_ptr<Network> network);
 private:
  const int kBlockSize = 8192;

  void GetSha1(const void *content, size_t size, unsigned char *md);
  std::string Sha1ToString(const std::basic_string<unsigned char> sha1);

  std::vector<FileInfo> files_;

  std::shared_ptr<Network> network_;
};

}

#endif
