#pragma once

#include "render_config.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace gpu {

struct alignas(16) OnOffCfg {
  std::uint32_t _padding0;
  std::uint32_t _padding1;
  std::uint32_t _padding2;
  std::uint32_t _padding3;
};

struct alignas(16) FlatCfg {
  float background_color[4];
};

struct alignas(16) PathCfg {
  std::uint32_t samples_per_pixel;
  std::uint32_t max_depth;
  std::uint32_t russian_roulette_limit;
  std::uint32_t _padding0;

  std::uint64_t random_seed;
  std::uint32_t _padding1;
  std::uint32_t _padding2;

  float background_color[4];
};

inline OnOffCfg lower_on_off_cfg(const ::ModeCfg &cfg) {
  if (cfg.mode != ::ModeCfg::Mode::ONOFF) {
    throw std::runtime_error("Cannot lower non-on-off configuration to gpu::OnOffCfg");
  }

  if (cfg.samples_per_pixel_edge != 1) {
    throw std::runtime_error("Antialiasing is not supported by the GPU backend");
  }

  return OnOffCfg{};
}

inline FlatCfg lower_flat_cfg(const ::ModeCfg &cfg) {
  if (cfg.mode != ::ModeCfg::Mode::FLAT) {
    throw std::runtime_error("Cannot lower non-flat configuration to gpu::FlatCfg");
  }

  if (cfg.samples_per_pixel_edge != 1) {
    throw std::runtime_error("Antialiasing is not supported by the GPU backend");
  }

  FlatCfg lowered_cfg{};
  lowered_cfg.background_color[0] = 0.f;
  lowered_cfg.background_color[1] = 0.f;
  lowered_cfg.background_color[2] = 0.f;
  lowered_cfg.background_color[3] = 1.f; // black, opaque
  // TODO add background color to the program API in the future

  return lowered_cfg;
}

inline PathCfg lower_path_cfg(const ::ModeCfg &cfg) {
  if (cfg.mode != ::ModeCfg::Mode::PATH) {
    throw std::runtime_error("Cannot lower non-path configuration to gpu::PathCfg");
  }

  if (!std::holds_alternative<::PathCfg>(cfg.params)) {
    throw std::runtime_error("ModeCfg is PATH but does not contain PathCfg parameters");
  }

  PathCfg lowered_cfg{};

  int sppe = cfg.samples_per_pixel_edge;
  // NOTE not used for antialiasing for now, but merely to deicde how many rays
  // to trace through the center of each pixel
  lowered_cfg.samples_per_pixel = static_cast<std::uint32_t>(sppe * sppe);

  ::PathCfg params = std::get<::PathCfg>(cfg.params);
  lowered_cfg.max_depth = static_cast<std::uint32_t>(params.max_depth);
  lowered_cfg.russian_roulette_limit = static_cast<std::uint32_t>(params.russian_roulette_lim);
  lowered_cfg.random_seed = static_cast<std::uint64_t>(params.seq_number);
  // TODO fix after exposing the seed instead of the sequence number in the program API

  lowered_cfg.background_color[0] = 0.f;
  lowered_cfg.background_color[1] = 0.f;
  lowered_cfg.background_color[2] = 0.f;
  lowered_cfg.background_color[3] = 1.f; // black, opaque
  // TODO add background color to the program API in the future

  return lowered_cfg;
}

static_assert(sizeof(OnOffCfg) == 16);
static_assert(alignof(OnOffCfg) == 16);
static_assert(std::is_standard_layout_v<OnOffCfg>);
static_assert(std::is_trivially_copyable_v<OnOffCfg>);

static_assert(sizeof(FlatCfg) == 16);
static_assert(alignof(FlatCfg) == 16);
static_assert(std::is_standard_layout_v<FlatCfg>);
static_assert(std::is_trivially_copyable_v<FlatCfg>);

static_assert(sizeof(PathCfg) == 48);
static_assert(alignof(PathCfg) == 16);
static_assert(offsetof(PathCfg, background_color) == 32);
static_assert(std::is_standard_layout_v<PathCfg>);
static_assert(std::is_trivially_copyable_v<PathCfg>);

} // namespace gpu
