/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef kpbackground_h
#define kpbackground_h

#include <qsize.h>
#include <global.h>
#include <kpimage.h>
#include <kpclipartcollection.h>

class KPresenterDoc;
class QPainter;
class QPixmap;
class KPGradientCollection;
class QPicture;
class DCOPObject;
class QDomDocument;
class QDomElement;
class KPrPage;

/******************************************************************/
/* Class: KPBackGround                                            */
/* The background for a given page.                               */
/******************************************************************/

class KPBackGround
{
public:
    KPBackGround( KPImageCollection *_imageCollection, KPGradientCollection *_gradientCollection,
                  KPClipartCollection *_clipartCollection, KPrPage *_page );
    ~KPBackGround()
    {; }

    DCOPObject* dcopObject();

    void setBackType( BackType _backType )
    { backType = _backType; }
    void setBackView( BackView _backView )
    { backView = _backView; }
    void setBackColor1( QColor _color )
    { removeGradient(); backColor1 = _color; }
    void setBackColor2( QColor _color )
    { removeGradient(); backColor2 = _color; }
    void setBackColorType( BCType _bcType )
    { removeGradient(); bcType = _bcType; }
    void setBackUnbalanced( bool _unbalanced )
    { removeGradient(); unbalanced = _unbalanced; }
    void setBackXFactor( int _xfactor )
    { removeGradient(); xfactor = _xfactor; }
    void setBackYFactor( int _yfactor )
    { removeGradient(); yfactor = _yfactor; }
    void setBackPixmap( const QString &_filename, QDateTime _lastModified );
    void setBackClipart(  const QString &_filename, QDateTime _lastModified );
    void setPageEffect( PageEffect _pageEffect )
    { pageEffect = _pageEffect; }
    void setPageTimer( int _pageTimer )
    { pageTimer = _pageTimer; }
    void setPageSoundEffect( bool _soundEffect )
    { soundEffect = _soundEffect; }
    void setPageSoundFileName( QString _soundFileName )
    { soundFileName = _soundFileName; }

    void setBgSize( QSize _size, bool visible = true );

    BackType getBackType() const
    { return backType; }
    BackView getBackView() const
    { return backView; }
    QColor getBackColor1() const
    { return backColor1; }
    QColor getBackColor2() const
    { return backColor2; }
    BCType getBackColorType() const
    { return bcType; }
    KPImageKey getBackPixKey() const
    { return backImage.key(); }
    KPClipartKey getBackClipKey() const
    { return backClipart.key(); }

    PageEffect getPageEffect() const
    { return pageEffect; }
    bool getBackUnbalanced() const
    { return unbalanced; }
    int getBackXFactor() const
    { return xfactor; }
    int getBackYFactor() const
    { return yfactor; }
    int getPageTimer() const
    { return pageTimer; }
    bool getPageSoundEffect() const
    { return soundEffect; }
    QString getPageSoundFileName() const
    { return soundFileName; }

    QSize getSize() const
    { return ext; }

    void draw( QPainter *_painter, bool _drawBorders );

    void restore();

    QDomElement save( QDomDocument &doc );
    void load( const QDomElement &element );

protected:
    void drawBackColor( QPainter *_painter );
    void drawBackPix( QPainter *_painter );
    void drawBorders( QPainter *_painter );
    void drawHeaderFooter( QPainter *_painter );
    void removeGradient();

    BackType backType;
    BackView backView;
    QColor backColor1;
    QColor backColor2;
    BCType bcType;
    PageEffect pageEffect;
    bool unbalanced;
    int xfactor, yfactor;
    int pageTimer;
    bool soundEffect;
    QString soundFileName;

    KPImage backImage;
    KPImageCollection *imageCollection;
    KPGradientCollection *gradientCollection;
    KPClipartCollection *clipartCollection;
    QPixmap *gradient;
    KPClipart backClipart;

    QSize ext;
    KPrPage *m_page;
    int footerHeight;

    DCOPObject *dcop;

};

#endif
