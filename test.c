#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct {
    int rouge;
    int vert;
    int bleu;
    int row;
    int col;
} pixel;

struct noeud;
typedef struct arete {
    struct noeud *pt;
    float poids;
    struct arete *suiv;
} arete;
typedef struct noeud {
    int num;
    int region;
    pixel *pt;
    arete *edges;
} noeud;
noeud *legraphe = NULL;

float diff_couleur(pixel *p1, pixel *p2) {
    if (!p1 || !p2) { fprintf(stderr, "Erreur : un pixel est NULL\n"); return 0; }
    int dr = p1->rouge - p2->rouge;
    int dg = p1->vert - p2->vert;
    int db = p1->bleu - p2->bleu;
    return sqrt(dr * dr + dg * dg + db * db);
}

void ajouter_arete(noeud *n1, noeud *n2, float poids) {
    arete *edge = malloc(sizeof(arete));
    if (!edge) { fprintf(stderr, "Erreur d'allocation mémoire pour une arete\n"); exit(1); }
    edge->pt = n2;
    edge->poids = poids;
    edge->suiv = n1->edges;
    n1->edges = edge;
}

void free_graph(noeud *graph, int num_nodes) {
    for (int i = 0; i < num_nodes; i++) {
        arete *edge = graph[i].edges;
        while (edge) { arete *tmp = edge; edge = edge->suiv; free(tmp); }
    }
    free(graph);
}

void creer_graphe(pixel *pixels, int width, int height, float seuil) {
    int num_nodes = width * height;
    legraphe = malloc(num_nodes * sizeof(noeud));
    if (!legraphe) { fprintf(stderr, "Erreur d'allocation mémoire pour le graphe\n"); exit(1); }
    for (int i = 0; i < num_nodes; i++) {
        legraphe[i].num = i;
        legraphe[i].region = 0;
        legraphe[i].pt = &pixels[i];
        legraphe[i].edges = NULL;
    }
    for (int i = 0; i < num_nodes; i++) {
        int row = i / width, col = i % width;
        if (col < width - 1) {
            float poids = diff_couleur(&pixels[i], &pixels[i+1]);
            if (poids <= seuil) {
                ajouter_arete(&legraphe[i], &legraphe[i+1], poids);
                ajouter_arete(&legraphe[i+1], &legraphe[i], poids);
            }
        }
        if (row < height - 1) {
            float poids = diff_couleur(&pixels[i], &pixels[i+width]);
            if (poids <= seuil) {
                ajouter_arete(&legraphe[i], &legraphe[i+width], poids);
                ajouter_arete(&legraphe[i+width], &legraphe[i], poids);
            }
        }
    }
}

void parcours_iteratif(noeud *start, int region, int num_nodes) {
    noeud **stack = malloc(num_nodes * sizeof(noeud *));
    if (!stack) { fprintf(stderr, "Erreur d'allocation mémoire pour le stack\n"); exit(1); }
    int top = 0;
    stack[top++] = start;
    while (top > 0) {
        noeud *current = stack[--top];
        if (current->region != 0) continue;
        current->region = region;
        for (arete *edge = current->edges; edge; edge = edge->suiv) {
            if (edge->pt->region == 0) stack[top++] = edge->pt;
        }
    }
    free(stack);
}

void compresser_image(pixel *pixels, int width, int height, float seuil) {
    creer_graphe(pixels, width, height, seuil);
    int num_nodes = width * height, region = 0;
    for (int i = 0; i < num_nodes; i++) {
        if (legraphe[i].region == 0) { region++; parcours_iteratif(&legraphe[i], region, num_nodes); }
    }
}

void appliquer_compression(pixel *pixels, int width, int height) {
    int num_nodes = width * height, max_region = 0;
    for (int i = 0; i < num_nodes; i++) {
        if (legraphe[i].region > max_region)
            max_region = legraphe[i].region;
    }
    long *sumR = calloc(max_region+1, sizeof(long));
    long *sumG = calloc(max_region+1, sizeof(long));
    long *sumB = calloc(max_region+1, sizeof(long));
    int *count = calloc(max_region+1, sizeof(int));
    if (!sumR || !sumG || !sumB || !count) { fprintf(stderr, "Erreur d'allocation mémoire pour le calcul des moyennes\n"); exit(1); }
    for (int i = 0; i < num_nodes; i++) {
        int reg = legraphe[i].region;
        if (reg > 0) {
            sumR[reg] += pixels[i].rouge;
            sumG[reg] += pixels[i].vert;
            sumB[reg] += pixels[i].bleu;
            count[reg]++;
        }
    }
    for (int i = 0; i < num_nodes; i++) {
        int reg = legraphe[i].region;
        if (reg > 0 && count[reg] > 0) {
            int avgR = sumR[reg] / count[reg];
            int avgG = sumG[reg] / count[reg];
            int avgB = sumB[reg] / count[reg];
            pixels[i].rouge = avgR;
            pixels[i].vert = avgG;
            pixels[i].bleu = avgB;
        }
    }
    free(sumR);
    free(sumG);
    free(sumB);
    free(count);
}

FILE *open_ppm(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) { fprintf(stderr, "Impossible d'ouvrir le fichier %s\n", filename); exit(1); }
    return fp;
}

