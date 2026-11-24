// Fuzz polybar's action-string parser (#module.action.data), the format of IPC ACTION messages,
// legacy FIFO commands and click handlers, then round-trip the result through the encoder.
#include <exception>
#include <tuple>

#include "utils/actions.hpp"

using namespace polybar;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::string s(reinterpret_cast<const char*>(data), size);
  // Contract: parse_action_string asserts its argument starts with '#'; its only caller
  // (controller::process_inputs, src/components/controller.cpp) routes a command there only when
  // `cmd.front() == '#'`.
  if (s.empty() || s.front() != '#') return -1;
  try {
    auto a = actions_util::parse_action_string(s);
    std::string again = actions_util::get_action_string(std::get<0>(a), std::get<1>(a), std::get<2>(a));
    auto b = actions_util::parse_action_string(again);
    (void)b;
  } catch (const std::exception&) {
  }
  return 0;
}
