/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIVIO_CUSTOM_DRAG_DATA_H
#define KIVIO_CUSTOM_DRAG_DATA_H

class KivioPage;

class KivioCustomDragData
{
public:
    KivioCustomDragData();
    ~KivioCustomDragData();

    float x, y;         // mouse coordinates
    float dx, dy;       // change in mouse coordinates
    int id;             // drag id
    KivioPage *page;  // page the drag is happening on
    float scale;        // the scaling factor
};

#endif

