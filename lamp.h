#ifndef LAMP_LAMP_H
#define LAMP_LAMP_H

#include "playlistitem.h"
#include "data_types.h"

/*********************************************************
 * GTK Application Functions
*********************************************************/

AppWidgets *appWidgetsCreate();

int stateObserver(gpointer data);

void configureUI(AppState *state);

GtkWidget *windowInit(AppState *state);

AppState *appStateCreate(GApplication *app);

void activate(GApplication *app, gpointer user_data);

void appActivate(GApplication *app, gpointer user_data);

void appOpen(GApplication *application, GFile **files, gint n_files, const gchar *hint, gpointer user_data);

/*********************************************************
 * Playback Functions
*********************************************************/

void play(AppState *state);

void playNext(AppState *state, gboolean force);

void playThreadStop(AppState *state);

void playThreadCreate(AppState *state);

static void clearShuffle(AppState *state);

static void shufflePlaylist(AppState *state);

void playBtnCB(GtkButton *self, gpointer user_data);

void stopBtnCB(GtkButton *self, gpointer user_data);

void nextBtnCB(GtkButton *self, gpointer user_data);

void prevBtnCB(GtkButton *self, gpointer user_data);

void pauseBtnCB(GtkButton *self, gpointer user_data);

void volumeChangedCb(GtkRange *self, gpointer user_data);

void repeatTglCB(GtkToggleButton *self, gpointer user_data);

void shuffleTglCB(GtkToggleButton *self, gpointer user_data);

int setNextItem(AppState *state, gboolean backwards, gboolean force);

void playlistitemActivate(GtkListView *list, guint position, AppState *state);

void playlistitemActivateCb(GtkListView *list, guint position, gpointer unused);

gboolean positionChangedCb(GtkRange *self, GtkScrollType *scroll, gdouble value, gpointer user_data);

/*********************************************************
 * Playlist Functions
*********************************************************/

void playlistAddFile(GListStore *playlist, GFile *file);

void playlistLoad(const char *filename, AppState *state);

void playlistSave(const char *filename, AppState *state);

void addFile(GSimpleAction *action, GVariant *parameter, gpointer data);

void openFolder(GSimpleAction *action, GVariant *parameter, gpointer data);

void onFileAddCb(GtkNativeDialog *native, int response, gpointer user_data);

void openPlaylist(GSimpleAction *action, GVariant *parameter, gpointer data);

void savePlaylist(GSimpleAction *action, GVariant *parameter, gpointer data);

void playlistClear(GSimpleAction *action, GVariant *parameter, gpointer data);

void onFolderAddCb(GtkNativeDialog *native, int response, gpointer user_data);

void onPlaylistOpenCb(GtkNativeDialog *native, int response, gpointer user_data);

void onPlaylistSaveCb(GtkNativeDialog *native, int response, gpointer user_data);

void playlistitemBindCb(GtkListItemFactory *factory, GtkListItem *list_item, gpointer *user_data);

/*********************************************************
 * User Interface Functions
*********************************************************/

void setupUI(AppState *state);

void resetUI(AppState *state);

void updateUI(AppState *state);

void setPositionStr(AppState *state);

void setDefaultImage(GtkWidget *image);

void playlistitemSetupCb(GtkListItemFactory *factory, GtkListItem *list_item);

int setPictureToWidget(const char *filename, GtkWidget *widget, int width, int height);

//guint create_generic_signal(const gchar *name);

#endif //LAMP_LAMP_H