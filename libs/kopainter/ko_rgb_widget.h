/* This file is part of the KDE project
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
#ifndef KO_RGB_WIDGET_H
#define KO_RGB_WIDGET_H

#include "qwidget.h"
//Added by qt3to4:
#include <QLabel>
#include <Q3GridLayout>

#include <koffice_export.h>
#include <kdualcolorbutton.h>

class KoFrameButton;
class Q3GridLayout;
class QColor;
class KoColorSlider;
class QLabel;
class QSpinBox;
class KDualColorButton;
class KoColorSlider;
class QColor;

class KoRGBWidget
     : public QWidget
{
    Q_OBJECT
    typedef QWidget super;

public:
    KoRGBWidget(QWidget *parent = 0L, const char *name = 0);
    virtual ~KoRGBWidget() {}

public slots:
    /**
     * Set the current color to c. Do not emit the color changed signals
     */
    virtual void setFgColor(const QColor & c);
    virtual void setBgColor(const QColor & c);

signals:

    /**
     * Emitted when the current color is changed.
     */
    virtual void sigFgColorChanged(const QColor & c);
    virtual void sigBgColorChanged(const QColor & c);


protected slots:

    virtual void slotRChanged(int r);
    virtual void slotGChanged(int g);
    virtual void slotBChanged(int b);

    void slotFGColorSelected(const QColor& c);
    void slotBGColorSelected(const QColor& c);
    void currentChanged(KDualColorButton::DualColor);

private:

    void update(const QColor fgColor, const QColor);

private:

    KoColorSlider *mRSlider;
    KoColorSlider *mGSlider;
    KoColorSlider *mBSlider;
    QLabel *mRLabel;
    QLabel *mGLabel;
    QLabel *mBLabel;
    QSpinBox *mRIn;
    QSpinBox *mGIn;
    QSpinBox *mBIn;
    KDualColorButton *m_ColorButton;

    QColor m_fgColor;
    QColor m_bgColor;
};

#endif
