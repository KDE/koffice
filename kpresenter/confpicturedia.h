/* This file is part of the KDE project

   Copyright (C) 2002 Toshitaka Fujioka <fujioka@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef confpicturedia_h
#define confpicturedia_h

#include <kdialogbase.h>

#include <qframe.h>
#include <qlabel.h>

#include <global.h>

class QPainter;
class QLabel;
class KIntNumInput;
class QGroupBox;
class QPushButton;
class QRadioButton;
class QCheckBox;

/******************************************************************/
/* class PicturePreview                                           */
/******************************************************************/

class PicturePreview : public QFrame
{
    Q_OBJECT

public:
    PicturePreview( QWidget* parent, const char* name, PictureMirrorType _mirrorType,
                    int _dept, bool _swapRGB, int _bright, QPixmap _origPixmap );
    ~PicturePreview() {}

    void setDepth( int depth);
    void setMirrorType (PictureMirrorType _t);
public slots:
    void slotNormalPicture();
    void slotHorizontalMirrorPicture();
    void slotVerticalMirrorPicture();
    void slotHorizontalAndVerticalMirrorPicture();

    void slotPictureDepth0();
    void slotPictureDepth1();
    void slotPictureDepth8();
    void slotPictureDepth16();
    void slotPictureDepth32();

    void slotSwapRGBPicture( bool _on );

    void slotBrightValue( int _value );

protected:
    virtual void drawContents( QPainter *painter );

    PictureMirrorType mirrorType;
    int depth;
    bool swapRGB;
    int bright;
    QPixmap origPixmap;
};

/******************************************************************/
/* class ConfPictureDia                                           */
/******************************************************************/

class ConfPictureDia : public KDialogBase
{
    Q_OBJECT

public:
    ConfPictureDia( QWidget *parent, const char *name, PictureMirrorType _mirrorType,
                    int _depth, bool _swapRGB, int _bright, QPixmap _origPixmap );
    ~ConfPictureDia();

    PictureMirrorType getPictureMirrorType() const { return mirrorType; }
    int getPictureDepth() const { return depth; }
    bool getPictureSwapRGB() const { return swapRGB; }
    int getPictureBright() const { return bright; }

protected:
    QRadioButton *m_normalPicture, *m_horizontalMirrorPicture, *m_verticalMirrorPicture, *m_horizontalAndVerticalMirrorPicture;
    QRadioButton *m_depth0, *m_depth1, *m_depth8, *m_depth16, *m_depth32;
    QCheckBox *m_swapRGBCheck;
    KIntNumInput *m_brightValue;

    QGroupBox *gSettings;
    PicturePreview *picturePreview;
    QPixmap origPixmap;

    PictureMirrorType mirrorType, oldMirrorType;

    int depth, bright, oldDepth, oldBright;
    bool swapRGB, oldSwapRGB;

protected slots:
    void slotNormalPicture();
    void slotHorizontalMirrorPicture();
    void slotVerticalMirrorPicture();
    void slotHorizontalAndVerticalMirrorPicture();

    void slotPictureDepth0();
    void slotPictureDepth1();
    void slotPictureDepth8();
    void slotPictureDepth16();
    void slotPictureDepth32();

    void slotSwapRGBPicture( bool _on );

    void slotBrightValue( int _value );

    void slotReset();

    void Apply() { emit confPictureDiaOk(); }

signals:
    void confPictureDiaOk();

};

#endif
