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

#ifndef __kpresenter_doc_h__
#define __kpresenter_doc_h__

class KPresenterDoc;
class KPresenterView;
class DCOPObject;

#include <koDocument.h>
#include <koDocumentChild.h>
#include <koQueryTypes.h>

#include <qapplication.h>
#include <qlist.h>
#include <qobject.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qdatetime.h>

#include <koPageLayoutDia.h>

#include "kpobject.h"
#include "kpresenter_view.h"
#include "global.h"
#include "kpbackground.h"

#include "kppixmapcollection.h"
#include "kpgradientcollection.h"
#include "kpclipartcollection.h"
#include "commandhistory.h"

#include <komlParser.h>
#include <komlStreamFeed.h>
#include <komlWriter.h>
#include <ktempfile.h>

#include <iostream.h>
#include <fstream.h>
#include <torben.h>
#include <strstream.h>

#define MIME_TYPE "application/x-kpresenter"
#define EDITOR "IDL:KPresenter/KPresenterDocument:1.0"

class KPFooterHeaderEditor;
class KPTextObject;
class StyleDia;
class KPPartObject;

/******************************************************************/
/* class KPresenterChild					  */
/******************************************************************/
class KPresenterChild : public KoDocumentChild
{
public:

    // constructor - destructor
    KPresenterChild( KPresenterDoc *_kpr, KoDocument* _doc, const QRect& _rect, int, int );
    KPresenterChild( KPresenterDoc *_kpr );
    ~KPresenterChild();

    // get parent
    KPresenterDoc* parent() { return (KPresenterDoc*)parent(); }

    virtual KoDocument *hitTest( const QPoint &, const QWMatrix & );

};

/*****************************************************************/
/* class KPresenterDoc						 */
/*****************************************************************/
class KPresenterDoc : public KoDocument
{
    Q_OBJECT

public:

    // constructor - destructor
    KPresenterDoc( QObject* doc = 0, const char* name = 0, bool singleViewMode = false );
    ~KPresenterDoc();

    KoMainWindow* createShell();
    KoView* createView( QWidget* parent, const char* name );

    // Drawing
    virtual void paintContent( QPainter& painter, const QRect& rect, bool transparent = FALSE );

    // save
    virtual bool save( ostream&, const char *_format );
    virtual bool completeSaving( KoStore* _store );

    // load
    virtual bool loadXML( KOMLParser&, KoStore* );
    virtual bool loadChildren( KoStore* _store );

    virtual bool initDoc() { return insertNewTemplate( 0, 0, TRUE ); }
    void initEmpty();

    // get mime type
    virtual QCString mimeType() const { return QCString( MIME_TYPE ); }

    // get output- and inputformats
    virtual QStrList outputFormats();
    virtual QStrList inputFormats();

    // insert an object
    virtual void insertObject( const QRect&, KoDocumentEntry&, int, int );

    // change geomentry of a child
    virtual void changeChildGeometry( KPresenterChild*, const QRect&, int, int );

    // page layout
    void setPageLayout( KoPageLayout, int, int );
    KoPageLayout pageLayout() {return _pageLayout; }

    // insert a page
    unsigned int insertNewPage( int, int, bool _restore=true );
    bool insertNewTemplate( int, int, bool clean=false );

    // get number of pages nad objects
    unsigned int getPageNums() {return _backgroundList.count(); }
    unsigned int objNums() {return _objectList->count(); }

    // background
    void setBackColor( unsigned int, QColor, QColor, BCType, bool, int, int );
    void setBackPixFilename( unsigned int, QString );
    void setBackClipFilename( unsigned int, QString );
    void setBackView( unsigned int, BackView );
    void setBackType( unsigned int, BackType );
    bool setPenBrush( QPen, QBrush, LineEnd, LineEnd, FillType, QColor, QColor, BCType,
		      bool, int, int );
    bool setLineBegin( LineEnd );
    bool setLineEnd( LineEnd );
    bool setPieSettings( PieType, int, int );
    bool setRectSettings( int, int );
    void setPageEffect( unsigned int, PageEffect );
    bool setPenColor( QColor, bool );
    bool setBrushColor( QColor, bool );
    BackType getBackType( unsigned int );
    BackView getBackView( unsigned int );
    QString getBackPixFilename( unsigned int );
    QString getBackClipFilename( unsigned int );
    QDateTime getBackPixLastModified( unsigned int );
    QDateTime getBackClipLastModified( unsigned int );
    QColor getBackColor1( unsigned int );
    QColor getBackColor2( unsigned int );
    bool getBackUnbalanced( unsigned int );
    int getBackXFactor( unsigned int );
    int getBackYFactor( unsigned int );
    BCType getBackColorType( unsigned int );
    PageEffect getPageEffect( unsigned int );
    QPen getPen( QPen );
    QBrush getBrush( QBrush );
    LineEnd getLineBegin( LineEnd );
    LineEnd getLineEnd( LineEnd );
    FillType getFillType( FillType );
    QColor getGColor1( QColor );
    QColor getGColor2( QColor );
    BCType getGType( BCType );
    PieType getPieType( PieType );
    bool getGUnbalanced( bool );
    int getGXFactor( int );
    int getGYFactor( int );
    int getRndX( int );
    int getRndY( int );
    int getPieLength( int );
    int getPieAngle( int );

