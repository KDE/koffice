/*
 * This file is part of Krita
 *
 * Copyright (c) 1999 Matthias Elter (me@kde.org)
 * Copyright (c) 2001-2002 Igor Jansen (rm@kde.org)
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

#include "ko_rgb_widget.h"

#include <qlayout.h>
#include <q3hbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qtooltip.h>
#include <qcolor.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include <kdebug.h>
#include <klocale.h>

#include <koFrameButton.h>
#include <koColorSlider.h>
#include <kcolordialog.h>

KoRGBWidget::KoRGBWidget(QWidget *parent, const char *name) : super(parent, name)
{
    m_ColorButton = new KDualColorButton(this);
    m_ColorButton ->  setFixedSize(m_ColorButton->sizeHint());
    Q3GridLayout *mGrid = new Q3GridLayout(this, 3, 5, 5, 2);

    /* setup color sliders */
    mRSlider = new KoColorSlider(this);
    mRSlider->setMaximumHeight(20);
    mRSlider->slotSetRange(0, 255);
    mRSlider->setFocusPolicy( Qt::ClickFocus );

    mGSlider = new KoColorSlider(this);
    mGSlider->setMaximumHeight(20);
    mGSlider->slotSetRange(0, 255);
    mGSlider->setFocusPolicy( Qt::ClickFocus );

    mBSlider = new KoColorSlider(this);
    mBSlider->setMaximumHeight(20);
    mBSlider->slotSetRange(0, 255);
    mBSlider->setFocusPolicy( Qt::ClickFocus );

    /* setup slider labels */
    mRLabel = new QLabel("R:", this);
    mRLabel->setFixedWidth(12);
    mRLabel->setFixedHeight(20);
    mGLabel = new QLabel("G:", this);
    mGLabel->setFixedWidth(12);
    mGLabel->setFixedHeight(20);
    mBLabel = new QLabel("B:", this);
    mBLabel->setFixedWidth(12);
    mBLabel->setFixedHeight(20);

    /* setup spin box */
    mRIn = new QSpinBox(0, 255, 1, this);
    mRIn->setFixedWidth(50);
    mRIn->setFixedHeight(20);
    mRIn->setFocusPolicy( Qt::ClickFocus );
    mRIn->setToolTip( i18n( "Red" ) );

    mGIn = new QSpinBox(0, 255, 1, this);
    mGIn->setFixedWidth(50);
    mGIn->setFixedHeight(20);
    mGIn->setFocusPolicy( Qt::ClickFocus );
    mGIn->setToolTip( i18n( "Green" ) );

    mBIn = new QSpinBox(0, 255, 1, this);
    mBIn->setFixedWidth(50);
    mBIn->setFixedHeight(20);
    mBIn->setFocusPolicy( Qt::ClickFocus );
    mBIn->setToolTip( i18n( "Blue" ) );

    mGrid->addMultiCellWidget(m_ColorButton, 0, 3, 0, 0, Qt::AlignTop);
    mGrid->addWidget(mRLabel, 0, 1);
    mGrid->addWidget(mGLabel, 1, 1);
    mGrid->addWidget(mBLabel, 2, 1);
    mGrid->addMultiCellWidget(mRSlider, 0, 0, 2, 3);
    mGrid->addMultiCellWidget(mGSlider, 1, 1, 2, 3);
    mGrid->addMultiCellWidget(mBSlider, 2, 2, 2, 3);
    mGrid->addWidget(mRIn, 0, 4);
    mGrid->addWidget(mGIn, 1, 4);
    mGrid->addWidget(mBIn, 2, 4);

    connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
    connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));
    connect(m_ColorButton, SIGNAL(currentChanged(KDualColorButton::DualColor)), this, SLOT(currentChanged(KDualColorButton::DualColor)));

    /* connect color sliders */
    connect(mRSlider, SIGNAL(valueChanged(int)), this, SLOT(slotRChanged(int)));
    connect(mGSlider, SIGNAL(valueChanged(int)), this, SLOT(slotGChanged(int)));
    connect(mBSlider, SIGNAL(valueChanged(int)), this, SLOT(slotBChanged(int)));

    /* connect spin box */
    connect(mRIn, SIGNAL(valueChanged(int)), this, SLOT(slotRChanged(int)));
    connect(mGIn, SIGNAL(valueChanged(int)), this, SLOT(slotGChanged(int)));
    connect(mBIn, SIGNAL(valueChanged(int)), this, SLOT(slotBChanged(int)));

    update(Qt::black, Qt::white);
}

