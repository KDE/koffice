/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <stdio.h>
#include <string.h> /* strcpy, strlen */

#include <glib-object.h>

#include "libgimpbase/gimpbase.h"
#include "libgimpcolor/gimpcolor.h"

#include "core/core-types.h"

#include "base/tile.h"
#include "base/tile-manager.h"
#include "base/tile-manager-private.h"

#include "core/gimpchannel.h"
#include "core/gimpdrawable.h"
#include "core/gimpgrid.h"
#include "core/gimpimage.h"
#include "core/gimpimage-grid.h"
#include "core/gimpimage-guides.h"
#include "core/gimplayer.h"
#include "core/gimplayer-floating-sel.h"
#include "core/gimplayermask.h"
#include "core/gimplist.h"
#include "core/gimpparasitelist.h"
#include "core/gimpunit.h"

#include "text/gimptextlayer.h"
#include "text/gimptextlayer-xcf.h"

#include "vectors/gimpanchor.h"
#include "vectors/gimpstroke.h"
#include "vectors/gimpbezierstroke.h"
#include "vectors/gimpvectors.h"
#include "vectors/gimpvectors-compat.h"

#include "xcf-private.h"
#include "xcf-read.h"
#include "xcf-seek.h"
#include "xcf-write.h"

#include "gimp-intl.h"


static bool xcf_save_image_props   (XcfInfo           *info,
                                    KisImage         *gimage,
                                    GError           **error);
static bool xcf_save_layer_props   (XcfInfo           *info,
                                    KisImage         *gimage,
                                    KisLayer         *layer,
                                    GError           **error);
static bool xcf_save_channel_props (XcfInfo           *info,
                                    KisImage         *gimage,
                                    GimpChannel       *channel,
                                    GError           **error);
static bool xcf_save_prop          (XcfInfo           *info,
                                    KisImage         *gimage,
                                    PropType           prop_type,
                                    GError           **error,
                                    ...);
static bool xcf_save_layer         (XcfInfo           *info,
                                    KisImage         *gimage,
                                    KisLayer         *layer,
                                    GError           **error);
static bool xcf_save_channel       (XcfInfo           *info,
                                    KisImage         *gimage,
                                    GimpChannel       *channel,
                                    GError           **error);
static bool xcf_save_hierarchy     (XcfInfo           *info,
                                    TileManager       *tiles,
                                    GError           **error);
static bool xcf_save_level         (XcfInfo           *info,
                                    TileManager       *tiles,
                                    GError           **error);
static bool xcf_save_tile          (XcfInfo           *info,
                                    Tile              *tile,
                                    GError           **error);
static bool xcf_save_tile_rle      (XcfInfo           *info,
                                    Tile              *tile,
                                    guchar            *rlebuf,
                                    GError           **error);
static bool xcf_save_parasite      (XcfInfo           *info,
                                    KisAnnotation      *parasite,
                                    GError           **error);
static bool xcf_save_parasite_list (XcfInfo           *info,
                                    KisAnnotationList  *parasite,
                                    GError           **error);
static bool xcf_save_old_paths     (XcfInfo           *info,
                                    KisImage         *gimage,
                                    GError           **error);
static bool xcf_save_vectors       (XcfInfo           *info,
                                    KisImage         *gimage,
                                    GError           **error);


/* private convenience macros */
#define xcf_write_int32_check_error(info, data, count) G_STMT_START {   \
        info->cp += xcf_write_int32 (info->fp, data, count, &tmp_error); \
        if (tmp_error)                                                  \
        {                                                               \
            g_propagate_error (error, tmp_error);                       \
            return FALSE;                                               \
        }                                                               \
    } G_STMT_END

#define xcf_write_int8_check_error(info, data, count) G_STMT_START {    \
        info->cp += xcf_write_int8 (info->fp, data, count, &tmp_error); \
        if (tmp_error)                                                  \
        {                                                               \
            g_propagate_error (error, tmp_error);                       \
            return FALSE;                                               \
        }                                                               \
    } G_STMT_END

#define xcf_write_float_check_error(info, data, count) G_STMT_START {   \
        info->cp += xcf_write_float (info->fp, data, count, &tmp_error); \
        if (tmp_error)                                                  \
        {                                                               \
            g_propagate_error (error, tmp_error);                       \
            return FALSE;                                               \
        }                                                               \
    } G_STMT_END

#define xcf_write_string_check_error(info, data, count) G_STMT_START {  \
        info->cp += xcf_write_string (info->fp, data, count, &tmp_error); \
        if (tmp_error)                                                  \
        {                                                               \
            g_propagate_error (error, tmp_error);                       \
            return FALSE;                                               \
        }                                                               \
    } G_STMT_END

#define xcf_write_int32_print_error(info, data, count) G_STMT_START {   \
        info->cp += xcf_write_int32 (info->fp, data, count, &error);    \
        if (error)                                                      \
        {                                                               \
            g_message (_("Error saving XCF file: %s"),                  \
                       error->message);                                 \
            return FALSE;                                               \
        }                                                               \
    } G_STMT_END

#define xcf_write_int8_print_error(info, data, count) G_STMT_START {    \
        info->cp += xcf_write_int8 (info->fp, data, count, &error);     \
        if (error)                                                      \
        {                                                               \
            g_message (_("Error saving XCF file: %s"),                  \
                       error->message);                                 \
            return FALSE;                                               \
        }                                                               \
    } G_STMT_END

#define xcf_write_float_print_error(info, data, count) G_STMT_START {   \
        info->cp += xcf_write_float (info->fp, data, count, &error);    \
        if (error)                                                      \
        {                                                               \
            g_message (_("Error saving XCF file: %s"),                  \
                       error->message);                                 \
            return FALSE;                                               \
        }                                                               \
    } G_STMT_END

#define xcf_write_string_print_error(info, data, count) G_STMT_START {  \
        info->cp += xcf_write_string (info->fp, data, count, &error);   \
        if (error)                                                      \
        {                                                               \
            g_message (_("Error saving XCF file: %s"),                  \
                       error->message);                                 \
            return FALSE;                                               \
        }                                                               \
    } G_STMT_END

#define xcf_write_prop_type_check_error(info, prop_type) G_STMT_START { \
        Q_INT32 _prop_int32 = prop_type;                                \
        xcf_write_int32_check_error (info, &_prop_int32, 1);            \
    } G_STMT_END

#define xcf_write_prop_type_print_error(info, prop_type) G_STMT_START { \
        Q_INT32 _prop_int32 = prop_type;                                \
        xcf_write_int32_print_error (info, &_prop_int32, 1);            \
    } G_STMT_END

