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

/* texture: the Texture slider of Lightroom.

   It changes the contrast of the medium-sized details (skin, foliage,
   fabric) and leaves alone both the finest scale (noise, sharpening) and
   the large structures (local contrast / clarity).

   The luminance is taken in log2 (EV) so that the effect is the same in
   the shadows and in the highlights. The band of medium details is the
   difference of two gaussian blurs of it; adding it back, scaled, raises
   (positive amount) or smooths (negative amount) the texture. The RGB
   ratios are kept, so colors do not shift. */

#include "bauhaus/bauhaus.h"
#include "common/gaussian.h"
#include "common/imagebuf.h"
#include "common/iop_profile.h"
#include "control/control.h"
#include "develop/develop.h"
#include "develop/imageop.h"
#include "develop/imageop_gui.h"
#include "develop/tiling.h"
#include "gui/gtk.h"
#include "iop/iop_api.h"

#include <gtk/gtk.h>
#include <math.h>
#include <stdlib.h>

DT_MODULE_INTROSPECTION(1, dt_iop_texture_params_t)

// scales of the band of medium details, in pixels of the full image
#define TEXTURE_FINE_SIGMA 1.2f
#define TEXTURE_COARSE_SIGMA 5.0f

typedef struct dt_iop_texture_params_t
{
  float amount; // $MIN: -1.0 $MAX: 1.0 $DEFAULT: 0.0 $DESCRIPTION: "texture"
} dt_iop_texture_params_t;

typedef struct dt_iop_texture_gui_data_t
{
  GtkWidget *amount;
} dt_iop_texture_gui_data_t;

typedef dt_iop_texture_params_t dt_iop_texture_data_t;

const char *name()
{
  return C_("modulename", "texture");
}

const char *aliases()
{
  return _("detail|structure|skin|smooth");
}

const char **description(dt_iop_module_t *self)
{
  return dt_iop_set_description
    (self,
     _("increase or smooth the medium-sized details of the image,\n"
       "such as skin, foliage or fabric, without touching the noise\n"
       "and the large structures"),
     _("creative"),
     _("linear, RGB, scene-referred"),
     _("frequential, RGB"),
     _("linear, RGB, scene-referred"));
}

int default_group()
{
  return IOP_GROUP_TONE | IOP_GROUP_EFFECTS;
}

int flags()
{
  return IOP_FLAGS_SUPPORTS_BLENDING | IOP_FLAGS_ALLOW_TILING;
}

dt_iop_colorspace_type_t default_colorspace(dt_iop_module_t *self,
                                            dt_dev_pixelpipe_t *pipe,
                                            dt_dev_pixelpipe_iop_t *piece)
{
  return IOP_CS_RGB;
}

static inline float _coarse_sigma(const dt_dev_pixelpipe_iop_t *piece,
                                  const dt_iop_roi_t *roi_in)
{
  return TEXTURE_COARSE_SIGMA * roi_in->scale / piece->iscale;
}

void tiling_callback(dt_iop_module_t *self,
                     dt_dev_pixelpipe_iop_t *piece,
                     const dt_iop_roi_t *roi_in,
                     const dt_iop_roi_t *roi_out,
                     dt_develop_tiling_t *tiling)
{
  // in + out, three single channel buffers and the blur scratch
  tiling->factor = 2.0f + 3.0f * 0.25f + 0.5f;
  tiling->maxbuf = 1.0f;
  tiling->overhead = 0;
  tiling->overlap = (int)ceilf(4.0f * _coarse_sigma(piece, roi_in));
  tiling->align = 1;
}

