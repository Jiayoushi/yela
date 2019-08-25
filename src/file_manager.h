#ifndef YELA_FILE_MANAGER_H_
#define YELA_FILE_MANAGER_H_

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

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
  FileManager();
  void Upload(const std::string &filename); 
 private:
  void GetSha1(const void *content, size_t size, unsigned char *md);
  std::string Sha1ToString(const ustring &sha1);

  const int kBlockSize;
  std::vector<FileInfo> files_;
};

}

#endif