    // raise and lower objs
    void raiseObjs( int, int );
    void lowerObjs( int, int );

    // insert/change objects
    void insertPicture( QString, int, int, int _x = 10, int _y = 10 );
    void insertClipart( QString, int, int );
    void changePicture( QString, int, int );
    void changeClipart( QString, int, int );
    void insertLine( QRect, QPen, LineEnd, LineEnd, LineType, int, int );
    void insertRectangle( QRect, QPen, QBrush, FillType, QColor, QColor, BCType, int, int, bool ,int, int, int, int );
    void insertCircleOrEllipse( QRect, QPen, QBrush, FillType, QColor, QColor, BCType, bool ,int, int, int, int );
    void insertPie( QRect, QPen pen, QBrush brush, FillType ft, QColor g1, QColor g2,
		    BCType gt, PieType pt, int _angle, int _len, LineEnd lb, LineEnd le, bool ,int, int, int diffx, int diffy );
    void insertText( QRect, int, int, QString text = QString::null, KPresenterView *_view = 0L );
    void insertAutoform( QRect, QPen, QBrush, LineEnd, LineEnd, FillType, QColor,
			 QColor, BCType, QString, bool ,int, int, int, int );

    // get list of pages and objects
    QList<KPBackGround> *backgroundList() {return &_backgroundList; }
    QList<KPObject> *objectList() {return _objectList; }

    // get - set raster
    unsigned int rastX() {return _rastX; }
    unsigned int rastY() {return _rastY; }
    unsigned int getRastX() {return _rastX; }
    unsigned int getRastY() {return _rastY; }
    void setRasters( unsigned int rx, unsigned int ry, bool _replace = true );

    // get - set options for editmodi
    QColor txtBackCol() {return _txtBackCol; }
    QColor getTxtBackCol() {return _txtBackCol; }
    void setTxtBackCol( QColor c ) {_otxtBackCol = _txtBackCol; _txtBackCol = c; }

    // get - set roundedness
    unsigned int getRndX() {return _xRnd; }
    unsigned int getRndY() {return _yRnd; }

    // get values for screenpresentations
    bool spInfinitLoop() {return _spInfinitLoop; }
    bool spManualSwitch() {return _spManualSwitch; }
    void setInfinitLoop( bool il ) {_spInfinitLoop = il; }
    void setManualSwitch( bool ms ) {_spManualSwitch = ms; }

    // size of page
    QRect getPageSize( unsigned int, int, int, float fakt=1.0, bool decBorders = true );

    // delete/reorder obejcts
    void deleteObjs( bool _add = true );
    void copyObjs( int, int );
    void pasteObjs( int, int, int );
    void savePage( const QString &file, int pgnum );

    // repaint all views
    void repaint( bool );

    void repaint( QRect );
    void repaint( KPObject* );

    // stuff for screen-presentations
    QValueList<int> reorderPage( unsigned int, int, int, float fakt = 1.0 );
    int getPageOfObj( int, int, int, float fakt = 1.0 );

    QPen presPen() {return _presPen; }
    void setPresPen( QPen p ) {_presPen = p; }

    int numSelected();
    KPObject* getSelectedObj();

    void restoreBackground( int );
    void loadStream( istream&, int );

    void deSelectAllObj();

    KPPixmapCollection *getPixmapCollection() { return &_pixmapCollection; }
    KPGradientCollection *getGradientCollection() { return &_gradientCollection; }
    KPClipartCollection *getClipartCollection() { return &_clipartCollection; }

    CommandHistory *commands() { return &_commands; }

    void alignObjsLeft();
    void alignObjsCenterH();
    void alignObjsRight();
    void alignObjsTop();
    void alignObjsCenterV();
    void alignObjsBottom();
    void replaceObjs();

    PresSpeed getPresSpeed() { return presSpeed; }
    void setPresSpeed( PresSpeed _presSpeed ) { presSpeed = _presSpeed; }

    int getLeftBorder();
    int getTopBorder();
    int getBottomBorder();

