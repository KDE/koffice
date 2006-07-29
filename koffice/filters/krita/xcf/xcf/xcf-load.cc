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
#include <string.h>		/* strcmp, memcmp */

//#include <glib-object.h>

//#include "libgimpbase/gimpbase.h"
//#include "libgimpcolor/gimpcolor.h"

//#include "core/core-types.h"

//#include "base/tile.h"
//#include "base/tile-manager.h"
//#include "base/tile-manager-private.h"

//#include "config/gimpcoreconfig.h"

//#include "core/gimp.h"
//#include "core/gimpcontainer.h"
//#include "core/gimpdrawable.h"
//#include "core/gimpgrid.h"
//#include "core/gimpimage.h"
//#include "core/gimpimage-grid.h"
//#include "core/gimpimage-guides.h"
//#include "core/gimplayer.h"
//#include "core/gimplayer-floating-sel.h"
//#include "core/gimplayermask.h"
//#include "core/gimpparasitelist.h"
//#include "core/gimpselection.h"
//#include "core/gimptemplate.h"
//#include "core/gimpunit.h"

//#include "text/gimptextlayer.h"
//#include "text/gimptextlayer-xcf.h"

//#include "vectors/gimpanchor.h"
//#include "vectors/gimpstroke.h"
//#include "vectors/gimpbezierstroke.h"
//#include "vectors/gimpvectors.h"
//#include "vectors/gimpvectors-compat.h"

#include "xcf-private.h"
#include "xcf-load.h"
#include "xcf-read.h"
#include "xcf-seek.h"

//#include "gimp-intl.h"

static bool xcf_load_image_props (XcfInfo * info, KisImage * gimage);
static bool xcf_load_layer_props (XcfInfo * info,
				  KisImage * gimage,
				  KisLayer * layer,
				  bool * apply_mask,
				  bool * edit_mask,
				  bool * show_mask,
				  Q_INT32 * text_layer_flags);
static bool xcf_load_channel_props (XcfInfo * info,
				    KisImage * gimage,
				    GimpChannel ** channel);
static bool xcf_load_prop (XcfInfo * info,
			   PropType * prop_type, Q_INT32 * prop_size);
static KisLayer *xcf_load_layer (XcfInfo * info, KisImage * gimage);
//static GimpChannel   * xcf_load_channel       (XcfInfo      *info,
//                                               KisImage    *gimage);
//static GimpLayerMask * xcf_load_layer_mask    (XcfInfo      *info,
//                                               KisImage    *gimage);
static bool xcf_load_hierarchy (XcfInfo * info, TileManager * tiles);
static bool xcf_load_level (XcfInfo * info, TileManager * tiles);
static bool xcf_load_tile (XcfInfo * info, Tile * tile);
static bool xcf_load_tile_rle (XcfInfo * info,
			       Tile * tile, Q_INT32 data_length);
//static GimpParasite  * xcf_load_parasite      (XcfInfo      *info);
static bool xcf_load_old_paths (XcfInfo * info, KisImage * gimage);
static bool xcf_load_old_path (XcfInfo * info, KisImage * gimage);
static bool xcf_load_vectors (XcfInfo * info, KisImage * gimage);
static bool xcf_load_vector (XcfInfo * info, KisImage * gimage);

#ifdef SWAP_FROM_FILE
static bool xcf_swap_func (Q_INT32 fd,
			   Tile * tile, Q_INT32 cmd, gpointer user_data);
#endif


KisImage *
xcf_load_image (XcfInfo * info)
{
    KisImage *gimage;
    KisLayer *layer;
    //GimpChannel  *channel;
    //KisAnnotation *parasite;
    Q_INT32 saved_pos;
    Q_INT32 offset;
    Q_INT32 width;
    Q_INT32 height;
    Q_INT32 image_type;
    Q_INT32 num_successful_elements = 0;

    /* read in the image width, height and type */
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & width, 1);
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & height, 1);
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & image_type, 1);

    gimage = gimp_create_image (gimp, width, height, image_type, FALSE);

    gimp_image_undo_disable (gimage);

    /* read the image properties */
    if (!xcf_load_image_props (info, gimage))
        goto hard_error;

    /* check for a GimpGrid parasite */
    parasite = gimp_image_parasite_find (GIMP_IMAGE (gimage),
                                         gimp_grid_parasite_name ());
    if (parasite)
    {
        GimpGrid *grid = gimp_grid_from_parasite (parasite);

        if (grid)
	{
            gimp_parasite_list_remove (GIMP_IMAGE (gimage)->parasites,
                                       gimp_parasite_name (parasite));

            gimp_image_set_grid (GIMP_IMAGE (gimage), grid, FALSE);
	}
    }

    while (TRUE)
    {
        /* read in the offset of the next layer */
        info->cp += xcf_read_int32 (info->fp, &offset, 1);

        /* if the offset is 0 then we are at the end
         *  of the layer list.
         */
        if (offset == 0)
            break;

        /* save the current position as it is where the
         *  next layer offset is stored.
         */
        saved_pos = info->cp;

        /* seek to the layer offset */
        if (!xcf_seek_pos (info, offset, NULL))
            goto error;

        /* read in the layer */
        layer = xcf_load_layer (info, gimage);
        if (!layer)
            goto error;

        num_successful_elements++;

        /* add the layer to the image if its not the floating selection */
        if (layer != info->floating_sel)
            gimp_image_add_layer (gimage, layer,
                                  gimp_container_num_children (gimage->layers));

        /* restore the saved position so we'll be ready to
         *  read the next offset.
         */
        if (!xcf_seek_pos (info, saved_pos, NULL))
            goto error;
    }

    while (TRUE)
    {
        /* read in the offset of the next channel */
        info->cp += xcf_read_int32 (info->fp, &offset, 1);

        /* if the offset is 0 then we are at the end
         *  of the channel list.
         */
        if (offset == 0)
            break;

        /* save the current position as it is where the
         *  next channel offset is stored.
         */
        saved_pos = info->cp;

        /* seek to the channel offset */
        if (!xcf_seek_pos (info, offset, NULL))
            goto error;

        /* read in the layer */
        channel = xcf_load_channel (info, gimage);
        if (!channel)
            goto error;

        num_successful_elements++;

        /* add the channel to the image if its not the selection */
        if (channel != gimage->selection_mask)
            gimp_image_add_channel (gimage, channel, -1);

        /* restore the saved position so we'll be ready to
         *  read the next offset.
         */
        if (!xcf_seek_pos (info, saved_pos, NULL))
            goto error;
    }

    if (info->floating_sel && info->floating_sel_drawable)
        floating_sel_attach (info->floating_sel, info->floating_sel_drawable);

    if (info->active_layer)
        gimp_image_set_active_layer (gimage, info->active_layer);

    if (info->active_channel)
        gimp_image_set_active_channel (gimage, info->active_channel);

    gimp_image_set_filename (gimage, info->filename);

    if (info->tattoo_state > 0)
        gimp_image_set_tattoo_state (gimage, info->tattoo_state);

    gimp_image_undo_enable (gimage);

    return gimage;

