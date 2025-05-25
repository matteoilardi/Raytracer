// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------
// ------------ LIBRARY FOR RANDOM NUMBER GENERATION -----------------
// ------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------
// INCLUDED LIBRARIES
// ------------------------------------------------------------------------------------------------------------
#pragma once

#include <iostream>
#include <cstdint> // library for fixed size integer types

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
  /// Default constructor


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
  ///@brief generate random number and advance internal state
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
    return static_cast<float>(ran) / pow(2.f, 32);
  }

  ///@brief extract random numbers and discard them
  ///@param how many numbers to discard
  void discard(int n) {
    uint32_t ran;
    while (n > 0) {
      ran = random();
      n--;
    }
    return;
  };
};
