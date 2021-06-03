#include <sys/stat.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <mpg123.h>
#include <dirent.h>
#include <stdio.h>
#include <ao/ao.h>
#include <time.h>
#include "lamp.h"

#define BITS 8

/* GLOBAL VARIABLES */
int meta;
int paused = 0;
int cleanup = 0;
int *shuffle_arr;
int shuffle_current = 0;
int current_index = 0;
int total_count = 0;
gdouble volume = 80.0;
pthread_t play_th;
pthread_attr_t attr;
pthread_mutex_t mx_pause;
pthread_mutex_t mx_cleanup;
pthread_cond_t cv_cleanup;
char *current_str;
char *current_playlist;
GtkWindow *window;
GtkImage *g_img_logo;
GtkLabel *g_lbl_name;
GtkLabel *g_lbl_album;
GtkLabel *g_lbl_artist;
GtkLabel *g_lbl_comment;
GtkLabel *g_lbl_position;
GtkRange *g_sld_position;
GtkListBox *g_lst_playlist;
GtkToggleButton *g_tgl_repeat;
GtkToggleButton *g_tgl_shuffle;
GtkLabel *g_lbl_playlist;
m_context p_ctx;
GtkCssProvider *style;

int main(int argc, char *argv[]) {
    pthread_mutex_init(&mx_pause, NULL);
    pthread_mutex_init(&mx_cleanup, NULL);
    pthread_cond_init(&cv_cleanup, NULL);
    current_str = malloc(sizeof(char) * 1024);

    GtkBuilder *builder;
    GtkWidget *g_btn_play;
    GtkWidget *g_btn_pause;
    GtkWidget *g_btn_stop;
    GtkWidget *g_btn_next;
    GtkWidget *g_btn_prev;
    GtkWidget *g_itm_add;
    GtkWidget *g_itm_save;
    GtkWidget *g_itm_open;
    GtkWidget *g_itm_clear;
    GtkWidget *g_itm_folder;
    GtkRange *g_sld_volume;
    GtkWidget *g_tgl_above;

    gtk_init(&argc, &argv);
    builder = gtk_builder_new();
    gtk_builder_add_from_resource(builder, "/glade/lamp.glade", NULL);

    window = GTK_WINDOW(gtk_builder_get_object(builder, "main"));
    gtk_window_set_position(window, GTK_WIN_POS_CENTER);
    gtk_widget_add_events(GTK_WIDGET(window), GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window), "size-allocate", G_CALLBACK(window_size_allocate), NULL);
    g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(window_key_press_callback), NULL);

    style = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(style, "/glade/style.css");
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(style), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    g_btn_play = GTK_WIDGET(gtk_builder_get_object(builder, "btn_play"));
    g_btn_stop = GTK_WIDGET(gtk_builder_get_object(builder, "btn_stop"));
    g_btn_pause = GTK_WIDGET(gtk_builder_get_object(builder, "btn_pause"));
    g_btn_next = GTK_WIDGET(gtk_builder_get_object(builder, "btn_next"));
    g_btn_prev = GTK_WIDGET(gtk_builder_get_object(builder, "btn_prev"));
    g_sld_volume = GTK_RANGE(gtk_builder_get_object(builder, "sld_volume"));
    g_tgl_above = GTK_WIDGET(gtk_builder_get_object(builder, "tgl_above"));
    g_itm_add = GTK_WIDGET(gtk_builder_get_object(builder, "itm_add"));
    g_itm_folder = GTK_WIDGET(gtk_builder_get_object(builder, "itm_folder"));
    g_itm_open = GTK_WIDGET(gtk_builder_get_object(builder, "itm_open"));
    g_itm_save = GTK_WIDGET(gtk_builder_get_object(builder, "itm_save"));
    g_itm_open = GTK_WIDGET(gtk_builder_get_object(builder, "itm_open"));
    g_itm_clear = GTK_WIDGET(gtk_builder_get_object(builder, "itm_clear"));
    g_img_logo = GTK_IMAGE(gtk_builder_get_object(builder, "img_logo"));
    g_lbl_name = GTK_LABEL(gtk_builder_get_object(builder, "lbl_name"));
    g_lbl_album = GTK_LABEL(gtk_builder_get_object(builder, "lbl_album"));
    g_lbl_artist = GTK_LABEL(gtk_builder_get_object(builder, "lbl_artist"));
    g_lbl_comment = GTK_LABEL(gtk_builder_get_object(builder, "lbl_comment"));
    g_lbl_position = GTK_LABEL(gtk_builder_get_object(builder, "lbl_position"));
    g_sld_position = GTK_RANGE(gtk_builder_get_object(builder, "sld_position"));
    g_tgl_repeat = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "tgl_repeat"));
    g_tgl_shuffle = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "tgl_shuffle"));
    g_lst_playlist = GTK_LIST_BOX(gtk_builder_get_object(builder, "lst_playlist"));
    g_lbl_playlist = GTK_LABEL(gtk_builder_get_object(builder, "lbl_playlist"));

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            add_file_to_playlist(g_lst_playlist, argv[i]);
        }
        current_str = argv[argc - 1];
        get_totatl_count();
    }

    set_default_album_art();
    gtk_range_set_value(g_sld_volume, volume);
    // TODO load last used playlist...
    // TODO store playlist as binary files...
    gtk_label_set_text(g_lbl_playlist, "Untintled");

    gtk_builder_connect_signals(builder, NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(g_btn_play, "released", G_CALLBACK(play_callback), NULL);
    g_signal_connect(g_btn_stop, "released", G_CALLBACK(stop_callback), NULL);
    g_signal_connect(g_btn_pause, "released", G_CALLBACK(pause_callback), NULL);
    g_signal_connect(g_btn_next, "released", G_CALLBACK(next_callback), NULL);
    g_signal_connect(g_btn_prev, "released", G_CALLBACK(prev_callback), NULL);
    g_signal_connect(g_sld_volume, "value-changed", G_CALLBACK(volume_changed), NULL);
    g_signal_connect(g_tgl_above, "toggled", G_CALLBACK(keep_above_callback), NULL);
    g_signal_connect(g_tgl_shuffle, "toggled", G_CALLBACK(shuffle_callback), NULL);
    g_signal_connect(g_itm_add, "activate", G_CALLBACK(add_to_playlist_callback), NULL);
    g_signal_connect(g_itm_folder, "activate", G_CALLBACK(open_folder_callback), NULL);
    g_signal_connect(g_itm_save, "activate", G_CALLBACK(save_playlist_callback), NULL);
    g_signal_connect(g_itm_open, "activate", G_CALLBACK(open_playlist_callback), NULL);
    g_signal_connect(g_itm_clear, "activate", G_CALLBACK(clear_playlist_callback), NULL);
    g_signal_connect(g_lst_playlist, "row-activated", G_CALLBACK(playlist_row_activated), NULL);
    g_signal_connect(g_sld_position, "change-value", G_CALLBACK(position_changed), NULL);

//    gtk_drag_dest_set(GTK_WIDGET(g_lst_playlist), GTK_DEST_DEFAULT_ALL, file_entries, GTK_TARGET_OTHER_APP, GDK_ACTION_MOVE);
//    g_signal_connect (GTK_WIDGET(g_lst_playlist), "drag-data-received", G_CALLBACK(playlist_row_drag_file_received), NULL);

    g_object_unref(builder);
    gtk_widget_show(GTK_WIDGET(window));
    gtk_main();

    free(current_str);
    pthread_mutex_destroy(&mx_pause);
    pthread_mutex_destroy(&mx_cleanup);
    pthread_cond_destroy(&cv_cleanup);
    return 0;
}

