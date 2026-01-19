# HEAD


# [1.0.0] - 2026-01-19

## 2026-01-18
- Major internal refactor [PR#17](https://github.com/matteoilardi/Raytracer/pull/14): 
    - unified coding style
    - revised ownership
    - changes to CLI and DSL are neglegible
- Stop supporting demo command

## 2025-06-19
- Start supporting Constructive Solid Geometry [PR#15](https://github.com/matteoilardi/Raytracer/pull/15)

## 2025-06-09
- Implement lexer and parser supporting scene description in a source file [PR#13](https://github.com/matteoilardi/Raytracer/pull/13)

## 2025-06-09
- Implement path tracing algorithm [PR#9](https://github.com/matteoilardi/Raytracer/pull/9)
- Implement flat renderer [PR#9](https://github.com/matteoilardi/Raytracer/pull/9)
- Finish materials.cpp library implementation (including ImagePigment) [PR#9](https://github.com/matteoilardi/Raytracer/pull/9)
- Implement PCG (random number generator) [PR#9](https://github.com/matteoilardi/Raytracer/pull/9)

## 2025-05-29
- Implement point light tracing [PR#11](https://github.com/matteoilardi/Raytracer/pull/11)

# [0.2.0] - 2025-05-28

## 2025-05-21
- Implement demo command [PR#8](https://github.com/matteoilardi/Raytracer/pull/8)
- Create shapes.hpp library [PR#8](https://github.com/matteoilardi/Raytracer/pull/8)
- Import external library CLI11 [PR#8](https://github.com/matteoilardi/Raytracer/pull/8)
- Implement workflow for continuous integration [PR#6](https://github.com/matteoilardi/Raytracer/pull/6)
- Adopt Google Test unit testing framework [PR#6](https://github.com/matteoilardi/Raytracer/pull/6)

## 2025-05-02
- Fix an issue with the vertical order of the images [#5](https://github.com/matteoilardi/Raytracer/pull/5)


# [0.1.0] - 2025-04-13
- First release of the code
- Create main executable implementing PFM to PNG image conversion
- Create colors.hpp, geometry.hpp and cameras.hpp libraries
- Import external library stb_image_write.h