#define xcf_check_error(x) G_STMT_START {       \
        if (! (x))                              \
            return FALSE;                       \
    } G_STMT_END

#define xcf_print_error(x) G_STMT_START {               \
        if (! (x))                                      \
        {                                               \
            g_message (_("Error saving XCF file: %s"),  \
                       error->message);                 \
            return FALSE;                               \
        }                                               \
    } G_STMT_END


void
xcf_save_choose_format (XcfInfo   *info,
                        KisImage *gimage)
{
    KisLayer *layer;
    GList     *list;

    Q_INT32 save_version = 0;                /* default to oldest */

    if (gimage->cmap)
        save_version = 1;                   /* need version 1 for colormaps */

    for (list = GIMP_LIST (gimage->layers)->list;
         list && save_version < 2;
         list = g_list_next (list))
    {
        layer = GIMP_LAYER (list->data);

        switch (layer->mode)
        {
            /* new layer modes not supported by gimp-1.2 */
        case GIMP_SOFTLIGHT_MODE:
        case GIMP_GRAIN_EXTRACT_MODE:
        case GIMP_GRAIN_MERGE_MODE:
        case GIMP_COLOR_ERASE_MODE:
            save_version = 2;
            break;

        default:
            break;
        }
    }

    info->file_version = save_version;
}

Q_INT32
xcf_save_image (XcfInfo   *info,
		KisImage *gimage)
{
    KisLayer   *layer;
    KisLayer   *floating_layer;
    GimpChannel *channel;
    Q_INT32      saved_pos;
    Q_INT32      offset;
    Q_UINT32        nlayers;
    Q_UINT32        nchannels;
    GList       *list;
    bool     have_selection;
    Q_INT32         t1, t2, t3, t4;
    QCString        version_tag[14];
    GError      *error = NULL;

    floating_layer = gimp_image_floating_sel (gimage);
    if (floating_layer)
        floating_sel_relax (floating_layer, FALSE);

    /* write out the tag information for the image */
    if (info->file_version > 0)
    {
        sprintf (version_tag, "gimp xcf v%03d", info->file_version);
    }
    else
    {
        strcpy (version_tag, "gimp xcf file");
    }
    xcf_write_int8_print_error  (info, (Q_UINT8 *) version_tag, 14);

    /* write out the width, height and image type information for the image */
    xcf_write_int32_print_error (info, (Q_INT32 *) &gimage->width, 1);
    xcf_write_int32_print_error (info, (Q_INT32 *) &gimage->height, 1);
    xcf_write_int32_print_error (info, (Q_INT32 *) &gimage->base_type, 1);

    /* determine the number of layers and channels in the image */
    nlayers   = (Q_UINT32) gimp_container_num_children (gimage->layers);
    nchannels = (Q_UINT32) gimp_container_num_children (gimage->channels);

    /* check and see if we have to save out the selection */
    have_selection = gimp_channel_bounds (gimp_image_get_mask (gimage),
                                          &t1, &t2, &t3, &t4);
    if (have_selection)
        nchannels += 1;

    /* write the property information for the image.
     */

    xcf_print_error (xcf_save_image_props (info, gimage, &error));

    /* save the current file position as it is the start of where
     *  we place the layer offset information.
     */
    saved_pos = info->cp;

    /* seek to after the offset lists */
    xcf_print_error (xcf_seek_pos (info,
                                   info->cp + (nlayers + nchannels + 2) * 4,
                                   &error));

    for (list = GIMP_LIST (gimage->layers)->list;
         list;
         list = g_list_next (list))
    {
        layer = (KisLayer *) list->data;

        /* save the start offset of where we are writing
         *  out the next layer.
         */
        offset = info->cp;

        /* write out the layer. */
        xcf_print_error (xcf_save_layer (info, gimage, layer, &error));

        /* seek back to where we are to write out the next
         *  layer offset and write it out.
         */
        xcf_print_error (xcf_seek_pos (info, saved_pos, &error));
        xcf_write_int32_print_error (info, &offset, 1);

        /* increment the location we are to write out the
         *  next offset.
         */
        saved_pos = info->cp;

        /* seek to the end of the file which is where
         *  we will write out the next layer.
         */
        xcf_print_error (xcf_seek_end (info, &error));
    }

    /* write out a '0' offset position to indicate the end
     *  of the layer offsets.
     */
    offset = 0;
    xcf_print_error (xcf_seek_pos (info, saved_pos, &error));
    xcf_write_int32_print_error (info, &offset, 1);
    saved_pos = info->cp;
    xcf_print_error (xcf_seek_end (info, &error));

    list = GIMP_LIST (gimage->channels)->list;

    while (list || have_selection)
    {
        if (list)
	{
            channel = (GimpChannel *) list->data;

            list = g_list_next (list);
	}
        else
	{
            channel = gimage->selection_mask;
            have_selection = FALSE;
	}

        /* save the start offset of where we are writing
         *  out the next channel.
         */
        offset = info->cp;

        /* write out the layer. */
        xcf_print_error (xcf_save_channel (info, gimage, channel, &error));

        /* seek back to where we are to write out the next
         *  channel offset and write it out.
         */
        xcf_print_error (xcf_seek_pos (info, saved_pos, &error));
        xcf_write_int32_print_error (info, &offset, 1);

        /* increment the location we are to write out the
         *  next offset.
         */
        saved_pos = info->cp;

        /* seek to the end of the file which is where
         *  we will write out the next channel.
         */
        xcf_print_error (xcf_seek_end (info, &error));
    }

    /* write out a '0' offset position to indicate the end
     *  of the channel offsets.
     */
    offset = 0;
    xcf_print_error (xcf_seek_pos (info, saved_pos, &error));
    xcf_write_int32_print_error (info, &offset, 1);
    saved_pos = info->cp;

    if (floating_layer)
        floating_sel_rigor (floating_layer, FALSE);

    return !ferror(info->fp);
}