static void play_callback(GtkWidget *widget, gpointer data) {
    if (paused) {
        paused = !paused;
    } else if (!play_th) {
        play_th_create();
        reset_global_vars();
    } else {
        mpg123_seek_frame(p_ctx.mh, 0, SEEK_SET);
    }
    pthread_mutex_unlock(&mx_pause);
}

static void *play(void *filename) {
    cleanup = 1;
    ao_initialize();
    mpg123_init();
    play_context_init(&p_ctx, filename);
    mpg123_volume(p_ctx.mh, volume / 100.0);
    gtk_range_set_range(g_sld_position, 0, p_ctx.duration);
    gtk_range_set_fill_level(g_sld_position, p_ctx.duration);
    gtk_range_set_value(g_sld_position, 0.0);
    highlight_row(current_index);

    /* @formatter:off */
    pthread_cleanup_push(cleanup_play_th, NULL)
            if (p_ctx.dev == NULL) {
                g_print("Error opening sound device.\n");
                pthread_exit(NULL);
            }

            if (!get_id3_data(&p_ctx)) { set_current_filename_title(); }

            do {
                while (mpg123_read(p_ctx.mh, p_ctx.buffer, p_ctx.buffer_size, &p_ctx.done) == MPG123_OK) {
                    pthread_mutex_lock(&mx_pause);
                    if (play_th) {
                        ao_play(p_ctx.dev, p_ctx.buffer, p_ctx.done);
                        double pos = min((double) mpg123_tellframe(p_ctx.mh) * p_ctx.frame_d, p_ctx.duration);
                        gtk_range_set_value(g_sld_position, pos);
                        char *str_position = seconds_to_str((int) pos);
                        gtk_label_set_text(g_lbl_position, str_position);
                        g_free(str_position);
                    }
                    pthread_mutex_unlock(&mx_pause);
                }
                mpg123_seek_frame(p_ctx.mh, 0, SEEK_SET);
            } while (gtk_toggle_button_get_active(g_tgl_repeat));
    pthread_cleanup_pop(1);
    /* @formatter:on */
    pthread_cleanup_push((void *) play_next, NULL)
    pthread_cleanup_pop(1);
    pthread_exit(NULL);
}