error:
    if (num_successful_elements == 0)
        goto hard_error;

    g_message ("XCF: This file is corrupt!  I have loaded as much\n"
               "of it as I can, but it is incomplete.");

    gimp_image_undo_enable (gimage);

    return gimage;

hard_error:
    g_message ("XCF: This file is corrupt!  I could not even\n"
               "salvage any partial image data from it.");

    g_object_unref (gimage);

    return NULL;
}

static bool
xcf_load_image_props (XcfInfo * info, KisImage * gimage)
{
    PropType prop_type;
    Q_INT32 prop_size;

    while (TRUE)
    {
        if (!xcf_load_prop (info, &prop_type, &prop_size))
            return FALSE;

        switch (prop_type)
	{
	case PROP_END:
            return TRUE;

	case PROP_COLORMAP:
            if (info->file_version == 0)
	    {
                Q_INT32 i;

                g_message (_("XCF warning: version 0 of XCF file format\n"
                             "did not save indexed colormaps correctly.\n"
                             "Substituting grayscale map."));
                info->cp +=
                    xcf_read_int32 (info->fp, (Q_INT32 *) & gimage->num_cols, 1);
                gimage->cmap = g_new (guchar, gimage->num_cols * 3);
                if (!xcf_seek_pos (info, info->cp + gimage->num_cols, NULL))
                    return FALSE;

                for (i = 0; i < gimage->num_cols; i++)
		{
                    gimage->cmap[i * 3 + 0] = i;
                    gimage->cmap[i * 3 + 1] = i;
                    gimage->cmap[i * 3 + 2] = i;
		}
	    }
            else
	    {
                info->cp +=
                    xcf_read_int32 (info->fp, (Q_INT32 *) & gimage->num_cols, 1);
                gimage->cmap = g_new (guchar, gimage->num_cols * 3);
                info->cp +=
                    xcf_read_int8 (info->fp,
                                   (Q_UINT8 *) gimage->cmap,
                                   gimage->num_cols * 3);
	    }

            /* discard color map, if image is not indexed, this is just
             * sanity checking to make sure gimp doesn't end up with an
             * image state that is impossible.
             */
            if (gimp_image_base_type (gimage) != GIMP_INDEXED)
	    {
                g_free (gimage->cmap);
                gimage->cmap = NULL;
                gimage->num_cols = 0;
	    }
            break;

	case PROP_COMPRESSION:
        {
	    Q_UINT8 compression;

	    info->cp +=
                xcf_read_int8 (info->fp, (Q_UINT8 *) & compression, 1);

	    if ((compression != COMPRESS_NONE) &&
		(compression != COMPRESS_RLE) &&
		(compression != COMPRESS_ZLIB) &&
		(compression != COMPRESS_FRACTAL))
            {
		g_message ("unknown compression type: %d", (int) compression);
		return FALSE;
            }

	    info->compression = compression;
        }
        break;

	case PROP_GUIDES:
        {
	    Q_INT3232 position;
	    Q_INT328 orientation;
	    Q_INT32 i, nguides;

	    nguides = prop_size / (4 + 1);
	    for (i = 0; i < nguides; i++)
            {
		info->cp +=
                    xcf_read_int32 (info->fp, (Q_INT32 *) & position, 1);
		info->cp +=
                    xcf_read_int8 (info->fp, (Q_UINT8 *) & orientation, 1);

		/*  skip -1 guides from old XCFs  */
		if (position < 0)
                    continue;

		switch (orientation)
                {
                case XCF_ORIENTATION_HORIZONTAL:
		    gimp_image_add_hguide (gimage, position, FALSE);
		    break;

                case XCF_ORIENTATION_VERTICAL:
		    gimp_image_add_vguide (gimage, position, FALSE);
		    break;

                default:
		    g_message ("guide orientation out of range in XCF file");
		    continue;
                }
            }

	    /*  this is silly as the order of guides doesn't really matter,
	     *  but it restores the list to it's original order, which
	     *  cannot be wrong  --Mitch
	     */
	    gimage->guides = g_list_reverse (gimage->guides);
        }
        break;

	case PROP_RESOLUTION:
        {
	    float xres, yres;

	    info->cp += xcf_read_float (info->fp, &xres, 1);
	    info->cp += xcf_read_float (info->fp, &yres, 1);
	    if (xres < GIMP_MIN_RESOLUTION || xres > GIMP_MAX_RESOLUTION ||
		yres < GIMP_MIN_RESOLUTION || yres > GIMP_MAX_RESOLUTION)
            {
		g_message ("Warning, resolution out of range in XCF file");
		xres = gimage->gimp->config->default_image->xresolution;
		yres = gimage->gimp->config->default_image->yresolution;
            }
	    gimage->xresolution = xres;
	    gimage->yresolution = yres;
        }
        break;

	case PROP_TATTOO:
        {
	    info->cp += xcf_read_int32 (info->fp, &info->tattoo_state, 1);
        }
        break;

	case PROP_PARASITES:
        {
	    glong base = info->cp;
	    KisAnnotation *p;

	    while (info->cp - base < prop_size)
            {
		p = xcf_load_parasite (info);
		gimp_image_parasite_attach (gimage, p);
		gimp_parasite_free (p);
            }
	    if (info->cp - base != prop_size)
                g_message ("Error while loading an image's parasites");
        }
        break;

	case PROP_UNIT:
        {
	    Q_INT32 unit;

	    info->cp += xcf_read_int32 (info->fp, &unit, 1);

	    if ((unit <= GIMP_UNIT_PIXEL) ||
		(unit >=
		 _gimp_unit_get_number_of_built_in_units (gimage->gimp)))
            {
		g_message ("Warning, unit out of range in XCF file, "
			   "falling back to inches");
		unit = GIMP_UNIT_INCH;
            }

	    gimage->resolution_unit = unit;
        }
        break;

	case PROP_PATHS:
            xcf_load_old_paths (info, gimage);
            break;

	case PROP_USER_UNIT:
        {
	    QCString *unit_strings[5];
	    float factor;
	    Q_INT32 digits;
	    GimpUnit unit;
	    Q_INT32 num_units;
	    Q_INT32 i;

	    info->cp += xcf_read_float (info->fp, &factor, 1);
	    info->cp += xcf_read_int32 (info->fp, &digits, 1);
	    info->cp += xcf_read_string (info->fp, unit_strings, 5);

	    for (i = 0; i < 5; i++)
                if (unit_strings[i] == NULL)
                    unit_strings[i] = g_strdup ("");

	    num_units = _gimp_unit_get_number_of_units (gimage->gimp);

	    for (unit =
                     _gimp_unit_get_number_of_built_in_units (gimage->gimp);
		 unit < num_units; unit++)
            {
		/* if the factor and the identifier match some unit
		 * in unitrc, use the unitrc unit
		 */
		if ((ABS (_gimp_unit_get_factor (gimage->gimp,
						 unit) - factor) < 1e-5) &&
		    (strcmp (unit_strings[0],
			     _gimp_unit_get_identifier (gimage->gimp,
							unit)) == 0))
                {
		    break;
                }
            }

	    /* no match */
	    if (unit == num_units)
                unit = _gimp_unit_new (gimage->gimp,
                                       unit_strings[0],
                                       factor,
                                       digits,
                                       unit_strings[1],
                                       unit_strings[2],
                                       unit_strings[3], unit_strings[4]);

	    gimage->resolution_unit = unit;

	    for (i = 0; i < 5; i++)
                g_free (unit_strings[i]);
        }
        break;

	case PROP_VECTORS:
        {
	    Q_INT32 base = info->cp;

	    if (xcf_load_vectors (info, gimage))
            {
		if (base + prop_size != info->cp)
                {
		    g_warning
                        ("Mismatch in PROP_VECTORS size: skipping %d bytes.",
                         base + prop_size - info->cp);
		    xcf_seek_pos (info, base + prop_size, NULL);
                }
            }
	    else
            {
		/* skip silently since we don't understand the format and
		 * xcf_load_vectors already explained what was wrong
		 */
		xcf_seek_pos (info, base + prop_size, NULL);
            }
        }
        break;

	default:
#ifdef GIMP_UNSTABLE
            g_printerr ("unexpected/unknown image property: %d (skipping)",
                        prop_type);
#endif
            {
                Q_UINT8 buf[16];
                Q_UINT32 amount;

                while (prop_size > 0)
                {
                    amount = MIN (16, prop_size);
                    info->cp += xcf_read_int8 (info->fp, buf, amount);
                    prop_size -= MIN (16, amount);
                }
            }
            break;
	}
    }

    return FALSE;
}