static bool
xcf_save_image_props (XcfInfo   *info,
		      KisImage *gimage,
		      GError   **error)
{
    KisAnnotation *parasite = NULL;
    GimpUnit      unit     = gimp_image_get_unit (gimage);

    /* check and see if we should save the colormap property */
    if (gimage->cmap)
        xcf_check_error (xcf_save_prop (info, gimage, PROP_COLORMAP, error,
                                        gimage->num_cols, gimage->cmap));

    if (info->compression != COMPRESS_NONE)
        xcf_check_error (xcf_save_prop (info, gimage, PROP_COMPRESSION,
                                        error, info->compression));

    if (gimage->guides)
        xcf_check_error (xcf_save_prop (info, gimage, PROP_GUIDES,
                                        error, gimage->guides));

    xcf_check_error (xcf_save_prop (info, gimage, PROP_RESOLUTION, error,
                                    gimage->xresolution, gimage->yresolution));

    xcf_check_error (xcf_save_prop (info, gimage, PROP_TATTOO, error,
                                    gimage->tattoo_state));

    if (gimp_parasite_list_length (gimage->parasites) > 0)
        xcf_check_error (xcf_save_prop (info, gimage, PROP_PARASITES,
                                        error, gimage->parasites));

    if (unit < _gimp_unit_get_number_of_built_in_units (gimage->gimp))
        xcf_check_error (xcf_save_prop (info, gimage, PROP_UNIT, error, unit));

    if (gimp_container_num_children (gimage->vectors) > 0)
    {
        if (gimp_vectors_compat_is_compatible (gimage))
            xcf_check_error (xcf_save_prop (info, gimage, PROP_PATHS, error));
        else
            xcf_check_error (xcf_save_prop (info, gimage, PROP_VECTORS, error));
    }

    if (unit >= _gimp_unit_get_number_of_built_in_units (gimage->gimp))
        xcf_check_error (xcf_save_prop (info, gimage, PROP_USER_UNIT, error, unit));

    if (GIMP_IS_GRID (gimage->grid))
    {
        GimpGrid *grid = gimp_image_get_grid (gimage);

        parasite = gimp_grid_to_parasite (grid);
        gimp_parasite_list_add (GIMP_IMAGE (gimage)->parasites, parasite);
    }

    if (gimp_parasite_list_length (GIMP_IMAGE (gimage)->parasites) > 0)
    {
        xcf_check_error (xcf_save_prop (info, gimage, PROP_PARASITES, error,
                                        GIMP_IMAGE (gimage)->parasites));
    }

    if (parasite)
    {
        gimp_parasite_list_remove (GIMP_IMAGE (gimage)->parasites,
                                   gimp_parasite_name (parasite));
        gimp_parasite_free (parasite);
    }

    xcf_check_error (xcf_save_prop (info, gimage, PROP_END, error));

    return TRUE;
}

static bool
xcf_save_layer_props (XcfInfo   *info,
		      KisImage *gimage,
		      KisLayer *layer,
		      GError   **error)
{
    KisAnnotation *parasite = NULL;

    if (layer == gimp_image_get_active_layer (gimage))
        xcf_check_error (xcf_save_prop (info, gimage, PROP_ACTIVE_LAYER, error));

    if (layer == gimp_image_floating_sel (gimage))
    {
        info->floating_sel_drawable = layer->fs.drawable;
        xcf_check_error (xcf_save_prop (info, gimage, PROP_FLOATING_SELECTION,
                                        error));
    }

    xcf_check_error (xcf_save_prop (info, gimage, PROP_OPACITY, error,
                                    layer->opacity));
    xcf_check_error (xcf_save_prop (info, gimage, PROP_VISIBLE, error,
                                    gimp_item_get_visible (GIMP_ITEM (layer))));
    xcf_check_error (xcf_save_prop (info, gimage, PROP_LINKED, error,
                                    gimp_item_get_linked (GIMP_ITEM (layer))));
    xcf_check_error (xcf_save_prop (info, gimage, PROP_LOCK_ALPHA,
                                    error, layer->lock_alpha));

    if (layer->mask)
    {
        xcf_check_error (xcf_save_prop (info, gimage, PROP_APPLY_MASK,
                                        error, layer->mask->apply_mask));
        xcf_check_error (xcf_save_prop (info, gimage, PROP_EDIT_MASK,
                                        error, layer->mask->edit_mask));
        xcf_check_error (xcf_save_prop (info, gimage, PROP_SHOW_MASK,
                                        error, layer->mask->show_mask));
    }
    else
    {
        xcf_check_error (xcf_save_prop (info, gimage, PROP_APPLY_MASK,
                                        error, FALSE));
        xcf_check_error (xcf_save_prop (info, gimage, PROP_EDIT_MASK,
                                        error, FALSE));
        xcf_check_error (xcf_save_prop (info, gimage, PROP_SHOW_MASK,
                                        error, FALSE));
    }

    xcf_check_error (xcf_save_prop (info, gimage, PROP_OFFSETS, error,
                                    GIMP_ITEM (layer)->offset_x,
                                    GIMP_ITEM (layer)->offset_y));
    xcf_check_error (xcf_save_prop (info, gimage, PROP_MODE, error,
                                    layer->mode));
    xcf_check_error (xcf_save_prop (info, gimage, PROP_TATTOO, error,
                                    GIMP_ITEM (layer)->tattoo));

    if (GIMP_IS_TEXT_LAYER (layer) && GIMP_TEXT_LAYER (layer)->text)
    {
        GimpTextLayer *text_layer = GIMP_TEXT_LAYER (layer);
        Q_INT32        flags      = gimp_text_layer_get_xcf_flags (text_layer);

        gimp_text_layer_xcf_save_prepare (text_layer);

        if (flags)
            xcf_check_error (xcf_save_prop (info,
                                            gimage, PROP_TEXT_LAYER_FLAGS, error,
                                            flags));
    }

    if (gimp_parasite_list_length (GIMP_ITEM (layer)->parasites) > 0)
    {
        xcf_check_error (xcf_save_prop (info, gimage, PROP_PARASITES, error,
                                        GIMP_ITEM (layer)->parasites));
    }

    if (parasite)
    {
        gimp_parasite_list_remove (GIMP_ITEM (layer)->parasites,
                                   gimp_parasite_name (parasite));
        gimp_parasite_free (parasite);
    }

    xcf_check_error (xcf_save_prop (info, gimage, PROP_END, error));

    return TRUE;
}

