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

#ifndef kwdoc_h
#define kwdoc_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

class KWDocument;
class KPrinter;
class KWTextImage;
class KWTextFrameSet;
class KWPictureFrameSet;
class KWMailMergeDataBase;
class KWFrameSet;
class KWTableFrameSet;
class KWPartFrameSet;
class KoStyle;
class KWFrameStyle;
class KWTableStyle;
class KWTableTemplate;
#define KWStyle KoStyle
class KWFrame;
class KWView;
class KWViewMode;
class KMacroCommand;
class KoDocumentEntry;
class QPainter;
class KSpellConfig;
class KSpell;
class KoAutoFormat;
class KCommand;
class KoCommandHistory;
class KoVariable;
class KoVariableFormatCollection;
class KWVariableCollection;
class KoTextObject;
class KWBgSpellCheck;
class KoStyleCollection;
class KWFrameStyleCollection;
class KWTableStyleCollection;
class KWTableTemplateCollection;
class KWFootNoteVariable;
class DCOPObject;

class QFont;
class QStringList;
class QRect;

class KOSpellConfig;

namespace KFormula {
    class Document;
    class DocumentWrapper;
}

class KoTextParag;

#include <koDocument.h>
#include <koGlobal.h>
#include <koDocumentChild.h>
#include <koPictureCollection.h>
#include <kozoomhandler.h>
#include <koUnit.h>
#include "kwanchorpos.h"
#include "defs.h"

#include <qmap.h>
#include <qptrlist.h>
#include <qfont.h>
#include <qvaluevector.h>
#include <kostyle.h>
#include <kocommandhistory.h>
class KWPartFrameSet;
/******************************************************************/
/* Class: KWChild                                              */
/******************************************************************/
class KWChild : public KoDocumentChild
{
public:
    KWChild( KWDocument *_wdoc, const QRect& _rect, KoDocument *_doc );
    KWChild( KWDocument *_wdoc );
    ~KWChild();

    KWDocument* parent()const
    { return m_pKWordDoc; }

    void setPartFrameSet( KWPartFrameSet* fs ) { m_partFrameSet = fs; }
    KWPartFrameSet * partFrameSet() const { return m_partFrameSet; }
    virtual KoDocument* hitTest( const QPoint& p, const QWMatrix& _matrix = QWMatrix() );

protected:
    KWDocument *m_pKWordDoc;
    KWPartFrameSet *m_partFrameSet;
};

struct bookMark
{
    QString bookname;
    int paragStartIndex;
    int paragEndIndex;
    QString frameSetName;
    int cursorStartIndex;
    int cursorEndIndex;
};

class KWBookMark
{
public:
    KWBookMark(const QString &_name);
    KWBookMark(const QString &_name, KoTextParag *_startParag, KoTextParag *_endParag, KWFrameSet *_frameSet, int _start, int _end);
    ~KWBookMark();
    QString bookMarkName()const { return m_name;}
    void setBookMarkName( const QString & _name ) { m_name = _name;}
    KWFrameSet * frameSet()const{ return m_frameSet;}
    void setFrameSet(KWFrameSet * _frame) { m_frameSet = _frame;}

    KoTextParag *startParag() const { return m_startParag;}
    void setStartParag( KoTextParag *_parag ) { m_startParag = _parag;}

    KoTextParag *endParag() const { return m_endParag;}
    void setEndParag( KoTextParag *_parag ) { m_endParag = _parag;}

    void setBookmarkStartIndex( int _pos ) { m_startIndex = _pos;}
    int bookmarkStartIndex() const  { return m_startIndex ; }

    void setBookmarkEndIndex( int _end ) { m_endIndex = _end;}
    int bookmarkEndIndex() const  { return m_endIndex ; }

private:
    QString m_name;
    KoTextParag *m_startParag;
    KoTextParag *m_endParag;
    KWFrameSet *m_frameSet;
    int m_startIndex;
    int m_endIndex;
};

/******************************************************************/
/* Class: KWDocument                                           */
/******************************************************************/