static bool
xcf_load_layer_props (XcfInfo * info,
		      KisImage * gimage,
		      KisLayer * layer,
		      bool * apply_mask,
		      bool * edit_mask,
		      bool * show_mask, Q_INT32 * text_layer_flags)
{
    PropType prop_type;
    Q_INT32 prop_size;

    while (TRUE)
    {
        if (!xcf_load_prop (info, &prop_type, &prop_size))
            return FALSE;

        switch (prop_type)
	{
	case PROP_END:
            return TRUE;

	case PROP_ACTIVE_LAYER:
            info->active_layer = layer;
            break;

	case PROP_FLOATING_SELECTION:
            info->floating_sel = layer;
            info->cp +=
                xcf_read_int32 (info->fp,
                                (Q_INT32 *) & info->floating_sel_offset, 1);
            break;

	case PROP_OPACITY:
        {
	    Q_INT32 opacity;

	    info->cp += xcf_read_int32 (info->fp, &opacity, 1);
	    layer->opacity = CLAMP ((gdouble) opacity / 255.0,
				    GIMP_OPACITY_TRANSPARENT,
				    GIMP_OPACITY_OPAQUE);
        }
        break;

	case PROP_VISIBLE:
        {
	    bool visible;

	    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & visible, 1);
	    gimp_item_set_visible (GIMP_ITEM (layer),
				   visible ? TRUE : FALSE, FALSE);
        }
        break;

	case PROP_LINKED:
        {
	    bool linked;

	    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & linked, 1);
	    gimp_item_set_linked (GIMP_ITEM (layer),
				  linked ? TRUE : FALSE, FALSE);
        }
        break;

	case PROP_LOCK_ALPHA:
            info->cp +=
                xcf_read_int32 (info->fp, (Q_INT32 *) & layer->lock_alpha, 1);
            break;

	case PROP_APPLY_MASK:
            info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) apply_mask, 1);
            break;

	case PROP_EDIT_MASK:
            info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) edit_mask, 1);
            break;

	case PROP_SHOW_MASK:
            info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) show_mask, 1);
            break;

	case PROP_OFFSETS:
            info->cp +=
                xcf_read_int32 (info->fp,
                                (Q_INT32 *) & GIMP_ITEM (layer)->offset_x, 1);
            info->cp +=
                xcf_read_int32 (info->fp,
                                (Q_INT32 *) & GIMP_ITEM (layer)->offset_y, 1);
            break;

	case PROP_MODE:
            info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & layer->mode, 1);
            break;

	case PROP_TATTOO:
            info->cp += xcf_read_int32 (info->fp,
                                        (Q_INT32 *) & GIMP_ITEM (layer)->tattoo,
                                        1);
            break;

	case PROP_PARASITES:
        {
	    glong base = info->cp;
	    KisAnnotation *p;

	    while (info->cp - base < prop_size)
            {
		p = xcf_load_parasite (info);
		gimp_item_parasite_attach (GIMP_ITEM (layer), p);
		gimp_parasite_free (p);
            }
	    if (info->cp - base != prop_size)
                g_message ("Error while loading a layer's parasites");
        }
        break;

	case PROP_TEXT_LAYER_FLAGS:
            info->cp += xcf_read_int32 (info->fp, text_layer_flags, 1);
            break;

	default:
        {
	    Q_UINT8 buf[16];
	    Q_UINT32 amount;

#ifdef GIMP_UNSTABLE
	    g_printerr ("unexpected/unknown layer property: %d (skipping)",
			prop_type);
#endif
	    while (prop_size > 0)
            {
		amount = MIN (16, prop_size);
		info->cp += xcf_read_int8 (info->fp, buf, amount);
		prop_size -= MIN (16, amount);
            }
        }
        break;
	}
    }

    return FALSE;
}

