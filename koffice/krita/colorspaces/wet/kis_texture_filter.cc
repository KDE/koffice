/*
 * kis_texture_filter.cc -- Part of Krita
 *
 * Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kdebug.h>
#include <kis_view.h>
#include <kis_image.h>
#include <kis_debug_areas.h>
#include "kis_texture_painter.h"
#include "kis_texture_filter.h"

void WetPaintDevAction::act(KisPaintDeviceSP device,  Q_INT32 w,  Q_INT32 h) const {
    KisColorSpace * cs = device->colorSpace();

    if (cs->id() != KisID("WET","")) {
        kdDebug(DBG_AREA_CMS) << "You set this kind of texture on non-wet layers!.\n";
        return;
    } else {
        kdDebug(DBG_AREA_CMS) << "Wet Paint Action activated!\n";
    }

    // XXX if params of the painter get configurable, make them here configurable as well?
    KisTexturePainter painter(device);
    painter.createTexture(0, 0, w, h);
    painter.end();
}

