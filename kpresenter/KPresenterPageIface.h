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

    virtual DCOPRef groupObjects();

    virtual int numTextObject() const;


    virtual QString manualTitle()const;
    virtual void insertManualTitle(const QString & title);
    virtual QString pageTitle( const QString &_title ) const;

    virtual void setNoteText( const QString &_text );
    virtual QString noteText( )const;

    virtual unsigned int objNums() const;
    virtual int numSelected() const;
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
    virtual QString pageSoundFileName()const;

    virtual bool pageSoundEffect() const;
    virtual int pageTimer() const;
    virtual int backYFactor() const;
    virtual int backXFactor() const;

    virtual int backType()const ;
    virtual int backView()const;
    virtual QColor backColor1()const;
    virtual QColor backColor2()const ;
    virtual int backColorType()const;
    virtual QString backPixFilename()const;
    virtual QString backClipFilename()const;
    virtual int pageEffect()const;
    virtual QString pageEffectString( )const;
    virtual void setPageEffect(const QString & );

    virtual bool backUnbalanced()const ;

    virtual bool setRectSettings( int _rx, int _ry );

    virtual int pieAngle( int pieAngle )const;
    virtual int pieLength( int pieLength )const;

    virtual QRect pageRect()const;

    virtual bool isSlideSelected();
    virtual void slideSelected(bool _b);
    virtual void changePicture( const QString & );
    virtual void changeClipart( const QString & );

    //return -1 if there is not a rndY or rndX defined
    virtual int rndY();
    virtual int rndX();
    virtual void setBackGroundColor1(const QColor &col);
    virtual void setBackGroundColor2(const QColor &col);
    virtual void setBackGroundColorType(const QString &type);

    DCOPRef insertRectangle(int x,int y, int h, int w);
    DCOPRef insertEllipse(int x,int y, int h, int w );
    DCOPRef insertPie( int x,int y, int h, int w );
    DCOPRef insertLineH( int x,int y, int h, int w, bool rev );
    DCOPRef insertLineV( int x,int y, int h, int w, bool rev );
    DCOPRef insertLineD1( int x,int y, int h, int w, bool rev );
    DCOPRef insertLineD2( int x,int y, int h, int w, bool rev );
    DCOPRef insertTextObject( int x,int y, int h, int w );
    DCOPRef insertPicture( const QString & file,int x,int y, int h, int w );
    DCOPRef insertClipart( const QString & file,int x,int y, int h, int w );

    void deSelectAllObj();

    bool oneObjectTextExist() const ;
    bool isOneObjectSelected() const;

private:
    KPrPage *m_page;

};

#endif
