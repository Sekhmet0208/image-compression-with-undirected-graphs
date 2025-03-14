# Image Compression with Segmentation and RLE

## Overview

This project implements a simple image compression technique in C. It reads an image in the binary PPM (P6) format, performs segmentation by grouping similar pixels based on a user-defined threshold, replaces each segmented region with its average color, compresses the result using Run-Length Encoding (RLE), and then decompresses the RLE file to recreate a PPM image.

## Features

- **PPM Image Input:** Reads binary PPM (P6) images.
- **Segmentation:** Groups neighboring pixels into regions based on color similarity using an iterative DFS to avoid deep recursion.
- **Color Averaging:** Computes the average color for each segmented region.
- **RLE Compression:** Applies Run-Length Encoding (RLE) on the processed pixel data to reduce file size.
- **Decompression:** Recovers the image from the RLE-compressed file, generating a standard PPM image.

## Requirements

- A C compiler (e.g., gcc)
- Standard C libraries: `stdio.h`, `stdlib.h`, `math.h`, `string.h`

## Compilation

To compile the code, run:

```bash
gcc -o test test.c -lm
```

The `-lm` flag links the math library required for the square root calculations.

## Usage

Execute the compiled binary with the following command-line arguments:

1. **Input File:** Path to the input PPM image.
2. **Threshold:** A float value that sets the similarity threshold for segmentation.

Example:

```bash
./test input_image.ppm 10
```

Upon execution, the program will:
- Read the input PPM image.
- Segment the image based on the threshold and compute the average color for each region.
- Compress the image data using RLE and save it as `compressed_rle.rle` (a custom RLE format).
- Decompress the RLE file and produce a `decompressed.ppm` file.

## Important Notes

- **Custom Format:** The RLE file (`compressed_rle.rle`) is saved in a custom format with an RLE header. It does not follow the standard PPM specification and cannot be opened by conventional image viewers.
- **Decompressed Output:** The decompressed file (`decompressed.ppm`) is a standard PPM image and can be viewed with typical image viewers that support PPM.
- **Compression Impact:** The segmentation process modifies pixel colors based on regions but does not change the overall image dimensions. The RLE step reduces file size by compressing runs of identical pixels. The actual compression rate depends on the image content and the chosen threshold.


## Author

Fliou Abdelrahman - Université Paris 8