class KWDocument : public KoDocument, public KoZoomHandler
{
    Q_OBJECT
    Q_PROPERTY( int numPages READ numPages )
    Q_PROPERTY( double ptTopBorder READ ptTopBorder )
    Q_PROPERTY( double ptBottomBorder READ ptBottomBorder )
    Q_PROPERTY( double ptLeftBorder READ ptLeftBorder )
    Q_PROPERTY( double ptRightBorder READ ptRightBorder )
    Q_PROPERTY( double ptPaperHeight READ ptPaperHeight )
    Q_PROPERTY( double ptPaperWidth READ ptPaperWidth )
    Q_PROPERTY( double ptColumnWidth READ ptColumnWidth )
    Q_PROPERTY( double ptColumnSpacing READ ptColumnSpacing )
    Q_PROPERTY( double gridX READ gridX WRITE setGridX )
    Q_PROPERTY( double gridY READ gridY WRITE setGridY )
    Q_PROPERTY( double indentValue READ indentValue WRITE setIndentValue )
    Q_PROPERTY( int nbPagePerRow READ nbPagePerRow WRITE setNbPagePerRow )
    Q_PROPERTY( double defaultColumnSpacing READ defaultColumnSpacing WRITE setDefaultColumnSpacing )
    Q_PROPERTY( int maxRecentFiles READ maxRecentFiles )
    Q_PROPERTY( QString globalLanguage READ globalLanguage WRITE setGlobalLanguage )
    Q_PROPERTY( bool globalHyphenation READ globalHyphenation WRITE setGlobalHyphenation )
#if 0 // KWORD_HORIZONTAL_LINE
    // MOC_SKIP_BEGIN
    Q_PROPERTY( QStringList horizontalLinePath READ horizontalLinePath WRITE setHorizontalLinePath )
    // MOC_SKIP_END
#endif
    Q_PROPERTY( bool insertDirectCursor READ insertDirectCursor WRITE setInsertDirectCursor )
    Q_PROPERTY( QString picturePath READ picturePath WRITE setPicturePath )
    Q_PROPERTY( QStringList personalExpressionPath READ personalExpressionPath WRITE setPersonalExpressionPath )
    Q_PROPERTY( bool viewFormattingBreak READ viewFormattingBreak WRITE setViewFormattingBreak )
    Q_PROPERTY( bool viewFormattingTabs READ viewFormattingTabs WRITE setViewFormattingTabs )
    Q_PROPERTY( bool viewFormattingSpace READ viewFormattingSpace WRITE setViewFormattingSpace )
    Q_PROPERTY( bool viewFormattingEndParag READ viewFormattingEndParag WRITE setViewFormattingEndParag )
    Q_PROPERTY( bool cursorInProtectedArea READ cursorInProtectedArea WRITE setCursorInProtectedArea )
    Q_PROPERTY( bool pgUpDownMovesCaret READ pgUpDownMovesCaret WRITE setPgUpDownMovesCaret )
    Q_PROPERTY( bool allowAutoFormat READ allowAutoFormat WRITE setAllowAutoFormat )
    Q_PROPERTY( int undoRedoLimit READ undoRedoLimit WRITE setUndoRedoLimit )

public:
    KWDocument( QWidget *parentWidget = 0, const char *widgetName = 0, QObject* parent = 0, const char* name = 0, bool singleViewMode = false );
    ~KWDocument();

    enum ProcessingType {WP = 0, DTP = 1};
    /*
    static const int U_FONT_FAMILY_SAME_SIZE;
    static const int U_FONT_ALL_SAME_SIZE;
    static const int U_COLOR;
    static const int U_INDENT;
    static const int U_BORDER;
    static const int U_ALIGN;
    static const int U_NUMBERING;
    static const int U_FONT_FAMILY_ALL_SIZE;
    static const int U_FONT_ALL_ALL_SIZE;
    static const int U_TABS;
    static const int U_SMART;
    */

    /** when in position to select rows/cols from a table (slightly on the left/top of the table),
        * where exactly is the table? if it's NONE, we are not in position to select rows/cols */
    enum TableToSelectPosition {TABLE_POSITION_NONE = 0, TABLE_POSITION_RIGHT = 1, TABLE_POSITION_BOTTOM = 2};

    static const int DISTANCE_TABLE_SELECT_ROWCOL = 5;

    static const int CURRENT_SYNTAX_VERSION;

public:
    virtual bool initDoc();
    void initEmpty();

    virtual bool loadXML( QIODevice *, const QDomDocument & dom );
    virtual bool loadChildren( KoStore *_store );
    virtual QDomDocument saveXML();
    void processPictureRequests();
    void processAnchorRequests();
    bool processFootNoteRequests();

    int syntaxVersion( ) const { return m_syntaxVersion; }

    // Called by KWFrame*'s loading code to emit correct progress info
    void progressItemLoaded();

    /**
     * Draw as embedded.
     */
    virtual void paintContent( QPainter& painter, const QRect& rect, bool transparent = false, double zoomX = 1.0, double zoomY = 1.0 );

    virtual QPixmap generatePreview( const QSize &size );

    /**
     * @param emptyRegion The region is modified to subtract the areas painted, thus
     *                    allowing the caller to determine which areas remain to be painted.
     */
    void createEmptyRegion( const QRect & crect, QRegion & emptyRegion, KWViewMode * viewMode );
    /**
     * Erase the empty space defined by @p emptySpaceRegion.
     * Usually used to clear the space where there is no frame (e.g. page margins).
     */
    void eraseEmptySpace( QPainter * painter, const QRegion & emptySpaceRegion, const QBrush & brush );

    virtual void setEmpty();

    virtual void addView( KoView *_view );
    virtual void removeView( KoView *_view );

    virtual void addShell( KoMainWindow *shell );

    virtual void insertObject( const KoRect& _rect, KoDocumentEntry& _e );
    void setPageLayout( const KoPageLayout& _layout, const KoColumns& _cl, const KoKWHeaderFooter& _hf, bool updateViews = true );

    void getPageLayout( KoPageLayout& _layout, KoColumns& _cl, KoKWHeaderFooter& _hf );

    KWTextFrameSet * textFrameSet ( unsigned int _num ) const;
    // Return the frameset number @p _num
    KWFrameSet *frameSet( unsigned int _num )
    { return m_lstFrameSet.at( _num ); }

    // Return the frameset with a given name
    KWFrameSet * frameSetByName( const QString & name );

