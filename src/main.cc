#include <iostream>

#include "chip8.h"
#include "core.h"

using chip8::get_hello_message;

int function(int v) { return v; }

int main() {
  std::cout << get_hello_message() << std::endl;
  return 0;
}