static void play_th_create() {
    while (cleanup) { pthread_cond_wait(&cv_cleanup, &mx_cleanup); }
    pthread_mutex_unlock(&mx_cleanup);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&play_th, &attr, play, current_str);
    pthread_attr_destroy(&attr);
}

static void stop_play_th() {
    if (play_th) {
        pthread_cancel(play_th);
        reset_global_vars();
        play_th = 0;
    }
    reset_widgets();
}

static void pause_callback(GtkWidget *widget, gpointer data) {
    pause_player();
}

static void stop_callback(GtkWidget *widget, gpointer data) {
    stop_play_th();
}

static void play_next(int reverse, int force) {
    int res = 0;
    GtkListBoxRow *row;
    if (gtk_toggle_button_get_active(g_tgl_shuffle) && shuffle_arr) {
        if (reverse) { shuffle_current = --shuffle_current < 0 ? total_count : shuffle_current; }
        else { shuffle_current = ++shuffle_current > total_count ? 0 : shuffle_current; }
        current_index = shuffle_arr[shuffle_current];
        if (!(gtk_list_box_get_row_at_index(g_lst_playlist, current_index))) {
            current_index = shuffle_arr[0];
            if (force) { res = 1; }
        } else { res = 1; }
    } else if (reverse) {
        current_index--;
        if (!gtk_list_box_get_row_at_index(g_lst_playlist, current_index)) {
            current_index = (int) max(0, get_totatl_count());
            if (force) { res = 1; }
        } else { res = 1; }
    } else {
        current_index++;
        if (!(gtk_list_box_get_row_at_index(g_lst_playlist, current_index))) {
            current_index = 0;
            if (force) { res = 1; }
        } else { res = 1; }
    }
    if (res && (row = gtk_list_box_get_row_at_index(g_lst_playlist, current_index))) {
        set_current_row_filename(row);
        play_th_create();
        if (force) {
            gtk_widget_grab_focus(GTK_WIDGET(row));
        }
    }
}

static void next_callback(GtkWidget *widget, gpointer data) {
    stop_play_th();
    play_next(0, 1);
}

static void prev_callback(GtkWidget *widget, gpointer data) {
    stop_play_th();
    play_next(1, 1);
}

static void playlist_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    stop_play_th();
    current_index = gtk_list_box_row_get_index(row);
    set_current_row_filename(row);
    play_th_create();
}

static void volume_changed(GtkRange *range, gpointer user_data) {
    volume = gtk_range_get_value(range);
    mpg123_volume(p_ctx.mh, volume / 100.0);
}

static void position_changed(GtkRange *range, GtkScrollType scroll, double value, gpointer user_data) {
    if (!play_th) return;
    off_t frame = mpg123_timeframe(p_ctx.mh, value);
    mpg123_seek_frame(p_ctx.mh, frame, SEEK_SET);
    gtk_range_set_value(GTK_RANGE(range), value);
}

gboolean window_key_press_callback(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    if (event->key.keyval == GDK_KEY_space) {
        pause_player();
    } else if (event->key.keyval == GDK_KEY_Delete) {
        GList *rows = gtk_list_box_get_selected_rows(g_lst_playlist);
        while (rows) {
            gtk_widget_destroy(GTK_WIDGET(rows->data));
            rows = rows->next;
            total_count--;
        }
        g_list_free(rows);
        if (gtk_toggle_button_get_active(g_tgl_shuffle)) {
            clear_shuffle();
            shuffle_playlist();
        }
    }
    return 1;
}