    /** Returns the frame under the mouse. (or 0 if no frame is found)
     * @param border is set to true if the mouse is on a border of the frame found.
     * @param firstNonSelected when set to true, frameUnderMouse searches for the frame on
     *        top that is not selected.
     */
    KWFrame * frameUnderMouse( const QPoint& nPoint, bool* border = 0L, bool firstNonSelected = false );

    // Return the total number of framesets
    unsigned int getNumFrameSets()
    { return m_lstFrameSet.count(); }

    // Generate a new name for a frameset. @p templateName must contain a %1 [for a number].
    QString generateFramesetName( const QString & templateName );

    // Prefer this over frameSet(i), if iterating over all of them
    QPtrListIterator<KWFrameSet> framesetsIterator() const { return QPtrListIterator<KWFrameSet>(m_lstFrameSet); }

    QValueList<KoTextObject *> visibleTextObjects(KWViewMode *viewmode) const;

    void addFrameSet( KWFrameSet *f, bool finalize = true );
    // Remove frameset from list (don't delete)
    void removeFrameSet( KWFrameSet *f );

    // Frame/table deletion - with undo/redo support
    // Moved to KWDocument so that dialogs can call them if necessary
    void deleteTable( KWTableFrameSet *groupManager );
    void deleteFrame( KWFrame * frame );

    void deleteSelectedFrames();

    // Those distances are in _pixels_, i.e. with zoom and resolution applied.
    unsigned int topBorder() const { return static_cast<unsigned int>(zoomItY( m_pageLayout.ptTop )); }
    unsigned int bottomBorder() const { return static_cast<unsigned int>(zoomItY( m_pageLayout.ptBottom )); }
    unsigned int leftBorder() const { return static_cast<unsigned int>(zoomItX( m_pageLayout.ptLeft )); }
    unsigned int rightBorder() const { return static_cast<unsigned int>(zoomItX( m_pageLayout.ptRight )); }
    // WARNING: don't multiply this value by the number of the page, this leads to rounding problems.
    unsigned int paperHeight() const { return static_cast<unsigned int>(zoomItY( m_pageLayout.ptHeight )); }
    unsigned int paperWidth() const { return static_cast<unsigned int>(zoomItX( m_pageLayout.ptWidth )); }
    unsigned int columnSpacing() const { return static_cast<unsigned int>(zoomItX( m_pageColumns.ptColumnSpacing )); }
    // Top of the page number pgNum, in pixels (in the normal coord system)
    unsigned int pageTop( int pgNum /*0-based*/ ) const { return zoomItY( ptPageTop( pgNum ) ); }

    // Those distances are in _pt_, i.e. the real distances, stored in m_pageLayout
    double ptTopBorder() const { return m_pageLayout.ptTop; }
    double ptBottomBorder() const { return m_pageLayout.ptBottom; }
    double ptLeftBorder() const { return m_pageLayout.ptLeft; }
    double ptRightBorder() const { return m_pageLayout.ptRight; }
    double ptPaperHeight() const { return m_pageLayout.ptHeight; }
    double ptPaperWidth() const { return m_pageLayout.ptWidth; }
    double ptColumnWidth() const;
    double ptColumnSpacing() const { return m_pageColumns.ptColumnSpacing; }
    double ptPageTop( int pgNum /*0-based*/ ) const { return pgNum * m_pageLayout.ptHeight; }
    double ptFootnoteBodySpacing() const { return m_pageHeaderFooter.ptFootNoteBodySpacing; }

    unsigned int numColumns() const { return m_pageColumns.columns; }

    /** Returns 0-based page number where rect is.
     * @param _rect should be a rect in real pt coordinates.
     * Use isOutOfPage to check that the rectangle is fully contained in that page.
     */
    int getPageOfRect( KoRect & _rect ) const;

    /** Return true if @p r (in real pt coordinates) is out of the page @p page */
    bool isOutOfPage( KoRect & r, int page ) const;

    void repaintAllViews( bool erase = false );
    /** Update all views of this document, area can be cleared before redrawing with the
     * _erase flag. (false implied). All views EXCEPT the argument _view are updated ( give 0L for all )
     */
    void repaintAllViewsExcept( KWView *_view, bool _erase = false );


    /**
     * schedule a repaint of all views but don't execute immediately
     */
    void delayedRepaintAllViews();

    /**
     * schedule a frame layout (e.g. for footnotes) but don't execute immediately
     */
    void delayedRecalcFrames( int fromPage );

    /**
     * Return a double-buffer pixmap of (at least) the given size.
     */
    QPixmap* doubleBufferPixmap( const QSize& );
    /**
     * Call this when you're done with the double-buffer pixmap (at the
     * end of the current painting, for all objects that need to be painted).
     * If it's too big, KWDocument will delete it to save memory.
     */
    void maybeDeleteDoubleBufferPixmap();

    /**
     * Tell this method when a frame is moved / resized / created / deleted
     * and everything will be update / repainted accordingly.
     * If one view is already uptodate, pass it in @p view.
     */
    void frameChanged( KWFrame * frame, KWView * view = 0L );
    void framesChanged( const QPtrList<KWFrame> & frames, KWView * view = 0L );