void read_ppm_header(FILE *fp, int *width, int *height, int *maxval) {
    char magic[3];
    if (fscanf(fp, "%2s", magic) != 1) { fprintf(stderr, "Erreur lors de la lecture du format de l'image\n"); exit(1); }
    if (strcmp(magic, "P6") != 0) { fprintf(stderr, "Format d'image non supporté (doit être P6)\n"); exit(1); }
    int c = fgetc(fp);
    while (c == '#') { while (c != '\n' && c != EOF) c = fgetc(fp); c = fgetc(fp); }
    ungetc(c, fp);
    if (fscanf(fp, "%d %d", width, height) != 2) { fprintf(stderr, "Erreur lors de la lecture des dimensions\n"); exit(1); }
    if (fscanf(fp, "%d", maxval) != 1) { fprintf(stderr, "Erreur lors de la lecture de la valeur maximale\n"); exit(1); }
    fgetc(fp);
}

pixel *create_pixels(FILE *fp, int width, int height) {
    int num_pixels = width * height;
    pixel *pixels = malloc(num_pixels * sizeof(pixel));
    if (!pixels) { fprintf(stderr, "Erreur d'allocation mémoire pour les pixels\n"); exit(1); }
    for (int i = 0; i < num_pixels; i++) {
        int r = fgetc(fp), g = fgetc(fp), b = fgetc(fp);
        if (r == EOF || g == EOF || b == EOF) { fprintf(stderr, "Erreur lors de la lecture des pixels\n"); free(pixels); exit(1); }
        pixels[i].rouge = r;
        pixels[i].vert = g;
        pixels[i].bleu = b;
        pixels[i].row = i / width;
        pixels[i].col = i % width;
    }
    return pixels;
}

void sauvegarder_image_RLE(const char *nom_fichier, pixel *pixels, int width, int height, int maxval) {
    FILE *fp = fopen(nom_fichier, "wb");
    if (!fp) { fprintf(stderr, "Impossible d'ouvrir le fichier %s en écriture\n", nom_fichier); return; }
    fprintf(fp, "RLE\n%d %d\n%d\n", width, height, maxval);
    int num_pixels = width * height, i = 0;
    while (i < num_pixels) {
        int run_length = 1;
        while (i + run_length < num_pixels && run_length < 255 &&
               pixels[i].rouge == pixels[i+run_length].rouge &&
               pixels[i].vert == pixels[i+run_length].vert &&
               pixels[i].bleu == pixels[i+run_length].bleu)
        {
            run_length++;
        }
        fputc(run_length, fp);
        fputc(pixels[i].rouge, fp);
        fputc(pixels[i].vert, fp);
        fputc(pixels[i].bleu, fp);
        i += run_length;
    }
    fclose(fp);
}

void generer_image_compressee(pixel *pixels, int width, int height, float seuil, int maxval) {
    compresser_image(pixels, width, height, seuil);
    appliquer_compression(pixels, width, height);
    sauvegarder_image_RLE("compressed_rle.rle", pixels, width, height, maxval);
}

void decompresser_image(const char *fichier_rle, const char *fichier_ppm) {
    FILE *fp = fopen(fichier_rle, "rb");
    if (!fp) { fprintf(stderr, "Impossible d'ouvrir le fichier RLE %s\n", fichier_rle); exit(1); }
    char header[4];
    if (!fgets(header, sizeof(header), fp)) { fprintf(stderr, "Erreur lors de la lecture de l'en-tête RLE\n"); exit(1); }
    if (strncmp(header, "RLE", 3) != 0) { fprintf(stderr, "Format RLE invalide\n"); exit(1); }
    int width, height, maxval;
    if (fscanf(fp, "%d %d\n%d\n", &width, &height, &maxval) != 3) { fprintf(stderr, "Erreur lors de la lecture du header RLE\n"); exit(1); }
    int num_pixels = width * height;
    pixel *pixels = malloc(num_pixels * sizeof(pixel));
    if (!pixels) { fprintf(stderr, "Erreur d'allocation mémoire pour pixels dans la décompression\n"); exit(1); }
    int i = 0;
    while (i < num_pixels) {
        int run_length = fgetc(fp);
        if (run_length == EOF) break;
        int r = fgetc(fp), g = fgetc(fp), b = fgetc(fp);
        if (r == EOF || g == EOF || b == EOF) break;
        for (int j = 0; j < run_length && i < num_pixels; j++) {
            pixels[i].rouge = r;
            pixels[i].vert = g;
            pixels[i].bleu = b;
            pixels[i].row = i / width;
            pixels[i].col = i % width;
            i++;
        }
    }
    fclose(fp);
    FILE *out = fopen(fichier_ppm, "wb");
    if (!out) { fprintf(stderr, "Impossible d'ouvrir le fichier %s pour l'écriture\n", fichier_ppm); exit(1); }
    fprintf(out, "P6\n%d %d\n%d\n", width, height, maxval);
    for (i = 0; i < num_pixels; i++) {
        fputc(pixels[i].rouge, out);
        fputc(pixels[i].vert, out);
        fputc(pixels[i].bleu, out);
    }
    fclose(out);
    free(pixels);
}

int main(int argc, char *argv[]) {
    if (argc != 3) { fprintf(stderr, "Usage: %s fichier_entree seuil\n", argv[0]); return 1; }
    const char *nom_fichier_entree = argv[1];
    float seuil = atof(argv[2]);
    FILE *fp = open_ppm(nom_fichier_entree);
    int width, height, maxval;
    read_ppm_header(fp, &width, &height, &maxval);
    pixel *pixels = create_pixels(fp, width, height);
    fclose(fp);
    generer_image_compressee(pixels, width, height, seuil, maxval);
    free_graph(legraphe, width * height);
    free(pixels);
    decompresser_image("compressed_rle.rle", "decompressed.ppm");
    return 0;
}