static bool
xcf_load_channel_props (XcfInfo * info,
			KisImage * gimage, GimpChannel ** channel)
{
    PropType prop_type;
    Q_INT32 prop_size;

    while (TRUE)
    {
        if (!xcf_load_prop (info, &prop_type, &prop_size))
            return FALSE;

        switch (prop_type)
	{
	case PROP_END:
            return TRUE;

	case PROP_ACTIVE_CHANNEL:
            info->active_channel = *channel;
            break;

	case PROP_SELECTION:
            g_object_unref (gimage->selection_mask);
            gimage->selection_mask =
                gimp_selection_new (gimage,
                                    gimp_item_width (GIMP_ITEM (*channel)),
                                    gimp_item_height (GIMP_ITEM (*channel)));
            g_object_ref (gimage->selection_mask);
            gimp_item_sink (GIMP_ITEM (gimage->selection_mask));

            tile_manager_unref (GIMP_DRAWABLE (gimage->selection_mask)->tiles);
            GIMP_DRAWABLE (gimage->selection_mask)->tiles =
                GIMP_DRAWABLE (*channel)->tiles;
            GIMP_DRAWABLE (*channel)->tiles = NULL;
            g_object_unref (*channel);
            *channel = gimage->selection_mask;
            (*channel)->boundary_known = FALSE;
            (*channel)->bounds_known = FALSE;
            break;

	case PROP_OPACITY:
        {
	    Q_INT32 opacity;

	    info->cp += xcf_read_int32 (info->fp, &opacity, 1);
	    (*channel)->color.a = opacity / 255.0;
        }
        break;

	case PROP_VISIBLE:
        {
	    bool visible;

	    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & visible, 1);
	    gimp_item_set_visible (GIMP_ITEM (*channel),
				   visible ? TRUE : FALSE, FALSE);
        }
        break;

	case PROP_LINKED:
        {
	    bool linked;

	    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & linked, 1);
	    gimp_item_set_linked (GIMP_ITEM (*channel),
				  linked ? TRUE : FALSE, FALSE);
        }
        break;

	case PROP_SHOW_MASKED:
        {
	    bool show_masked;

	    info->cp +=
                xcf_read_int32 (info->fp, (Q_INT32 *) & show_masked, 1);
	    gimp_channel_set_show_masked (*channel, show_masked);
        }
        break;

	case PROP_COLOR:
        {
	    guchar col[3];

	    info->cp += xcf_read_int8 (info->fp, (Q_UINT8 *) col, 3);

	    gimp_rgb_set_uchar (&(*channel)->color, col[0], col[1], col[2]);
        }
        break;

	case PROP_TATTOO:
            info->cp +=
                xcf_read_int32 (info->fp, &GIMP_ITEM (*channel)->tattoo, 1);
            break;

	case PROP_PARASITES:
        {
	    glong base = info->cp;
	    KisAnnotation *p;

	    while ((info->cp - base) < prop_size)
            {
		p = xcf_load_parasite (info);
		gimp_item_parasite_attach (GIMP_ITEM (*channel), p);
		gimp_parasite_free (p);
            }
	    if (info->cp - base != prop_size)
                g_message ("Error while loading a channel's parasites");
        }
        break;

	default:
#ifdef GIMP_UNSTABLE
            g_printerr ("unexpected/unknown channel property: %d (skipping)",
                        prop_type);
#endif

            {
                Q_UINT8 buf[16];
                Q_UINT32 amount;

                while (prop_size > 0)
                {
                    amount = MIN (16, prop_size);
                    info->cp += xcf_read_int8 (info->fp, buf, amount);
                    prop_size -= MIN (16, amount);
                }
            }
            break;
	}
    }

    return FALSE;
}

static bool
xcf_load_prop (XcfInfo * info, PropType * prop_type, Q_INT32 * prop_size)
{
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) prop_type, 1);
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) prop_size, 1);
    return TRUE;
}

