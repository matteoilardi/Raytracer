// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR RANDOM NUMBER GENERATION -----------------
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

<<<<<<< HEAD
#include <cmath>   // library for math functions
#include <cstdint> // library for fixed size integer types
#include <iostream>
#include <numbers> // library for pi
#include <utility>
=======
#include <cmath>
#include <cstdint> // library for fixed size integer types
#include <iostream>
>>>>>>> 23d135870e7b4ced8d7748de18f5c9e402d6d823

// ------------------------------------------------------------------------------------------------------------
// --------GLOBAL FUNCTIONS, CONSTANTS, FORWARD DECLARATIONS------------------
// ----------------------------------------------------------------------------------------

class PCG;

//-------------------------------------------------------------------------------------------------------------
// -----------PCG CLASS------------------
// ------------------------------------------------------------------------------------------------------------

/// @brief Permuted congruential generator
class PCG {
public:
  //-------Properties--------

  uint64_t state; // internal state of the generator
  uint64_t inc; // increment, different increments generate different orthogonal sequences from the same internal states
  // TODO is it true?

  //-----------Constructors-----------

  /// Constructor with parameters
  ///@param initial state of the generator
  ///@param sequence number of the generator (sets initial increment)
  PCG(uint64_t init_state = 42ull, uint64_t init_seq = 54ull) {
    state = 0ull;
    inc = (init_seq << 1ull) | 1ull;
    discard(1);
    state += init_state;
    discard(1);
  };

  //------------Methods-----------

  ///@brief generate random uint32 and advance internal state
  uint32_t random() {
    uint64_t old_state = state;
    state = old_state * 6364136223846793005ull + inc;

    uint32_t xorshifted = static_cast<uint32_t>(((old_state >> 18ull) ^ old_state) >> 27ull);
    uint32_t rot = static_cast<uint32_t>(old_state >> 59ull);

    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31u));
  };

  ///@brief generate random float uniformly distributed in [0, 1)
  float random_float() {
    uint32_t ran = random();
    return static_cast<float>(ran) / std::pow(2.f, 32);
  }

  ///@brief Generate random (theta, phi) sampling Phong distribution on hemiphere
  ///@param n
  ///@details Phong: p(omega) = (n+1)/2pi * cos^n(theta)
  std::pair<float, float> random_phong(int n) {
    // sample theta: the cumultaive of the marginal for theta is: P(theta) = 1 - (cos(theta))^(n+1)
    float x = random_float();
    float theta = std::acos(std::pow(x, 1.f/(n+1)));

    // sample phi
    float phi = random_float() * 2*std::numbers::pi;

    return {theta, phi};
  }

  ///@brief generate random (theta, phi) sampling uniform distribution on hemisphere
  std::pair<float, float> random_unif_hemisphere() {
    return random_phong(0); // Phong distribution for n = 0 is the uniform distribution
  }

  ///@brief extract random numbers and discard them
  ///@param how many numbers to discard
  void discard(int n) {
    while (n > 0) {
      random(); // Intentionally discard the return value of random(); calling it advances the RNG state, which is the
                // purpose here. Assigning to a dummy variable may trigger a compiler warning for unused variable.
      n--;
    }
    return;
  };
};
