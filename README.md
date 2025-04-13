# Raytracer

This is a simple raytracer written in C++. It is a work in progress for Prof. Maurizio Tomasi's course *Numerical Techniques for Photorealistic Image Generation* (AY2024–2025) at the Department of Physics, University of Milan.

## Installation

**Coming soon**

## Usage

As of now, the program supports converting a PFM (Portable FloatMap) image into a PNG image. To do this, run the program from the command line with the following arguments:

```bash
./raytracer <input.pfm> <alpha> <gamma> <output.png>
```

- `<input.pfm>`: The name of the input PFM file.
- `<alpha>`: A multiplier for scaling the pixel values.
- `<gamma>`: The gamma correction value for your monitor (e.g., 2.2).
- `<output.png>`: The name of the output image file (PNG format).

> ⚠️ Rendering features are still under development. Image generation is currently limited to PFM-to-PNG conversion.

To run the available test suite and verify the core components, use:

```bash
./colorTest
./geometryTest
```

## Scene Files

**Work in progress**

## History

**Coming soon**

## License

This project is licensed under the MIT License. See [LICENSE.md](./LICENSE.md) for details.
