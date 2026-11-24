// Fuzz tags::dispatch::parse — the renderer-facing interpreter of formatting strings (alignment,
// offsets, colours, action blocks, tray position) — with a renderer that only tracks x positions,
// like the FakeRenderer in tests/unit_tests/tags/dispatch.cpp.
#include <exception>
#include <map>

#include "components/renderer_interface.hpp"
#include "components/types.hpp"
#include "fuzz_common.hpp"
#include "tags/action_context.hpp"
#include "tags/dispatch.hpp"

using namespace polybar;
using namespace polybar::tags;

namespace {
class FuzzRenderer : public renderer_interface {
 public:
  explicit FuzzRenderer(action_context& c) : renderer_interface(c) {}
  void render_offset(const context& ctxt, const extent_val offset) override { x[ctxt.get_alignment()] += offset.value; }
  void render_text(const context& ctxt, const string&& str) override { x[ctxt.get_alignment()] += str.size(); }
  void change_alignment(const context&) override {}
  double get_x(const context& ctxt) const override {
    auto it = x.find(ctxt.get_alignment());
    return it == x.end() ? 0 : it->second;
  }
  double get_alignment_start(const alignment) const override { return 0; }
  void apply_tray_position(const context&) override {}

 private:
  std::map<alignment, double> x;
};
}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  action_context actx;
  FuzzRenderer r(actx);
  dispatch d(mayhem_fuzz::null_logger(), actx);
  bar_settings settings{};
  try {
    d.parse(settings, r, std::string(reinterpret_cast<const char*>(data), size));
    // Exercise the action bookkeeping the bar uses on clicks.
    for (int btn = 1; btn < static_cast<int>(mousebtn::BTN_COUNT); btn++) {
      action_t id = actx.has_action(static_cast<mousebtn>(btn), 0);
      if (id != NO_ACTION) (void)actx.get_action(id);
    }
    for (int x = 0; x < 64; x += 7) {
      for (const auto& kv : actx.get_actions(x)) {
        // bar::handle(button_press) only resolves ids that are not NO_ACTION (src/components/bar.cpp).
        if (kv.second != NO_ACTION) (void)actx.get_action(kv.second);
      }
    }
  } catch (const std::exception&) {
  }
  return 0;
}
