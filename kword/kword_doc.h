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

#ifndef kword_doc_h
#define kword_doc_h

class KWordDocument;
class KWPage;
class QPrinter;
class KWCharImage;
class KWPictureFrameSet;
class KWSerialLetterDataBase;
class KWContents;

#include <koDocument.h>
#include <koPageLayoutDia.h>
#include <koQueryTypes.h>

#include <iostream>

#include "kword_view.h"
#include "fc.h"
#include "parag.h"
#include "paraglayout.h"
#include "formatcollection.h"
#include "imagecollection.h"
#include "kword_frame.h"
#include "variable.h"
#include "footnote.h"
#include "autoformat.h"
#include "kword_undo.h"

#include <qlist.h>
#include <qpainter.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qintdict.h>
#include <qstringlist.h>
#include <qrect.h>
#include <qdict.h>

/******************************************************************/
/* Class: KWordChild						  */
/******************************************************************/

class KWordChild : public KoDocumentChild
{
public:
    KWordChild( KWordDocument *_wdoc, const QRect& _rect, KoDocument *_doc, int diffx, int diffy );
    KWordChild( KWordDocument *_wdoc );
    ~KWordChild();

    KWordDocument* parent()
    { return m_pKWordDoc; }

protected:
    KWordDocument *m_pKWordDoc;

};

/******************************************************************/
/* Class: KWordDocument						  */
/******************************************************************/

class KWordDocument : public KoDocument
{
    Q_OBJECT

public:
    KWordDocument( KoDocument* doc = 0, const char* name = 0 );
    ~KWordDocument();

    enum ProcessingType {WP = 0, DTP = 1};
    static const int U_FONT_FAMILY_SAME_SIZE = 1;
    static const int U_FONT_ALL_SAME_SIZE = 2;
    static const int U_COLOR = 4;
    static const int U_INDENT = 8;
    static const int U_BORDER = 16;
    static const int U_ALIGN = 32;
    static const int U_NUMBERING = 64;
    static const int U_FONT_FAMILY_ALL_SIZE = 128;
    static const int U_FONT_ALL_ALL_SIZE = 256;
    static const int U_TABS = 512;
    static const int U_SMART = 1024;

protected:
    virtual bool hasToWriteMultipart();

public:
    // IDL
    virtual bool initDoc();

    // C++
    virtual bool loadXML( KOMLParser& parser, KoStore *_store );
    virtual bool loadChildren( KoStore *_store );
    virtual bool save( ostream& out, const char* _format );
    virtual bool completeSaving( KoStore *_store );

    virtual bool loadTemplate( const char *_url );

    // IDL
    Shell* createShell();
    View* createView( QWidget* parent, const char* name );
    virtual QString configFile() const;
    virtual void paintContent( QPainter& painter, const QRect& rect, bool transparent = FALSE );

    virtual QCString mimeType() const
    { return MIME_TYPE; }

    virtual bool isModified()
    { return m_bModified; }

    // C++
    virtual void setModified( bool _c ) { m_bModified = _c; if ( _c ) m_bEmpty = false; }
    virtual bool isEmpty() { return m_bEmpty; }

    unsigned int viewCount() { return m_lstViews.count(); }

    virtual QStrList outputFormats();
    virtual QStrList inputFormats();

    virtual void addView( KWordView *_view );
    virtual void removeView( KWordView *_view );

    virtual void insertObject( const QRect& _rect, KoDocumentEntry& _e, int diffx, int diffy );
    virtual void changeChildGeometry( KWordChild *_child, const QRect& );

    virtual QListIterator<KWordChild> childIterator();

    void setPageLayout( KoPageLayout _layout, KoColumns _cl, KoKWHeaderFooter _hf );

    void getPageLayout( KoPageLayout& _layout, KoColumns& _cl, KoKWHeaderFooter& _hf )
    { _layout = pageLayout; _cl = pageColumns; _hf = pageHeaderFooter; }

