// Fuzz polybar's IPC wire-format decoder (ipc::decoder::on_read), which parses every message a
// client writes to polybar's unix socket. The decoder is a chunk-driven state machine, so the
// first input byte picks the chunk size the remaining bytes are delivered in (0 = all at once).
#include <exception>

#include "fuzz_common.hpp"
#include "ipc/decoder.hpp"
#include "ipc/msg.hpp"

using namespace polybar;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  if (size < 1) return -1;
  size_t chunk = data[0];
  data++;
  size--;
  if (chunk == 0) chunk = size ? size : 1;
  size_t total = 0;
  ipc::decoder dec(mayhem_fuzz::null_logger(), [&total](uint8_t, ipc::type_t, const std::vector<uint8_t>& msg) {
    total += msg.size();
  });
  try {
    size_t off = 0;
    while (off < size && !dec.closed()) {
      size_t n = std::min(chunk, size - off);
      dec.on_read(data + off, n);
      off += n;
    }
  } catch (const ipc::decoder::error&) {
  }
  return 0;
}
