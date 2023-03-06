//
//          |                | _)       |    _) |
//   __ \   |   _` |  |   |  |  |   __|  __|  |  __|   _ \  __ `__ \       __|
//   |   |  |  (   |  |   |  |  | \__ \  |    |  |     __/  |   |   |     (
//   .__/  _| \__,_| \__, | _| _| ____/ \__| _| \__| \___| _|  _|  _| _| \___|
//  _|               ____/
//

#include "playlistitem.h"

static GObject *playlist_item_constructor(GType type, guint n_construct_properties, GObjectConstructParam *construct_properties) {
    GObject *obj;
    PlaylistItemClass *klass;
    GObjectClass *parent_class;
    klass = PLAYLIST_ITEM_CLASS(g_type_class_peek(PLAYLIST_ITEM_TYPE));
    parent_class = G_OBJECT_CLASS(g_type_class_peek_parent(klass));
    obj = parent_class->constructor(type, n_construct_properties, construct_properties);
    return obj;
}

static void playlist_item_init(GTypeInstance *instance, gpointer g_class) {
    PlaylistItem *self = (PlaylistItem *) instance;
}

static void playlist_item_class_init(gpointer g_class, gpointer g_class_data) {
    GObjectClass *gobject_class = G_OBJECT_CLASS (g_class);
    PlaylistItemClass *klass = PLAYLIST_ITEM_CLASS(g_class);
    gobject_class->constructor = playlist_item_constructor;
}

GType playlist_item_get_type(void) {
    static GType type = 0;
    if (type == 0) {
        static const GTypeInfo info = {
            sizeof(PlaylistItemClass),
            NULL,                       /* base_init */
            NULL,                       /* base_finalize */
            playlist_item_class_init,   /* class_init */
            NULL,                       /* class_finalize */
            NULL,                       /* class_data */
            sizeof(PlaylistItem),
            0,                          /* n_preallocs */
            playlist_item_init          /* instance_init */
        };
        type = g_type_register_static(G_TYPE_OBJECT, "PlaylistItemType", &info, 0);
    }
    return type;
}