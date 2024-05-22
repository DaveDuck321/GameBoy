#ifndef headless_h
#define headless_h

#include "frontend.hpp"
#include "io.hpp"

#include <iostream>
#include <memory>

namespace gb {

class Headless : public IOFrontend {
  std::ostream* os;

 public:
  explicit Headless(std::ostream& os) : os(&os){};

  auto getKeyPressState() -> Key override { return Key::NONE; };
  auto sendSerial(uint8_t value) -> void override { *os << std::hex << value; };
  auto addPixel(int, int, int) -> void override{};
  auto commitRender() -> void override{};
  auto isFrameScheduled() -> bool override { return false; };
  auto isExitRequested() -> bool override { return false; };
};

}  // namespace gb
#endif
