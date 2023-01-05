#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX 1000000

// Structure pixel de l'image
typedef struct
{
    int rouge;
    int vert;
    int bleu;
    int row;
    int col;
} pixel;

// Structure noeud du graphe
typedef struct node
{
    int num;
    int vu;
    pixel *pt;
    struct arete *next;
} noeud;

// Structure arete du graphe
typedef struct arete
{
    noeud *pt;
    float poids;
    struct arete *suiv;
} arete;

// Graphe étant ici un tableau de noeud
noeud legraphe[MAX];

// On calcule la différence de couleur entre deux pixels
float diff_couleur(pixel *p1, pixel *p2)
{
    if (p1 == NULL)
    {
        printf("Erreur : p1 est NULL\n");
        return 0;
    }

    int dr = p1->rouge - p2->rouge;
    int dg = p1->vert - p2->vert;
    int db = p1->bleu - p2->bleu;
    return sqrt(dr * dr + dg * dg + db * db);
}

// Fonction pour ajouter une arete entre deux pixels voisins dans le graphe
void ajouter_arete(noeud *n1, noeud *n2, float poids)
{
    arete *edge = malloc(sizeof(arete));
    edge->pt = n2;
    edge->poids = poids;
    edge->suiv = n1->next;
    n1->next = edge;
}
void parcours(noeud *node, int region)
{
    if (node->next == NULL)
    {
        return;
    }

    node->vu = region;

    arete *edge = node->next;

    while (edge)
    {
        if (!edge->pt->vu)
        {
            parcours(edge->pt, region);
        }
        edge = edge->suiv;
    }
}

// Fonction pour ajouter les aretes du graphe entre les pixels voisins
void creer_graphe(pixel *pixels, int width, int height, float seuil)
{

    if (pixels == NULL)
    {
        fprintf(stderr, "Error: pixels est NULL\n");
        exit(1);
    }

    if (width <= 0 || height <= 0)
    {
        fprintf(stderr, "Error: width ou height invalide\n");
        exit(1);
    }

    // On initialise les noeuds dans le graphe
    for (int i = 0; i < width * height; i++)
    {
        legraphe[i].num = i;
        legraphe[i].vu = 0;
        legraphe[i].pt = &pixels[i];
        legraphe[i].next = NULL;
    }

    // on rajoute les aretes
    for (int i = 0; i < width * height; i++)
    {
        // on recupere les colonnes et lignes du pixel courant
        int row = i / width;
        int col = i % width;

        // on ajoute une arete au voisin de droite s'il y en a un
        if (col < width - 1)
        {
            float poids = diff_couleur(&pixels[i], &pixels[i + 1]);
            if (poids <= seuil)
            {
                ajouter_arete(&legraphe[i], &legraphe[i + 1], poids);
            }
        }

        // on ajoute une arete au voisin du bas s'il y en a un
        if (row < height - 1)
        {
            float poids = diff_couleur(&pixels[i], &pixels[i + width]);
            if (poids <= seuil)
            {
                ajouter_arete(&legraphe[i], &legraphe[i + width], poids);
            }
        }
    }
}

// compresser l'image en utilisant les pixels voisins
void compresser_image(pixel *pixels, int width, int height, float seuil)
{
    creer_graphe(pixels, width, height, seuil);

    int region = 0;
    for (int i = 0; i < width * height; i++)
    {
        if (!legraphe[i].vu)
        {
            region++;
            parcours(&legraphe[i], region);
        }
    }
}

// ouvrir le fichier ppm
FILE *open_ppm(char *filename)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        printf("Unable to open file %s\n", filename);
        exit(1);
    }
    return fp;
}

