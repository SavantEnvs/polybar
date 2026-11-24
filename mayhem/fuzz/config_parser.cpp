// Fuzz polybar's INI config parser and value dereferencing through the public API:
// config_parser(logger, path).parse("") — the same call polybar makes on the user's config file —
// then config::get<T> on every key the input names (${section.key}, ${env:..}, inheritance,
// type conversions). The input is the whole config file.
#include <unistd.h>

#include <exception>

#include "components/config.hpp"
#include "components/config_parser.hpp"
#include "components/types.hpp"
#include "fuzz_common.hpp"
#include "utils/color.hpp"

using namespace polybar;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::string path = mayhem_fuzz::write_temp(data, size, ".ini");
  if (path.empty()) return 0;
  std::string text(reinterpret_cast<const char*>(data), size);
  try {
    config_parser parser(mayhem_fuzz::null_logger(), std::string(path));
    config conf = parser.parse("");
    for (const auto& sk : mayhem_fuzz::ini_keys(text)) {
      const std::string& s = sk.first;
      const std::string& k = sk.second;
      try { (void)conf.get<std::string>(s, k); } catch (const std::exception&) {}
      try { (void)conf.get<int>(s, k); } catch (const std::exception&) {}
      try { (void)conf.get<bool>(s, k); } catch (const std::exception&) {}
      try { (void)conf.get<double>(s, k); } catch (const std::exception&) {}
      try { (void)conf.get<rgba>(s, k); } catch (const std::exception&) {}
      try { (void)conf.get<extent_val>(s, k); } catch (const std::exception&) {}
      try { (void)conf.get<spacing_val>(s, k); } catch (const std::exception&) {}
      try { (void)conf.get<percentage_with_offset>(s, k); } catch (const std::exception&) {}
      try { (void)conf.get_list<std::string>(s, k, {}); } catch (const std::exception&) {}
    }
  } catch (const std::exception&) {
  }
  unlink(path.c_str());
  return 0;
}
