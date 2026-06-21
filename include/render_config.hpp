#pragma once

#include <variant>

struct PathCfg {
  int n_rays;
  int russian_roulette_lim;
  int max_depth;
  int seq_number;
};

struct ModeCfg {
  enum class Mode { PATH, FLAT, POINTLIGHT, ONOFF } mode;
  std::variant<std::monostate, PathCfg> params;
  int samples_per_pixel_edge; // Antialiasing
};
