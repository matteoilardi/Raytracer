# Raytracer

A simple raytracer written in modern C++ for educational and experimental purposes. Developed for the course *Numerical Techniques for Photorealistic Image Generation* (AY2024–2025) by Prof. Maurizio Tomasi at the Department of Physics, University of Milan.

## Features

- Ray tracing with four rendering algorithms: on/off tracing, flat tracing (also called flat shading), point light tracing, and path tracing.
- External scene description files with variable definitions and parsing.
- Conversion of HDR `.pfm` images to `.png` with tone mapping.
- Optional animation generation via a shell script (requires `ffmpeg`).

## Installation

This project uses CMake and requires a C++23-compatible compiler.

```bash
git clone https://github.com/matteoilardi/Raytracer
cd Raytracer
mkdir build
cd build
cmake ..
make
```

The installation defaults to Release mode. If you want Debug mode instead, add the flag `-DCMAKE_BUILD_TYPE=Debug` to the CMake invocation.

Except for Google Test (fetched remotely), all dependencies are header-only: [CLI11](https://github.com/CLIUtils/CLI11) and [stb_image_write](https://github.com/nothings/stb).

---

## Command-Line Usage

The executable supports **exactly one subcommand**, chosen among:

- `render`   – render a scene from a text description file
- `pfm2png`  – convert an HDR PFM image into a PNG

Invoke the program as:

```bash
./raytracer <subcommand> [options]
```

---

## pfm2png — Convert PFM to PNG

Convert an HDR `.pfm` file to a tone-mapped `.png` image.

```bash
./raytracer pfm2png input.pfm -o out
```

The command produces:
- `out.png`


### Positional Argument

| Argument | Description |
|--------|-------------|
| `input` | Path to the PFM input file (required) |


### Other options

| Option | Description |
|------|------------|
| `-o`, `--output-file` | Output file name stem (default: `out`) |
| `-a`, `--alpha` | Exposure normalization factor (default: `0.18`) |
| `-g`, `--gamma` | Gamma correction factor (default: `2.2`) |
| `--dark` | Use a fixed exposure suitable for very dark images |

---

## render — Render a Scene from its Description

Render a scene described in a custom DSL.

```bash
./raytracer render scene.txt -o output --width 1280 --height 960 --antialiasing 2
```

This command produces:
- `output.pfm` (HDR intermediate result)
- `output.png` (tone-mapped LDR image)

### Positional Argument

| Argument | Description |
|--------|-------------|
| `source` | Path to the scene description file (required) |

### Image Size

| Option | Description |
|------|-------------|
| `--width` | Image width in pixels (default: `1280`) |
| `--height` | Image height in pixels (default: `960`) |

### Output and Tone Mapping

| Option | Description |
|------|-------------|
| `-o`, `--output-file` | Output file name stem (default: `out`) |
| `-a`, `--alpha` | Exposure normalization factor (default: `0.18`) |
| `-g`, `--gamma` | Gamma correction factor (default: `2.2`) |
| `--dark` | Use default exposure for dark scenes |

### Rendering Mode

Select the rendering algorithm:

```bash
-m, --mode <onoff|flat|pointlight|path>
```

Default mode is `flat`.

| Mode | Description |
|-----|-------------|
| `onoff` | Binary visibility rendering |
| `flat` | Flat shading |
| `pointlight` | Point-light illumination |
| `path` | Monte Carlo path tracing |

### Path Tracing Parameters (mode = `path`)

These options are only meaningful when using `--mode path`.

| Option | Description |
|------|-------------|
| `--n_rays` | Number of rays scattered at each hit (default: `10`) |
| `--roulette` | Ray depth before Russian roulette starts (default: `3`) |
| `--max-depth` | Maximum ray recursion depth (default: `5`) |
| `--seq-number` | PCG random generator sequence number (default: `54`) |

### Antialiasing

| Option | Description |
|------|-------------|
| `--antialiasing` | Samples per pixel edge (square root of samples per pixel, default: `1`) |

### Scene Variables from Command Line

Floating-point variables defined in the scene file can be overridden from the command line:

```bash
--define-float name=value
```

Example:

```bash
./raytracer render scene.txt --define-float radius=1.5 --define-float intensity=3.0
```

Command-line definitions take priority over values defined in the scene file.

---

## Scene File Format

The scene file uses a custom text format supporting:

- Camera definition
- Materials and textures
- Object creation and transformations
- Base primitives (spheres and planes)
- Constructive Solid Geometry (CSG)
- Named float variables

Examples:
- `samples/demo_scene.txt`
- `samples/csg_example.txt`

A formal grammar is provided in `EBNF.md`.

---

## Demo Animation

A sample path-tracing animation is provided at `samples/demo_path_tracing_animation.mp4`.
[▶ Watch the animation](https://github.com/matteoilardi/Raytracer/tree/main/samples/demo_path_tracing_animation.mp4)

The animation is obtained by rendering a sequence of frames with varying camera parameters and assembling them into a video.

To generate a similar animation:

1. Render multiple frames while changing camera angles or scene parameters.
2. Save each frame as a PNG.
3. Use `ffmpeg` to assemble the frames into a video.

A helper script is provided:

```bash
cd scripts
./generate_animation.sh
```

`ffmpeg` must be installed and available in your system `PATH`.

---

## Unit Testing

Google Test is used for unit testing.

```bash
cd build
ctest
```

Tests are located in the `test/` directory.

---

## License

This project is licensed under the MIT License. See `LICENSE.md` for details.

---

## History

See `CHANGELOG.md`.

---

## Authors

Developed by Master’s students in Theoretical Physics at the University of Milan.  
Contributions, bug reports, and suggestions are welcome.

