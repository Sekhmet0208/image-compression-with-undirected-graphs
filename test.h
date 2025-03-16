#ifndef TEST_H
#define TEST_H

#include <stdio.h>

typedef struct {
    int rouge;
    int vert;
    int bleu;
    int row;
    int col;
} pixel;

void generer_image_compressee(pixel *pixels, int width, int height, float seuil, int maxval);
void decompresser_image(const char *fichier_rle, const char *fichier_ppm);
pixel *create_pixels(FILE *fp, int width, int height);
void read_ppm_header(FILE *fp, int *width, int *height, int *maxval);
FILE *open_ppm(const char *filename);

#endif
