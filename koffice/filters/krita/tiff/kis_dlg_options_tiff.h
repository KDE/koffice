/*
 *  Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_DLG_OPTIONS_TIFF_H
#define KIS_DLG_OPTIONS_TIFF_H

#include <kdialogbase.h>
#include <kis_tiff_converter.h>

class KisWdgOptionsTIFF;
/**
	@author Cyrille Berger <cberger@cberger.net>
*/
class KisDlgOptionsTIFF : public KDialogBase
{
    Q_OBJECT
    public:
        KisDlgOptionsTIFF(QWidget *parent=0, const char *name=0);
        ~KisDlgOptionsTIFF();
    public slots:
        void activated ( int index );
        void flattenToggled(bool);
        KisTIFFOptions options();
    public:
        KisWdgOptionsTIFF* optionswdg;
};

#endif
