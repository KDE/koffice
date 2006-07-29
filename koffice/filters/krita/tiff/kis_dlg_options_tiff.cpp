/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#include "kis_dlg_options_tiff.h"

#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qslider.h>
#include <qwidgetstack.h>

#include <kapplication.h>
#include <kcombobox.h>
#include <klocale.h>

#include "kis_wdg_options_tiff.h"

KisDlgOptionsTIFF::KisDlgOptionsTIFF(QWidget *parent, const char *name)
    : KDialogBase(parent, name, false, i18n("TIFF Export Options"), KDialogBase::Ok | KDialogBase::Cancel)
{
    optionswdg = new KisWdgOptionsTIFF(this);
    activated(0);
    connect(optionswdg->kComboBoxCompressionType, SIGNAL(activated ( int )), this, SLOT(activated ( int ) ));
    connect(optionswdg->flatten, SIGNAL(toggled(bool)), this, SLOT(flattenToggled( bool) ) );
    setMainWidget(optionswdg);
    kapp->restoreOverrideCursor();
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum) );
}

KisDlgOptionsTIFF::~KisDlgOptionsTIFF()
{
}

void KisDlgOptionsTIFF::activated ( int index )
{
/*    optionswdg->groupBoxJPEG->hide();
    optionswdg->groupBoxDeflate->hide();
    optionswdg->groupBoxCCITGroupCCITG3->hide();
    optionswdg->groupBoxPixarLog->hide();*/
    switch(index)
    {
        case 1:
            optionswdg->codecsOptionsStack->raiseWidget(1);
//             optionswdg->groupBoxJPEG->show();
            break;
        case 2:
            optionswdg->codecsOptionsStack->raiseWidget(2);
//             optionswdg->groupBoxDeflate->show();
            break;
        case 6:
            optionswdg->codecsOptionsStack->raiseWidget(3);
//             optionswdg->groupBoxCCITGroupCCITG3->show();
            break;
        case 8:
            optionswdg->codecsOptionsStack->raiseWidget(4);
//             optionswdg->groupBoxPixarLog->show();
            break;
        default:
            optionswdg->codecsOptionsStack->raiseWidget(0);
    }
}

void KisDlgOptionsTIFF::flattenToggled(bool t)
{
    optionswdg->alpha->setEnabled(t);
    if(!t)
    {
        optionswdg->alpha->setChecked(true);
    }
}


KisTIFFOptions KisDlgOptionsTIFF::options()
{
    KisTIFFOptions options;
    switch(optionswdg->kComboBoxCompressionType->currentItem ())
    {
        case 0:
            options.compressionType = COMPRESSION_NONE;
            break;
        case 1:
            options.compressionType = COMPRESSION_JPEG;
            break;
        case 2:
            options.compressionType = COMPRESSION_DEFLATE;
            break;
        case 3:
            options.compressionType = COMPRESSION_LZW;
            break;
        case 4:
            options.compressionType = COMPRESSION_JP2000;
            break;
        case 5:
            options.compressionType = COMPRESSION_CCITTRLE;
            break;
        case 6:
            options.compressionType = COMPRESSION_CCITTFAX3;
            break;
        case 7:
            options.compressionType = COMPRESSION_CCITTFAX4;
            break;
        case 8:
            options.compressionType = COMPRESSION_PIXARLOG;
            break;
    }
    options.predictor = optionswdg->kComboBoxPredictor->currentItem() + 1;
    options.alpha = optionswdg->alpha->isChecked();
    options.flatten = optionswdg->flatten->isChecked();
    options.jpegQuality = optionswdg->qualityLevel->value();
    options.deflateCompress = optionswdg->compressionLevelDeflate->value();
    options.faxMode = optionswdg->kComboBoxFaxMode->currentItem() + 1;
    options.pixarLogCompress = optionswdg->compressionLevelPixarLog->value();
    
    return options;
}

#include "kis_dlg_options_tiff.moc"