static void add_to_playlist_callback(GtkMenuItem *menuitem, gpointer user_data) {
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    chooser = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new("Open File", window, action, ("Cancel"), GTK_RESPONSE_CANCEL, ("Open"), GTK_RESPONSE_ACCEPT, NULL));
    gtk_file_chooser_set_select_multiple(chooser, 1);
    int res = gtk_dialog_run(GTK_DIALOG(chooser));
    if (res == GTK_RESPONSE_ACCEPT) {
        GSList *filenames;
        filenames = gtk_file_chooser_get_filenames(chooser);
        while (filenames) {
            char *dot = strrchr(filenames->data, '.');
            if (dot && !strcmp(dot, ".mp3")) add_file_to_playlist(g_lst_playlist, filenames->data);
            filenames = filenames->next;
        }
        if (!current_index) { set_current_row_filename(NULL); }
        g_free(filenames);
    }
    gtk_widget_destroy(GTK_WIDGET(chooser));
    get_totatl_count();
    if (gtk_toggle_button_get_active(g_tgl_shuffle)) {
        clear_shuffle();
        shuffle_playlist();
    }
}

static void open_folder_callback(GtkMenuItem *menuitem, gpointer user_data) {
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    chooser = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new("Open Folder", window, action, ("Cancel"), GTK_RESPONSE_CANCEL, ("Open"), GTK_RESPONSE_ACCEPT, NULL));
    gtk_file_chooser_set_select_multiple((chooser), 0);
    int res = gtk_dialog_run(GTK_DIALOG(chooser));
    if (res == GTK_RESPONSE_ACCEPT) {
        DIR *dir;
        struct dirent *entry;
        char *dirname = gtk_file_chooser_get_current_folder(chooser);
        if (!(dir = opendir(dirname))) {
            g_free(dirname);
            gtk_widget_destroy(GTK_WIDGET(chooser));
            return;
        }
        while ((entry = readdir(dir))) {
            char *dot = strrchr(entry->d_name, '.');
            char *abs_path = malloc(sizeof(char) * 1024);
            sprintf(abs_path, "%s/%s", dirname, entry->d_name);
            if (dot && !strcmp(dot, ".mp3")) add_file_to_playlist(g_lst_playlist, abs_path);
            free(abs_path);
        }
        set_current_row_filename(NULL);
        g_free(dirname);
    }
    gtk_widget_destroy(GTK_WIDGET(chooser));
    get_totatl_count();
    if (gtk_toggle_button_get_active(g_tgl_shuffle)) {
        clear_shuffle();
        shuffle_playlist();
    }
}

static void save_playlist_callback(GtkMenuItem *menuitem, gpointer user_data) {
    if (!gtk_list_box_get_row_at_index(g_lst_playlist, 0)) { return; }
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    chooser = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new("Save File", window, action, ("Cancel"), GTK_RESPONSE_CANCEL, ("Save"), GTK_RESPONSE_ACCEPT, NULL));
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
    const gchar *playlist_name = gtk_label_get_text(g_lbl_playlist);
    if (current_playlist) {
        gtk_file_chooser_set_filename(chooser, current_playlist);
    } else if (playlist_name && strlen(playlist_name)) {
        gtk_file_chooser_set_current_name(chooser, playlist_name);
    } else {
        gtk_file_chooser_set_current_name(chooser, "Untitled");
    }
    gint res = gtk_dialog_run(GTK_DIALOG(chooser));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename(chooser);
        save_playlist(filename);
        gtk_label_set_text(g_lbl_playlist, g_path_get_basename(filename));
        if (current_playlist) free(current_playlist);
        current_playlist = strdup(filename);
        g_free(filename);
        printf("%s\n", current_playlist);
    }
    gtk_widget_destroy(GTK_WIDGET(chooser));
}

static void open_playlist_callback(GtkMenuItem *menuitem, gpointer user_data) {
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    chooser = GTK_FILE_CHOOSER(gtk_file_chooser_dialog_new("Open File", window, action, ("Cancel"), GTK_RESPONSE_CANCEL, ("Open"), GTK_RESPONSE_ACCEPT, NULL));
    gtk_file_chooser_set_select_multiple(chooser, 0);
    int res = gtk_dialog_run(GTK_DIALOG(chooser));
    if (res == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename(chooser);
        if (filename) {
            FILE *fp = fopen(filename, "r");
            int line_max_len = 1024;
            char *line = malloc(sizeof(char) * line_max_len);
            char *dot;
            clear_playlist();
            while (fgets(line, line_max_len, fp)) {
                dot = strrchr(line, '.');
                line[strcspn(line, "\r\n")] = '\0';
                if (dot && !strcmp(dot, ".mp3")) { add_file_to_playlist(g_lst_playlist, line); }
            }
            gtk_label_set_text(g_lbl_playlist, g_path_get_basename(filename));
            current_playlist = strdup(filename);
            set_current_row_filename(NULL);
            free(line);
            fclose(fp);
        }
        g_free(filename);
    }
    gtk_widget_destroy(GTK_WIDGET(chooser));
    get_totatl_count();
    if (gtk_toggle_button_get_active(g_tgl_shuffle)) {
        shuffle_playlist();
    }
}