void process(dt_iop_module_t *self,
             dt_dev_pixelpipe_iop_t *piece,
             const void *const ivoid,
             void *const ovoid,
             const dt_iop_roi_t *const roi_in,
             const dt_iop_roi_t *const roi_out)
{
  if(!dt_iop_have_required_input_format(4, self, piece->colors, ivoid, ovoid, roi_in, roi_out))
    return;

  const dt_iop_texture_data_t *const d = piece->data;
  const int width = roi_out->width;
  const int height = roi_out->height;
  const size_t npixels = (size_t)width * height;
  const float *const restrict in = (const float *)ivoid;
  float *const restrict out = (float *)ovoid;

  const float fine = TEXTURE_FINE_SIGMA * roi_in->scale / piece->iscale;
  const float coarse = _coarse_sigma(piece, roi_in);

  // nothing to do, or the details are too small to be seen at this size
  if(fabsf(d->amount) < 1e-4f || coarse < 0.5f || width < 3 || height < 3)
  {
    dt_iop_image_copy_by_size(out, in, width, height, 4);
    return;
  }

  float *restrict lum = NULL, *restrict fine_blur = NULL, *restrict coarse_blur = NULL;
  if(!dt_iop_alloc_image_buffers(self, roi_in, roi_out,
                                 1, &lum, 1, &fine_blur, 1, &coarse_blur, 0))
  {
    dt_iop_copy_image_roi(out, in, 4, roi_in, roi_out);
    return;
  }

  // luminance weights of the working profile (linear RGB to Y)
  dt_aligned_pixel_t weight = { 0.2627f, 0.6780f, 0.0593f, 0.0f }; // Rec. 2020
  const dt_iop_order_iccprofile_info_t *const profile =
    dt_ioppr_get_pipe_current_profile_info(self, piece->pipe);
  if(profile)
    for(int c = 0; c < 3; c++) weight[c] = profile->matrix_in[1][c];

  DT_OMP_FOR()
  for(size_t k = 0; k < npixels; k++)
  {
    const float *const px = in + 4 * k;
    const float Y = weight[0] * px[0] + weight[1] * px[1] + weight[2] * px[2];
    const float L = log2f(MAX(Y, 1e-6f));
    lum[k] = L;
    fine_blur[k] = L;
    coarse_blur[k] = L;
  }

  if(fine >= 0.5f) dt_gaussian_mean_blur(fine_blur, width, height, 1, fine);
  dt_gaussian_mean_blur(coarse_blur, width, height, 1, coarse);

  // positive amounts raise the band up to 2.5x, negative ones remove it
  const float gain = d->amount > 0.0f ? 1.5f * d->amount : d->amount;

  DT_OMP_FOR()
  for(size_t k = 0; k < npixels; k++)
  {
    const float *const px = in + 4 * k;
    float *const po = out + 4 * k;

    // medium details in EV, softly limited to half an EV so that strong
    // edges do not halo, and faded out in the deep shadows
    float band = fine_blur[k] - coarse_blur[k];
    band = band / (1.0f + 2.0f * fabsf(band));
    const float Y = exp2f(lum[k]);
    const float shadows = Y / (Y + 0.002f);
    const float ratio = exp2f(gain * band * shadows);

    for_three_channels(c) po[c] = px[c] * ratio;
    po[3] = px[3];
  }

  dt_free_align(lum);
  dt_free_align(fine_blur);
  dt_free_align(coarse_blur);
}

void gui_init(dt_iop_module_t *self)
{
  dt_iop_texture_gui_data_t *g = IOP_GUI_ALLOC(texture);

  g->amount = dt_bauhaus_slider_from_params(self, "amount");
  dt_bauhaus_slider_set_format(g->amount, "%");
  dt_bauhaus_slider_set_digits(g->amount, 0);
  if(dt_conf_get_bool("plugins/darkroom/lightroom_ui"))
    dt_bauhaus_slider_set_format(g->amount, ""); // -100..+100, like Lightroom
  gtk_widget_set_tooltip_text(g->amount,
                              _("positive values bring out medium-sized details,\n"
                                "negative values smooth them (skin)"));
}

// clang-format off
// modelines: These editor modelines have been set for all relevant files by tools/update_modelines.py
// vim: shiftwidth=2 expandtab tabstop=2 cindent
// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-spaces modified;
// clang-format on
