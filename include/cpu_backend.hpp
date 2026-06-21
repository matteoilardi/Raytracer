#pragma once

#include "cameras.hpp"
#include "profiling.hpp"
#include "render_config.hpp"
#include "renderers.hpp"
#include "scenefiles.hpp"
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace cpu {

class Backend {
public:
  std::unique_ptr<HdrImage> render(const ::Scene &scene, const ::ModeCfg &cfg,
                                   std::int32_t width, std::int32_t height) const {
    ImageTracer tracer{
      std::make_unique<HdrImage>(width, height),
      scene.camera,
      cfg.samples_per_pixel_edge
    };

    std::unique_ptr<Renderer> renderer = build_renderer(cfg, scene.world);
    tracer.fire_all_rays([&renderer](const Ray &ray) { return (*renderer)(ray); }, show_progress);
    return std::move(tracer.image);
  }

private:
  static std::unique_ptr<Renderer> build_renderer(const ::ModeCfg &mode_cfg, const World &world) {
    switch (mode_cfg.mode) {
    case ::ModeCfg::Mode::ONOFF:
      return std::make_unique<OnOffTracer>(world);
    case ::ModeCfg::Mode::FLAT:
      return std::make_unique<FlatTracer>(world, BLACK);
    case ::ModeCfg::Mode::PATH: {
      auto pcg_init_seq = static_cast<std::uint64_t>(std::get<::PathCfg>(mode_cfg.params).seq_number);
      auto pcg = std::make_unique<PCG>(42ull, pcg_init_seq);

      return std::make_unique<PathTracer>(
          world, std::move(pcg), std::get<::PathCfg>(mode_cfg.params).n_rays,
          std::get<::PathCfg>(mode_cfg.params).russian_roulette_lim,
          std::get<::PathCfg>(mode_cfg.params).max_depth, BLACK);
    }
    case ::ModeCfg::Mode::POINTLIGHT:
      return std::make_unique<PointLightTracer>(world, DARK_GREY, BLACK);
    }

    throw std::runtime_error("Unknown rendering mode in CPU backend");
  }
};

} // namespace cpu
