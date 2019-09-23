#include <string.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>

#include "yela.h"
#include "cmd.h"

int main(int argc, char *argv[]) {
  yela::Arguments arguments = yela::GetCommandLine(argc, argv);

  yela::Yela yela(arguments);
  yela.Run();

  return 0;
}