void KoRGBWidget::slotRChanged(int r)
{
    if (m_ColorButton->current() == KDualColorButton::Foreground)
        slotFGColorSelected( QColor(r, m_fgColor.green(), m_fgColor.blue()));
    else
        slotBGColorSelected( QColor(r, m_bgColor.green(), m_bgColor.blue()));
}

void KoRGBWidget::slotGChanged(int g)
{
    if (m_ColorButton->current() == KDualColorButton::Foreground)
        slotFGColorSelected( QColor( m_fgColor.red(), g, m_fgColor.blue()));
    else
        slotBGColorSelected( QColor( m_bgColor.red(), g, m_bgColor.blue()));;
}

void KoRGBWidget::slotBChanged(int b)
{
    if (m_ColorButton->current() == KDualColorButton::Foreground)
        slotFGColorSelected( QColor( m_fgColor.red(), m_fgColor.green(), b));
    else
        slotBGColorSelected( QColor( m_bgColor.red(), m_bgColor.green(), b));
}

void KoRGBWidget::setFgColor(const QColor & c)
{
    blockSignals(true);
    slotFGColorSelected(c);
    blockSignals(false);
}

void KoRGBWidget::setBgColor(const QColor & c)
{
    blockSignals(true);
    slotBGColorSelected(c);
    blockSignals(false);
}

void KoRGBWidget::update(const QColor fgColor, const QColor bgColor)
{

    m_fgColor = fgColor;
    m_bgColor = bgColor;

    QColor color = (m_ColorButton->current() == KDualColorButton::Foreground)? m_fgColor : m_bgColor;

    int r = color.red();
    int g = color.green();
    int b = color.blue();

    mRSlider->blockSignals(true);
    mRIn->blockSignals(true);
    mGSlider->blockSignals(true);
    mGIn->blockSignals(true);
    mBSlider->blockSignals(true);
    mBIn->blockSignals(true);

    mRSlider->slotSetColor1(QColor(0, g, b));
    mRSlider->slotSetColor2(QColor(255, g, b));
    mRSlider->slotSetValue(r);
    mRIn->setValue(r);

    mGSlider->slotSetColor1(QColor(r, 0, b));
    mGSlider->slotSetColor2(QColor(r, 255, b));
    mGSlider->slotSetValue(g);
    mGIn->setValue(g);

    mBSlider->slotSetColor1(QColor(r, g, 0));
    mBSlider->slotSetColor2(QColor(r, g, 255));
    mBSlider->slotSetValue(b);
    mBIn->setValue(b);

    mRSlider->blockSignals(false);
    mRIn->blockSignals(false);
    mGSlider->blockSignals(false);
    mGIn->blockSignals(false);
    mBSlider->blockSignals(false);
    mBIn->blockSignals(false);
}

void KoRGBWidget::slotFGColorSelected(const QColor& c)
{
    m_fgColor = c;
    disconnect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));
    m_ColorButton->setForeground( m_fgColor );
    connect(m_ColorButton, SIGNAL(fgChanged(const QColor &)), this, SLOT(slotFGColorSelected(const QColor &)));

    update( m_fgColor, m_bgColor);
    emit sigFgColorChanged(m_fgColor);
}

void KoRGBWidget::slotBGColorSelected(const QColor& c)
{
    m_bgColor = c;

    disconnect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));
    m_ColorButton->setBackground( m_bgColor );
    connect(m_ColorButton, SIGNAL(bgChanged(const QColor &)), this, SLOT(slotBGColorSelected(const QColor &)));

    update(m_fgColor, m_bgColor);
    emit sigBgColorChanged(m_bgColor);
}

void KoRGBWidget::currentChanged(KDualColorButton::DualColor s)
{
   if(s == KDualColorButton::Foreground)
     slotFGColorSelected(m_ColorButton->currentColor());
   else
     slotBGColorSelected(m_ColorButton->currentColor());
}

#include "ko_rgb_widget.moc"
