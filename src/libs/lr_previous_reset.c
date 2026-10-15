/*
    This file is part of darktable,
    Copyright (C) 2026 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Lightroom Classic layout: "Previous" and "Reset" buttons at the bottom
   of the right panel of the darkroom (Develop) view.
   - Previous: apply all the settings of the previously edited photo
   - Reset: back to the default settings (undoable with ctrl+z) */

#include "common/darktable.h"
#include "common/history.h"
#include "common/image.h"
#include "control/conf.h"
#include "control/control.h"
#include "control/signal.h"
#include "develop/develop.h"
#include "gui/gtk.h"
#include "libs/lib.h"
#include "libs/lib_api.h"

DT_MODULE(1)

typedef struct dt_lib_lr_previous_reset_t
{
  dt_imgid_t current;
  dt_imgid_t previous;
  GtkWidget *previous_button;
} dt_lib_lr_previous_reset_t;

const char *name(dt_lib_module_t *self)
{
  return _("previous & reset");
}

dt_view_type_flags_t views(dt_lib_module_t *self)
{
  return dt_conf_get_bool("plugins/darkroom/lightroom_ui") ? DT_VIEW_DARKROOM : DT_VIEW_NONE;
}

uint32_t container(dt_lib_module_t *self)
{
  return DT_UI_CONTAINER_PANEL_RIGHT_BOTTOM;
}

gboolean expandable(dt_lib_module_t *self)
{
  return FALSE;
}

int position(const dt_lib_module_t *self)
{
  return 1;
}

static void _track_image(dt_lib_module_t *self)
{
  dt_lib_lr_previous_reset_t *d = self->data;
  const dt_imgid_t imgid = darktable.develop ? darktable.develop->image_storage.id : NO_IMGID;
  if(!dt_is_valid_imgid(imgid) || imgid == d->current) return;

  if(dt_is_valid_imgid(d->current)) d->previous = d->current;
  d->current = imgid;
  gtk_widget_set_sensitive(d->previous_button, dt_is_valid_imgid(d->previous));
}

static void _image_changed_callback(gpointer instance, dt_lib_module_t *self)
{
  _track_image(self);
}

static void _refresh_darkroom(const dt_imgid_t imgid)
{
  dt_dev_modulegroups_set(darktable.develop, dt_dev_modulegroups_get(darktable.develop));
  dt_image_synch_xmp(imgid);
  dt_control_queue_redraw_center();
}

static void _reset_clicked(GtkButton *button, dt_lib_module_t *self)
{
  const dt_imgid_t imgid = darktable.develop->image_storage.id;
  if(!dt_is_valid_imgid(imgid)) return;

  dt_dev_undo_start_record(darktable.develop);
  // discard the edit: the default (auto-applied) settings come back
  dt_history_delete(imgid, FALSE);
  dt_dev_undo_end_record(darktable.develop);

  _refresh_darkroom(imgid);
  dt_control_log(_("settings reset to default"));
}

static void _previous_clicked(GtkButton *button, dt_lib_module_t *self)
{
  dt_lib_lr_previous_reset_t *d = self->data;
  const dt_imgid_t imgid = darktable.develop->image_storage.id;
  if(!dt_is_valid_imgid(imgid)) return;

  if(!dt_is_valid_imgid(d->previous) || d->previous == imgid)
  {
    dt_control_log(_("there is no previous photo yet"));
    return;
  }

  dt_dev_write_history(darktable.develop);
  dt_dev_undo_start_record(darktable.develop);
  // like Lightroom: the photo gets the settings of the previous one,
  // so start from the defaults and add all the previous photo's settings
  dt_history_delete(imgid, FALSE);
  dt_history_copy_and_paste_on_image(d->previous, imgid, TRUE, NULL, FALSE, FALSE, FALSE);
  dt_dev_undo_end_record(darktable.develop);

  _refresh_darkroom(imgid);
  dt_control_log(_("settings of the previous photo applied"));
}

void gui_init(dt_lib_module_t *self)
{
  dt_lib_lr_previous_reset_t *d = g_malloc0(sizeof(dt_lib_lr_previous_reset_t));
  self->data = d;
  d->current = NO_IMGID;
  d->previous = NO_IMGID;

  self->widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_name(self->widget, "lr-bottom-buttons");
  gtk_box_set_homogeneous(GTK_BOX(self->widget), TRUE);

  d->previous_button = gtk_button_new_with_label(_("Previous"));
  gtk_widget_set_tooltip_text(d->previous_button,
                              _("apply the settings of the previously edited photo"));
  g_signal_connect(d->previous_button, "clicked", G_CALLBACK(_previous_clicked), self);
  gtk_widget_set_sensitive(d->previous_button, FALSE);

  GtkWidget *reset = gtk_button_new_with_label(_("Reset"));
  gtk_widget_set_tooltip_text(reset, _("reset this photo to the default settings\n"
                                       "(ctrl+z brings the edit back)"));
  g_signal_connect(reset, "clicked", G_CALLBACK(_reset_clicked), self);

  gtk_box_pack_start(GTK_BOX(self->widget), d->previous_button, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(self->widget), reset, TRUE, TRUE, 0);
  gtk_widget_show_all(self->widget);

  DT_CONTROL_SIGNAL_HANDLE(DT_SIGNAL_DEVELOP_IMAGE_CHANGED, _image_changed_callback);
  DT_CONTROL_SIGNAL_HANDLE(DT_SIGNAL_DEVELOP_INITIALIZE, _image_changed_callback);
}

void gui_cleanup(dt_lib_module_t *self)
{
  g_free(self->data);
  self->data = NULL;
}
