#pragma once

#include "render_config.hpp"
#include "gpu_lowering.hpp"
#include <memory>

class HdrImage;

namespace mtl {

/// @brief Host-side facade for the Metal rendering backend.
class Backend {
public:
  Backend();
  ~Backend();

  Backend(const Backend &) = delete;
  Backend &operator=(const Backend &) = delete;

  Backend(Backend &&) noexcept;
  Backend &operator=(Backend &&) noexcept;

  /// @brief Render a flattened GPU scene with the Metal backend.
  std::unique_ptr<HdrImage> render(const gpu::Scene &scene, const ::ModeCfg &cfg,
                                   std::int32_t width, std::int32_t height) const;

private:
  struct Impl;
  std::unique_ptr<Impl> impl;
};

} // namespace mtl
