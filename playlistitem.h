#ifndef LAMP4_PLAYLISTITEM_H
#define LAMP4_PLAYLISTITEM_H

#include <gtk-4.0/gtk/gtk.h>

#define PLAYLIST_ITEM_TYPE              (playlist_item_get_type())
#define PLAYLIST_ITEM(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), PLAYLIST_ITEM_TYPE, PlaylistItem))
#define PLAYLIST_ITEM_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), PLAYLIST_ITEM_TYPE, PlaylistItemClass))
#define IS_PLAYLIST_ITEM(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), PLAYLIST_ITEM_TYPE))
#define IS_PLAYLIST_ITEM_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), PLAYLIST_ITEM_TYPE))
#define PLAYLIST_ITEM_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), PLAYLIST_ITEM_TYPE, PlaylistItemClass))

GType playlist_item_get_type(void);

static void playlist_item_class_init(gpointer g_class, gpointer g_class_data);

static void playlist_item_init(GTypeInstance *instance, gpointer g_class);

typedef struct _PlaylistItem PlaylistItem;
typedef struct _PlaylistItemClass PlaylistItemClass;

struct _PlaylistItem {
    GObject parent;
    char filePath[1024];
    char name[128];
};

struct _PlaylistItemClass {
    GObjectClass parent;
};

#endif //LAMP4_PLAYLISTITEM_H
