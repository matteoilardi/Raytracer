// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR RANDOM NUMBER GENERATION -----------------
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include <cmath>   // library for math functions
#include <cstdint> // library for fixed size integer types
#include <iostream>
#include <numbers> // library for pi
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
  uint64_t inc;   // increment, different increments generate different orthogonal sequences from the same internal states

  // TODO is it true?
  // ANSWER Yes, since it is because increment is alway odd so any two different increments are coprime with 2^64 and
  // produce two not overlapping cycles which I believe (read chatgpt) is what people call 'being orthogonal' (you can
  // actually prove it with modular arithmetic, things like chinese remainder theorem, etc that I have forgotten...)
  // REMOVE_TAG when you read this comment

  //-----------Constructors-----------
  /// Default constructor

  /// Constructor with parameters
  ///@param initial state of the generator
  ///@param sequence number of the generator (sets initial increment)
  PCG(uint64_t init_state = 42ull, uint64_t init_seq = 54ull) {
    state = 0ull;
    inc = (init_seq << 1ull) | 1ull; // shift 1 bit to the left and replace least significant bit with 1, forcing inc to be odd
    discard(1);
    state += init_state;
    discard(1);
  };

  //------------Methods-----------
  ///@brief generate random uint32 and advance internal state
  uint32_t random() {
    uint64_t old_state = state;

    // advance internal state with linear congruential generator (state*multiplier + increment) mod 2^64
    state = old_state * 6364136223846793005ull + inc; 

    //generate the random number with PCG algorithm
    uint32_t xorshifted = static_cast<uint32_t>(((old_state >> 18ull) ^ old_state) >> 27ull); //scramble the bits 
    uint32_t rot = static_cast<uint32_t>(old_state >> 59ull); //pick the 5 most significant bits (64-59=5)

    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31u));
  };

  ///@brief generate random float uniformly distributed in [0, 1)
  float random_float() {
    uint32_t ran = random();
    return static_cast<float>(ran) / std::pow(2.f, 32);
  }

  ///@brief Generate random (theta, phi) sampling Phong distribution on hemiphere
  ///@param n exponent cos^n(theta) in the Phong distribution
  ///@details Phong: p(Omega) d_Omega = (n+1)/2pi * cos^n(theta)
  std::pair<float, float> random_phong(int n) {
    // sample theta: the cumultaive of the marginal for theta is: P(theta) = 1 - (cos(theta))^(n+1)
    float x = random_float();
    float theta = std::acos(std::pow(x, 1.f / (n + 1)));

    // sample phi: the conditional distribution for phi is actually independent of the theta value you pick
    float phi = random_float() * 2 * std::numbers::pi;

    return {theta, phi};
  }

  ///@brief random (theta, phi) sampling from uniform distribution on hemisphere
  std::pair<float, float> random_unif_hemisphere() {
    return random_phong(0); // Phong distribution for n = 0 is the uniform distribution
  }

  ///@brief extract random numbers and discard them, used to advance the internal state of the generator
  ///@param n how many numbers to discard
  void discard(int n) {
    uint32_t ran;
    while (n > 0) {
      ran = random();
      n--;
    }
    return;
  };
};
