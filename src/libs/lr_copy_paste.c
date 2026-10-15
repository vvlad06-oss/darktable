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

/* Lightroom Classic layout: "Copy..." and "Paste" buttons at the bottom
   of the left panel of the darkroom (Develop) view. */

#include "common/darktable.h"
#include "common/history.h"
#include "control/conf.h"
#include "control/control.h"
#include "control/jobs/control_jobs.h"
#include "develop/develop.h"
#include "gui/gtk.h"
#include "libs/lib.h"
#include "libs/lib_api.h"

DT_MODULE(1)

const char *name(dt_lib_module_t *self)
{
  return _("copy & paste settings");
}

dt_view_type_flags_t views(dt_lib_module_t *self)
{
  return dt_conf_get_bool("plugins/darkroom/lightroom_ui") ? DT_VIEW_DARKROOM : DT_VIEW_NONE;
}

uint32_t container(dt_lib_module_t *self)
{
  return DT_UI_CONTAINER_PANEL_LEFT_BOTTOM;
}

gboolean expandable(dt_lib_module_t *self)
{
  return FALSE;
}

int position(const dt_lib_module_t *self)
{
  return 0;
}

static void _copy_clicked(GtkButton *button, dt_lib_module_t *self)
{
  const dt_imgid_t imgid = darktable.develop->image_storage.id;
  if(!dt_is_valid_imgid(imgid)) return;

  // like Lightroom's "Copy Settings" dialog: choose what to copy
  if(dt_history_copy_parts(imgid))
    dt_control_log(_("settings copied"));
}

static void _paste_clicked(GtkButton *button, dt_lib_module_t *self)
{
  const dt_imgid_t imgid = darktable.develop->image_storage.id;
  if(!dt_is_valid_imgid(imgid)) return;

  if(!dt_is_valid_imgid(darktable.view_manager->copy_paste.copied_imageid))
  {
    dt_control_log(_("copy the settings of a photo first"));
    return;
  }
  if(darktable.view_manager->copy_paste.copied_imageid == imgid)
  {
    dt_control_log(_("select another photo in the filmstrip to paste the settings"));
    return;
  }

  dt_dev_undo_start_record(darktable.develop);
  dt_control_paste_history(g_list_append(NULL, GINT_TO_POINTER(imgid)));
  dt_dev_undo_end_record(darktable.develop);
  dt_dev_modulegroups_set(darktable.develop, dt_dev_modulegroups_get(darktable.develop));
}

void gui_init(dt_lib_module_t *self)
{
  self->data = NULL;
  self->widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_name(self->widget, "lr-bottom-buttons");
  gtk_box_set_homogeneous(GTK_BOX(self->widget), TRUE);

  GtkWidget *copy = gtk_button_new_with_label(_("Copy..."));
  gtk_widget_set_tooltip_text(copy, _("copy the settings of this photo,\n"
                                      "a dialog lets you choose which ones"));
  g_signal_connect(copy, "clicked", G_CALLBACK(_copy_clicked), self);

  GtkWidget *paste = gtk_button_new_with_label(_("Paste"));
  gtk_widget_set_tooltip_text(paste, _("paste the copied settings onto this photo"));
  g_signal_connect(paste, "clicked", G_CALLBACK(_paste_clicked), self);

  gtk_box_pack_start(GTK_BOX(self->widget), copy, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(self->widget), paste, TRUE, TRUE, 0);
  gtk_widget_show_all(self->widget);

}

void gui_cleanup(dt_lib_module_t *self)
{
}
