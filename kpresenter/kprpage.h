/* This file is part of the KDE project
   Copyright (C) 2002 Laurent MONTEL <lmontel@mandrakesoft.com>

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

#ifndef KPRPAGE_H
#define KPRPAGE_H

#include <qwidget.h>
#include <qptrlist.h>
#include <global.h>
#include <koPoint.h>
#include "kpbackground.h"
#include <koRect.h>
#include <kostyle.h>
#include <qvariant.h>
class KURL;
class KPTextView;
class KPObject;
class KPresenterDoc;
class KPresenterView;
class KoDocumentEntry;
class KoPageLayout;
class KCommand;
class KoPointArray;
class DCOPObject;
class KPTextObject;
class KoTextObject;
class KPPixmapObject;
class KoStyle;
class KPPartObject;

class KPrPage
{
public:

    // constructor - destructor
    KPrPage(KPresenterDoc *_doc);
    virtual ~KPrPage();

    virtual DCOPObject* dcopObject();

    KPresenterDoc * kPresenterDoc() const {return m_doc; }

    QString manualTitle()const;
    void insertManualTitle(const QString & title);
    QString pageTitle( const QString &_title ) const;

    void setNoteText( const QString &_text );
    QString noteText( )const;

    const QPtrList<KPObject> & objectList() const { return m_objectList;}

    KPObject *getObject(int num);

    void appendObject(KPObject *);
    void insertObject(KPObject *_oldObj, KPObject *_newObject);
    void takeObject(KPObject *_obj);
    void removeObject( int pos);
    void insertObject(KPObject *_obj,int pos);
    void completeLoading( bool _clean, int lastObj );



    KoRect getPageRect() const;

    QRect getZoomPageRect()const;

    void setObjectList( QPtrList<KPObject> _list ) {
        m_objectList.setAutoDelete( false ); m_objectList = _list; m_objectList.setAutoDelete( false );
    }

    unsigned int objNums() const { return m_objectList.count(); }

    int numTextObject() const;
    KPTextObject *textFrameSet( unsigned int _num ) const;

    KCommand * deleteObjs( bool _add=true );
    int numSelected() const;
    void pasteObjs( const QByteArray & data, int nbCopy = 1, double angle = 0.0 , double _increaseX=0.0, double increaseY = 0.0, double moveX=0.0, double moveY=0.0);
    KCommand * replaceObjs( bool createUndoRedo, double _orastX, double _orastY,const QColor & _txtBackCol, const QColor & _otxtBackCol);

    void copyObjs(QDomDocument &doc, QDomElement &presenter) const;

    KPObject* getSelectedObj() const;
    KPPixmapObject* getSelectedImage() const;

    ImageEffect getImageEffect(ImageEffect eff) const;
    KCommand * setImageEffect(ImageEffect eff, QVariant param1, QVariant param2, QVariant param3);

    void groupObjects();
    KCommand * ungroupObjects();
    void raiseObjs( bool forward );
    void lowerObjs( bool backward );
    bool getCheckConcavePolygon( bool check ) const;
    int getCornersValue( int corners ) const;
    int getSharpnessValue( int sharpness ) const;
    PictureMirrorType getPictureMirrorType( PictureMirrorType type ) const;
    int getPictureDepth( int depth ) const;
    bool getPictureSwapRGB( bool swapRGB ) const;
    bool getPictureGrayscal( bool grayscal ) const;
    int getPictureBright( int bright ) const;
    QPixmap getPicturePixmap() const;
    int getRndY( int _ry ) const;
    int getRndX( int _rx ) const;
    int getPieAngle( int pieAngle ) const;
    int getPieLength( int pieLength ) const;
    bool getSticky( bool s ) const;
    PieType getPieType( PieType pieType ) const;
    int getGYFactor( int yfactor )const;
    int getGXFactor( int xfactor )const;
    bool getGUnbalanced( bool  unbalanced ) const;
    bool getBackUnbalanced( unsigned int ) const;
    BCType getGType( BCType gt )const;
    QColor getGColor2( const QColor &g2 ) const;
    QColor getGColor1( const QColor & g1)const;
    FillType getFillType( FillType ft ) const;
    QBrush getBrush( const QBrush &brush )const;
    LineEnd getLineEnd( LineEnd le ) const;
    LineEnd getLineBegin( LineEnd lb ) const;

    bool getProtect( bool p ) const;
    bool differentProtect( bool p) const;

    bool getKeepRatio( bool p ) const;
    bool differentKeepRatio( bool p) const;

    bool getProtectContent(bool prot) const;

    KCommand* setPen( const QPen &pen, LineEnd lb, LineEnd le, int flags, QPtrList<KPObject> list);
    KCommand* setBrush( const QBrush &brush, FillType ft,const  QColor& g1, const QColor &g2, BCType gt,
                        bool unbalanced, int xfactor, int yfactor, int flags, QPtrList<KPObject> list);

    QPen getPen( const QPen & pen ) const;

    // insert an object
    virtual KPPartObject* insertObject( const KoRect&, KoDocumentEntry& );

    void insertRectangle( const KoRect &r, const QPen & pen, const QBrush &brush, FillType ft, const QColor &g1, const QColor & g2,BCType gt, int rndX, int rndY, bool unbalanced, int xfactor, int yfactor );

    void insertCircleOrEllipse( const KoRect &r, const QPen &pen, const QBrush &brush, FillType ft, const QColor &g1, const QColor &g2, BCType gt, bool unbalanced, int xfactor, int yfactor );

    void insertPie( const KoRect &r, const QPen &pen, const QBrush &brush, FillType ft, const QColor &g1, const QColor &g2,BCType gt, PieType pt, int _angle, int _len, LineEnd lb,LineEnd le,bool unbalanced, int xfactor, int yfactor );

    KPTextObject* insertTextObject( const KoRect& r, const QString& text = QString::null, KPresenterView *_view = 0L );
    void insertLine( const KoRect &r, const QPen &pen, LineEnd lb, LineEnd le, LineType lt );

    void insertAutoform( const KoRect &r, const QPen &pen, const QBrush &brush, LineEnd lb, LineEnd le, FillType ft,const QColor &g1, const QColor &g2, BCType gt, const QString &fileName, bool unbalanced,int xfactor, int yfactor );

    void insertFreehand( const KoPointArray &points, const KoRect &r, const QPen &pen,LineEnd lb, LineEnd le );
    void insertPolyline( const KoPointArray &points, const KoRect &r, const QPen &pen,LineEnd lb, LineEnd le );
    void insertQuadricBezierCurve( const KoPointArray &points, const KoPointArray &allPoints, const KoRect &r, const QPen &pen,LineEnd lb, LineEnd le );
    void insertCubicBezierCurve( const KoPointArray &points, const KoPointArray &allPoints, const KoRect &r, const QPen &pen,LineEnd lb, LineEnd le );

    void insertPolygon( const KoPointArray &points, const KoRect &r, const QPen &pen, const QBrush &brush, FillType ft,const QColor &g1, const QColor &g2, BCType gt, bool unbalanced, int xfactor, int yfactor, bool _checkConcavePolygon, int _cornersValue, int _sharpnessValue );

    void insertClosedLine( const KoPointArray &points, const KoRect &r, const QPen &pen, const QBrush &brush, FillType ft,const QColor &g1, const QColor &g2,
                           BCType gt, bool unbalanced, int xfactor, int yfactor, ToolEditMode _mode );


    KCommand *alignObjsLeft(const KoRect &rect = KoRect());
    KCommand *alignObjsCenterH(const KoRect &rect= KoRect());
    KCommand *alignObjsRight(const KoRect &rect= KoRect());
    KCommand *alignObjsTop(const KoRect &rect= KoRect());
    KCommand *alignObjsCenterV(const KoRect &rect= KoRect());
    KCommand *alignObjsBottom(const KoRect &rect= KoRect());

    int getPenBrushFlags( QPtrList<KPObject>list ) const;
    KCommand* setPieSettings( PieType pieType, int angle, int len, int flags );
    KCommand* setRectSettings( int _rx, int _ry, int flags );
    KCommand* setPolygonSettings( bool _checkConcavePolygon, int _cornersValue, int _sharpnessValue, int flags );
    KCommand* setPictureSettings( PictureMirrorType _mirrorType, int _depth, bool _swapRGB, bool _grayscal, int _bright );
    KCommand* setBrushColor( const QColor &c, bool fill, QPtrList<KPObject> list );

    void slotRepaintVariable();
    void recalcPageNum();
    void changePicture( const KURL & url );
    void insertPicture( const QString &, int _x = 10, int _y = 10 );
    void insertPicture( const QString &_file, const KoRect &_rect );

    void enableEmbeddedParts( bool f );
    void deletePage( );

    KPBackGround *background() const { return kpbackground;}

    void makeUsedPixmapList();

    void setBackColor( const QColor &backColor1, const QColor &backColor2, BCType bcType,
			    bool unbalanced, int xfactor, int yfactor );
    void setBackPixmap( const KoPictureKey & key );
    bool getBackUnbalanced(  )const;
    void setBackClipart( const KoPictureKey & key );
    void setBackView( BackView backView );
    void setBackType( BackType backType );

    void setPageEffect(  PageEffect pageEffect );
    void setPageTimer(  int pageTimer );
    void setPageSoundEffect(  bool soundEffect );
    void setPageSoundFileName(  const QString &fileName );
    BackType getBackType(  ) const ;
    BackView getBackView( )const ;
    KoPictureKey getBackPixKey( )const ;
    KoPictureKey getBackClipKey(  )const ;
    QColor getBackColor1( )const ;
    QColor getBackColor2()const ;
    int getBackXFactor()const ;
    int getBackYFactor( )const;
    BCType getBackColorType( )const;
    PageEffect getPageEffect( )const;
    int getPageTimer(  )const;
    bool getPageSoundEffect( )const;
    QString getPageSoundFileName()const;

    QValueList<int> reorderPage() const;

    bool isSlideSelected()const {return  m_selectedSlides;}
    void slideSelected(bool _b){m_selectedSlides=_b;}

    void setInsPictureFile( const QString &_file ) { m_pictureFile = _file; }

    QString insPictureFile() const { return m_pictureFile; }

    void deSelectAllObj();
    void deSelectObj( KPObject *kpobject );
    QDomElement saveObjects( QDomDocument &doc, QDomElement &objects, double yoffset, KoZoomHandler* zoomHandler, int saveOnlyPage ) const;

    bool oneObjectTextExist(bool forceAllTextObject = true) ;
    bool oneObjectTextSelected();
    bool isOneObjectSelected();
    bool haveASelectedPartObj();
    bool haveASelectedGroupObj();
    bool haveASelectedPixmapObj();

    KoRect getBoundingRect(const KoRect &rect, KPresenterDoc *doc) const;
    KoRect getBoundingAllObjectRect(const KoRect &rect, KPresenterDoc *doc) const;
    bool chPic( KPresenterView *_view);

    //return command when we move object
    KCommand *moveObject(KPresenterView *_view, int diffx,int diffy);
    KCommand *moveObject(KPresenterView *m_view,const KoPoint &_move,bool key);

    KCommand *rotateObj(float _newAngle, bool addAngle=false);
    KCommand *shadowObj(ShadowDirection dir,int dist, const QColor &col);
    KCommand *stickyObj(bool _sticky, KPrPage * currentPage);

    QPtrList<KoTextObject> objectText(QPtrList<KPObject> list);


    void repaintObj();

    KPObject * getCursor(const QPoint &pos );
    KPObject * getCursor(const KoPoint &pos );

    KPObject * getObjectResized(const KoPoint &pos, ModifyType modType, bool &desel, bool &_over, bool &_resize );
    KPObject* getObjectAt( const KoPoint&pos ) const;
    KPPixmapObject * picViewOrigHelper() const;
    void applyStyleChange( StyleChangeDefMap changed );

    void reactivateBgSpellChecking(bool refreshTextObj);

    bool canMoveOneObject() const;
    KCommand *alignVertical( VerticalAlignmentType _type );
    void changeTabStopValue ( double _tabStop );
    bool savePicture( KPresenterView *_view ) const;
    bool findTextObject( KPObject *obj );
    KPObject *nextTextObject(KPTextObject *obj);

    void getAllObjectSelectedList(QPtrList<KPObject> &lst,bool force = false );

protected:

private:
    void makeUsedPixmapListForGroupObject( KPObject *_obj );
    void completeLoadingForGroupObject( KPObject *_obj );


    // list of objects
    QPtrList<KPObject> m_objectList;
    KPresenterDoc *m_doc;
    KPBackGround *kpbackground;
    QString m_manualTitle;
    QString m_noteText;
    DCOPObject *dcop;
    bool m_selectedSlides;

    QString m_pictureFile;
};
#endif //KPRPAGE_H
