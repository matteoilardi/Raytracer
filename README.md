# Raytracer

A simple raytracer written in modern C++ for educational and experimental purposes. Developed for the course *Numerical Techniques for Photorealistic Image Generation* (AY2024–2025) by Prof. Maurizio Tomasi at the Department of Physics, University of Milan.


## Features

- Conversion of HDR `.pfm` images to `.png` with tone mapping.
- Ray tracing of a demo scene with diffusive materials.
- Optional animation generation via a shell script (requires `ffmpeg`).


## Installation

This project uses CMake and requires a C++17-compatible compiler.

```bash
git clone <repo-url>
cd raytracer
mkdir build
cd build
cmake ..
make
```
Google Test is included directly in the repository as a source dependency and built as part of the project.
All other dependencies are header-only: [CLI11](https://github.com/CLIUtils/CLI11) and [stb_image_write](https://github.com/nothings/stb).


## Command-Line Usage

The program supports two main subcommands via CLI11:


### Convert PFM to PNG

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


###  Generate a Demo Image

Render a basic scene composed of scaled spheres using on-off tracing:

```bash
./raytracer demo -o demo --width 1280 --height 960 --distance 1.0 --theta-deg 90 --phi-deg 0
```

| Option               | Description                                           |
|----------------------|-------------------------------------------------------|
| `--width`, `--height`| Output resolution (defaults: 1280x960)                |
| `--orthogonal`       | Use orthogonal projection (default: perspective)      |
| `-o`, `--output-file`| Output base name (saves `demo.pfm`, `demo.png`)       |
| `-d`, `--distance`   | Distance from scene origin (default: 1.0)             |
| `--theta-deg`        | Polar viewing angle (default: 90)                     |
| `--phi-deg`          | Azimuthal viewing angle (default: 0)                  |


### Generate a Demo Animation

Inside the `scripts/` directory, there is a helper script that generates multiple frames at varying observer angles and compiles them into a video.

```bash
cd scripts
./generate_animation.sh
```

> **Note**: `ffmpeg` must be installed and available in your system's `PATH`.


## Unit Testing

Google Test is used for testing core components. After building, to run tests:

```bash
cd build
ctest
```

Tests are located in the `tests/` directory.


## Scene Files

Support for external scene description files is **not yet implemented**, but is planned for future versions.


## License

This project is licensed under the MIT License. See [LICENSE.md](https://github.com/matteoilardi/Raytracer/blob/main/LICENSE.md) for details.


## History

See [CHANGELOG.MD](https://github.com/matteoilardi/Raytracer/blob/main/CHANGELOG.md).


## Author

Developed by Master’s students in Theoretical Physics at the University of Milan.  
Contributions, bug reports, and suggestions are welcome!