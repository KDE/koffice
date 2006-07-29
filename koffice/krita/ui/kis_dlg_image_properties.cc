/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qtextedit.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kcolorcombo.h>

#include <KoUnitWidgets.h>

#include "kis_factory.h"
#include "kis_meta_registry.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_dlg_image_properties.h"
#include "kis_profile.h"
#include "kis_types.h"
#include "kis_image.h"
#include "kis_config.h"
#include "kis_id.h"
#include "kis_cmb_idlist.h"
#include "squeezedcombobox.h"
#include "wdgnewimage.h"

KisDlgImageProperties::KisDlgImageProperties(KisImageSP image, QWidget *parent, const char *name)
    : super(parent, name, true, "", Ok | Cancel)
{

    setCaption(i18n("Image Properties"));
    m_page = new WdgNewImage(this);

    m_image = image;

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_page->txtName->setText(image->name());
    m_page->m_createButton->hide();
    KisConfig cfg;

    m_page->intWidth->setValue(image->width());
    m_page->intHeight->setValue(image->height());

    m_page->doubleResolution->setValue(image->xRes()); // XXX: separate values for x & y?

    //m_page->cmbColorSpaces->hide();
    //m_page->lblColorSpaces->setText(image->colorSpace()->id().name());
    KisIDList colorSpaces = KisMetaRegistry::instance()->csRegistry()->listKeys();
    KisIDList::iterator i = colorSpaces.find(KisID("WET",""));
    if (i != colorSpaces.end()) {
        colorSpaces.remove(i);
    }
    m_page->cmbColorSpaces->setIDList(colorSpaces);
    m_page->cmbColorSpaces->setCurrent(image->colorSpace()->id());
            
    fillCmbProfiles(image->colorSpace()->id());

    if (image->getProfile()) {
        m_page->cmbProfile->setCurrentText(image->getProfile()->productName());
    }
    else {
        m_page->cmbProfile->setCurrentItem(0);
    }

    m_page->sliderOpacity->setEnabled(false); // XXX re-enable when figured out a way to do this
    m_page->opacityPanel->hide();
    m_page->lblOpacity->hide();

    m_page->cmbColor->setEnabled(false); // XXX re-enable when figured out a way to do this
    m_page->cmbColor->hide();
    m_page->lblColor->hide();

    connect(m_page->cmbColorSpaces, SIGNAL(activated(const KisID &)),
        this, SLOT(fillCmbProfiles(const KisID &)));


}

KisDlgImageProperties::~KisDlgImageProperties()
{
    delete m_page;
}

int KisDlgImageProperties::imageWidth()
{
    return m_page->intWidth->value();
}

int KisDlgImageProperties::imageHeight()
{
    return m_page->intHeight->value();
}

int KisDlgImageProperties::opacity()
{
    return m_page->sliderOpacity->value();
}

QString KisDlgImageProperties::imageName()
{
    return m_page->txtName->text();
}

double KisDlgImageProperties::resolution()
{
    return m_page->doubleResolution->value();
}

QString KisDlgImageProperties::description()
{
    return m_page->txtDescription->text();
}

KisColorSpace * KisDlgImageProperties::colorSpace()
{
    return KisMetaRegistry::instance()->csRegistry()->getColorSpace(m_page->cmbColorSpaces->currentItem(), m_page->cmbProfile->currentText());
}

KisProfile * KisDlgImageProperties::profile()
{
    QValueVector<KisProfile *>  profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor( m_image->colorSpace()->id() );
    Q_UINT32 index = m_page->cmbProfile->currentItem();

    if (index < profileList.count()) {
        return profileList.at(index);
    } else {
        return 0;
    }
}

// XXX: Copy & paste from kis_dlg_create_img -- refactor to separate class
void KisDlgImageProperties::fillCmbProfiles(const KisID & s)
{

    KisColorSpaceFactory * csf = KisMetaRegistry::instance()->csRegistry()->get(s);
    m_page->cmbProfile->clear();
    QValueVector<KisProfile *>  profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor( csf );
        QValueVector<KisProfile *> ::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
        m_page->cmbProfile->insertItem((*it)->productName());
    }


}

#include "kis_dlg_image_properties.moc"

