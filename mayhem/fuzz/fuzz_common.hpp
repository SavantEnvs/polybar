// Shared helpers for the polybar fuzz harnesses (mayhem/fuzz/*.cpp).
#pragma once

#include <unistd.h>

#include <cstdint>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#include "components/logger.hpp"

namespace mayhem_fuzz {

// A silent logger, as polybar's own unit tests use (tests/unit_tests/*): nothing an input does can
// change its level, so keeping it across inputs is safe.
inline const polybar::logger& null_logger() {
  static polybar::logger log(polybar::loglevel::NONE);
  return log;
}

// Write the input to a fresh file under $TMPDIR (fallback /tmp) and return its path; empty on error.
// The caller unlinks it, so nothing outlives one input.
inline std::string write_temp(const uint8_t* data, size_t size, const char* suffix) {
  const char* dir = getenv("TMPDIR");
  std::string path = std::string(dir && *dir ? dir : "/tmp") + "/polybar-fuzz-XXXXXX" + suffix;
  std::vector<char> buf(path.begin(), path.end());
  buf.push_back('\0');
  int fd = mkstemps(buf.data(), static_cast<int>(std::char_traits<char>::length(suffix)));
  if (fd < 0) {
    return {};
  }
  size_t off = 0;
  while (off < size) {
    ssize_t n = write(fd, data + off, size - off);
    if (n <= 0) {
      break;
    }
    off += static_cast<size_t>(n);
  }
  close(fd);
  return std::string(buf.data());
}

// Collect (section, key) pairs that appear in an INI text, so the harness can query them back
// through polybar's public config::get<T> API (config exposes no section iterator).
inline std::vector<std::pair<std::string, std::string>> ini_keys(const std::string& text) {
  std::vector<std::pair<std::string, std::string>> out;
  std::string section;
  size_t pos = 0;
  while (pos <= text.size() && out.size() < 256) {
    size_t eol = text.find('\n', pos);
    if (eol == std::string::npos) eol = text.size();
    std::string line = text.substr(pos, eol - pos);
    pos = eol + 1;
    size_t b = line.find_first_not_of(" \t");
    if (b == std::string::npos) continue;
    line = line.substr(b);
    if (line[0] == '[') {
      size_t e = line.find(']');
      if (e != std::string::npos) section = line.substr(1, e - 1);
      continue;
    }
    size_t eq = line.find('=');
    if (eq == std::string::npos) continue;
    std::string key = line.substr(0, eq);
    size_t ke = key.find_last_not_of(" \t");
    if (ke == std::string::npos) continue;
    out.emplace_back(section, key.substr(0, ke + 1));
  }
  return out;
}

}  // namespace mayhem_fuzz