    QString uniqueFramesetName( const QString& oldName );
    /*
     * @param copyFootNote ...
     * @param dontCreateFootNote = true when we copy footnote into an other frameset than mainFrameSet => footnote is removed !
     * @param selectFrames if true, pasted frames are auto-selected. Set to false when loading from a file etc.
     */
    void pasteFrames( QDomElement topElem, KMacroCommand * macroCmd, bool copyFootNote = false, bool dontCreateFootNote = false, bool selectFrames = true );

    void insertEmbedded( KoStore *store, QDomElement topElem, KMacroCommand * macroCmd, double offset );
    void completePasting();

    KoStyleCollection * styleCollection()const  { return m_styleColl;}
    KWFrameStyleCollection * frameStyleCollection()const  { return m_frameStyleColl;}
    KWTableStyleCollection * tableStyleCollection()const  { return m_tableStyleColl;}
    KWTableTemplateCollection * tableTemplateCollection()const  { return m_tableTemplateColl;}

    QFont defaultFont() const { return m_defaultFont; }
    void setDefaultFont( const QFont & newFont ) {
        m_defaultFont = newFont;
    }

    int numPages() const { return m_pages; }

    KoPictureCollection *pictureCollection() { return &m_pictureCollection; }
    KoVariableFormatCollection *variableFormatCollection()const { return m_varFormatCollection; }

    QPtrList <KWView> getAllViews() { return m_lstViews; }

    /**
     * Insert a new page after another,
     * creating followup frames (but not headers/footers),
     * @param afterPageNum the page is inserted after the one specified here
     * If afterPageNum is -1, a page is inserted before page 0.
     * In all cases, the new page will have the number afterPageNum+1.
     * Use appendPage in WP mode, insertPage in DTP mode.
     */
    void insertPage( int afterPageNum );
    /**
     * Append a new page, creating followup frames (but not headers/footers),
     * and return the page number.
     */
    int appendPage();
    /**
     * Call this after appendPage, to get headers/footers on the new page,
     * and all the caches properly updated. This is separate from appendPage
     * so that KWFrameLayout can call appendPage() only.
     */
    void afterAppendPage( int num );
    /**
     * @return list of frames that will be copied onto the new page
     * Used by insertPage but also by KWTextFrameSet to check if it's worth
     * auto-inserting a new page (to avoid infinite loops if not)
     */
    QPtrList<KWFrame> framesToCopyOnNewPage( int afterPageNum ) const;
    /**
     * Remove a page. Call afterRemovePages() after removing one or more pages.
     */
    void removePage( int num );
    /**
     * Update things after removing one or more pages.
     */
    void afterRemovePages();

    /**
     * Check if we can remove empty page(s) from the end
     * If so, do it and return true.
     * Note that this doesn't call afterRemovePages, this is up to the caller.
     */
    bool tryRemovingPages();

    ProcessingType processingType()const { return m_processingType;  }

    MouseMeaning getMouseMeaning( const QPoint &nPoint, int keyState, KWFrame** pFrame = 0L );
    // The cursor for the current 'mouse click meaning'
    QCursor getMouseCursor( const QPoint& nPoint, int keyState );
    QPtrList<KWFrame> getSelectedFrames() const;
    KWFrame *getFirstSelectedFrame() const;
    int frameSetNum( KWFrameSet* fs ) { return m_lstFrameSet.findRef( fs ); }

    void lowerMainFrames( int pageNum );
    void lowerMainFrames( int pageNum, int lowestZOrder );

    // Those three method consider _all_ text framesets, even table cells
    QPtrList<KWTextFrameSet> allTextFramesets( bool onlyReadWrite ) const;
    int numberOfTextFrameSet( KWFrameSet* fs, bool onlyReadWrite );
    KWFrameSet * textFrameSetFromIndex( unsigned int _num, bool onlyReadWrite );


    /** Gather all the frames which are on a certain page and return them.
     * The list is ordered. @see KWFrameSet::framesInPage
     * @param pageNum the number of the page
     * @param sorted if true the list is ordered. should be true always.
     */
    QPtrList<KWFrame> framesInPage( int pageNum , bool sorted=true) const;


    /**
     * Max z-order among all frames on the given page
     */
    int maxZOrder( int pageNum ) const;
    // No minZOrder() method, because of the main frameset, see kwview::lowerFrame

    void updateAllFrames( int flags = 0xff /* see KWFrameSet::UpdateFramesFlags */ );
    void updateFramesOnTopOrBelow( int pageNum = -1 /* all */ );

    // The grid is in _pt_ now
    double gridX()const { return m_gridX; }
    double gridY()const { return m_gridY; }
    void setGridX(double _gridx);
    void setGridY(double _gridy) { m_gridY = _gridy; }

    // Currently unused. Not sure we want to go that way, now that we have
    // paragLayoutChanged and formatChanged in applyStyleChange.
    //int applyStyleChangeMask() { return styleMask; }
    //void setApplyStyleChangeMask( int _f ) { styleMask = _f; }

    // paragLayoutChanged is a set of flags for the parag layout - see the enum in KWParagLayout
    // formatChanged is a set of flags from KoTextFormat
    // If both are -1, it means the style has been deleted.
    void applyStyleChange( StyleChangeDefMap changed );
    void updateAllStyleLists();
    void updateStyleListOrder( const QStringList &list );