static void clear_playlist_callback(GtkMenuItem *menuitem, gpointer user_data) {
    clear_playlist();
    get_totatl_count();
}

static void clear_shuffle() {
    if (shuffle_arr) { free(shuffle_arr); }
    shuffle_arr = NULL;
    shuffle_current = 0;
}

static void add_file_to_playlist(GtkListBox *playlist, const char *filename) {
    GtkWidget *row = gtk_list_box_row_new();
    gtk_drag_dest_set(row, GTK_DEST_DEFAULT_ALL, row_entries, GTK_TARGET_SAME_APP, GDK_ACTION_MOVE);
    GtkWidget *handle = gtk_event_box_new();
    gtk_drag_source_set(handle, GDK_BUTTON1_MASK, row_entries, GTK_TARGET_SAME_APP, GDK_ACTION_MOVE);
    g_signal_connect (handle, "drag-begin", G_CALLBACK(drag_playlist_row), NULL);
    g_signal_connect (handle, "drag-data-get", G_CALLBACK(playlist_row_drag_data_get), NULL);
    g_signal_connect (row, "drag-data-received", G_CALLBACK(playlist_row_drag_data_received), NULL);
    GtkWidget *grid = gtk_grid_new();
    GtkWidget *lbl_path = gtk_label_new(filename);
    GtkWidget *lbl_name = gtk_label_new(g_path_get_basename(filename));
    PangoAttrList *attrlist = pango_attr_list_new();
    PangoAttribute *df_attr;
    PangoFontDescription *df_name;
    gtk_label_set_ellipsize(GTK_LABEL(lbl_path), PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(lbl_name), PANGO_ELLIPSIZE_END);
    gtk_label_set_xalign(GTK_LABEL(lbl_path), (gfloat) 0.0);
    gtk_label_set_xalign(GTK_LABEL(lbl_name), (gfloat) 0.0);
    gtk_widget_set_margin_start(lbl_path, 16);
    gtk_widget_set_margin_start(lbl_name, 16);
    gtk_widget_set_margin_end(lbl_path, 16);
    gtk_widget_set_margin_end(lbl_name, 16);
    gtk_widget_set_margin_top(lbl_name, 8);
    gtk_widget_set_margin_top(lbl_path, 4);
    gtk_widget_set_margin_bottom(lbl_path, 8);
    df_name = pango_font_description_new();
    pango_font_description_set_family(df_name, "Tahoma");
    pango_font_description_set_size(df_name, 14 * PANGO_SCALE);
    pango_font_description_set_weight(df_name, PANGO_WEIGHT_BOLD);
    df_attr = pango_attr_font_desc_new(df_name);
    pango_font_description_free(df_name);
    pango_attr_list_insert(attrlist, df_attr);
    gtk_label_set_attributes(GTK_LABEL(lbl_name), attrlist);
    gtk_grid_attach(GTK_GRID(grid), lbl_name, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), lbl_path, 0, 1, 1, 1);
    gtk_container_add(GTK_CONTAINER(handle), grid);
    gtk_container_add(GTK_CONTAINER(row), handle);
    gtk_list_box_prepend(playlist, row);
    gtk_widget_show_all(GTK_WIDGET(playlist));
    pango_attr_list_unref(attrlist);
}

static void drag_playlist_row(GtkWidget *widget, GdkDragContext *context, gpointer user_data) {
    GtkWidget *row = gtk_widget_get_ancestor(widget, GTK_TYPE_LIST_BOX_ROW);
    GdkRectangle *alloc = malloc(sizeof(GdkRectangle));
    gtk_widget_get_allocation(row, alloc);
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, alloc->width, alloc->height);
    cairo_t *cr = cairo_create(surface);
    GtkStyleContext *ctx = gtk_widget_get_style_context(row);
    gtk_style_context_add_class(ctx, "drag-row");
    gtk_widget_draw(row, cr);
    gtk_style_context_remove_class(ctx, "drag-row");
    gtk_drag_set_icon_surface(context, surface);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_free(alloc);
}