    KWFrameSet *getFrameSet( unsigned int _num )
    { return frames.at( _num ); }
    unsigned int getNumFrameSets()
    { return frames.count(); }
    void addFrameSet( KWFrameSet *f )
    { frames.append(f); updateAllFrames(); /*updateAllViews(0L);*/ }
    void delFrameSet( KWFrameSet *f )
    { frames.remove( f ); }

    KWParag *getFirstParag( unsigned int _num ) {
	if ( frames.at( _num )->getFrameType() == FT_TEXT )
	    return dynamic_cast<KWTextFrameSet*>( frames.at( _num ) )->getFirstParag();
	else
	    return 0L;
    }

    KWUserFont* getDefaultUserFont()
    { return defaultUserFont;  }	
    KWParagLayout *getDefaultParagLayout()
    { return defaultParagLayout; }

    QList<KWUserFont> userFontList;

    QList<KWDisplayFont> displayFontList;

    QList<KWParagLayout> paragLayoutList;

    KWUserFont* findUserFont( QString _fontname );

    KWDisplayFont* findDisplayFont( KWUserFont* _font, unsigned int _size, int _weight, bool _italic, bool _underline );

    KWParagLayout* findParagLayout( QString _name );

    KWParag* findFirstParagOfPage( unsigned int _page, unsigned int _frameset );
    KWParag* findFirstParagOfRect( unsigned int _ypos, unsigned int _page, unsigned int _frameset );

    unsigned int getPTTopBorder() { return pageLayout.ptTop; }
    unsigned int getPTBottomBorder() { return pageLayout.ptBottom; }
    unsigned int getPTLeftBorder() { return pageLayout.ptLeft; }
    unsigned int getPTRightBorder() { return pageLayout.ptRight; }
    unsigned int getPTPaperHeight() { return pageLayout.ptHeight; }
    unsigned int getPTPaperWidth() { return pageLayout.ptWidth; }
    unsigned int getPTColumnWidth() { return ptColumnWidth; }
    unsigned int getPTColumnSpacing() { return pageColumns.ptColumnSpacing; }
    float getMMPaperHeight() { return pageLayout.mmHeight; }
    float getINCHPaperHeight() { return pageLayout.inchHeight; }

    unsigned int getColumns() { return pageColumns.columns; }

    void print() {}

    bool isPTYInFrame( unsigned int _frameSet, unsigned int _frame, unsigned int _ypos );

    bool printLine( KWFormatContext &_fc, QPainter &_painter, int xOffset, int yOffset, int _w, int _h,
		    bool _viewFormattingChars = false, bool _drawVarBack = true, int dbx = -1, int dby = -1,
		    int dbw = -1, int dbh = -1, const QBrush &dbback = Qt::NoBrush );
    void printBorders( QPainter &_painter, int xOffset, int yOffset, int _w, int _h );

    void drawMarker( KWFormatContext &_fc, QPainter *_painter, int xOffset, int yOffset );

    void updateAllViews( KWordView *_view, bool _clear = false );
    void updateAllViewportSizes();
    void updateAllSelections();
    void setUnitToAll();
    void updateAllCursors();
    void drawAllBorders( bool back = true);
    void recalcWholeText( bool _cursor = false, bool _fast = false );
    void recalcWholeText( KWParag *start, unsigned int fs );

    int getPages() { return pages; }

    void setPages( int _pages )
    { pages = _pages;  }
    KWFormatCollection *getFormatCollection()
    { return &formatCollection; }

    KWImageCollection *getImageCollection()
    { return &imageCollection; }

    void insertPicture( QString _filename, KWPage *_paperWidget );

    void setSelStart( KWFormatContext &_fc )
    { selStart = _fc; }

    KWFormatContext *getSelStart()
    { return &selStart;  }
    void setSelEnd( KWFormatContext &_fc )
    { selEnd = _fc; }

    KWFormatContext *getSelEnd()
    { return &selEnd; }

