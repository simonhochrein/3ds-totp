#pragma once
#include <optional>

namespace totp {
enum class scene_type { LIST, SCAN, SETTINGS, EXIT };

class scene {
public:
  virtual ~scene() = default;

  [[nodiscard]] virtual std::optional<scene_type> update() = 0;

protected:
  scene() = default;
};
} // namespace totp