void sauvegarder_image_compressee(char *nom_fichier, pixel *pixels, int width, int height)
{

    FILE *fp = fopen(nom_fichier, "wb");
    if (!fp)
    {
        printf("Impossible d'ouvrir le fichier %s en écriture\n", nom_fichier);
        return;
    }
    fprintf(fp, "P6\n%d %d\n%d\n", width, height, 255);

    for (int i = 0; i < width * height; i++)
    {
        fwrite(&pixels[i].rouge, 1, 1, fp);
        fwrite(&pixels[i].vert, 1, 1, fp);
        fwrite(&pixels[i].bleu, 1, 1, fp);
    }

    fclose(fp);
}

pixel *create_pixels(FILE *fp, int width, int height)
{
    pixel *pixels = malloc(width * height * sizeof(pixel));
    if (!pixels)
    {
        printf("Error allocating memory for pixels\n");
        exit(1);
    }
    for (int i = 0; i < width * height; i++)
    {
        fread(&pixels[i].rouge, 1, 1, fp);
        fread(&pixels[i].vert, 1, 1, fp);
        fread(&pixels[i].bleu, 1, 1, fp);
        pixels[i].row = i / width;
        pixels[i].col = i % width;
    }
    return pixels;
}

void read_ppm_header(FILE *fp, int *width, int *height, int *maxval)
{
    char lebronjames[3];
    fscanf(fp, "%s %d %d %d", lebronjames, width, height, maxval);
    printf("%s %d %d %d", lebronjames, *width, *height, *maxval);

    if (lebronjames[0] != 'P' || (lebronjames[1] != '3' && lebronjames[1] != '6' && lebronjames[1] != '7'))
    {
        fprintf(stderr, "Invalid image format (must be 'P3', 'P6', or 'P7')\n");
        exit(1);
    }
}

void generer_image_compressee(pixel *pixels, int width, int height, float seuil)
{
    compresser_image(pixels, width, height, seuil);

    // on stocke l'image compressee
    pixel *compressed_image = malloc(width * height * sizeof(pixel));
    if (compressed_image == NULL)
    {
        fprintf(stderr, "Error: unable to allocate memory for compressed_image\n");
        return;
    }

    // on assigne une couleur a chaque region
    for (int i = 0; i < width * height; i++)
    {
        pixel *p = &pixels[i];
        int r = p->rouge;
        int g = p->vert;
        int b = p->bleu;
        compressed_image[i] = (pixel){r, g, b, p->row, p->col};
    }

    sauvegarder_image_compressee("compressed.ppm", compressed_image, width, height);

    free(compressed_image);
}

void decompresser_image(pixel *pixels, int width, int height)
{
    int region = 1;
    pixel *decompressed_image = malloc(width * height * sizeof(pixel));
    if (decompressed_image == NULL)
    {
        fprintf(stderr, "Error: unable to allocate memory for decompressed_image\n");
        return;
    }
    for (int i = 0; i < width * height; i++)
    {
        noeud *node = &legraphe[i];
        if (!node->vu)
        {
            parcours(node, region);
            region++;
        }
    }

    for (int i = 0; i < width * height; i++)
    {
        noeud *node = &legraphe[i];
        pixel *p = node->pt;
        int r = p->rouge;
        int g = p->vert;
        int b = p->bleu;
        pixels[i] = (pixel){r, g, b, p->row, p->col};
    }
    sauvegarder_image_compressee("decompressed.ppm", decompressed_image, width, height);

    free(decompressed_image);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s fichier_entree seuil\n", argv[0]);
        return 1;
    }

    FILE *fp = open_ppm(argv[1]);

    char *nom_fichier_entree = argv[1];

    float seuil = atof(argv[2]);

    int width, height, maxval;

    read_ppm_header(fp, &width, &height, &maxval);

    pixel *pixels = create_pixels(fp, width, height);

    pixel *nouveaux_pixels = malloc(width * height * sizeof(pixel));

    for (int i = 0; i < width * height; i++)
    {
        nouveaux_pixels[i] = pixels[i];
    }

    creer_graphe(pixels, width, height, seuil);

    generer_image_compressee(pixels, width, height, seuil);

    decompresser_image(pixels, width, height);


    free(nouveaux_pixels);

    free(pixels);

    return 0;
}
