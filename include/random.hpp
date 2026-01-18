// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR RANDOM NUMBER GENERATION -----------------
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include <cmath>
#include <cstdint> // fixed size integer types
#include <iostream>
#include <numbers>
#include <utility>

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

class PCG;

//-------------------------------------------------------------------------------------------------------------
// -----------PCG CLASS------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief Permuted congruential generator for randomam number generation
class PCG {
public:
  //-------Properties--------

  uint64_t state; // internal state of the generator
  uint64_t inc;   // increment, different increments generate orthogonal sequences from the same internal states

  //-----------Constructors-----------

  /// Constructor with parameters
  /// @param initial state of the generator
  /// @param sequence number of the generator (sets initial increment)
  PCG(uint64_t init_state = 42ull, uint64_t init_seq = 54ull) {
    state = 0ull;
    inc = (init_seq << 1ull) | 1ull; // shift 1 bit to the left and replace least significant bit with 1, forcing inc to be odd
    discard(1);
    state += init_state;
    discard(1);
  };

  //------------Methods-----------

  /// @brief Generate random uint32 and advance internal state
  uint32_t random() {
    uint64_t old_state = state;

    // Advance internal state with linear congruential generator (state*multiplier + increment) mod 2^64
    state = old_state * 6364136223846793005ull + inc;

    // Generate the random number with PCG algorithm
    uint32_t xorshifted = static_cast<uint32_t>(((old_state >> 18ull) ^ old_state) >> 27ull); // scramble the bits
    uint32_t rot = static_cast<uint32_t>(old_state >> 59ull); // pick the 5 most significant bits (64-59=5)

    return (xorshifted >> rot) | (xorshifted << ((32 - rot) & 31)); // (32 - rot) & 31 = (-rot) & 31, but avoids applying a minus
                                                                    // to an unsigned type, which may trigger a compiler warning
  };

  /// @brief Generate random float uniformly distributed in [0, 1)
  float random_float() {
    uint32_t ran = random();
    return static_cast<float>(ran) / std::pow(2.f, 32);
  }

  /// @brief Generate random (theta, phi) sampling Phong distribution on hemiphere
  /// @param n exponent cos^n(theta) in the Phong distribution
  /// @details Phong: p(Omega) d_Omega = (n+1)/2pi * cos^n(theta)
  std::pair<float, float> random_phong(int n) {
    // Sample theta: the cumultaive of the marginal for theta is: P(theta) = 1 - (cos(theta))^(n+1)
    float x = random_float();
    float theta = std::acos(std::pow(x, 1.f / (n + 1)));

    // Sample phi: the conditional distribution for phi is actually independent of the theta value you pick
    float phi = random_float() * 2 * std::numbers::pi_v<float>;

    return {theta, phi};
  }

  /// @brief Random (theta, phi) sampling from uniform distribution on hemisphere
  std::pair<float, float> random_unif_hemisphere() {
    return random_phong(0); // Phong distribution for n = 0 is the uniform distribution
  }

  /// @brief Extract random numbers and discard them, used to advance the internal state of the generator
  /// @param n how many numbers to discard
  void discard(int n) {
    while (n > 0) {
      random(); // Intentionally discard the return value of random(); calling it advances the RNG state, which is the
                // purpose here. Assigning to a dummy variable may trigger a compiler warning for unused variable.
      n--;
    }
    return;
  };
};