static bool
xcf_save_channel_props (XcfInfo     *info,
			KisImage   *gimage,
			GimpChannel *channel,
			GError     **error)
{
    guchar col[3];

    if (channel == gimp_image_get_active_channel (gimage))
        xcf_check_error (xcf_save_prop (info, gimage, PROP_ACTIVE_CHANNEL, error));

    if (channel == gimage->selection_mask)
        xcf_check_error (xcf_save_prop (info, gimage, PROP_SELECTION, error));

    xcf_check_error (xcf_save_prop (info, gimage, PROP_OPACITY, error,
                                    channel->color.a));
    xcf_check_error (xcf_save_prop (info, gimage, PROP_VISIBLE, error,
                                    gimp_item_get_visible (GIMP_ITEM (channel))));
    xcf_check_error (xcf_save_prop (info, gimage, PROP_LINKED, error,
                                    gimp_item_get_linked (GIMP_ITEM (channel))));
    xcf_check_error (xcf_save_prop (info, gimage, PROP_SHOW_MASKED, error,
                                    channel->show_masked));

    gimp_rgb_get_uchar (&channel->color, &col[0], &col[1], &col[2]);
    xcf_check_error (xcf_save_prop (info, gimage, PROP_COLOR, error, col));

    xcf_check_error (xcf_save_prop (info, gimage, PROP_TATTOO, error,
                                    GIMP_ITEM (channel)->tattoo));

    if (gimp_parasite_list_length (GIMP_ITEM (channel)->parasites) > 0)
        xcf_check_error (xcf_save_prop (info, gimage, PROP_PARASITES, error,
                                        GIMP_ITEM (channel)->parasites));

    xcf_check_error (xcf_save_prop (info, gimage, PROP_END, error));

    return TRUE;
}

static bool
xcf_save_prop (XcfInfo   *info,
	       KisImage *gimage,
	       PropType   prop_type,
	       GError   **error,
	       ...)
{
    Q_INT32 size;
    va_list args;

    GError *tmp_error = NULL;

    va_start (args, error);

    switch (prop_type)
    {
    case PROP_END:
        size = 0;

        xcf_write_prop_type_check_error (info, prop_type);
        xcf_write_int32_check_error (info, &size, 1);
        break;

    case PROP_COLORMAP:
    {
	Q_INT32  ncolors;
	guchar  *colors;

	ncolors = va_arg (args, Q_INT32);
	colors = va_arg (args, guchar*);
	size = 4 + ncolors * 3;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &ncolors, 1);
	xcf_write_int8_check_error  (info, colors, ncolors * 3);
    }
    break;

    case PROP_ACTIVE_LAYER:
    case PROP_ACTIVE_CHANNEL:
    case PROP_SELECTION:
        size = 0;

        xcf_write_prop_type_check_error (info, prop_type);
        xcf_write_int32_check_error (info, &size, 1);
        break;

    case PROP_FLOATING_SELECTION:
    {
	Q_INT32 dummy;

	dummy = 0;
	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	info->floating_sel_offset = info->cp;
	xcf_write_int32_check_error (info, &dummy, 1);
    }
    break;

    case PROP_OPACITY:
    {
	gdouble opacity;
        Q_INT32 uint_opacity;

	opacity = va_arg (args, gdouble);

        uint_opacity = opacity * 255.999;

	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &uint_opacity, 1);
    }
    break;

    case PROP_MODE:
    {
	Q_INT3232 mode;

	mode = va_arg (args, Q_INT3232);
	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, (Q_INT32 *) &mode, 1);
    }
    break;

    case PROP_VISIBLE:
    {
	Q_INT32 visible;

	visible = va_arg (args, Q_INT32);
	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &visible, 1);
    }
    break;

    case PROP_LINKED:
    {
	Q_INT32 linked;

	linked = va_arg (args, Q_INT32);
	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &linked, 1);
    }
    break;

    case PROP_LOCK_ALPHA:
    {
	Q_INT32 lock_alpha;

	lock_alpha = va_arg (args, Q_INT32);
	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &lock_alpha, 1);
    }
    break;

    case PROP_APPLY_MASK:
    {
	Q_INT32 apply_mask;

	apply_mask = va_arg (args, Q_INT32);
	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &apply_mask, 1);
    }
    break;

    case PROP_EDIT_MASK:
    {
	Q_INT32 edit_mask;

	edit_mask = va_arg (args, Q_INT32);
	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &edit_mask, 1);
    }
    break;

    case PROP_SHOW_MASK:
    {
	Q_INT32 show_mask;

	show_mask = va_arg (args, Q_INT32);
	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &show_mask, 1);
    }
    break;

    case PROP_SHOW_MASKED:
    {
	Q_INT32 show_masked;

	show_masked = va_arg (args, Q_INT32);
	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &show_masked, 1);
    }
    break;

    case PROP_OFFSETS:
    {
	Q_INT3232 offsets[2];

	offsets[0] = va_arg (args, Q_INT3232);
	offsets[1] = va_arg (args, Q_INT3232);
	size = 8;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, (Q_INT32 *) offsets, 2);
    }
    break;

    case PROP_COLOR:
    {
	guchar *color;

	color = va_arg (args, guchar*);
	size = 3;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int8_check_error  (info, color, 3);
    }
    break;

    case PROP_COMPRESSION:
    {
	Q_UINT8 compression;

	compression = (Q_UINT8) va_arg (args, Q_INT32);
	size = 1;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int8_check_error  (info, &compression, 1);
    }
    break;

    case PROP_GUIDES:
    {
	GList     *guides;
	GimpGuide *guide;
	Q_INT3232     position;
	Q_INT328      orientation;
	Q_INT32       nguides;

	guides = va_arg (args, GList *);
	nguides = g_list_length (guides);

	size = nguides * (4 + 1);

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);

	for (; guides; guides = g_list_next (guides))
        {
	    guide = (GimpGuide *) guides->data;

	    position = guide->position;

            switch (guide->orientation)
            {
            case GIMP_ORIENTATION_HORIZONTAL:
                orientation = XCF_ORIENTATION_HORIZONTAL;
                break;

            case GIMP_ORIENTATION_VERTICAL:
                orientation = XCF_ORIENTATION_VERTICAL;
                break;

            default:
                g_warning ("%s: skipping guide with bad orientation",
                           G_STRFUNC);
                continue;
            }

	    xcf_write_int32_check_error (info, (Q_INT32 *) &position,    1);
	    xcf_write_int8_check_error  (info, (Q_UINT8 *)  &orientation, 1);
        }
    }
    break;

    case PROP_RESOLUTION:
    {
	float xresolution, yresolution;

	/* we pass in floats,
           but they are promoted to double by the compiler */
	xresolution =  va_arg (args, double);
	yresolution =  va_arg (args, double);

	size = 4*2;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);

	xcf_write_float_check_error (info, &xresolution, 1);
	xcf_write_float_check_error (info, &yresolution, 1);
    }
    break;

    case PROP_TATTOO:
    {
	Q_INT32 tattoo;

	tattoo =  va_arg (args, Q_INT32);
	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &tattoo, 1);
    }
    break;

    case PROP_PARASITES:
    {
	KisAnnotationList *list;
	Q_INT32           base, length;
	long              pos;

	list = va_arg (args, KisAnnotationList *);

	if (gimp_parasite_list_persistent_length (list) > 0)
        {
            xcf_write_prop_type_check_error (info, prop_type);

	    /* because we don't know how much room the parasite list will take
	     * we save the file position and write the length later
	     */
            pos = info->cp;
	    xcf_write_int32_check_error (info, &length, 1);
	    base = info->cp;

            xcf_check_error (xcf_save_parasite_list (info, list, error));

	    length = info->cp - base;
	    /* go back to the saved position and write the length */
            xcf_check_error (xcf_seek_pos (info, pos, error));
	    xcf_write_int32 (info->fp, &length, 1, &tmp_error);
	    if (tmp_error)
            {
	        g_propagate_error (error, tmp_error);
	        return FALSE;
            }

            xcf_check_error (xcf_seek_end (info, error));
        }
    }
    break;

    case PROP_UNIT:
    {
	Q_INT32 unit;

	unit = va_arg (args, Q_INT32);

	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &unit, 1);
    }
    break;

    case PROP_PATHS:
    {
	Q_INT32 base, length;
	glong   pos;

        xcf_write_prop_type_check_error (info, prop_type);

        /* because we don't know how much room the paths list will take
         * we save the file position and write the length later
         */
        pos = info->cp;
        xcf_write_int32_check_error (info, &length, 1);

        base = info->cp;

        xcf_check_error (xcf_save_old_paths (info, gimage, error));

        length = info->cp - base;

        /* go back to the saved position and write the length */
        xcf_check_error (xcf_seek_pos (info, pos, error));
        xcf_write_int32 (info->fp, &length, 1, &tmp_error);
        if (tmp_error)
        {
            g_propagate_error (error, tmp_error);
            return FALSE;
        }

        xcf_check_error (xcf_seek_end (info, error));
    }
    break;

    case PROP_USER_UNIT:
    {
	GimpUnit     unit;
	const QCString *unit_strings[5];
	float       factor;
	Q_INT32      digits;

	unit = va_arg (args, Q_INT32);

	/* write the entire unit definition */
	unit_strings[0] = _gimp_unit_get_identifier (gimage->gimp, unit);
	factor          = _gimp_unit_get_factor (gimage->gimp, unit);
	digits          = _gimp_unit_get_digits (gimage->gimp, unit);
	unit_strings[1] = _gimp_unit_get_symbol (gimage->gimp, unit);
	unit_strings[2] = _gimp_unit_get_abbreviation (gimage->gimp, unit);
	unit_strings[3] = _gimp_unit_get_singular (gimage->gimp, unit);
	unit_strings[4] = _gimp_unit_get_plural (gimage->gimp, unit);

	size =
            2 * 4 +
            strlen (unit_strings[0]) ? strlen (unit_strings[0]) + 5 : 4 +
            strlen (unit_strings[1]) ? strlen (unit_strings[1]) + 5 : 4 +
            strlen (unit_strings[2]) ? strlen (unit_strings[2]) + 5 : 4 +
            strlen (unit_strings[3]) ? strlen (unit_strings[3]) + 5 : 4 +
            strlen (unit_strings[4]) ? strlen (unit_strings[4]) + 5 : 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_float_check_error (info, &factor, 1);
	xcf_write_int32_check_error (info, &digits, 1);
	xcf_write_string_check_error (info, (QCString **) unit_strings, 5);
    }
    break;

    case PROP_VECTORS:
    {
	Q_INT32 base, length;
	glong   pos;

        xcf_write_prop_type_check_error (info, prop_type);

        /* because we don't know how much room the paths list will take
         * we save the file position and write the length later
         */
        pos = info->cp;
        xcf_write_int32_check_error (info, &length, 1);

        base = info->cp;

        xcf_check_error (xcf_save_vectors (info, gimage, error));

        length = info->cp - base;

        /* go back to the saved position and write the length */
        xcf_check_error (xcf_seek_pos (info, pos, error));
        xcf_write_int32 (info->fp, &length, 1, &tmp_error);
        if (tmp_error)
        {
            g_propagate_error (error, tmp_error);
            return FALSE;
        }

        xcf_check_error (xcf_seek_end (info, error));
    }
    break;

    case PROP_TEXT_LAYER_FLAGS:
    {
	Q_INT32 flags;

	flags = va_arg (args, Q_INT32);
	size = 4;

        xcf_write_prop_type_check_error (info, prop_type);
	xcf_write_int32_check_error (info, &size, 1);
	xcf_write_int32_check_error (info, &flags, 1);
    }
    break;
    }

    va_end (args);

    return TRUE;
}