static void playlist_row_drag_data_get(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *data, guint info, guint time, gpointer user_data) {
    gtk_selection_data_set(data, gdk_atom_intern_static_string("GTK_LIST_BOX_ROW"), 32, (const guchar *) &widget, sizeof(gpointer));
}

static void playlist_row_drag_data_received(GtkWidget *widget, GdkDragContext *context, int x, int y, GtkSelectionData *data, guint info, guint time, gpointer user_data) {
    GtkWidget *handle = *(gpointer *) gtk_selection_data_get_data(data);
    GtkWidget *source = gtk_widget_get_ancestor(handle, GTK_TYPE_LIST_BOX_ROW);
    if (source == widget) { return; }
    GtkWidget *source_parent = gtk_widget_get_parent(source);
    GtkWidget *target_list = gtk_widget_get_parent(widget);
    int position = gtk_list_box_row_get_index(GTK_LIST_BOX_ROW (widget));
    g_object_ref (source);
    gtk_container_remove(GTK_CONTAINER (source_parent), source);
    gtk_list_box_insert(GTK_LIST_BOX (target_list), source, position);
    g_object_unref(source);
}

//static void playlist_row_drag_file_received(GtkWidget *widget, GdkDragContext *context, int x, int y, GtkSelectionData *data, guint info, guint time, gpointer user_data) {
//    char *result = (char *) gtk_selection_data_get_text(data);
//    printf("data %s:", result);
//    free(result);
//    fflush(stdout);
//}

static void highlight_row(int index) {
    GtkListBoxRow *row;
    if ((row = gtk_list_box_get_row_at_index(g_lst_playlist, index))) {
        GtkEventBox *handle;
        GList *children = gtk_container_get_children(GTK_CONTAINER(row));
        handle = GTK_EVENT_BOX(children->data);
        GtkStyleContext *row_context;
        row_context = gtk_widget_get_style_context(GTK_WIDGET(handle));
        gtk_style_context_add_provider(row_context, GTK_STYLE_PROVIDER(style), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        gtk_style_context_add_class(row_context, "highlight-row");
        g_list_free(children);
    }
}

static void unhighlight_row(int index) {
    GtkListBoxRow *row;
    if ((row = gtk_list_box_get_row_at_index(g_lst_playlist, index))) {
        GtkEventBox *handle;
        GList *children = gtk_container_get_children(GTK_CONTAINER(row));
        handle = GTK_EVENT_BOX(children->data);
        GtkStyleContext *row_context;
        row_context = gtk_widget_get_style_context(GTK_WIDGET(handle));
        gtk_style_context_remove_class(row_context, "highlight-row");
        g_list_free(children);
    }
}

static void keep_above_callback(GtkToggleButton *togglebutton, gpointer user_data) {
    gtk_window_set_keep_above(window, gtk_toggle_button_get_active(togglebutton));
}

static void shuffle_callback(GtkToggleButton *togglebutton, gpointer user_data) {
    clear_shuffle();
    if (gtk_toggle_button_get_active(togglebutton)) { shuffle_playlist(); }
}

static void shuffle_playlist() {
    int num1, num2, tmp;
    srand(time(0));
    shuffle_arr = malloc(sizeof(int) * total_count);
    for (int i = 0; i < total_count; i++) { shuffle_arr[i] = i; }
    for (int i = 0; i < total_count / 2; i++) {
        num1 = rand() % total_count;
        num2 = rand() % total_count;
        tmp = shuffle_arr[num1];
        shuffle_arr[num1] = shuffle_arr[num2];
        shuffle_arr[num2] = tmp;
    }
}

static void reset_global_vars() {
    paused = 0;
    pthread_mutex_unlock(&mx_pause);
}

static void pause_player() {
    if (play_th) {
        paused = !paused;
        if (paused) {
            pthread_mutex_lock(&mx_pause);
        } else {
            pthread_mutex_unlock(&mx_pause);
        }
    }
}

static void cleanup_play_th() {
    pthread_mutex_lock(&mx_cleanup);
    play_context_free(&p_ctx);
    reset_global_vars();
    reset_widgets();
    pthread_cond_signal(&cv_cleanup);
    pthread_mutex_unlock(&mx_cleanup);
    cleanup = 0;
}

static void play_context_init(m_context *ctx, const char *filename) {
    ctx->driver = ao_default_driver_id();
    ctx->mh = mpg123_new(NULL, &ctx->err);
    ctx->buffer_size = mpg123_outblock(ctx->mh);
    ctx->buffer = (char *) malloc(ctx->buffer_size * sizeof(unsigned char));
    mpg123_open(ctx->mh, (char *) filename);
    mpg123_getformat(ctx->mh, &ctx->rate, &ctx->channels, &ctx->encoding);
    ctx->format.bits = mpg123_encsize(ctx->encoding) * BITS;
    ctx->format.rate = (int) ctx->rate;
    ctx->format.channels = ctx->channels;
    ctx->format.byte_format = AO_FMT_NATIVE;
    ctx->format.matrix = 0;
    ctx->dev = ao_open_live(ctx->driver, &ctx->format, NULL);
    ctx->frames_c = mpg123_framelength(ctx->mh);
    ctx->frame_d = mpg123_tpf(ctx->mh);
    ctx->duration = (int) (ctx->frame_d * (double) ctx->frames_c);
}

static void play_context_free(m_context *ctx) {
    g_free(ctx->buffer);
    ao_close(ctx->dev);
    mpg123_close(ctx->mh);
    mpg123_delete(ctx->mh);
    mpg123_exit();
    ao_shutdown();
}

static void set_current_row_filename(GtkListBoxRow *row) {
    if (!row) { row = gtk_list_box_get_row_at_index(g_lst_playlist, 0); }
    if (row) {
        GList *handle = gtk_container_get_children(GTK_CONTAINER(row));
        GList *children = gtk_container_get_children(GTK_CONTAINER(handle->data));
        strcpy(current_str, gtk_label_get_text(GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(children->data), 0, 1))));
        g_list_free(children);
        g_list_free(handle);
    }
}

