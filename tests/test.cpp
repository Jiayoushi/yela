#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <utility>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <queue>
#include <vector>

class Node {
 public:
  Node(int pid, int stdin_fd, const std::string &input_filename):
    pid_(pid), stdin_fd_(stdin_fd) {
    ReadFile(input_filename);
  }

  void ReadFile(const std::string &input_filename) {
    std::ifstream file(input_filename);

    std::string line;

    // The first line is ignored, it will be handled by the running instance
    // instead of test code here.
    std::getline(file, line);

    // Read messages
    while (std::getline(file, line)) {
      std::istringstream iss(line);
      messages_.push(line);
    }
  }  

  int GetPendingMessagesCount() {
    return messages_.size();
  }

  void SendOnePendingMessage() {
    std::string msg = messages_.front();
    messages_.pop();

    //std::cout << "TEST: send:" << msg << ";" << std::endl;

    int bytes_sent = 0;
    if ((bytes_sent = write(stdin_fd_, msg.c_str(), msg.size())) < 0) {
      perror("TEST: send pending message using write failed.");
    } else {
      //std::cout << "TEST: bytes_send: " << bytes_sent << std::endl;
    }
    //sleep(1);
  }

  int GetPid() {
    return pid_;
  }

 private:
  int pid_;
  int stdin_fd_;
  std::queue<std::string> messages_;
};

std::pair<int, int> InitiateNode(const char *argv[]) {
  int stdin_pipe[2];

  if (pipe(stdin_pipe) < 0) {
    perror("TEST: Pipe failed");
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();
  if (pid < 0) {
    perror("TEST: Fork failed");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    close(stdin_pipe[0]); // 0 read end
  } else {
    close(stdin_pipe[1]); // 1 write end
    dup2(stdin_pipe[0], STDIN_FILENO);
    
    close(STDOUT_FILENO);

    if (execl("./yela", "./yela", argv[0], argv[1], (char *)nullptr) < 0) {
      perror("TEST: failed to execve");
      std::cerr << errno << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  return std::make_pair(pid, stdin_pipe[1]);
}

void InitiateNodes(std::vector<Node> &nodes) {
  std::vector<std::vector<std::string>> ports = {{"5001", "5002,5004"}, {"5002", "5003,5004"}, {"5003", "5002"}, {"5004", "5001,5002"}};

  const char *argv_1[] = {ports[0][0].c_str(), ports[0][1].c_str()};
  std::pair<int, int> pid_stdin_pair_1 = InitiateNode(argv_1);
  nodes.push_back(Node(pid_stdin_pair_1.first, pid_stdin_pair_1.second, "procedures/host1.txt"));

  const char *argv_2[] = {ports[1][0].c_str(), ports[1][1].c_str()};
  std::pair<int, int> pid_stdin_pair_2 = InitiateNode(argv_2);
  nodes.push_back(Node(pid_stdin_pair_2.first, pid_stdin_pair_2.second, "procedures/host2.txt"));

  const char *argv_3[] = {ports[2][0].c_str(), ports[2][1].c_str()};
  std::pair<int, int> pid_stdin_pair_3 = InitiateNode(argv_3);
  nodes.push_back(Node(pid_stdin_pair_3.first, pid_stdin_pair_3.second, "procedures/host3.txt"));

  const char *argv_4[] = {ports[3][0].c_str(), ports[3][1].c_str()};
  std::pair<int, int> pid_stdin_pair_4 = InitiateNode(argv_4);
  nodes.push_back(Node(pid_stdin_pair_4.first, pid_stdin_pair_4.second, "procedures/host4.txt"));
}

int main() {
  std::vector<Node> nodes;
  InitiateNodes(nodes);

  // Send message
  for (Node &node: nodes) {
    while (node.GetPendingMessagesCount() != 0) {
      node.SendOnePendingMessage();
    }
  }

  sleep(15);
  // Reap
  for (Node &node: nodes) {
    if (waitpid(node.GetPid(), nullptr, 1) < 0) {
      perror("TEST: Failed to reap");
    } else {
      //std::cout << "TEST: child reaped" << std::endl;
    }
  }

  return 0;
}
