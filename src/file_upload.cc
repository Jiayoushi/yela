#include "file_upload.h"

#include <openssl/sha.h>
#include <string>
#include <ios>
#include <cassert>
#include <sstream>

namespace yela {

UploadManager::UploadManager() {

}

UploadManager::~UploadManager() {

}

std::string UploadManager::Upload(const std::string &filename) {
  files_.push_back(FileInfo());

  FileInfo &file = files_.back();
  file.name = filename;

  std::basic_string<unsigned char> metafile;
  std::basic_string<unsigned char> metafile_sha1;

  std::ifstream fs(file.name, std::ios_base::binary);
  if (!fs.is_open()) {
    Log("Upload file failed: unable to open file \"" + file.name + "\"");
    return std::string();
  }

  for (int bytes_read = -1; bytes_read != 0; ) {
    unsigned char md[SHA_DIGEST_LENGTH];
    char buffer[kBlockSize];
    bytes_read = fs.readsome(buffer, kBlockSize);

    if (bytes_read == 0) {
      GetSha1(file.metafile.c_str(), file.metafile.size(), md);
      for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        metafile_sha1 += md[i];
      }
    } else {
      file.size += bytes_read;
      file.content.push_back("");
      for (int i = 0; i < bytes_read; ++i) {
        file.content.back() += buffer[i];
      }

      GetSha1(buffer, bytes_read, md);
      for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        metafile += md[i];
      }
    }
  }

  // Convert unsigned char to char in hex
  file.metafile_sha1 = Sha1ToString(metafile_sha1);
  file.metafile = Sha1ToString(metafile);

  Log("Upload file succeed: \"" + file.name + "\"  \n" +
      "meta sha1: \n" + file.metafile_sha1 + "\n" +
      "content sha1: \n" + file.metafile + "\n" +
      "file size: \n" + std::to_string(file.size) + "\n");

  return file.metafile;
}

std::string UploadManager::Sha1ToString(const std::basic_string<unsigned char> sha1) {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (int i = 0; i < sha1.size() / 20; ++i) {
    for (int j = 0; j < 20; ++j) {
      ss << std::setw(2) << (int)(sha1[i * 20 + j]);
    }
  }
  return ss.str();
}

int UploadManager::HasFile(const std::string &sha1) {
  Log("Files[0] metafile " + files_[0].metafile + ";");
  for (int i = 0; i < files_.size(); ++i) {
    if (sha1 == files_[i].metafile) {
      return i;
    }
  }

  return -1;
}

const FileInfo &UploadManager::GetFile(unsigned int index) {
  return files_[index];
}

void UploadManager::GetSha1(const void *content, size_t size,
                        unsigned char *md) {
  SHA_CTX ctx;
  if (SHA1_Init(&ctx) == 0) {
    Log("Upload file failed: unable to init sha ctx");
    return;
  }

  if (SHA1_Update(&ctx, content, size) == 0) {
    Log("Upload file failed: SHA1_Update failed");
    return;
  }
  if (SHA1_Final(md, &ctx) == 0) {
    Log("Upload file failed: SHA1_Final failed");
    return;
  }
}

void UploadManager::RegisterNetwork(std::shared_ptr<Network> network) {
  network_ = network;
}

bool UploadManager::CheckSha1(const std::string &data, const std::string &target_sha1) {
  unsigned char md[SHA_DIGEST_LENGTH];
  GetSha1(data.c_str(), data.size(), md); 

  std::basic_string<unsigned char> sha1;
  for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
    sha1 += md[i];
  }

  std::string hex_sha1 = Sha1ToString(sha1);

  return hex_sha1 == target_sha1;
}

}