static void set_current_filename_title() {
    char *str_name = g_path_get_basename(current_str);
    gtk_label_set_text(g_lbl_name, str_name);
    g_free(str_name);
}

static int get_id3_data(m_context *ctx) {
    mpg123_scan(ctx->mh);
    meta = mpg123_meta_check(ctx->mh);
    int res = 0;
    if (meta & MPG123_ID3 && mpg123_id3(ctx->mh, NULL, &ctx->v2) == MPG123_OK) {
        if (ctx->v2 != NULL) {
            if (ctx->v2->title && ctx->v2->title->p && ctx->v2->title->fill) {
                gtk_label_set_text(g_lbl_name, ctx->v2->title->p);
            } else { set_current_filename_title(); }
            if (ctx->v2->album && ctx->v2->album->p && ctx->v2->album->fill) gtk_label_set_text(g_lbl_album, ctx->v2->album->p);
            if (ctx->v2->artist && ctx->v2->artist->p && ctx->v2->artist->fill) gtk_label_set_text(g_lbl_artist, ctx->v2->artist->p);
            if (ctx->v2->comment && ctx->v2->comment->p && ctx->v2->comment->fill) gtk_label_set_text(g_lbl_comment, ctx->v2->comment->p);
            get_file_album_art();
            res = 1;
        }
    }
    return res;
}

static void set_default_album_art() {
    GdkPixbuf *logo = gdk_pixbuf_new_from_resource_at_scale("/res/img/logo.jpg", 128, 128, 1, NULL);
    gtk_image_set_from_pixbuf(g_img_logo, logo);
    g_object_unref(logo);
}

static void get_file_album_art() {
    if (p_ctx.v2 && p_ctx.v2->picture && p_ctx.v2->picture->size && p_ctx.v2->picture->data) {
        GdkPixbuf *res = NULL;
        GdkPixbufLoader *loader = NULL;
        loader = gdk_pixbuf_loader_new();
        gdk_pixbuf_loader_set_size(loader, 128, 128);
        gdk_pixbuf_loader_write(loader, p_ctx.v2->picture->data, p_ctx.v2->picture->size, NULL);
        res = gdk_pixbuf_loader_get_pixbuf(loader);
        gtk_image_set_from_pixbuf(g_img_logo, res);
        g_object_unref(res);
        g_object_unref(loader);
    } else { set_default_album_art(); }
}

static void reset_widgets() {
    gtk_range_set_range(g_sld_position, 0, 100.0);
    gtk_range_set_fill_level(g_sld_position, 0.0);
    gtk_range_set_value(g_sld_position, 0.0);
    gtk_label_set_text(g_lbl_position, "");
    gtk_label_set_text(g_lbl_name, "");
    gtk_label_set_text(g_lbl_album, "");
    gtk_label_set_text(g_lbl_artist, "");
    gtk_label_set_text(g_lbl_comment, "");
    unhighlight_row(current_index);
}