static bool
xcf_save_layer (XcfInfo   *info,
		KisImage *gimage,
		KisLayer *layer,
		GError   **error)
{
    Q_INT32 saved_pos;
    Q_INT32 offset;

    GError *tmp_error = NULL;

    /* check and see if this is the drawable that the floating
     *  selection is attached to.
     */
    if (GIMP_DRAWABLE (layer) == info->floating_sel_drawable)
    {
        saved_pos = info->cp;
        xcf_check_error (xcf_seek_pos (info, info->floating_sel_offset, error));
        xcf_write_int32_check_error (info, &saved_pos, 1);
        xcf_check_error (xcf_seek_pos (info, saved_pos, error));
    }

    /* write out the width, height and image type information for the layer */
    xcf_write_int32_check_error (info,
                                 (Q_INT32 *) &GIMP_ITEM (layer)->width, 1);
    xcf_write_int32_check_error (info,
                                 (Q_INT32 *) &GIMP_ITEM (layer)->height, 1);
    xcf_write_int32_check_error (info,
                                 (Q_INT32 *) &GIMP_DRAWABLE (layer)->type, 1);

    /* write out the layers name */
    xcf_write_string_check_error (info, &GIMP_OBJECT (layer)->name, 1);

    /* write out the layer properties */
    xcf_save_layer_props (info, gimage, layer, error);

    /* save the current position which is where the hierarchy offset
     *  will be stored.
     */
    saved_pos = info->cp;

    /* write out the layer tile hierarchy */
    xcf_check_error (xcf_seek_pos (info, info->cp + 8, error));
    offset = info->cp;

    xcf_check_error (xcf_save_hierarchy (info,
                                         GIMP_DRAWABLE(layer)->tiles, error));

    xcf_check_error (xcf_seek_pos (info, saved_pos, error));
    xcf_write_int32_check_error (info, &offset, 1);
    saved_pos = info->cp;

    /* write out the layer mask */
    if (layer->mask)
    {
        xcf_check_error (xcf_seek_end (info, error));
        offset = info->cp;

        xcf_check_error (xcf_save_channel (info,
                                           gimage, GIMP_CHANNEL(layer->mask),
                                           error));
    }
    else
        offset = 0;

    xcf_check_error (xcf_seek_pos (info, saved_pos, error));
    xcf_write_int32_check_error (info, &offset, 1);

    return TRUE;
}