    void updateAllFrameStyleLists();
    void updateAllTableStyleLists();

    bool isHeaderVisible() const { return m_headerVisible; }
    bool isFooterVisible() const { return m_footerVisible; }
    void setHeaderVisible( bool h );
    void setFooterVisible( bool f );
    bool hasEndNotes() const;

    // flags: see KWFrameLayout
    void recalcFrames( int fromPage = 0, int toPage = -1, uint flags = 0 );

    KoHFType getHeaderType() const { return m_pageHeaderFooter.header; }
    KoHFType getFooterType() const { return m_pageHeaderFooter.footer; }
    const KoKWHeaderFooter& headerFooterInfo() const { return m_pageHeaderFooter; }

    bool isOnlyOneFrameSelected();
    void setFrameMargins( double l, double r, double t, double b );
    void setFrameCoords( double x, double y, double w, double h );

    // The user-chosen global unit
    QString getUnitName()const { return KoUnit::unitName( m_unit ); }
    KoUnit::Unit getUnit()const { return m_unit; }
    void setUnit( KoUnit::Unit _unit );

    void addCommand( KCommand * cmd );

    KoCommandHistory * commandHistory() const { return m_commandHistory; }
    KoAutoFormat * getAutoFormat() const { return m_autoFormat; }

    // This is used upon loading, to delay certain things until completeLoading.
    // For KWTextImage
    void addTextImageRequest( KWTextImage *img );
    // For KWPictureFrameSet
    void addPictureRequest( KWPictureFrameSet *fs );
    // For KWTextParag
    void addAnchorRequest( const QString &framesetName, const KWAnchorPosition &anchorPos );
    // For KWFootNoteVariable
    void addFootNoteRequest( const QString &framesetName, KWFootNoteVariable* var );

    // This is used by loadFrameSets() and by KWCanvas to paste framesets
    KWFrameSet *loadFrameSet( QDomElement framesetElem, bool loadFrames = true , bool loadFootnote = true);
    void loadEmbeddedObjects( QDomElement& word );
    void saveEmbeddedObjects( QDomElement& parentElem, const QPtrList<KoDocumentChild>& childList );
    void loadEmbedded( const QDomElement &embedded );

    void recalcVariables( int type );

    KWVariableCollection *getVariableCollection()const {return m_varColl;}

    KWMailMergeDataBase *getMailMergeDataBase() const { return m_slDataBase; }
    int getMailMergeRecord() const;
    void setMailMergeRecord( int r );

    bool backgroundSpellCheckEnabled() const;
    void enableBackgroundSpellCheck( bool b );

    bool canRemovePage( int num );

    /**
     * Change the zoom factor to @p z (e.g. 150 for 150%)
     * and/or change the resolution, given in DPI.
     * This is done on startup and when printing.
     * The same call combines both so that all the updating done behind
     * the scenes is done only once, even if both zoom and DPI must be changed.
     */
    virtual void setZoomAndResolution( int zoom, int dpiX, int dpiY );

    void newZoomAndResolution( bool updateViews, bool forPrint );

    /**
     * Due to the way the text formatter works (it caches layout information in
     * the paragraphs and characters), we currently can't have one viewmode per view.
     * It has to be the same for all views.
     */
    KWViewMode *viewMode() const { return m_viewMode; }

    /**
     * Changes m_viewMode, and updates all views to this viewmode
     */
    void switchViewMode( KWViewMode * newViewMode );


    // useless method
    static QString getAttribute(QDomElement &element, const char *attributeName, const QString &defaultValue)
      {
          return element.attribute( attributeName, defaultValue );
      }

    static int getAttribute(QDomElement &element, const char *attributeName, int defaultValue)
      {
	QString value;
	if ( ( value = element.attribute( attributeName ) ) != QString::null )
	  return value.toInt();
	else
	  return defaultValue;
      }

    static double getAttribute(QDomElement &element, const char *attributeName, double defaultValue)
      {
	QString value;
	if ( ( value = element.attribute( attributeName ) ) != QString::null )
	  return value.toDouble();
	else
	  return defaultValue;
      }

    /**
     * get custom kspell config
     */

    void setKOSpellConfig(const KOSpellConfig& _kspell);
    KOSpellConfig * getKOSpellConfig()const {return m_pKOSpellConfig;}

#ifndef NDEBUG
    void printStyleDebug();
    void printDebug();
#endif

    /** calls layout() on all framesets  */
    void layout();

    /** call by undo/redo frame border => update all button border frame **/
    void refreshFrameBorderButton();

    // This settings has to be here [instead of KWView] because we need to
    // format paragraphs slightly differently (to add room for the CR char)
    bool viewFormattingChars() const { return m_viewFormattingChars; }
    void setViewFormattingChars(bool _b) { m_viewFormattingChars=_b; }

    bool viewFormattingEndParag() const { return m_viewFormattingEndParag; }
    void setViewFormattingEndParag(bool _b) { m_viewFormattingEndParag=_b; }

    bool viewFormattingSpace() const { return m_viewFormattingSpace; }
    void setViewFormattingSpace(bool _b) { m_viewFormattingSpace=_b; }

