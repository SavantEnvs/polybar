// Fuzz polybar's tag builder (components/builder.cpp), which modules use to turn their text,
// colours, fonts, offsets and click actions into a formatting string — including the escaping of
// action commands — and round-trip the result through the tag parser that consumes it.
#include <fuzzer/FuzzedDataProvider.h>

#include <exception>

#include "components/builder.hpp"
#include "components/types.hpp"
#include "tags/parser.hpp"
#include "utils/color.hpp"

using namespace polybar;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  FuzzedDataProvider fdp(data, size);
  bar_settings settings{};
  builder b{settings};
  int opened_actions = 0;
  try {
    for (int i = 0; i < 64 && fdp.remaining_bytes() > 0; i++) {
      switch (fdp.ConsumeIntegralInRange<int>(0, 13)) {
        case 0: b.node(fdp.ConsumeRandomLengthString(64)); break;
        case 1: b.node(fdp.ConsumeRandomLengthString(64), fdp.ConsumeIntegralInRange<int>(0, 8)); break;
        case 2: b.foreground(rgba{fdp.ConsumeIntegral<uint32_t>()}); break;
        case 3: b.background(rgba{fdp.ConsumeIntegral<uint32_t>()}); break;
        case 4: b.foreground_close(); break;
        case 5: b.underline(rgba{fdp.ConsumeIntegral<uint32_t>()}); break;
        case 6: b.overline(rgba{fdp.ConsumeIntegral<uint32_t>()}); break;
        case 7: b.font(fdp.ConsumeIntegralInRange<int>(0, 8)); break;
        case 8: b.offset(extent_val{extent_type::PIXEL, fdp.ConsumeFloatingPointInRange<float>(-1000, 1000)}); break;
        case 9: b.spacing(spacing_val{spacing_type::SPACE, fdp.ConsumeFloatingPointInRange<float>(0, 100)}); break;
        case 10:
          b.action(static_cast<mousebtn>(fdp.ConsumeIntegralInRange<int>(1, static_cast<int>(mousebtn::BTN_COUNT) - 1)),
                   fdp.ConsumeRandomLengthString(64));
          opened_actions++;
          break;
        case 11:
          if (opened_actions > 0) { b.action_close(); opened_actions--; }
          break;
        case 12: b.control(tags::controltag::R); break;
        case 13: b.underline_close(); b.overline_close(); b.background_close(); b.font_close(); break;
      }
    }
    std::string out = b.flush();
    tags::parser p;
    p.set(std::move(out));
    while (p.has_next_element()) (void)p.next_element();
  } catch (const std::exception&) {
  }
  return 0;
}
