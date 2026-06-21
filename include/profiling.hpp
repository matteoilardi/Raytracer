// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR PROFILING
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>


/// @brief Function wrapper to calculate and print the time taken for a process to run
/// @param callable object, most likely a lambda wrapping the process you want to profile
template<typename Func>
void run_with_timer(Func func) {
  auto start = std::chrono::steady_clock::now();
  func();
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;

  std::ios old_state{nullptr};
  old_state.copyfmt(std::cout);

  std::cout << std::endl;
  std::cout << std::fixed << std::setprecision(2);
  std::cout << "Elapsed time: " << elapsed_seconds.count() << " s" << std::endl;

  std::cout.copyfmt(old_state);
}

/// @brief Callback for progress bar generation
/// @param Float in range [0, 1] representing current progress
void show_progress(float progress) {
  const int bar_width = 50;
  int pos = static_cast<int>(bar_width * progress);

  std::ios old_state{nullptr};
  old_state.copyfmt(std::cout);

  std::cout << "\r["; // Carriage return
  for (int i = 0; i < bar_width; ++i) {
    if (i < pos) std::cout << "\033[1;32m█\033[0m";  // Green block
    else std::cout << " ";
  }
  std::cout << "] " << std::fixed << std::setprecision(2) << (progress * 100.f) << " %";
  std::cout.flush();

  std::cout.copyfmt(old_state);
}