    bool viewFormattingTabs() const { return m_viewFormattingTabs; }
    void setViewFormattingTabs(bool _b) { m_viewFormattingTabs=_b; }

    bool viewFormattingBreak() const { return m_viewFormattingBreak; }
    void setViewFormattingBreak(bool _b) { m_viewFormattingBreak=_b; }

    // Also view properties, but stored, loaded and saved here (lacking a more global object).
    bool viewFrameBorders() const { return m_viewFrameBorders; }
    void setViewFrameBorders( bool b ) { m_viewFrameBorders = b; }
    void setShowRuler(bool _ruler){ m_bShowRuler=_ruler; }
    bool showRuler() const { return m_bShowRuler; }

    bool showStatusBar() const { return m_bShowStatusBar;}
    void setShowStatusBar( bool _status ) { m_bShowStatusBar = _status;}

    bool showScrollBar() const { return m_bShowScrollBar; }
    void setShowScrollBar( bool _status ) { m_bShowScrollBar = _status;}

    bool pgUpDownMovesCaret() const { return m_pgUpDownMovesCaret; }
    void setPgUpDownMovesCaret( bool b ) { m_pgUpDownMovesCaret = b; }

    bool showdocStruct() const {return  m_bShowDocStruct;}
    void setShowDocStruct(bool _b){m_bShowDocStruct=_b;}

    bool allowAutoFormat() const { return m_bAllowAutoFormat; }
    void setAllowAutoFormat(bool _b){ m_bAllowAutoFormat=_b; }

    bool insertDirectCursor() const { return m_bInsertDirectCursor; }
    void setInsertDirectCursor(bool _b);


    // in pt
    double indentValue()const { return m_indent; }
    void setIndentValue(double _ind) { m_indent=_ind; }

    int nbPagePerRow() const{ return m_iNbPagePerRow; }
    void setNbPagePerRow(int _nb) { m_iNbPagePerRow=_nb; }

    int maxRecentFiles() const { return m_maxRecentFiles; }


    //void setLastViewMode(const QString &_mode){ m_lastViewMode = _mode;}
    //QString lastViewMode() const { return m_lastViewMode; }

    // in pt
    double defaultColumnSpacing()const{ return m_defaultColumnSpacing ;}
    void setDefaultColumnSpacing(double _val){ m_defaultColumnSpacing=_val; }
    /**
     * @returns the document for the formulas
     */
    KFormula::Document* getFormulaDocument();

    void reorganizeGUI();
    //necessary to update resize handle when you change layout
    // make zoom, add header, add footer etc...
    void updateResizeHandles();
    //necessary to force repaint resizehandle otherwise
    //when we change protect size attribute handle was not repainting
    void repaintResizeHandles();

    //necessary when we undo/Redo change protect content.
    void updateCursorType( );

    // Tell all views to stop editing this frameset, if they were doing so
    void terminateEditing( KWFrameSet * frameSet )
    { emit sig_terminateEditing( frameSet ); }

    void clearUndoRedoInfos();

    void refreshDocStructure(FrameSetType);
    void refreshDocStructure(int);

    int typeItemDocStructure(FrameSetType _type);

    void renameButtonTOC(bool b);

    void refreshMenuExpression();

    void refreshMenuCustomVariable();

    void frameSelectedChanged();

    void updateZoomRuler();

    // ## rename!
    void hasTOC(bool _b){ m_hasTOC=_b;}
    bool isTOC(){return m_hasTOC;}

    QString sectionTitle( int pageNum ) const;

    void updateRulerFrameStartEnd();
    void updateFrameStatusBarItem();

    /** Convert a color into a color to be displayed for it
     * (when using color schemes, we still want to print black on white).
     * See also KoTextFormat::defaultTextColor. */
    static QColor resolveTextColor( const QColor & col, QPainter * painter );
    static QColor defaultTextColor( QPainter * painter );
    static QColor resolveBgColor( const QColor & col, QPainter * painter );
    static QColor defaultBgColor( QPainter * painter );


    virtual DCOPObject* dcopObject();

    int undoRedoLimit() const;
    void setUndoRedoLimit(int _val);

    void updateContentsSize(){emit newContentsSize();}

    void refreshGUIButton();

    void initConfig();
    void saveConfig();
    void initUnit();
    void startBackgroundSpellCheck();
    void reactivateBgSpellChecking();
    //to be removed
    KWTextFrameSet* nextTextFrameSet(KWTextFrameSet *obj);

    void updateHeaderButton();
    void updateFooterButton();

    QStringList spellListIgnoreAll() const { return m_spellListIgnoreAll;}
    void addIgnoreWordAllList( const QStringList & _lst)
        { m_spellListIgnoreAll = _lst;}

    void addIgnoreWordAll( const QString & );
    void clearIgnoreWordAll( );
    void updateTextFrameSetEdit();
    void changeFootNoteConfig();
    void displayFootNoteFieldCode();


    double tabStopValue() const { return m_tabStop; }
    void setTabStopValue ( double _tabStop );

    TableToSelectPosition positionToSelectRowcolTable(const QPoint& nPoint, KWTableFrameSet **ppTable =0L);

    void changeBgSpellCheckingState( bool b );