    void deletePage( int _page, DelPageMode _delPageMode );
    void insertPage( int _page, InsPageMode _insPageMode, InsertPos _insPos );

    void setObjectList( QList<KPObject> *_list ) {
	_objectList->setAutoDelete( false ); _objectList = _list; _objectList->setAutoDelete( false );
    }

    int getPenBrushFlags();

    void enableEmbeddedParts( bool f );

    void setUnit( KoUnit _unit, QString __unit );

    QString getPageTitle( unsigned int pgNum, const QString &_title );

    KPTextObject *header() { return _header; }
    KPTextObject *footer() { return _footer; }
    bool hasHeader() { return _hasHeader; }
    bool hasFooter() { return _hasFooter; }
    void setHeader( bool b );
    void setFooter( bool b );
    KPFooterHeaderEditor *getHeaderFooterEdit() { return headerFooterEdit; }

    PresentSlides getPresentSlides() { return presentSlides; }
    const QMap<int,bool> &getSelectedSlides() { return selectedSlides; }
    void setPresentSlides( PresentSlides ps ) { presentSlides = ps; }
    void setSelectedSlides( const QMap<int,bool> &ss ) { selectedSlides = QMap<int,bool>( ss ); }

    QValueList<int> getSlides( int currPgNum );

    void copyPage( int num );
    virtual DCOPObject* dcopObject();

    KPresenterView *getFirstView() {
	return (KPresenterView*)firstView();
    }
    KPresenterView *getNextView() {
	return (KPresenterView*)nextView();
    }

    void groupObjects();
    void ungroupObjects();

signals:

    // document modified
    void sig_KPresenterModified();

    // object inserted - removed
    void sig_insertObject( KPresenterChild *_child, KPPartObject *_kppo );
    void sig_removeObject( KPresenterChild *_child );

    // update child geometry
    void sig_updateChildGeometry( KPresenterChild *_child );

protected slots:
    void slotUndoRedoChanged( QString, QString );

    void slotDocumentLoaded();

protected:

    // ************ functions ************
    /**
     * Overloaded function from @ref Document_impl. Saves all children.
     */
    virtual bool saveChildren( KoStore* _store, const char *_path );
    /*
     * Overloaded function from @ref KoDocument.
     *
     * @return true if one of the direct children wants to
     *		    be saved embedded. If there are no children or if
     *		    every direct child saves itself into its own file
     *		    then false is returned.
     */
    virtual bool hasToWriteMultipart();

    virtual void draw( QPaintDevice*, long int, long int, float _scale );

    void saveBackground( ostream& );
    void saveObjects( ostream& );
    void loadBackground( KOMLParser&, vector<KOMLAttrib>& );
    void loadObjects( KOMLParser&, vector<KOMLAttrib>&, bool _paste = false );
    virtual bool completeLoading( KoStore* /* _store */ );
    void makeUsedPixmapList();

    void addToRecentlyOpenedList( const QString &file );
    QStringList getRecentryOpenedList();

    // ************ variables ************

    struct PixCache
    {
	QString filename;
	QString pix_string;
    };

    // list of views and children
    // QList<KPresenterView> m_lstViews;
    // QList<KPresenterChild> m_lstChildren;
    KPresenterView *viewPtr;

    // page layout and background
    KoPageLayout _pageLayout;
    QList<KPBackGround> _backgroundList;

    // list of objects
    QList<KPObject> *_objectList;

    // screenpresentations
    bool _spInfinitLoop, _spManualSwitch;

    // options
    int _rastX, _rastY;
    int _xRnd, _yRnd;

    // options for editmode
    QColor _txtBackCol;
    QColor _otxtBackCol;

    bool _clean;
    int objStartY, objStartNum;
    bool docAlreadyOpen;

    int _orastX, _orastY;
    PresSpeed presSpeed;

    QPen _presPen;

    KPPixmapCollection _pixmapCollection;
    KPGradientCollection _gradientCollection;
    KPClipartCollection _clipartCollection;

    CommandHistory _commands;
    bool pasting;
    int pasteXOffset, pasteYOffset;

    KPTextObject *_header, *_footer;
    bool _hasHeader, _hasFooter;
    KPFooterHeaderEditor *headerFooterEdit;

    QValueList<KPPixmapDataCollection::Key> pixmapCollectionKeys;
    QValueList<KPClipartCollection::Key> clipartCollectionKeys;
    QStringList pixmapCollectionNames, clipartCollectionNames;
    KoPageLayout __pgLayout;
    int lastObj;

    QString urlIntern;

    PresentSlides presentSlides;
    QMap<int,bool> selectedSlides;
    QValueList<KPPixmapDataCollection::Key> usedPixmaps;
    DCOPObject *dcop;

    int saveOnlyPage;

};

#endif
