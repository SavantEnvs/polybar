// Fuzz polybar's formatting-tag parser (%{F#fff}%{A1:cmd:}...%{A}), which parses every string
// modules and user scripts emit. Same calling pattern as tests/unit_tests/tags/parser.cpp.
#include <exception>

#include "tags/parser.hpp"

using namespace polybar;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  tags::parser p;
  p.set(std::string(reinterpret_cast<const char*>(data), size));
  try {
    while (p.has_next_element()) {
      (void)p.next_element();
    }
  } catch (const tags::error&) {
  }
  return 0;
}