    // To position the cursor when opening a document
    QString initialFrameSet() const; // can be empty for "unset"
    int initialCursorParag() const;
    int initialCursorIndex() const;
    // Once we're done with this info, get rid of it
    void deleteInitialEditingInfo();

    bool cursorInProtectedArea()const;
    void setCursorInProtectedArea( bool b );

    SeparatorLinePos footNoteSeparatorLinePosition()const { return m_footNoteSeparatorLinePos;}
    void setFootNoteSeparatorLinePosition(SeparatorLinePos _pos) {m_footNoteSeparatorLinePos = _pos;}

    int footNoteSeparatorLineLength() const { return m_iFootNoteSeparatorLineLength;}
    void setFootNoteSeparatorLineLength( int _length){  m_iFootNoteSeparatorLineLength = _length;}

    double footNoteSeparatorLineWidth() const { return m_footNoteSeparatorLineWidth;}
    void setFootNoteSeparatorLineWidth( double _width){  m_footNoteSeparatorLineWidth=_width;}

    SeparatorLineLineType footNoteSeparatorLineType()const { return m_footNoteSeparatorLineType;}
    void setFootNoteSeparatorLineType( SeparatorLineLineType _type) {m_footNoteSeparatorLineType = _type;}


    void insertBookMark(const QString &_name, KWTextParag *_startparag, KWTextParag *_endparag, KWFrameSet *_frameSet, int _start, int _end);
    void deleteBookMark(const QString &_name);
    void renameBookMark(const QString &_oldname, const QString &_newName);

    KWBookMark * bookMarkByName( const QString & name );
    QStringList listOfBookmarkName(KWViewMode * viewMode)const;

    void paragraphDeleted( KoTextParag *_parag, KWFrameSet *frm);
    void spellCheckParagraphDeleted( KoTextParag *_parag,  KWTextFrameSet *frm);
    void paragraphModified(KoTextParag* _parag, int /*KoTextParag::ParagModifyType*/ _type, int start, int lenght);

    void initBookmarkList();
    void loadImagesFromStore( KoStore *_store );
    void loadPictureMap ( QDomElement& domElement );

    void configureSpellChecker();
    void testAndCloseAllFrameSetProtectedContent();
    void updateRulerInProtectContentMode();

    KoPageLayout pageLayout() const;

    QPtrListIterator<KWBookMark> bookmarkIterator() const { return QPtrListIterator<KWBookMark>(m_bookmarkList); }

    QStringList personalExpressionPath() const { return m_personalExpressionPath;}
    void setPersonalExpressionPath( const QStringList & );

#if 0 // KWORD_HORIZONTAL_LINE
    // MOC_SKIP_BEGIN
    QStringList horizontalLinePath() const { return m_horizontalLinePath;}
    void setHorizontalLinePath( const QStringList & );
    // MOC_SKIP_END
#endif

    QString picturePath()const { return m_picturePath; }
    void setPicturePath( const QString & _path ) { m_picturePath = _path ; }


    void updateDirectCursorButton();

    QString globalLanguage()const { return m_globalLanguage; }
    void setGlobalLanguage( const QString & _lang ){m_globalLanguage = _lang;}
    void addWordToDictionary( const QString & );

    bool globalHyphenation() const { return m_bGlobalHyphenation; }
    void setGlobalHyphenation ( bool _hyphen ) { m_bGlobalHyphenation = _hyphen; }

signals:
    void sig_insertObject( KWChild *_child, KWPartFrameSet* );

    // This is emitted by setPageLayout if updateViews=true
    void pageLayoutChanged( const KoPageLayout& );

    // Emitted when the scrollview contents must be resized (e.g. new page, new layout...)
    void newContentsSize();

    // This is emitted when the height of the text in the main frameset changes
    // Mostly useful for the text viewmode.
    void mainTextHeightChanged();

    // This is emitted when the number of pages changes.
    void pageNumChanged();

    void docStructureChanged(int);
    void sig_terminateEditing( KWFrameSet * fs );

    void sig_refreshMenuCustomVariable();

    void sig_frameSelectedChanged();

public slots:
    void slotRepaintChanged( KWFrameSet * frameset );
    void slotRepaintVariable();

    /** calls invalidate() on all framesets  */
    void invalidate(const KWFrameSet *skipThisFrameSet=0);

protected slots:
    void slotRecalcFrames();
    void slotRepaintAllViews();
    void slotDocumentRestored();
    void slotCommandExecuted();
    void slotDocumentInfoModifed();
    void slotChapterParagraphFormatted( KoTextParag* parag );

protected:
    void nextParagraphNeedingCheck();
    // fix up Z-order for import from older kword versions.
    void fixZOrders();
    QString checkSectionTitleInParag( KoTextParag* parag, KWTextFrameSet*, int pageNum ) const;
    KoView* createViewInstance( QWidget* parent, const char* name );

    virtual bool completeLoading( KoStore* _store );
    virtual bool completeSaving( KoStore *_store );

    void loadFrameSets( const QDomElement &framesets );
    void loadStyleTemplates( const QDomElement &styles );
    void saveStyle( KWStyle *sty, QDomElement parentElem );
    void saveFrameStyle( KWFrameStyle *sty, QDomElement parentElem );
    void saveTableStyle( KWTableStyle *sty, QDomElement parentElem );

