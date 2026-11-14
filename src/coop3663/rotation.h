#pragma once

#include <array>
#include <cmath>
#include <optional>

namespace cdmp::coop3663 {

// Shared diagnostic policy: finite quaternions with norm in [0.5, 2].
inline std::optional<std::array<float, 4>> normalized_rotation(std::array<float, 4> rotation) {
  double squared = 0;
  for (const float value : rotation) {
    if (!std::isfinite(value)) return std::nullopt;
    squared += static_cast<double>(value) * value;
  }
  const double norm = std::sqrt(squared);
  if (norm < 0.5 || norm > 2.0) return std::nullopt;
  for (float &value : rotation) value = static_cast<float>(value / norm);
  return rotation;
}

}  // namespace cdmp::coop3663
