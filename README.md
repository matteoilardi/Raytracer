# Raytracer

A simple raytracer written in modern C++ for educational and experimental purposes. Developed for the course *Numerical Techniques for Photorealistic Image Generation* (AY2024–2025) by Prof. Maurizio Tomasi at the Department of Physics, University of Milan.

## Features

- Ray tracing with four rendering algorithms: on/off tracing, flat shading, point light tracing, and path tracing.
- External scene description files with variable definitions and parsing.
- Conversion of HDR `.pfm` images to `.png` with tone mapping.
- Optional animation generation via a shell script (requires `ffmpeg`).

## Installation

This project uses CMake and requires a C++17-compatible compiler.

```bash
git clone <https://github.com/matteoilardi/Raytracer>
cd raytracer
mkdir build
cd build
cmake ..
make
```

Except for Google Test, which is fetched from remote, all dependencies are header-only: [CLI11](https://github.com/CLIUtils/CLI11) and [stb_image_write](https://github.com/nothings/stb).

## Command-Line Usage

The program supports three main subcommands via CLI11.

---

### 1. Convert PFM to PNG

Convert an HDR `.pfm` file to a tone-mapped `.png` image:

```bash
./raytracer pfm2png -i input.pfm -o output.png -a 0.18 -g 2.2
```

| Option               | Description                                     |
|----------------------|-------------------------------------------------|
| `-i`, `--input-file` | Input image in `.pfm` format (required)         |
| `-o`, `--output-file`| Output `.png` filename (required)               |
| `-a`, `--alpha`      | Exposure normalization factor (default: 0.18)   |
| `-g`, `--gamma`      | Gamma correction value (default: 2.2)           |

---

### 2. Generate a Demo Image

Render a built-in demo scene composed of an array of spheres (mode `onoff`) or a scene composed of a checkered floor, a reflective sphere, a diffusive sphere and a light-emitting sky (mode `path`).
The output PNG images can be found at `/samples/demo_onoff_tracing.png` and `/samples/demo_path_tracing.png` respectively.

```bash
./raytracer demo -m RENDER_MODE -o demo --width 1280 --height 960 --distance 1.0 --theta-deg 90 --phi-deg 180 --antialiasing 3
```

| Option               | Description                                                               |
|----------------------|---------------------------------------------------------------------------|
| `-m`, `--mode`       | Rendering mode: `onoff` (default) or `path`                               |
| `--width`, `--height`| Output resolution (defaults: 1280x960)                                    |
| `--orthogonal`       | Use orthogonal projection (default: perspective)                          |
| `-o`, `--output-file`| Output base name (saves `.pfm` and `.png`)                                |
| `-d`, `--distance`   | Scene origin - screen distance (excluded if using `--orthogonal`)         |
| `--theta-deg`        | Polar viewing angle in degrees (default: 90)                              |
| `--phi-deg`          | Azimuthal viewing angle in degrees (default: 180)                         |
| `--antialiasing`     | Samples per pixel edge (default: 3)                                       |

---

### 3. Render Scene from File

Render a scene described in a custom text-based file format:

```bash
./raytracer render -s scene.txt -o output --width 1280 --height 960 --antialiasing 3 --define-float radius=1.5
```

| Option               | Description                                           |
|----------------------|-------------------------------------------------------|
| `-s`, `--source`     | Path to the scene description file (required)         |
| `-o`, `--output-file`| Output base name (saves `.pfm` and `.png`)            |
| `--width`, `--height`| Output resolution (defaults: 1280x960)                |
| `--antialiasing`     | Samples per pixel edge (default: 3)                   |
| `--define-float`     | Define float variables (e.g., `--define-float radius=1.5`) |
| `-m`, `--mode`       | Rendering mode: `flat` (default), `onoff`, `point_light`, `path` |
| `--n_rays`           | Number of rays scattered at each hit (path tracing)   |
| `--roulette`         | Ray depth threshold for Russian roulette (path tracing) |
| `--max-depth`        | Maximum ray recursion depth (path tracing)           |

---

## Scene File Format

The scene file uses a custom format supporting:
- Camera orientation
- Materials definition
- Object creation using base shapes (currently spheres and planes), with support for Constructive Solid Geometry (CSG) operations
- Variable substitution with command-line overrides (via `--define-float`)

See `samples/demo_scene.txt` and `samples/csg_example.txt` for usage. A formal EBNF grammar description will be provided soon, along with more examples and use cases.

---

## Generate a Demo Animation

A helper script in `scripts/` generates an animation by rendering frames at varying angles:

```bash
cd scripts
./generate_animation.sh
```

> **Note**: `ffmpeg` must be installed and available in your system's `PATH`.

---

## Unit Testing

Google Test is used for testing core components:

```bash
cd build
ctest
```

Tests are located in the `test/` directory.

---

## License

This project is licensed under the MIT License. See [LICENSE.md](https://github.com/matteoilardi/Raytracer/blob/main/LICENSE.md) for details.

---

## History

See [CHANGELOG.MD](https://github.com/matteoilardi/Raytracer/blob/main/CHANGELOG.md).

---

## Authors

Developed by Master’s students in Theoretical Physics at the University of Milan.  
Contributions, bug reports, and suggestions are welcome!
