#pragma once

#include <cstdint>

namespace gb {

enum class Key : uint8_t;

class IOFrontend {
 public:
  virtual ~IOFrontend() = default;

  virtual auto getKeyPressState() -> Key = 0;
  virtual auto sendSerial(uint8_t value) -> void = 0;
  virtual auto addPixel(int color, int screenX, int screenY) -> void = 0;
  virtual auto commitRender() -> void{};
  virtual auto isFrameScheduled() -> bool = 0;
  virtual auto isExitRequested() -> bool = 0;
};

}  // namespace gb
