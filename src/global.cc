#include "global.h"

namespace yela {

volatile bool run_program;

bool RunProgram() {
  return run_program;
}

void StartProgram() {
  run_program = true;
}

void TerminateProgram() {
  run_program = false;
}

}
