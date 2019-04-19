#ifndef YELA_NODE_H_
#define YELA_NODE_H_

#include "node.h"

#include <iostream>

namespace yela {

Node::Node() {
}

Node::~Node() {
  std::cout << "Node has exited" << std::endl;
}



}

#endif