    void loadFrameStyleTemplates( const QDomElement &styles );
    void loadDefaultFrameStyleTemplates();
    void loadTableStyleTemplates( const QDomElement &styles );
    void loadDefaultTableStyleTemplates();
    void loadDefaultTableTemplates();

private:
    //private helper functions for frameUnderMouse
    /** return the top-most frame under mouse, using nPoint, always returns the first found. */
    KWFrame *topFrameUnderMouse( const QPoint& nPoint, bool* border=0L);
    /** Search the list of frames for a frame that is directly below the argument frame,
      * and return that.*/
    KWFrame *frameBelowFrame(const QPoint& nPoint, KWFrame *frame, bool *border=0L);
    /** Search if the frame (parent) has any embedded frames we might have clicked on and
        return that frame if so.  The algoritm will recursively search for the deepest
        inline frame we clicked on.
    */
    KWFrame *deepestInlineFrame(KWFrame *parent, const QPoint& nPoint, bool *border);


    // Variables:
    QPtrList<KWView> m_lstViews;
    QPtrList<KWChild> m_lstChildren;

    KoColumns m_pageColumns;
    KoKWHeaderFooter m_pageHeaderFooter;

    KoPictureCollection m_pictureCollection;

    QPtrList<KWFrameSet> m_lstFrameSet;

    int m_pages;
    unsigned int m_itemsLoaded;
    unsigned int m_nrItemsToLoad;

    ProcessingType m_processingType;
    double m_gridX, m_gridY;

    KoUnit::Unit m_unit;

    DCOPObject *dcop;

    KoCommandHistory * m_commandHistory;
    KoAutoFormat * m_autoFormat;

    // The viewmode used by all views.
    KWViewMode *m_viewMode;

    // Shared between loadXML and loadComplete
    QString m_urlIntern;

    QMap<KoPictureKey, QString> m_pictureMap;

    /// List used to help loading and saving images of the old type ("text image" of class KWTextImage)
    QPtrList<KWTextImage> m_textImageRequests;
    QPtrList<KWPictureFrameSet> m_pictureRequests;
    QMap<QString, KWAnchorPosition> m_anchorRequests;
    QMap<QString, KWFootNoteVariable *> m_footnoteVarRequests;

    QMap<QString,QString> * m_pasteFramesetsMap;

    KoVariableFormatCollection *m_varFormatCollection;
    KWMailMergeDataBase *m_slDataBase;
    int slRecordNum;

    /*
     * When a document is written out, the syntax version in use will be recorded. When read back
     * in, this variable reflects that value.
     */
    int m_syntaxVersion;

    QFont m_defaultFont;
    bool m_headerVisible, m_footerVisible;
    bool m_viewFrameBorders;
    bool m_bShowRuler;
    bool m_bShowDocStruct;
    bool m_hasTOC;
    bool m_bShowStatusBar;
    bool m_pgUpDownMovesCaret;
    bool m_repaintAllViewsPending;
    bool m_bAllowAutoFormat;
    bool m_bShowScrollBar;
    bool m_cursorInProtectectedArea;
    bool m_bInsertDirectCursor;
    bool m_bHasEndNotes;

    bool m_viewFormattingChars;
    bool m_viewFormattingEndParag;
    bool m_viewFormattingSpace;
    bool m_viewFormattingTabs;
    bool m_viewFormattingBreak;

    /// The wrapper that contains the formula's document and its
    /// actions. It owns the real document.
    KFormula::DocumentWrapper* m_formulaDocumentWrapper;

    double m_indent; // in pt
    double m_defaultColumnSpacing;

    int m_iNbPagePerRow;
    int m_maxRecentFiles;
    int m_recalcFramesPending;

    QString m_lastViewMode;
    KWVariableCollection *m_varColl;
    KWBgSpellCheck *m_bgSpellCheck;
    KOSpellConfig *m_pKOSpellConfig;
    KoStyleCollection *m_styleColl;
    KWFrameStyleCollection *m_frameStyleColl;
    KWTableStyleCollection *m_tableStyleColl;
    KWTableTemplateCollection *m_tableTemplateColl;


    SeparatorLinePos m_footNoteSeparatorLinePos;
    //it's a percentage of page.
    int m_iFootNoteSeparatorLineLength;

    double m_footNoteSeparatorLineWidth;

    SeparatorLineLineType m_footNoteSeparatorLineType;

    /** Page number -> section title array, for the Section variable.
     * Note that pages without a section title don't appear in the array. */
    QValueVector< QString > m_sectionTitles;

    double m_tabStop;
    QStringList m_spellListIgnoreAll;
    KWFrameSet * bgFrameSpellChecked;
    QPixmap* m_bufPixmap;

    class InitialEditing;
    InitialEditing *m_initialEditing;

    QPtrList<KWBookMark>m_bookmarkList;
    //necessary before when we load bookmark.
    QPtrList<bookMark>m_tmpBookMarkList;

    QStringList m_personalExpressionPath;
    QString m_picturePath;
    QString m_globalLanguage;
#if 0 // KWORD_HORIZONTAL_LINE
    // MOC_SKIP_BEGIN
    QStringList m_horizontalLinePath;
    // MOC_SKIP_END
#endif
    bool m_bGlobalHyphenation;
};


#endif