static bool
xcf_save_channel (XcfInfo      *info,
		  KisImage    *gimage,
		  GimpChannel  *channel,
		  GError      **error)
{
    Q_INT32 saved_pos;
    Q_INT32 offset;

    GError *tmp_error = NULL;

    /* check and see if this is the drawable that the floating
     *  selection is attached to.
     */
    if (GIMP_DRAWABLE (channel) == info->floating_sel_drawable)
    {
        saved_pos = info->cp;
        xcf_check_error (xcf_seek_pos (info, info->floating_sel_offset, error));
        xcf_write_int32_check_error (info, &saved_pos, 1);
        xcf_check_error (xcf_seek_pos (info, saved_pos, error));
    }

    /* write out the width and height information for the channel */
    xcf_write_int32_check_error (info,
                                 (Q_INT32 *) &GIMP_ITEM (channel)->width, 1);
    xcf_write_int32_check_error (info,
                                 (Q_INT32 *) &GIMP_ITEM (channel)->height, 1);

    /* write out the channels name */
    xcf_write_string_check_error (info, &GIMP_OBJECT (channel)->name, 1);

    /* write out the channel properties */
    xcf_save_channel_props (info, gimage, channel, error);

    /* save the current position which is where the hierarchy offset
     *  will be stored.
     */
    saved_pos = info->cp;

    /* write out the channel tile hierarchy */
    xcf_check_error (xcf_seek_pos (info, info->cp + 4, error));
    offset = info->cp;

    xcf_check_error (xcf_save_hierarchy (info,
                                         GIMP_DRAWABLE (channel)->tiles, error));

    xcf_check_error (xcf_seek_pos (info, saved_pos, error));
    xcf_write_int32_check_error (info, &offset, 1);
    saved_pos = info->cp;

    return TRUE;
}

static Q_INT32
xcf_calc_levels (Q_INT32 size,
		 Q_INT32 tile_size)
{
    int levels;

    levels = 1;
    while (size > tile_size)
    {
        size /= 2;
        levels += 1;
    }

    return levels;
}


static bool
xcf_save_hierarchy (XcfInfo     *info,
		    TileManager *tiles,
		    GError     **error)
{
    Q_INT32 saved_pos;
    Q_INT32 offset;
    Q_INT32 width;
    Q_INT32 height;
    Q_INT32 bpp;
    Q_INT32    i;
    Q_INT32    nlevels;
    Q_INT32    tmp1, tmp2;

    GError *tmp_error = NULL;

    width  = tile_manager_width (tiles);
    height = tile_manager_height (tiles);
    bpp    = tile_manager_bpp (tiles);

    xcf_write_int32_check_error (info, (Q_INT32 *) &width, 1);
    xcf_write_int32_check_error (info, (Q_INT32 *) &height, 1);
    xcf_write_int32_check_error (info, (Q_INT32 *) &bpp, 1);

    saved_pos = info->cp;

    tmp1 = xcf_calc_levels (width, TILE_WIDTH);
    tmp2 = xcf_calc_levels (height, TILE_HEIGHT);
    nlevels = MAX (tmp1, tmp2);

    xcf_check_error (xcf_seek_pos (info, info->cp + (1 + nlevels) * 4, error));

    for (i = 0; i < nlevels; i++)
    {
        offset = info->cp;

        if (i == 0)
	{
            /* write out the level. */
            xcf_check_error (xcf_save_level (info, tiles, error));
	}
        else
	{
            /* fake an empty level */
            tmp1 = 0;
            width  /= 2;
            height /= 2;
            xcf_write_int32_check_error (info, (Q_INT32 *) &width,  1);
            xcf_write_int32_check_error (info, (Q_INT32 *) &height, 1);
            xcf_write_int32_check_error (info, (Q_INT32 *) &tmp1,   1);
	}

        /* seek back to where we are to write out the next
         *  level offset and write it out.
         */
        xcf_check_error (xcf_seek_pos (info, saved_pos, error));
        xcf_write_int32_check_error (info, &offset, 1);

        /* increment the location we are to write out the
         *  next offset.
         */
        saved_pos = info->cp;

        /* seek to the end of the file which is where
         *  we will write out the next level.
         */
        xcf_check_error (xcf_seek_end (info, error));
    }

    /* write out a '0' offset position to indicate the end
     *  of the level offsets.
     */
    offset = 0;
    xcf_check_error (xcf_seek_pos (info, saved_pos, error));
    xcf_write_int32_check_error (info, &offset, 1);

    return TRUE;
}

static bool
xcf_save_level (XcfInfo     *info,
		TileManager *level,
		GError     **error)
{
    Q_INT32  saved_pos;
    Q_INT32  offset;
    Q_INT32  width;
    Q_INT32  height;
    Q_UINT32    ntiles;
    Q_INT32     i;
    guchar  *rlebuf;

    GError *tmp_error = NULL;

    width  = tile_manager_width (level);
    height = tile_manager_height (level);

    xcf_write_int32_check_error (info, (Q_INT32 *) &width, 1);
    xcf_write_int32_check_error (info, (Q_INT32 *) &height, 1);

    saved_pos = info->cp;

    /* allocate a temporary buffer to store the rle data before it is
       written to disk */
    rlebuf =
        g_malloc (TILE_WIDTH * TILE_HEIGHT * tile_manager_bpp (level) * 1.5);

    if (level->tiles)
    {
        ntiles = level->ntile_rows * level->ntile_cols;
        xcf_check_error (xcf_seek_pos (info, info->cp + (ntiles + 1) * 4, error));

        for (i = 0; i < ntiles; i++)
	{
            /* save the start offset of where we are writing
             *  out the next tile.
             */
            offset = info->cp;

            /* write out the tile. */
            switch (info->compression)
	    {
	    case COMPRESS_NONE:
                xcf_check_error(xcf_save_tile (info, level->tiles[i], error));
                break;
	    case COMPRESS_RLE:
                xcf_check_error (xcf_save_tile_rle (info, level->tiles[i],
                                                    rlebuf, error));
                break;
	    case COMPRESS_ZLIB:
                g_error ("xcf: zlib compression unimplemented");
                break;
	    case COMPRESS_FRACTAL:
                g_error ("xcf: fractal compression unimplemented");
                break;
	    }

            /* seek back to where we are to write out the next
             *  tile offset and write it out.
             */
            xcf_check_error (xcf_seek_pos (info, saved_pos, error));
            xcf_write_int32_check_error (info, &offset, 1);

            /* increment the location we are to write out the
             *  next offset.
             */
            saved_pos = info->cp;

            /* seek to the end of the file which is where
             *  we will write out the next tile.
             */
            xcf_check_error (xcf_seek_end (info, error));
	}
    }

    g_free (rlebuf);

    /* write out a '0' offset position to indicate the end
     *  of the level offsets.
     */
    offset = 0;
    xcf_check_error (xcf_seek_pos (info, saved_pos, error));
    xcf_write_int32_check_error (info, &offset, 1);

    return TRUE;

}

