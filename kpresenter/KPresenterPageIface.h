/* This file is part of the KDE project
   Copyright (C) 2002, Laurent MONTEL <lmontel@mandrakesoft.com>

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

#ifndef KPRESENTER_PAGE_IFACE_H
#define KPRESENTER_PAGE_IFACE_H

#include <KoDocumentIface.h>
#include <dcopref.h>

#include <qstring.h>
#include <qcolor.h>
class KPrPage;

class KPresenterPageIface :  virtual public DCOPObject
{
    K_DCOP
public:
    KPresenterPageIface( KPrPage *_page );

k_dcop:
    virtual DCOPRef object( int num );
    virtual DCOPRef selectedObject();
    virtual DCOPRef textObject( int num );

    virtual int numTextObject() const;


    virtual QString manualTitle()const;
    virtual void insertManualTitle(const QString & title);
    virtual QString pageTitle( const QString &_title ) const;

    virtual void setNoteText( const QString &_text );
    virtual QString noteText( )const;

    virtual unsigned int objNums() const;
    virtual int numSelected() const;
    virtual void groupObjects();
    virtual void ungroupObjects();
    virtual void raiseObjs();
    virtual void lowerObjs();
    virtual void copyObjs();

    virtual void alignObjsLeft();
    virtual void alignObjsCenterH();
    virtual void alignObjsRight();
    virtual void alignObjsTop();
    virtual void alignObjsCenterV();
    virtual void alignObjsBottom();

    virtual void slotRepaintVariable();
    virtual void recalcPageNum();

    virtual void setPageTimer(  int pageTimer );
    virtual void setPageSoundEffect(  bool soundEffect );
    virtual void setPageSoundFileName(  const QString &fileName );
    virtual QString getPageSoundFileName()const;

    virtual bool getPageSoundEffect() const;
    virtual int getPageTimer() const;
    virtual int getBackYFactor() const;
    virtual int getBackXFactor() const;

    virtual int getBackType()const ;
    virtual int getBackView()const;
    virtual QColor getBackColor1()const;
    virtual QColor getBackColor2()const ;
    virtual int getBackColorType()const;
    virtual QString getBackPixFilename()const;
    virtual QString getBackClipFilename()const;
    virtual int getPageEffect()const;
    virtual bool getBackUnbalanced()const ;

    virtual bool setRectSettings( int _rx, int _ry );

    virtual int getPieAngle( int pieAngle )const;
    virtual int getPieLength( int pieLength )const;

    virtual QRect getPageRect()const;

    virtual bool isSlideSelected();
    virtual void slideSelected(bool _b);
    virtual void changePicture( const QString & );
    virtual void changeClipart( const QString & );

    DCOPRef insertRectangle(const QRect & rect);
    DCOPRef insertEllipse( const QRect &rect );
    DCOPRef insertPie( const QRect &rect );
    DCOPRef insertLineH( const QRect& rect, bool rev );
    DCOPRef insertLineV( const QRect &rect, bool rev );
    DCOPRef insertLineD1( const QRect &rect, bool rev );
    DCOPRef insertLineD2( const QRect &rect, bool rev );




private:
    KPrPage *m_page;

};

#endif
