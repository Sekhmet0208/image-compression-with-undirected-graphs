
# Image Compression with Segmentation, RLE, and Graphical Interface

## Overview

This project implements a simple image compression technique in C that combines segmentation and Run-Length Encoding (RLE). It reads an image in the binary PPM (P6) format, segments it by grouping similar pixels based on a user-defined threshold, replaces each segmented region with its average color, and compresses the processed image using RLE. A decompression routine then recovers the image as a standard PPM file. Additionally, a GTK+ based Graphical User Interface (GUI) allows users to select the input file and adjust the threshold dynamically.

## Features

- **PPM Image Input:** Reads binary PPM (P6) images.
- **Segmentation:** Groups neighboring pixels into regions based on color similarity using an iterative DFS to avoid deep recursion.
- **Color Averaging:** Computes the average color for each segmented region.
- **RLE Compression:** Applies Run-Length Encoding (RLE) on the processed pixel data to reduce file size.
- **Decompression:** Recovers the image from the RLE-compressed file, generating a standard PPM image.
- **Graphical User Interface (GUI):**
  - File chooser dialogs for selecting the input image (for compression) and the RLE file (for decompression).
  - An input field to adjust the segmentation threshold dynamically.

## Requirements

- A C compiler (e.g., gcc)
- Standard C libraries: `stdio.h`, `stdlib.h`, `math.h`, `string.h`
- **GTK+ 3:** Ensure you have the GTK+ 3 development packages installed (e.g., `libgtk-3-dev` on Linux)

## Compilation

To compile the project (including the GUI), run:

```
gcc -o image_app gui.c test.c `pkg-config --cflags --libs gtk+-3.0` -lm
```

The `-lm` flag links the math library required for the square root calculations.

## Usage

Run the compiled application:

```bash
./image_app
```

The GUI will open with the following options:

- **Compression:**  
  Click the "Compresser l'image" button to open a file chooser dialog. Select the input PPM image and use the threshold value specified in the provided input field (default is 10.0). The program will process and compress the image using segmentation and RLE, saving the result as `compressed_rle.rle`.

- **Decompression:**  
  Click the "Décompresser l'image" button to open a file chooser dialog for selecting an RLE file. The program will decompress the selected file and produce a `decompressed.ppm` image.

## Important Notes

- **Custom RLE Format:**  
  The compressed file (`compressed_rle.rle`) is stored in a custom RLE format with an RLE header. It does not follow the standard PPM specification and cannot be opened with conventional image viewers.

- **Decompressed Output:**  
  The decompressed file (`decompressed.ppm`) is a standard PPM image and can be viewed with typical image viewers that support PPM.

- **Dynamic Threshold:**  
  The GUI allows you to adjust the segmentation threshold on the fly, which affects how the image is segmented and, consequently, the compression results.

## Author

Fliou Abdelrahman - Université Paris 8