static bool
xcf_save_tile (XcfInfo  *info,
	       Tile     *tile,
	       GError  **error)
{
    GError *tmp_error = NULL;

    tile_lock (tile);
    xcf_write_int8_check_error (info, tile_data_pointer (tile, 0, 0),
                                tile_size (tile));
    tile_release (tile, FALSE);

    return TRUE;
}

static bool
xcf_save_tile_rle (XcfInfo  *info,
		   Tile     *tile,
		   guchar   *rlebuf,
		   GError  **error)
{
    guchar *data, *t;
    unsigned int last;
    Q_INT32 state;
    Q_INT32 length;
    Q_INT32 count;
    Q_INT32 size;
    Q_INT32 bpp;
    Q_INT32 i, j;
    Q_INT32 len = 0;

    GError *tmp_error = NULL;

    tile_lock (tile);

    bpp = tile_bpp (tile);

    for (i = 0; i < bpp; i++)
    {
        data = (guchar*) tile_data_pointer (tile, 0, 0) + i;

        state = 0;
        length = 0;
        count = 0;
        size = tile_ewidth(tile) * tile_eheight(tile);
        last = -1;

        while (size > 0)
	{
            switch (state)
	    {
	    case 0:
                /* in state 0 we try to find a long sequence of
                 *  matching values.
                 */
                if ((length == 32768) ||
                    ((size - length) <= 0) ||
                    ((length > 1) && (last != *data)))
		{
                    count += length;
                    if (length >= 128)
		    {
                        rlebuf[len++] = 127;
                        rlebuf[len++] = (length >> 8);
                        rlebuf[len++] = length & 0x00FF;
                        rlebuf[len++] = last;
		    }
                    else
		    {
                        rlebuf[len++] = length - 1;
                        rlebuf[len++] = last;
		    }
                    size -= length;
                    length = 0;
		}
                else if ((length == 1) && (last != *data))
                    state = 1;
                break;

	    case 1:
                /* in state 1 we try and find a long sequence of
                 *  non-matching values.
                 */
                if ((length == 32768) ||
                    ((size - length) == 0) ||
                    ((length > 0) && (last == *data) &&
                     ((size - length) == 1 || last == data[bpp])))
		{
                    count += length;
                    state = 0;

                    if (length >= 128)
		    {
                        rlebuf[len++] = 255 - 127;
                        rlebuf[len++] = (length >> 8);
                        rlebuf[len++] = length & 0x00FF;
		    }
                    else
		    {
                        rlebuf[len++] = 255 - (length - 1);
		    }

                    t = data - length * bpp;
                    for (j = 0; j < length; j++)
		    {
                        rlebuf[len++] = *t;
                        t += bpp;
		    }

                    size -= length;
                    length = 0;
		}
                break;
	    }

            if (size > 0) {
                length += 1;
                last = *data;
                data += bpp;
            }
	}

        if (count != (tile_ewidth (tile) * tile_eheight (tile)))
            g_message ("xcf: uh oh! xcf rle tile saving error: %d", count);
    }
    xcf_write_int8_check_error (info, rlebuf, len);
    tile_release (tile, FALSE);

    return TRUE;
}

static bool
xcf_save_parasite (XcfInfo       *info,
                   KisAnnotation  *parasite,
                   GError       **error)
{
    if (gimp_parasite_is_persistent (parasite))
    {
        GError *tmp_error = NULL;

        xcf_write_string_check_error (info, &parasite->name,  1);
        xcf_write_int32_check_error  (info, &parasite->flags, 1);
        xcf_write_int32_check_error  (info, &parasite->size,  1);
        xcf_write_int8_check_error   (info, parasite->data, parasite->size);
    }

    return TRUE;
}

typedef struct
{
    XcfInfo *info;
    GError  *error;
} XcfParasiteData;

static void
xcf_save_parasite_func (QCString           *key,
                        KisAnnotation    *parasite,
                        XcfParasiteData *data)
{
    if (! data->error)
        xcf_save_parasite (data->info, parasite, &data->error);
}

static bool
xcf_save_parasite_list (XcfInfo           *info,
                        KisAnnotationList  *list,
                        GError           **error)
{
    XcfParasiteData data;

    data.info  = info;
    data.error = NULL;

    gimp_parasite_list_foreach (list, (GHFunc) xcf_save_parasite_func, &data);

    if (data.error)
    {
        g_propagate_error (error, data.error);
        return FALSE;
    }

    return TRUE;
}