static KisLayer *
xcf_load_layer (XcfInfo * info, KisImage * gimage)
{
    KisLayer *layer;
    GimpLayerMask *layer_mask;
    Q_INT32 hierarchy_offset;
    Q_INT32 layer_mask_offset;
    bool apply_mask = TRUE;
    bool edit_mask = FALSE;
    bool show_mask = FALSE;
    bool active;
    bool floating;
    Q_INT32 text_layer_flags = 0;
    Q_INT32 width;
    Q_INT32 height;
    Q_INT32 type;
    bool is_fs_drawable;
    QCString *name;

    /* check and see if this is the drawable the floating selection
     *  is attached to. if it is then we'll do the attachment in our caller.
     */
    is_fs_drawable = (info->cp == info->floating_sel_offset);

    /* read in the layer width, height, type and name */
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & width, 1);
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & height, 1);
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & type, 1);
    info->cp += xcf_read_string (info->fp, &name, 1);

    /* create a new layer */
    layer = gimp_layer_new (gimage, width, height,
                            type, name, 255, GIMP_NORMAL_MODE);
    g_free (name);
    if (!layer)
        return NULL;

    /* read in the layer properties */
    if (!xcf_load_layer_props (info, gimage, layer,
                               &apply_mask, &edit_mask, &show_mask,
                               &text_layer_flags))
        goto error;

    /* call the evil text layer hack that might change our layer pointer */
    active = (info->active_layer == layer);
    floating = (info->floating_sel == layer);

    if (gimp_text_layer_xcf_load_hack (&layer))
    {
        gimp_text_layer_set_xcf_flags (GIMP_TEXT_LAYER (layer),
                                       text_layer_flags);

        if (active)
            info->active_layer = layer;
        if (floating)
            info->floating_sel = layer;
    }

    /* read the hierarchy and layer mask offsets */
    info->cp += xcf_read_int32 (info->fp, &hierarchy_offset, 1);
    info->cp += xcf_read_int32 (info->fp, &layer_mask_offset, 1);

    /* read in the hierarchy */
    if (!xcf_seek_pos (info, hierarchy_offset, NULL))
        goto error;

    if (!xcf_load_hierarchy (info, GIMP_DRAWABLE (layer)->tiles))
        goto error;

    /* read in the layer mask */
    if (layer_mask_offset != 0)
    {
        if (!xcf_seek_pos (info, layer_mask_offset, NULL))
            goto error;

        layer_mask = xcf_load_layer_mask (info, gimage);
        if (!layer_mask)
            goto error;

        layer_mask->apply_mask = apply_mask;
        layer_mask->edit_mask = edit_mask;
        layer_mask->show_mask = show_mask;

        gimp_layer_add_mask (layer, layer_mask, FALSE);
    }

    /* attach the floating selection... */
    if (is_fs_drawable)
        info->floating_sel_drawable = GIMP_DRAWABLE (layer);

    return layer;

error:
    g_object_unref (layer);
    return NULL;
}

static GimpChannel *
xcf_load_channel (XcfInfo * info, KisImage * gimage)
{
    GimpChannel *channel;
    Q_INT32 hierarchy_offset;
    Q_INT32 width;
    Q_INT32 height;
    bool is_fs_drawable;
    QCString *name;
    GimpRGB color = { 0.0, 0.0, 0.0, GIMP_OPACITY_OPAQUE };

    /* check and see if this is the drawable the floating selection
     *  is attached to. if it is then we'll do the attachment in our caller.
     */
    is_fs_drawable = (info->cp == info->floating_sel_offset);

    /* read in the layer width, height and name */
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & width, 1);
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & height, 1);
    info->cp += xcf_read_string (info->fp, &name, 1);

    /* create a new channel */
    channel = gimp_channel_new (gimage, width, height, name, &color);
    g_free (name);
    if (!channel)
        return NULL;

    /* read in the channel properties */
    if (!xcf_load_channel_props (info, gimage, &channel))
        goto error;

    /* read the hierarchy and layer mask offsets */
    info->cp += xcf_read_int32 (info->fp, &hierarchy_offset, 1);

    /* read in the hierarchy */
    if (!xcf_seek_pos (info, hierarchy_offset, NULL))
        goto error;

    if (!xcf_load_hierarchy (info, GIMP_DRAWABLE (channel)->tiles))
        goto error;

    if (is_fs_drawable)
        info->floating_sel_drawable = GIMP_DRAWABLE (channel);

    return channel;

error:
    g_object_unref (channel);
    return NULL;
}

static GimpLayerMask *
xcf_load_layer_mask (XcfInfo * info, KisImage * gimage)
{
    GimpLayerMask *layer_mask;
    GimpChannel *channel;
    Q_INT32 hierarchy_offset;
    Q_INT32 width;
    Q_INT32 height;
    bool is_fs_drawable;
    QCString *name;
    GimpRGB color = { 0.0, 0.0, 0.0, GIMP_OPACITY_OPAQUE };

    /* check and see if this is the drawable the floating selection
     *  is attached to. if it is then we'll do the attachment in our caller.
     */
    is_fs_drawable = (info->cp == info->floating_sel_offset);

    /* read in the layer width, height and name */
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & width, 1);
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & height, 1);
    info->cp += xcf_read_string (info->fp, &name, 1);

    /* create a new layer mask */
    layer_mask = gimp_layer_mask_new (gimage, width, height, name, &color);
    g_free (name);
    if (!layer_mask)
        return NULL;

    /* read in the layer_mask properties */
    channel = GIMP_CHANNEL (layer_mask);
    if (!xcf_load_channel_props (info, gimage, &channel))
        goto error;

    /* read the hierarchy and layer mask offsets */
    info->cp += xcf_read_int32 (info->fp, &hierarchy_offset, 1);

    /* read in the hierarchy */
    if (!xcf_seek_pos (info, hierarchy_offset, NULL))
        goto error;

    if (!xcf_load_hierarchy (info, GIMP_DRAWABLE (layer_mask)->tiles))
        goto error;

    /* attach the floating selection... */
    if (is_fs_drawable)
        info->floating_sel_drawable = GIMP_DRAWABLE (layer_mask);

    return layer_mask;

error:
    g_object_unref (layer_mask);
    return NULL;
}

static bool
xcf_load_hierarchy (XcfInfo * info, TileManager * tiles)
{
    Q_INT32 saved_pos;
    Q_INT32 offset;
    Q_INT32 junk;
    Q_INT32 width;
    Q_INT32 height;
    Q_INT32 bpp;

    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & width, 1);
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & height, 1);
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & bpp, 1);

    /* make sure the values in the file correspond to the values
     *  calculated when the TileManager was created.
     */
    if (width != tile_manager_width (tiles) ||
        height != tile_manager_height (tiles) ||
        bpp != tile_manager_bpp (tiles))
        return FALSE;

    /* load in the levels...we make sure that the number of levels
     *  calculated when the TileManager was created is the same
     *  as the number of levels found in the file.
     */

    info->cp += xcf_read_int32 (info->fp, &offset, 1);	/* top level */

    /* discard offsets for layers below first, if any.
     */
    do
    {
        info->cp += xcf_read_int32 (info->fp, &junk, 1);
    }
    while (junk != 0);

    /* save the current position as it is where the
     *  next level offset is stored.
     */
    saved_pos = info->cp;

    /* seek to the level offset */
    if (!xcf_seek_pos (info, offset, NULL))
        return FALSE;

    /* read in the level */
    if (!xcf_load_level (info, tiles))
        return FALSE;

    /* restore the saved position so we'll be ready to
     *  read the next offset.
     */
    if (!xcf_seek_pos (info, saved_pos, NULL))
        return FALSE;

    return TRUE;
}