    void drawSelection( QPainter &_painter, int xOffset, int yOffset,
			KWFormatContext *_selStart = 0L, KWFormatContext *_selEnd = 0L );
    void setSelection( bool _has )
    { hasSelection = _has; }

    bool has_selection()
    { return hasSelection; }

    void deleteSelectedText( KWFormatContext *_fc );
    void copySelectedText();
    void setFormat( KWFormat &_format, int flags = KWFormat::All );

    void paste( KWFormatContext *_fc, QString _string, KWPage *_page,
		KWFormat *_format = 0L, const QString &_mime = "text/plain" );

    void appendPage( unsigned int _page, bool redrawBackgroundWhenAppendPage = TRUE );

    ProcessingType getProcessingType()
    { return processingType;  }

    int getFrameSet( unsigned int mx, unsigned int my );
    int selectFrame( unsigned int mx, unsigned int my, bool simulate = false );
    void deSelectFrame( unsigned int mx, unsigned int my );
    void deSelectAllFrames();
    QCursor getMouseCursor( unsigned int mx, unsigned int my );
    KWFrame *getFirstSelectedFrame();
    KWFrame *getFirstSelectedFrame( int &_frameset );
    int getFrameSetNum( KWFrameSet* fs ) { return frames.findRef( fs ); }
    KWFrameSet *getFirstSelectedFrameSet();

    void print( QPainter *painter, QPrinter *printer, float left_margin, float top_margin );

    void updateAllFrames();

    int getRastX() { return rastX; }
    int getRastY() { return rastY; }

    int getApplyStyleTemplate() { return applyStyleTemplate; }
    void setApplyStyleTemplate( int _f ) { applyStyleTemplate = _f; }

    void updateAllStyles();
    void updateAllStyleLists();

    void setStyleChanged( QString _name );
    bool isStyleChanged( QString _name );

    bool loaded() { return _loaded; }

    bool hasHeader() { return _header; }
    bool hasFooter() { return _footer; }
    void setHeader( bool h );
    void setFooter( bool f );

    void recalcFrames( bool _cursor = false, bool _fast = false );

    KoHFType getHeaderType() { return pageHeaderFooter.header; }
    KoHFType getFooterType() { return pageHeaderFooter.footer; }

    bool canResize( KWFrameSet *frameset, KWFrame *frame, int page, int diff );

    bool needRedraw() { return _needRedraw; }
    void setNeedRedraw( bool _r ) { _needRedraw = _r; }

    void addGroupManager( KWGroupManager *gm ) { grpMgrs.append( gm ); }
    unsigned int getNumGroupManagers() { return grpMgrs.count(); }
    KWGroupManager *getGroupManager( int i ) { return grpMgrs.at( i ); }
    void delGroupManager( KWGroupManager *g )
    { grpMgrs.remove( g ); }

    QPen setBorderPen( KWParagLayout::Border _brd );
    void enableEmbeddedParts( bool f );

    bool getAutoCreateNewFrame();
    RunAround getRunAround();
    KWUnit getRunAroundGap();

    void setAutoCreateNewFrame( bool _auto );
    void setRunAround( RunAround _ra );
    void setRunAroundGap( KWUnit _gap );

    void getFrameMargins( KWUnit &l, KWUnit &r, KWUnit &t, KWUnit &b );
    bool isOnlyOneFrameSelected();
    KWFrameSet *getFrameCoords( unsigned int &x, unsigned int &y, unsigned int &w, unsigned int &h, unsigned int &num );

    void setFrameMargins( KWUnit l, KWUnit r, KWUnit t, KWUnit b );
    void setFrameCoords( unsigned int x, unsigned int y, unsigned int w, unsigned int h );

    QString getUnit() { return unit; }
    void setUnit( QString _unit ) { unit = _unit; }

    int &getNumParags() { return numParags; }

    void saveParagInUndoBuffer( QList<KWParag> parags, int frameset, KWFormatContext *_fc );
    void saveParagInUndoBuffer( KWParag *parag, int frameset, KWFormatContext *_fc );

    void undo();
    void redo();

