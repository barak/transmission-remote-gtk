/*
 * transmission-remote-gtk - A GTK RPC client to Transmission
 * Copyright (C) 2011-2013  Alan Fitton

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef TRG_MODEL_H_
#define TRG_MODEL_H_

#include <gtk/gtk.h>

guint trg_model_remove_removed(GtkListStore *model, gint serial_column, gint64 currentSerial);

gboolean find_existing_model_item(GtkTreeModel *model, gint search_column, gint64 id,
                                  GtkTreeIter *iter);

#endif /* TRG_MODEL_H_ */