static bool
xcf_load_level (XcfInfo * info, TileManager * tiles)
{
    Q_INT32 saved_pos;
    Q_INT32 offset, offset2;
    Q_UINT32 ntiles;
    Q_INT32 width;
    Q_INT32 height;
    Q_INT32 i;
    Q_INT32 fail;
    Tile *previous;
    Tile *tile;

    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & width, 1);
    info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & height, 1);

    if (width != tile_manager_width (tiles) ||
        height != tile_manager_height (tiles))
        return FALSE;

    /* read in the first tile offset.
     *  if it is '0', then this tile level is empty
     *  and we can simply return.
     */
    info->cp += xcf_read_int32 (info->fp, &offset, 1);
    if (offset == 0)
        return TRUE;

    /* Initialise the reference for the in-memory tile-compression
     */
    previous = NULL;

    ntiles = tiles->ntile_rows * tiles->ntile_cols;
    for (i = 0; i < ntiles; i++)
    {
        fail = FALSE;

        if (offset == 0)
	{
            g_message ("not enough tiles found in level");
            return FALSE;
	}

        /* save the current position as it is where the
         *  next tile offset is stored.
         */
        saved_pos = info->cp;

        /* read in the offset of the next tile so we can calculate the amount
           of data needed for this tile */
        info->cp += xcf_read_int32 (info->fp, &offset2, 1);

        /* if the offset is 0 then we need to read in the maximum possible
           allowing for negative compression */
        if (offset2 == 0)
            offset2 = offset + TILE_WIDTH * TILE_WIDTH * 4 * 1.5;
        /* 1.5 is probably more
           than we need to allow */

        /* seek to the tile offset */
        if (!xcf_seek_pos (info, offset, NULL))
            return FALSE;

        /* get the tile from the tile manager */
        tile = tile_manager_get (tiles, i, TRUE, TRUE);

        /* read in the tile */
        switch (info->compression)
	{
	case COMPRESS_NONE:
            if (!xcf_load_tile (info, tile))
                fail = TRUE;
            break;
	case COMPRESS_RLE:
            if (!xcf_load_tile_rle (info, tile, offset2 - offset))
                fail = TRUE;
            break;
	case COMPRESS_ZLIB:
            g_error ("xcf: zlib compression unimplemented");
            fail = TRUE;
            break;
	case COMPRESS_FRACTAL:
            g_error ("xcf: fractal compression unimplemented");
            fail = TRUE;
            break;
	}

        if (fail)
	{
            tile_release (tile, TRUE);
            return FALSE;
	}

        /* To potentially save memory, we compare the
         *  newly-fetched tile against the last one, and
         *  if they're the same we copy-on-write mirror one against
         *  the other.
         */
        if (previous != NULL)
	{
            tile_lock (previous);
            if (tile_ewidth (tile) == tile_ewidth (previous) &&
                tile_eheight (tile) == tile_eheight (previous) &&
                tile_bpp (tile) == tile_bpp (previous) &&
                memcmp (tile_data_pointer (tile, 0, 0),
                        tile_data_pointer (previous, 0, 0),
                        tile_size (tile)) == 0)
                tile_manager_map (tiles, i, previous);
            tile_release (previous, FALSE);
	}
        tile_release (tile, TRUE);
        previous = tile_manager_get (tiles, i, FALSE, FALSE);

        /* restore the saved position so we'll be ready to
         *  read the next offset.
         */
        if (!xcf_seek_pos (info, saved_pos, NULL))
            return FALSE;

        /* read in the offset of the next tile */
        info->cp += xcf_read_int32 (info->fp, &offset, 1);
    }

    if (offset != 0)
    {
        g_message ("encountered garbage after reading level: %d", offset);
        return FALSE;
    }

    return TRUE;
}

static bool
xcf_load_tile (XcfInfo * info, Tile * tile)
{
#ifdef SWAP_FROM_FILE

    if (!info->swap_num)
    {
        info->ref_count = g_new (int, 1);
        info->swap_num = tile_swap_add (info->filename,
                                        xcf_swap_func, info->ref_count);
    }

    tile->swap_num = info->swap_num;
    tile->swap_offset = info->cp;
    *info->ref_count += 1;

#else

    info->cp += xcf_read_int8 (info->fp, tile_data_pointer (tile, 0, 0),
                               tile_size (tile));

#endif

    return TRUE;
}

static bool
xcf_load_tile_rle (XcfInfo * info, Tile * tile, int data_length)
{
    guchar *data;
    guchar val;
    Q_INT32 size;
    Q_INT32 count;
    Q_INT32 length;
    Q_INT32 bpp;
    Q_INT32 i, j;
    Q_INT32 nmemb_read_successfully;
    guchar *xcfdata, *xcfodata, *xcfdatalimit;

    data = tile_data_pointer (tile, 0, 0);
    bpp = tile_bpp (tile);

    xcfdata = xcfodata = g_malloc (data_length);

    /* we have to use fread instead of xcf_read_* because we may be
       reading past the end of the file here */
    nmemb_read_successfully = fread ((QCString *) xcfdata, sizeof (QCString),
                                     data_length, info->fp);
    info->cp += nmemb_read_successfully;

    xcfdatalimit = &xcfodata[nmemb_read_successfully - 1];

    for (i = 0; i < bpp; i++)
    {
        data = (guchar *) tile_data_pointer (tile, 0, 0) + i;
        size = tile_ewidth (tile) * tile_eheight (tile);
        count = 0;

        while (size > 0)
	{
            if (xcfdata > xcfdatalimit)
	    {
                goto bogus_rle;
	    }

            val = *xcfdata++;

            length = val;
            if (length >= 128)
	    {
                length = 255 - (length - 1);
                if (length == 128)
		{
                    if (xcfdata >= xcfdatalimit)
		    {
                        goto bogus_rle;
		    }

                    length = (*xcfdata << 8) + xcfdata[1];
                    xcfdata += 2;
		}

                count += length;
                size -= length;

                if (size < 0)
		{
                    goto bogus_rle;
		}

                if (&xcfdata[length - 1] > xcfdatalimit)
		{
                    goto bogus_rle;
		}

                while (length-- > 0)
		{
                    *data = *xcfdata++;
                    data += bpp;
		}
	    }
            else
	    {
                length += 1;
                if (length == 128)
		{
                    if (xcfdata >= xcfdatalimit)
		    {
                        goto bogus_rle;
		    }

                    length = (*xcfdata << 8) + xcfdata[1];
                    xcfdata += 2;
		}

                count += length;
                size -= length;

                if (size < 0)
		{
                    goto bogus_rle;
		}

                if (xcfdata > xcfdatalimit)
		{
                    goto bogus_rle;
		}

                val = *xcfdata++;

                for (j = 0; j < length; j++)
		{
                    *data = val;
                    data += bpp;
		}
	    }
	}
    }
    g_free (xcfodata);
    return TRUE;

bogus_rle:
    if (xcfodata)
        g_free (xcfodata);
    return FALSE;
}