    void updateTableHeaders( QList<KWGroupManager> &grpMgrs );

    QIntDict<KWVariableFormat> &getVarFormats() { return varFormats; }

    long int getPageNum( int bottom );

    KWFootNoteManager &getFootNoteManager() { return footNoteManager; }
    void setNoteType( KWFootNoteManager::NoteType nt ) { footNoteManager.setNoteType( nt ); }
    KWFootNoteManager::NoteType getNoteType() { return footNoteManager.getNoteType(); }

    KWAutoFormat &getAutoFormat() { return autoFormat; }

    void setPageLayoutChanged( bool c ) { pglChanged = c; }

    bool getPageLayoutChanged() { return pglChanged; }

    void addImageRequest( const QString &filename, KWCharImage *img );
    void addImageRequest( const QString &filename, KWPictureFrameSet *fs );

    void registerVariable( KWVariable *var );
    void unregisterVariable( KWVariable *var );
    QList<KWVariable> *getVariables() {
	return &variables;
    }

    void setVariableValue( const QString &name, const QString &value );
    QString getVariableValue( const QString &name );

    KWSerialLetterDataBase *getSerialLetterDataBase() const;
    int getSerialLetterRecord() const;
    void setSerialLetterRecord( int r );

    bool onLineSpellCheck() const {
	return spellCheck;
    }
    void setOnLineSpellCheck( bool b ) {
	spellCheck = b;
    }

    void createContents();
    KWContents *getContents() {
	return contents;
    }

signals:
    void sig_imageModified();
    void sig_insertObject( KWordChild *_child, KWPartFrameSet* );
    void sig_updateChildGeometry( KWordChild *_child );
    void sig_removeObject( KWordChild *_child );

protected slots:
    void slotUndoRedoChanged( QString, QString );

protected:
    virtual bool saveChildren( KoStore *_store, const char *_path );

    virtual bool completeLoading( KoStore* /* _store */ );

    virtual void draw( QPaintDevice*, long int _width, long int _height,
		       float _scale );

    void loadFrameSets( KOMLParser&, vector<KOMLAttrib>& );
    void loadStyleTemplates( KOMLParser&, vector<KOMLAttrib>& );

    void addStyleTemplate( KWParagLayout *pl );

    QList<KWordView> m_lstViews;
    QList<KWordChild> m_lstChildren;

    bool m_bModified;

    KoPageLayout pageLayout;
    KoColumns pageColumns;
    KoKWHeaderFooter pageHeaderFooter;

    unsigned int ptColumnWidth;

    KWFormatCollection formatCollection;
    KWImageCollection imageCollection;
    QList<KWFrameSet> frames;
    QList<KWGroupManager> grpMgrs;

    KWUserFont *defaultUserFont;
    KWParagLayout *defaultParagLayout;

    int pages;

    KWFormatContext selStart, selEnd;
    bool hasSelection;

    ProcessingType processingType;
    int rastX, rastY;

    bool m_bEmpty;

    int applyStyleTemplate;

    QStrList changedStyles;

    bool _loaded;

    QPixmap ret_pix;

    bool _header, _footer;

    KWUserFont *cUserFont;
    KWDisplayFont *cDisplayFont;
    KWParagLayout *cParagLayout;

    bool _needRedraw;

    QString unit;
    int numParags;

    KWCommandHistory history;
    QIntDict<KWVariableFormat> varFormats;
    KWFootNoteManager footNoteManager;
    KWAutoFormat autoFormat;

    QString urlIntern;
    bool pglChanged;

    QStringList pixmapKeys, pixmapNames;
    QDict<KWCharImage> imageRequests;
    QDict<KWPictureFrameSet> imageRequests2;
    QList<KWVariable> variables;
    QMap< QString, QString > varValues;
    KWSerialLetterDataBase *slDataBase;
    int slRecordNum;

    bool spellCheck;
    KWContents *contents;

    Shell *tmpShell;
    QRect tmpShellSize;
    
};

#endif
