// Fuzz polybar's drawtype loaders (label tokens %name:min:max:suffix%, min/maxlen, ellipsis, UTF-8
// truncation; ramps; animations) the way modules load them: from a config file parsed by
// config_parser. The input is the config file; the label/ramp/animation are read from
// [module/fuzz], and every token is replaced with a fixed UTF-8 string before rendering.
#include <unistd.h>

#include <exception>

#include "components/config.hpp"
#include "components/config_parser.hpp"
#include "drawtypes/animation.hpp"
#include "drawtypes/label.hpp"
#include "drawtypes/ramp.hpp"
#include "fuzz_common.hpp"

using namespace polybar;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  std::string path = mayhem_fuzz::write_temp(data, size, ".ini");
  if (path.empty()) return 0;
  static const char* const tokens[] = {"%a%", "%name%", "%percentage%", "%output%", "%index%", "%icon%"};
  try {
    config_parser parser(mayhem_fuzz::null_logger(), std::string(path));
    config conf = parser.parse("");
    const std::string sec = "module/fuzz";
    try {
      auto l = drawtypes::load_optional_label(conf, sec, "label");
      if (l) {
        for (const char* t : tokens) {
          l->reset_tokens();
          l->replace_token(t, "h\xc3\xa9llo w\xe2\x82\xacrld 123456789");
          (void)l->get();
        }
        (void)l->has_token("%a%");
      }
    } catch (const std::exception&) {}
    try {
      auto r = drawtypes::load_ramp(conf, sec, "ramp");  // required, as modules load it
      if (r && *r) {  // modules only index a non-empty ramp
        for (float p : {0.0f, 33.3f, 50.0f, 100.0f, 250.0f}) (void)r->get_by_percentage(p);
        (void)r->get_by_percentage_with_borders(42.0f, 0.0f, 100.0f);
      }
    } catch (const std::exception&) {}
    try {
      auto a = drawtypes::load_animation(conf, sec, "animation");  // required, as modules load it
      if (a) (void)a->get();
    } catch (const std::exception&) {}
  } catch (const std::exception&) {
  }
  unlink(path.c_str());
  return 0;
}
