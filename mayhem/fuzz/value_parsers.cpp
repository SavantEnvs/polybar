// Fuzz the small value parsers polybar applies to config values and tag arguments:
// rgba(hex) / color_util::simplify_hex, units_utils::parse_extent / parse_spacing, and the
// UTF-8 / splitting helpers in string_util.
#include <exception>

#include "utils/color.hpp"
#include "utils/string.hpp"
#include "utils/units.hpp"

using namespace polybar;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::string s(reinterpret_cast<const char*>(data), size);
  try { rgba c{s}; (void)c.has_color(); (void)static_cast<uint32_t>(c); } catch (const std::exception&) {}
  try { (void)color_util::simplify_hex(s); } catch (const std::exception&) {}
  try { (void)units_utils::parse_extent(s); } catch (const std::exception&) {}
  try { (void)units_utils::parse_spacing(s); } catch (const std::exception&) {}
  try {
    (void)string_util::char_len(s);
    (void)string_util::utf8_truncate(std::string(s), size / 2);
    (void)string_util::split(s, ',');
    (void)string_util::tokenize(s, ':');
    (void)string_util::trim(std::string(s));
    (void)string_util::strip_trailing_newline(s);
    (void)string_util::replace_all(s, "%", "%%");
  } catch (const std::exception&) {
  }
  return 0;
}
