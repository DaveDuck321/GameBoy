#ifndef headless_h
#define headless_h

#include "frontend.hpp"
#include "io.hpp"

#include <iostream>
#include <optional>
#include <span>

namespace gb {

class Headless : public IOFrontend {
  std::ostream* os;

 public:
  explicit Headless(std::ostream& os) : os(&os) {};

  auto getKeyPressState() -> Key override { return Key::NONE; };
  auto sendSerial(uint8_t value) -> void override { *os << std::hex << value; };
  auto addPixel(int, int, int) -> void override {};
  auto commitRender() -> void override {};
  auto isFrameScheduled() -> bool override { return false; };
  auto isExitRequested() -> bool override { return false; };

  auto try_flush_audio(std::span<std::pair<float, float>> samples)
      -> std::optional<size_t> override {
    return samples.size();
  };
};

}  // namespace gb
#endif
