//
//   |                              |
//   |   _` |  __ `__ \   __ \      __ \
//   |  (   |  |   |   |  |   |     | | |
//  _| \__,_| _|  _|  _|  .__/  _| _| |_|
//                       _|
//

#ifndef LAMP_LAMP_H
#define LAMP_LAMP_H

#include "playlistitem.h"
#include "data_types.h"

/*********************************************************
 * GTK Application Functions
*********************************************************/

AppWidgets *appWidgetsCreate();

void configureUI(AppState *state);

GtkWidget *windowInit(AppState *state);

gboolean stateObserver(gpointer user_data);

AppState *appStateCreate(GApplication *app);

void activate(GApplication *app, gpointer user_data);

void appActivate(GApplication *app, gpointer user_data);

void appOpen(GApplication *application, GFile **files, gint n_files, const gchar *hint, gpointer user_data);

gboolean keypressHandler(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer user_data);

/*********************************************************
 * Playback Functions
*********************************************************/

void play(AppState *state);

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

void playNext(AppState *state, gboolean backward, gboolean force);

int setNextItem(AppState *state, gboolean backwards, gboolean force);

void playlistitemActivate(GtkListView *list, guint position, AppState *state);

void playlistitemActivateCb(GtkListView *list, guint position, gpointer user_data);

gboolean positionChangedCb(GtkRange *self, GtkScrollType *scroll, gdouble value, gpointer user_data);

/*********************************************************
 * Playlist Functions
*********************************************************/

void playlistLoad(const char *filename, AppState *state);

void playlistSave(const char *filename, AppState *state);

void addFile(GSimpleAction *action, GVariant *parameter, gpointer data);

void playlistAddFile(GListStore *playlist, GFile *file, AppState *state);

void openFolder(GSimpleAction *action, GVariant *parameter, gpointer data);

void onFileAddCb(GtkFileDialog *self, GAsyncResult *res, gpointer user_data);

void openPlaylist(GSimpleAction *action, GVariant *parameter, gpointer data);

void savePlaylist(GSimpleAction *action, GVariant *parameter, gpointer data);

void playlistClear(GSimpleAction *action, GVariant *parameter, gpointer data);

void onFolderAddCb(GtkFileDialog *self, GAsyncResult *res, gpointer user_data);

void onPlaylistSaveCb(GtkFileDialog *self, GAsyncResult *res, gpointer user_data);

void onPlaylistOpenCb(GtkFileDialog *self, GAsyncResult *res, gpointer user_data);

void playlistitemBindCb(GtkListItemFactory *factory, GtkListItem *list_item, gpointer *user_data);

void playlistitemUnbindCb(GtkListItemFactory *factory, GtkListItem *list_item, gpointer *user_data);

/*********************************************************
 * User Interface Functions
*********************************************************/

void setupUI(AppState *state);

void resetUI(AppState *state);

void setSizes(AppState *state);

void updateUI(AppState *state);

void setPositionStr(AppState *state);

void setDefaultImage(GtkWidget *image);

void highlightRow(AppState *state, guint position);

void unHighlightRow(AppState *state, guint position);

GtkWidget *getRowAtPosition(AppState *state, guint position);

int setPictureToWidget(const char *filename, GtkWidget *widget, int width, int height);

void timeLblPressed(GtkGestureClick *self, gint n_press, gdouble x, gdouble y, gpointer user_data);

void playlistitemSetupCb(GtkListItemFactory *factory, GtkListItem *list_item, gpointer *user_data);

//guint create_generic_signal(const gchar *name);

#endif //LAMP_LAMP_H