static KisAnnotation *
xcf_load_parasite (XcfInfo * info)
{
    KisAnnotation *p;

    p = g_new (KisAnnotation, 1);
    info->cp += xcf_read_string (info->fp, &p->name, 1);
    info->cp += xcf_read_int32 (info->fp, &p->flags, 1);
    info->cp += xcf_read_int32 (info->fp, &p->size, 1);
    p->data = g_new (QCString, p->size);
    info->cp += xcf_read_int8 (info->fp, p->data, p->size);

    return p;
}

static bool
xcf_load_old_paths (XcfInfo * info, KisImage * gimage)
{
    Q_INT32 num_paths;
    Q_INT32 last_selected_row;
    GimpVectors *active_vectors;

    info->cp += xcf_read_int32 (info->fp, &last_selected_row, 1);
    info->cp += xcf_read_int32 (info->fp, &num_paths, 1);

    while (num_paths-- > 0)
        xcf_load_old_path (info, gimage);

    active_vectors = (GimpVectors *)
                     gimp_container_get_child_by_index (gimage->vectors, last_selected_row);

    if (active_vectors)
        gimp_image_set_active_vectors (gimage, active_vectors);

    return TRUE;
}

static bool
xcf_load_old_path (XcfInfo * info, KisImage * gimage)
{
    QCString *name;
    Q_INT32 locked;
    Q_UINT8 state;
    Q_INT32 closed;
    Q_INT32 num_points;
    Q_INT32 version;		/* changed from num_paths */
    GimpTattoo tattoo = 0;
    GimpVectors *vectors;
    GimpVectorsCompatPoint *points;
    Q_INT32 i;

    info->cp += xcf_read_string (info->fp, &name, 1);
    info->cp += xcf_read_int32 (info->fp, &locked, 1);
    info->cp += xcf_read_int8 (info->fp, &state, 1);
    info->cp += xcf_read_int32 (info->fp, &closed, 1);
    info->cp += xcf_read_int32 (info->fp, &num_points, 1);
    info->cp += xcf_read_int32 (info->fp, &version, 1);

    if (version == 2)
    {
        Q_INT32 dummy;

        /* Had extra type field and points are stored as doubles */
        info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & dummy, 1);
    }
    else if (version == 3)
    {
        Q_INT32 dummy;

        /* Has extra tatto field */
        info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & dummy, 1);
        info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & tattoo, 1);
    }
    else if (version != 1)
    {
        g_warning ("Unknown path type. Possibly corrupt XCF file");

        return FALSE;
    }

    /* skip empty compatibility paths */
    if (num_points == 0)
        return FALSE;

    points = g_new0 (GimpVectorsCompatPoint, num_points);

    for (i = 0; i < num_points; i++)
    {
        if (version == 1)
	{
            Q_INT3232 x;
            Q_INT3232 y;

            info->cp += xcf_read_int32 (info->fp, &points[i].type, 1);
            info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & x, 1);
            info->cp += xcf_read_int32 (info->fp, (Q_INT32 *) & y, 1);

            points[i].x = x;
            points[i].y = y;
	}
        else
	{
            float x;
            float y;

            info->cp += xcf_read_int32 (info->fp, &points[i].type, 1);
            info->cp += xcf_read_float (info->fp, &x, 1);
            info->cp += xcf_read_float (info->fp, &y, 1);

            points[i].x = x;
            points[i].y = y;
	}
    }

    vectors =
        gimp_vectors_compat_new (gimage, name, points, num_points, closed);

    g_free (name);
    g_free (points);

    GIMP_ITEM (vectors)->linked = locked;

    if (tattoo)
        GIMP_ITEM (vectors)->tattoo = tattoo;

    gimp_image_add_vectors (gimage, vectors,
                            gimp_container_num_children (gimage->vectors));

    return TRUE;
}

static bool
xcf_load_vectors (XcfInfo * info, KisImage * gimage)
{
    Q_INT32 version;
    Q_INT32 active_index;
    Q_INT32 num_paths;
    GimpVectors *active_vectors;
    Q_INT32 base;

#ifdef GIMP_XCF_PATH_DEBUG
    g_printerr ("xcf_load_vectors\n");
#endif

    base = info->cp;

    info->cp += xcf_read_int32 (info->fp, &version, 1);

    if (version != 1)
    {
        g_message ("Unknown vectors version: %d (skipping)", version);
        return FALSE;
    }

    info->cp += xcf_read_int32 (info->fp, &active_index, 1);
    info->cp += xcf_read_int32 (info->fp, &num_paths, 1);

#ifdef GIMP_XCF_PATH_DEBUG
    g_printerr ("%d paths (active: %d)\n", num_paths, active_index);
#endif

    while (num_paths-- > 0)
        if (!xcf_load_vector (info, gimage))
            return FALSE;

    active_vectors = (GimpVectors *)
                     gimp_container_get_child_by_index (gimage->vectors, active_index);

    if (active_vectors)
        gimp_image_set_active_vectors (gimage, active_vectors);

#ifdef GIMP_XCF_PATH_DEBUG
    g_printerr ("xcf_load_vectors: loaded %d bytes\n", info->cp - base);
#endif
    return TRUE;
}

