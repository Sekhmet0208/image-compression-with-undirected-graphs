#include <gtk/gtk.h>
#include "test.h"

static char *choose_file(GtkWindow *parent, const char *title) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new(title,
                                                    parent,
                                                    GTK_FILE_CHOOSER_ACTION_OPEN,
                                                    "_Annuler", GTK_RESPONSE_CANCEL,
                                                    "_Ouvrir", GTK_RESPONSE_ACCEPT,
                                                    NULL);
    char *filename = NULL;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
        filename = gtk_file_chooser_get_filename(chooser);
    }
    gtk_widget_destroy(dialog);
    return filename;
}

typedef struct {
    GtkWindow *window;
    GtkEntry  *threshold_entry;
} AppData;

static void on_compress_clicked(GtkWidget *widget, gpointer data) {
    AppData *app_data = (AppData *) data;
    GtkWindow *parent = app_data->window;

    char *input_file = choose_file(parent, "Sélectionnez l'image à compresser");
    if (!input_file) {
        GtkWidget *err = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                  "Aucun fichier sélectionné !");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        return;
    }
    
    const char *threshold_text = gtk_entry_get_text(app_data->threshold_entry);
    float seuil = atof(threshold_text); // Conversion du texte en float

    FILE *fp = open_ppm(input_file);
    if (!fp) {
        GtkWidget *err = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                  "Impossible d'ouvrir le fichier !");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        g_free(input_file);
        return;
    }
    int width, height, maxval;
    read_ppm_header(fp, &width, &height, &maxval);
    pixel *pixels = create_pixels(fp, width, height);
    fclose(fp);
    
    generer_image_compressee(pixels, width, height, seuil, maxval);
    free(pixels);
    g_free(input_file);
    
    GtkWidget *dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                               "Compression terminée !");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void on_decompress_clicked(GtkWidget *widget, gpointer data) {
    GtkWindow *parent = GTK_WINDOW(data);
    char *input_file = choose_file(parent, "Sélectionnez le fichier RLE à décompresser");
    if (!input_file) {
        GtkWidget *err = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
                                                  GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                                  "Aucun fichier sélectionné !");
        gtk_dialog_run(GTK_DIALOG(err));
        gtk_widget_destroy(err);
        return;
    }
    decompresser_image(input_file, "decompressed.ppm");
    g_free(input_file);
    
    GtkWidget *dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                               "Décompression terminée !");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Interface Compression d'Image");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 250);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    GtkWidget *threshold_label = gtk_label_new("Seuil:");
    gtk_box_pack_start(GTK_BOX(hbox), threshold_label, FALSE, FALSE, 0);
    GtkWidget *threshold_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(threshold_entry), "10.0");
    gtk_box_pack_start(GTK_BOX(hbox), threshold_entry, FALSE, FALSE, 0);
    
    GtkWidget *compress_button = gtk_button_new_with_label("Compresser l'image");
    AppData app_data;
    app_data.window = GTK_WINDOW(window);
    app_data.threshold_entry = GTK_ENTRY(threshold_entry);
    g_signal_connect(compress_button, "clicked", G_CALLBACK(on_compress_clicked), &app_data);
    gtk_box_pack_start(GTK_BOX(vbox), compress_button, TRUE, TRUE, 0);
    
    GtkWidget *decompress_button = gtk_button_new_with_label("Décompresser l'image");
    g_signal_connect(decompress_button, "clicked", G_CALLBACK(on_decompress_clicked), window);
    gtk_box_pack_start(GTK_BOX(vbox), decompress_button, TRUE, TRUE, 0);
    
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