static void save_playlist(const char *filename) {
    FILE *fp = fopen(filename, "w");
    struct stat *fs = malloc(sizeof(struct stat));
    stat(filename, fs);
    fchmod(fp->_fileno, fs->st_mode | S_IWGRP | S_IRGRP | S_IWOTH | S_IROTH);
    free(fs);
    for (int i = 0;; i++) {
        GtkListBoxRow *row = gtk_list_box_get_row_at_index(g_lst_playlist, i);
        if (!row) { break; }
        GList *children = gtk_container_get_children(GTK_CONTAINER(row));
        const gchar *str = gtk_label_get_text(GTK_LABEL(gtk_grid_get_child_at(GTK_GRID(children->data), 0, 1)));
        fputs(str, fp);
        fputc('\n', fp);
        g_list_free(children);
    }
    fclose(fp);
}

static int get_totatl_count() {
    total_count = 0;
    while (gtk_list_box_get_row_at_index(g_lst_playlist, total_count)) { total_count++; };
    return total_count;
}

static void clear_playlist() {
    GtkWidget *row;
    GtkListBox *playlist = g_lst_playlist;
    while ((row = GTK_WIDGET(gtk_list_box_get_row_at_index(playlist, 0)))) {
        gtk_widget_destroy(row);
    }
    clear_shuffle();
}

/* Make window height only resizable */
static void window_size_allocate(GtkWidget *widget, GdkRectangle *allocation, gpointer user_data) {
    GdkGeometry hints;
    hints.min_width = 0;
    hints.max_width = 500;
    hints.min_height = 0;
    hints.max_height = G_MAXINT;
    gtk_window_set_geometry_hints(GTK_WINDOW (widget), NULL, &hints, (GdkWindowHints) (GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE));
}

static char *seconds_to_str(int seconds) {
    char *str = malloc(sizeof(char) * 24);
    sprintf(str, "%d:%02d", seconds / 60, seconds % 60);
    return str;
}

static double min(double v_1, double v_2) {
    if (v_1 < v_2) return v_1;
    return v_2;
}

static double max(double v_1, double v_2) {
    if (v_1 < v_2) return v_2;
    return v_1;
}

/*
* SOME UNUSED CODE PARTS...
*
* // Create GDKPixbuf from file...
* FILE *album_art = fopen("tmp_album_art_file", "wb");
* fwrite(pkt.data, pkt.size, 1, album_art);
* fclose(album_art);
* res = gdk_pixbuf_new_from_file_at_scale("tmp_album_art_file", 128, 128, 1, NULL);
* remove("tmp_album_art_file");
*
* if (G_IS_INITIALLY_UNOWNED(grid)) { g_object_ref_sink(row); }
* if (G_IS_INITIALLY_UNOWNED(grid)) { g_object_ref_sink(grid); }
* if (G_IS_INITIALLY_UNOWNED(grid)) { g_object_ref_sink(lbl_path); }
* if (G_IS_INITIALLY_UNOWNED(grid)) { g_object_ref_sink(lbl_name); }
*
* static GdkPixbuf *get_file_album_art(AVFormatContext *format) {
*     GdkPixbuf *res = NULL;
*     GdkPixbufLoader *loader = NULL;
*     for (int i = 0; i < format->nb_streams; i++) {
*         if (format->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
*             AVPacket pkt = format->streams[i]->attached_pic;
*             loader = gdk_pixbuf_loader_new();
*             gdk_pixbuf_loader_set_size(loader, 128, 128);
*             gdk_pixbuf_loader_write(loader, pkt.data, pkt.size, NULL);
*             res = gdk_pixbuf_loader_get_pixbuf(loader);
*             av_packet_unref(&pkt);
*             break;
*         }
*     }
*     return res;
* }
*/

// if (ctx->v1 != NULL) {
//     if (strlen(ctx->v1->title)) {
//         gtk_label_set_text(GTK_LABEL(g_lbl_name), ctx->v1->title);
//     } else { set_current_filename_title(); }
//     if (strlen(ctx->v1->album)) gtk_label_set_text(GTK_LABEL(g_lbl_album), ctx->v1->album);
//     if (strlen(ctx->v1->artist)) gtk_label_set_text(GTK_LABEL(g_lbl_artist), ctx->v1->artist);
//     if (strlen(ctx->v1->comment)) gtk_label_set_text(GTK_LABEL(g_lbl_comment), ctx->v1->comment);
//     res = 1;
// }