static bool
xcf_load_vector (XcfInfo * info, KisImage * gimage)
{
    QCString *name;
    GimpTattoo tattoo = 0;
    Q_INT32 visible;
    Q_INT32 linked;
    Q_INT32 num_parasites;
    Q_INT32 num_strokes;
    GimpVectors *vectors;
    Q_INT32 i;

#ifdef GIMP_XCF_PATH_DEBUG
    g_printerr ("xcf_load_vector\n");
#endif

    info->cp += xcf_read_string (info->fp, &name, 1);
    info->cp += xcf_read_int32 (info->fp, &tattoo, 1);
    info->cp += xcf_read_int32 (info->fp, &visible, 1);
    info->cp += xcf_read_int32 (info->fp, &linked, 1);
    info->cp += xcf_read_int32 (info->fp, &num_parasites, 1);
    info->cp += xcf_read_int32 (info->fp, &num_strokes, 1);

#ifdef GIMP_XCF_PATH_DEBUG
    g_printerr
        ("name: %s, tattoo: %d, visible: %d, linked: %d, num_parasites %d, "
         "num_strokes %d\n", name, tattoo, visible, linked, num_parasites,
         num_strokes);
#endif

    vectors = gimp_vectors_new (gimage, name);

    GIMP_ITEM (vectors)->visible = visible ? TRUE : FALSE;
    GIMP_ITEM (vectors)->linked = linked ? TRUE : FALSE;

    if (tattoo)
        GIMP_ITEM (vectors)->tattoo = tattoo;

    for (i = 0; i < num_parasites; i++)
    {
        KisAnnotation *parasite;

        parasite = xcf_load_parasite (info);

        if (!parasite)
            return FALSE;

        gimp_item_parasite_attach (GIMP_ITEM (vectors), parasite);
        gimp_parasite_free (parasite);
    }

    for (i = 0; i < num_strokes; i++)
    {
        Q_INT32 stroke_type_id;
        Q_INT32 closed;
        Q_INT32 num_axes;
        Q_INT32 num_control_points;
        Q_INT32 type;
        float coords[6] = { 0.0, 0.0, 1.0, 0.5, 0.5, 0.5 };
        GimpStroke *stroke;
        Q_INT32 j;

        GValueArray *control_points;
        GValue value = { 0, };
        GimpAnchor anchor;
        GType stroke_type;

        g_value_init (&value, GIMP_TYPE_ANCHOR);

        info->cp += xcf_read_int32 (info->fp, &stroke_type_id, 1);
        info->cp += xcf_read_int32 (info->fp, &closed, 1);
        info->cp += xcf_read_int32 (info->fp, &num_axes, 1);
        info->cp += xcf_read_int32 (info->fp, &num_control_points, 1);

#ifdef GIMP_XCF_PATH_DEBUG
        g_printerr ("stroke_type: %d, closed: %d, num_axes %d, len %d\n",
                    stroke_type_id, closed, num_axes, num_control_points);
#endif

        switch (stroke_type_id)
	{
	case XCF_STROKETYPE_BEZIER_STROKE:
            stroke_type = GIMP_TYPE_BEZIER_STROKE;
            break;

	default:
            g_printerr ("skipping unknown stroke type\n");
            xcf_seek_pos (info,
                          info->cp + 4 * num_axes * num_control_points, NULL);
            continue;
	}

        control_points = g_value_array_new (num_control_points);

        anchor.selected = FALSE;

        for (j = 0; j < num_control_points; j++)
	{
            info->cp += xcf_read_int32 (info->fp, &type, 1);
            info->cp += xcf_read_float (info->fp, coords, num_axes);

            anchor.type = type;
            anchor.position.x = coords[0];
            anchor.position.y = coords[1];
            anchor.position.pressure = coords[2];
            anchor.position.xtilt = coords[3];
            anchor.position.ytilt = coords[4];
            anchor.position.wheel = coords[5];

            g_value_set_boxed (&value, &anchor);
            g_value_array_append (control_points, &value);

#ifdef GIMP_XCF_PATH_DEBUG
            g_printerr ("Anchor: %d, (%f, %f, %f, %f, %f, %f)\n", type,
                        coords[0], coords[1], coords[2], coords[3],
                        coords[4], coords[5]);
#endif
	}

        g_value_unset (&value);

        stroke = g_object_new (stroke_type,
                               "closed", closed,
                               "control-points", control_points, NULL);

        gimp_vectors_stroke_add (vectors, stroke);
    }

    gimp_image_add_vectors (gimage, vectors,
                            gimp_container_num_children (gimage->vectors));

    return TRUE;
}

#ifdef SWAP_FROM_FILE

static bool
xcf_swap_func (Q_INT32 fd, Tile * tile, Q_INT32 cmd, gpointer user_data)
{
    Q_INT32 bytes;
    Q_INT32 err;
    Q_INT32 nleft;
    Q_INT32 *ref_count;

    switch (cmd)
    {
    case SWAP_IN:
        lseek (fd, tile->swap_offset, SEEK_SET);

        bytes = tile_size (tile);
        tile_alloc (tile);

        nleft = bytes;
        while (nleft > 0)
	{
            do
	    {
                err = read (fd, tile->data + bytes - nleft, nleft);
	    }
            while ((err == -1) && ((errno == EAGAIN) || (errno == EINTR)));

            if (err <= 0)
	    {
                g_message ("unable to read tile data from xcf file: "
                           "%d ( %d ) bytes read", err, nleft);
                return FALSE;
	    }

            nleft -= err;
	}
        break;

    case SWAP_OUT:
    case SWAP_DELETE:
    case SWAP_COMPRESS:
        ref_count = user_data;
        *ref_count -= 1;
        if (*ref_count == 0)
	{
            tile_swap_remove (tile->swap_num);
            g_free (ref_count);
	}

        tile->swap_num = 1;
        tile->swap_offset = -1;

        return TRUE;
    }

    return FALSE;
}

#endif
