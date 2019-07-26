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
    // We need to sleep, so that each message is seperated.
    sleep(1);

    std::string msg = messages_.front();
    messages_.pop();

    int bytes_sent = 0;
    if ((bytes_sent = write(stdin_fd_, msg.c_str(), msg.size())) < 0) {
      perror("TEST: send pending message using write failed.");
    }
  }

  void SendExitMessage() {
    std::string msg = "EXIT";
    int bytes_sent = 0;
    if ((bytes_sent = write(stdin_fd_, msg.c_str(), msg.size())) < 0) {
      perror("TEST: send pending message using write failed.");
    }
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

    if (execl("./yela", "./yela", argv[0], (char *)nullptr) < 0) {
      perror("TEST: failed to execve");
      std::cerr << errno << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  return std::make_pair(pid, stdin_pipe[1]);
}

void InitiateNodes(std::vector<Node> &nodes) {
  std::vector<std::string> ports = {"5001", "5002", "5003", "5004"};
  std::vector<std::string> files = {"procedures/host1.txt", "procedures/host2.txt", "procedures/host3.txt", "procedures/host4.txt"};

  for (int i = 0; i < ports.size(); ++i) {
    const char *argv[] = {ports[i].c_str()};
    std::pair<int, int> pid_stdin = InitiateNode(argv);
    nodes.push_back(Node(pid_stdin.first, pid_stdin.second, files[i]));
  }
}

int main() {
  std::vector<Node> nodes;
  InitiateNodes(nodes);

  int total_msg = 0;
  for (Node &node: nodes) {
    total_msg += node.GetPendingMessagesCount();
  }

  // Send message
  while (total_msg != 0) {
    for (Node &node: nodes) {
      if (node.GetPendingMessagesCount() != 0) {
        node.SendOnePendingMessage();
        --total_msg;
      }
    }
  }

  sleep(20);
  // Send EXIT message
  for (Node &node: nodes) {
    while (node.GetPendingMessagesCount() != 0) {
      node.SendExitMessage();
    }
  }

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