static bool
xcf_save_old_paths (XcfInfo    *info,
                    KisImage  *gimage,
                    GError    **error)
{
    GimpVectors *active_vectors;
    Q_INT32      num_paths;
    Q_INT32      active_index = 0;
    GList       *list;
    GError      *tmp_error = NULL;

    /* Write out the following:-
     *
     * last_selected_row (Q_INT32)
     * number_of_paths (Q_INT32)
     *
     * then each path:-
     */

    num_paths = gimp_container_num_children (gimage->vectors);

    active_vectors = gimp_image_get_active_vectors (gimage);

    if (active_vectors)
        active_index = gimp_container_get_child_index (gimage->vectors,
                                                       GIMP_OBJECT (active_vectors));

    xcf_write_int32_check_error (info, &active_index, 1);
    xcf_write_int32_check_error (info, &num_paths,    1);

    for (list = GIMP_LIST (gimage->vectors)->list;
         list;
         list = g_list_next (list))
    {
        GimpVectors            *vectors = list->data;
        QCString                  *name;
        Q_INT32                 locked;
        Q_UINT8                  state;
        Q_INT32                 version;
        Q_INT32                 pathtype;
        Q_INT32                 tattoo;
        GimpVectorsCompatPoint *points;
        Q_INT3232                  num_points;
        Q_INT3232                  closed;
        Q_INT32                    i;

        /*
         * name (string)
         * locked (Q_INT32)
         * state (QCString)
         * closed (Q_INT32)
         * number points (Q_INT32)
         * version (Q_INT32)
         * pathtype (Q_INT32)
         * tattoo (Q_INT32)
         * then each point.
         */

        points = gimp_vectors_compat_get_points (vectors, &num_points, &closed);

        /* if no points are generated because of a faulty path we should
         * skip saving the path - this is unfortunately impossible, because
         * we already saved the number of paths and I wont start seeking
         * around to fix that cruft  */

        name     = (QCString *) gimp_object_get_name (GIMP_OBJECT (vectors));
        locked   = gimp_item_get_linked (GIMP_ITEM (vectors));
        state    = closed ? 4 : 2;  /* EDIT : ADD  (editing state, 1.2 compat) */
        version  = 3;
        pathtype = 1;  /* BEZIER  (1.2 compat) */
        tattoo   = gimp_item_get_tattoo (GIMP_ITEM (vectors));

        xcf_write_string_check_error (info, &name,       1);
        xcf_write_int32_check_error  (info, &locked,     1);
        xcf_write_int8_check_error   (info, &state,      1);
        xcf_write_int32_check_error  (info, &closed,     1);
        xcf_write_int32_check_error  (info, &num_points, 1);
        xcf_write_int32_check_error  (info, &version,    1);
        xcf_write_int32_check_error  (info, &pathtype,   1);
        xcf_write_int32_check_error  (info, &tattoo,     1);

        for (i = 0; i < num_points; i++)
        {
            float x;
            float y;

            x = points[i].x;
            y = points[i].y;

            /*
             * type (Q_INT32)
             * x (float)
             * y (float)
             */

            xcf_write_int32_check_error (info, &points[i].type, 1);
            xcf_write_float_check_error (info, &x,              1);
            xcf_write_float_check_error (info, &y,              1);
        }

        g_free (points);
    }

    return TRUE;
}

static bool
xcf_save_vectors (XcfInfo    *info,
                  KisImage  *gimage,
                  GError    **error)
{
    GimpVectors *active_vectors;
    Q_INT32      version      = 1;
    Q_INT32      active_index = 0;
    Q_INT32      num_paths;
    GList       *list;
    GList       *stroke_list;
    GError      *tmp_error = NULL;

    /* Write out the following:-
     *
     * version (Q_INT32)
     * active_index (Q_INT32)
     * num_paths (Q_INT32)
     *
     * then each path:-
     */

    active_vectors = gimp_image_get_active_vectors (gimage);

    if (active_vectors)
        active_index = gimp_container_get_child_index (gimage->vectors,
                                                       GIMP_OBJECT (active_vectors));

    num_paths = gimp_container_num_children (gimage->vectors);

    xcf_write_int32_check_error (info, &version,      1);
    xcf_write_int32_check_error (info, &active_index, 1);
    xcf_write_int32_check_error (info, &num_paths,    1);

    for (list = GIMP_LIST (gimage->vectors)->list;
         list;
         list = g_list_next (list))
    {
        GimpVectors      *vectors = list->data;
        KisAnnotationList *parasites;
        QCString            *name;
        Q_INT32           tattoo;
        Q_INT32           visible;
        Q_INT32           linked;
        Q_INT32           num_parasites;
        Q_INT32           num_strokes;

        /*
         * name (string)
         * tattoo (Q_INT32)
         * visible (Q_INT32)
         * linked (Q_INT32)
         * num_parasites (Q_INT32)
         * num_strokes (Q_INT32)
         *
         * then each parasite
         * then each stroke
         */

        parasites = GIMP_ITEM (vectors)->parasites;

        name          = (QCString *) gimp_object_get_name (GIMP_OBJECT (vectors));
        visible       = gimp_item_get_visible (GIMP_ITEM (vectors));
        linked        = gimp_item_get_linked (GIMP_ITEM (vectors));
        tattoo        = gimp_item_get_tattoo (GIMP_ITEM (vectors));
        num_parasites = gimp_parasite_list_persistent_length (parasites);
        num_strokes   = g_list_length (vectors->strokes);

        xcf_write_string_check_error (info, &name,          1);
        xcf_write_int32_check_error  (info, &tattoo,        1);
        xcf_write_int32_check_error  (info, &visible,       1);
        xcf_write_int32_check_error  (info, &linked,        1);
        xcf_write_int32_check_error  (info, &num_parasites, 1);
        xcf_write_int32_check_error  (info, &num_strokes,   1);

        xcf_check_error (xcf_save_parasite_list (info, parasites, error));

        for (stroke_list = g_list_first (vectors->strokes);
             stroke_list;
             stroke_list = g_list_next (stroke_list))
        {
            GimpStroke *stroke;
            Q_INT32     stroke_type;
            Q_INT32     closed;
            Q_INT32     num_axes;
            GArray     *control_points;
            Q_INT32        i;

            Q_INT32     type;
            float      coords[6];

            /*
             * stroke_type (Q_INT32)
             * closed (Q_INT32)
             * num_axes (Q_INT32)
             * num_control_points (Q_INT32)
             *
             * then each control point.
             */

            stroke = GIMP_STROKE (stroke_list->data);

            if (GIMP_IS_BEZIER_STROKE (stroke))
            {
                stroke_type = XCF_STROKETYPE_BEZIER_STROKE;
                num_axes = 2;   /* hardcoded, might be increased later */
            }
            else
            {
                g_printerr ("Skipping unknown stroke type!\n");
                continue;
            }

            control_points = gimp_stroke_control_points_get (stroke, &closed);

            xcf_write_int32_check_error (info, &stroke_type,         1);
            xcf_write_int32_check_error (info, &closed,              1);
            xcf_write_int32_check_error (info, &num_axes,            1);
            xcf_write_int32_check_error (info, &control_points->len, 1);

            for (i = 0; i < control_points->len; i++)
            {
                GimpAnchor *anchor;

                anchor = & (g_array_index (control_points, GimpAnchor, i));

                type      = anchor->type;
                coords[0] = anchor->position.x;
                coords[1] = anchor->position.y;
                coords[2] = anchor->position.pressure;
                coords[3] = anchor->position.xtilt;
                coords[4] = anchor->position.ytilt;
                coords[5] = anchor->position.wheel;

                /*
                 * type (Q_INT32)
                 *
                 * the first num_axis elements of:
                 * [0] x (float)
                 * [1] y (float)
                 * [2] pressure (float)
                 * [3] xtilt (float)
                 * [4] ytilt (float)
                 * [5] wheel (float)
                 */

                xcf_write_int32_check_error (info, &type, 1);
                xcf_write_float_check_error (info, coords, num_axes);
            }

            g_array_free (control_points, TRUE);
        }
    }

    return TRUE;
}
