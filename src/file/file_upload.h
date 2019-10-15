#ifndef YELA_FILE_UPLOAD_H_
#define YELA_FILE_UPLOAD_H_

#include <iomanip>

#include "../global.h"
#include "../log.h"
#include "../base/stoppable.h"

namespace yela {

struct FileInfo {
  std::string name;

  // These are sha1 hash in hex format
  std::string metafile; // metafile of the content
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
  int HasFile(const std::string &sha1);
  const FileInfo &GetFile(unsigned int index);
  void RegisterNetwork(std::shared_ptr<Network> network);
  bool CheckSha1(const std::string &data, const std::string &target_sha1);
  int FindFileByName(const std::string &filename);
  std::vector<int> FindFileByKeywords(const std::vector<std::string> &keywords);
 private:
  std::string GetBaseFileName(const std::string &filename);

  const int kBlockSize = 8192;

  void GetSha1(const void *content, size_t size, unsigned char *md);
  std::string Sha1ToString(const std::basic_string<unsigned char> sha1);

  // TODO: make files as shared_ptr
  std::vector<FileInfo> files_;

  std::shared_ptr<Network> network_;
};

}

#endif
