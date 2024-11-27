#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace gb {

enum class Key : uint8_t;

class IOFrontend {
 public:
  virtual ~IOFrontend() = default;

  virtual auto getKeyPressState() -> Key = 0;
  virtual auto sendSerial(uint8_t value) -> void = 0;
  virtual auto addPixel(int color, int screenX, int screenY) -> void = 0;
  virtual auto commitRender() -> void = 0;
  virtual auto isFrameScheduled() -> bool = 0;
  virtual auto isExitRequested() -> bool = 0;

  virtual auto get_approx_audio_sample_freq() -> size_t { return 1028; };
  virtual auto try_flush_audio(std::span<std::pair<float, float>> samples)
      -> std::optional<size_t> = 0;
};

}  // namespace gb
