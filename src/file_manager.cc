#include "file_manager.h"

#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

namespace yela {

FileManager::FileManager():
  kBlockSize(8192) {

}

void FileManager::Search(const std::string &filename) {

}

void FileManager::HandleReply(const Message &msg) {

}

void FileManager::Download(const std::string &filename) {
  
}

int FileManager::Upload(const std::string &filename) {
  files_.push_back(FileInfo());

  FileInfo &file = files_.back();
  file.name = filename;
    
  std::ifstream fs(file.name, std::ios_base::binary);
  if (!fs.is_open()) {
    Log("Upload file failed: unable to open file \"" + file.name + "\"");
    return -1;
  }

  for (int bytes_read = -1; bytes_read != 0; ) {
    unsigned char md[SHA_DIGEST_LENGTH];
    char buffer[kBlockSize];
    bytes_read = fs.readsome(buffer, kBlockSize);
    
    if (bytes_read == 0) {
      GetSha1(file.content_sha1.c_str(), file.content_sha1.size(), md);
      for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        file.meta_sha1 += md[i];
      }
    } else {
      file.size += bytes_read;
      file.content.push_back("");
      for (int i = 0; i < bytes_read; ++i) {
        file.content.back() += buffer[i];
      }

      GetSha1(buffer, bytes_read, md);
      for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        file.content_sha1 += md[i];
      }
    }
  }

  Log("Upload file succeed: \"" + file.name + "\"  \n" + 
      "meta sha1: \n" + Sha1ToString(file.meta_sha1) + "\n" +
      "content sha1: \n" + Sha1ToString(file.content_sha1) + "\n" + 
      "file size: \n" + std::to_string(file.size) + "\n");

  return 0;
}

std::string FileManager::Sha1ToString(const ustring &sha1) {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (int i = 0; i < sha1.size() / 20; ++i) {
    for (int j = 0; j < 20; ++j) {
      ss << std::setw(2) << (int)sha1[i * 20 + j];
    }
    ss << std::endl;
  }
  return ss.str();
}

void FileManager::GetSha1(const void *content, size_t size, 
                        unsigned char *md) {
  SHA_CTX ctx;
  if (SHA1_Init(&ctx) == 0) {
    Log("Upload file failed: unable to init sha ctx");
    return;
  }

  if (SHA1_Update(&ctx, 
                  content, size) == 0) {
    Log("Upload file failed: SHA1_Update failed");
    return;
  }
  if (SHA1_Final(md, &ctx) == 0) {
    Log("Upload file failed: SHA1_Final failed");
    return;
  } 
}

}
