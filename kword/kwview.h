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

#ifndef kwview_h
#define kwview_h

#include <kprinter.h>
#include <qbrush.h>

#include "koborder.h"
#include "defs.h"
#include "kwtextparag.h"

#include <koPageLayoutDia.h>
#include <koView.h>

class DCOPObject;

class KWDocStruct;
class KoRuler;
class KWCanvas;
class KWChild;
class KWDocument;
class KWGUI;
class KWFrame;
class KWPartFrameSet;
class KoSearchContext;
class KWStyleManager;
class KWTableDia;
class KWView;
class QResizeEvent;
class KSpell;
class QScrollView;
class QSplitter;
class KAction;
class KActionMenu;
class KSelectAction;
class KToggleAction;
class KFontSizeAction;
class KFontAction;
class KWTextFrameSetEdit;
class KoTextFormatInterface;
class TKSelectColorAction;
class KoPartSelectAction;
class KoCharSelectDia;
class KWTextFrameset;
class KMacroCommand;
class KWFrameSet;
class KoFindReplace;
class KWFindReplace;
class KoTextFormat;
class KoTextParag;
class KoFontDia;
class KoParagDia;

/******************************************************************/
/* Class: KWView						  */
/******************************************************************/

class KWView : public KoView
{
    Q_OBJECT

public:
    KWView( QWidget *_parent, const char *_name, KWDocument *_doc );
    virtual ~KWView();

    virtual DCOPObject* dcopObject();

    // Those methods update the UI (from the given formatting info)
    // They do NOT do anything to the text
    void showFormat( const KoTextFormat &currentFormat );
    void showAlign( int align );
    void showCounter( KoParagCounter &c );
    void showParagBorders( const KoBorder& _left, const KoBorder& _right,
                          const KoBorder& _top, const KoBorder& _bottom );
    void showFrameBorders( const KoBorder& _left, const KoBorder& _right,
                          const KoBorder& _top, const KoBorder& _bottom );

    void showStyle( const QString & styleName );
    void showRulerIndent( double _leftMargin, double _firstLine, double _rightMargin );
    void showZoom( int zoom ); // show a zoom value in the combo
    void setZoom( int zoom, bool updateViews ); // change the zoom value

    bool viewFrameBorders() const { return m_viewFrameBorders; }
    void setViewFrameBorders(bool b);

    // Currently unused
    //bool viewTableGrid() const { return m_viewTableGrid; }
    //void setViewTableGrid(bool _b) { m_viewTableGrid = _b;}

//    virtual void setNoteType(KWFootNoteManager::NoteType nt, bool change=true);

    KWDocument *kWordDocument() { return m_doc; }
    KWGUI *getGUI() { return m_gui; }
    void updateStyleList();

    void initGui();

    int currentPage() const { return m_currentPage; }

    /**
     * Overloaded from View
     */
    bool doubleClickActivation() const;
    /**
     * Overloaded from View
     */
    QWidget* canvas();
    /**
     * Overloaded from View
     */
    int canvasXOffset() const;
    /**
     * Overloaded from View
     */
    int canvasYOffset() const;
    /**
     * Overloaded vrom View
     */
    void canvasAddChild( KoViewChild *child );

    virtual void setupPrinter( KPrinter &printer );
    virtual void print( KPrinter &printer );
    void openPopupMenuInsideFrame(KWFrame *, const QPoint &);
    void openPopupMenuEditFrame(const QPoint &);
    void openPopupMenuChangeAction( const QPoint & );

    void initConfig();

    void updatePopupMenuChangeAction();
    void changeNbOfRecentFiles(int _nb);

    void changeZoomMenu( int zoom=-1);

    void refreshMenuExpression();

    void deleteFrame(bool _warning=true);
    void clearSelection();

    QPopupMenu * popupMenu( const QString& name );

    QPtrList<KAction> &dataToolActionList() { return m_actionList; }
    QPtrList<KAction> &variableActionList() { return m_variableActionList; }

    static void checkClipboard( QMimeSource *data, bool &providesImage, bool &providesKWordText, bool &providesKWord, bool &providesFormula );

    void insertInlinePicture();

    void displayFrameInlineInfo();

    void initGUIButton();

    void updateHeaderFooterButton();
    void updateFooter();
    void updateHeader();

public slots:
    void fileStatistics();
    void editCut();
    void editCopy();
    void editPaste();
    void editSelectAll();
    void editFind();
    void editReplace();
    void editDeleteFrame();
    void editCustomVars();
    void editMailMergeDataBase();

    void viewTextMode();
    void viewPageMode();
    void viewPreviewMode();
    void slotViewFormattingChars();
    void slotViewFrameBorders();
    void viewHeader();
    void viewFooter();
    void viewFootNotes();
    void viewEndNotes();
    void viewZoom( const QString &s );

    void insertTable();
    void insertPicture();
    void insertSpecialChar();
    void insertFrameBreak();
    void insertVariable();
    void insertFootNoteEndNote();
    void insertContents();
    void insertLink();
    void insertComment();
    void removeComment();

    void formatFont();
    void formatParagraph();
    void formatPage();
    void formatFrameSet();

    void extraSpelling();
    void extraAutoFormat();
    void extraStylist();
    void extraCreateTemplate();

    void toolsCreateText();
    void insertFormula( QMimeSource* source=0 );
    void toolsPart();

    void tableInsertRow();
    void tableInsertCol();
    void tableResizeCol();
    void tableDeleteRow();
    void tableDeleteCol();
    void tableJoinCells();
    void tableSplitCells();
    void tableUngroupTable();
    void tableDelete();

    void slotStyleSelected();
    void textStyleSelected( int );
    void textSizeSelected( int );
    void increaseFontSize();
    void decreaseFontSize();
    void textFontSelected( const QString & );
    void textBold();
    void textItalic();
    void textUnderline();
    void textStrikeOut();
    void textColor();
    void textAlignLeft();
    void textAlignCenter();
    void textAlignRight();
    void textAlignBlock();
    void textSuperScript();
    void textSubScript();
    void textIncreaseIndent();
    void textDecreaseIndent();
    void textDefaultFormat();
    void slotCounterStyleSelected();

    // Text and Frame borders.
    void borderOutline();
    void borderLeft();
    void borderRight();
    void borderTop();
    void borderBottom();
    void borderColor();
    void borderWidth( const QString &width );
    void borderStyle( const QString &style );
    void backgroundColor();

    void showFormulaToolbar( bool show );

    void configure();
    void configureCompletion();

    void newPageLayout( KoPageLayout _layout );
    void newLeftIndent( double _leftIndent);
    void newFirstIndent( double _firstIndent);
    void newRightIndent( double _rightIndent);

    void clipboardDataChanged();
    void tabListChanged( const KoTabulatorList & tabList );

    void updatePageInfo();
    void updateFrameStatusBarItem();

    void slotSpecialChar(QChar , const QString &);
    void slotFrameSetEditChanged();
    void showMouseMode( int /*KWCanvas::MouseMode*/ _mouseMode );
    void frameSelectedChanged();
    void docStructChanged(int _type);
    void slotHRulerDoubleClicked();
    void slotHRulerDoubleClicked( double );

    void pageNumChanged();

    void insertExpression();

    void renameButtonTOC(bool b);

    void changeCaseOfText();

    void editPersonalExpr();

    void slotUpdateRuler();
    void slotEmbedImage( const QString &filename );

    void insertCustomVariable();
    void insertNewCustomVariable();
    void slotSpecialCharDlgClosed();

    void refreshCustomMenu();

    void changePicture();
    void changeClipart();

    void configureHeaderFooter();
    void inlineFrame();
    /** Move the selected frame above maximum 1 frame that is in front of it. */
    void raiseFrame();
    /** Move the selected frame behind maximum 1 frame that is behind it */
    void lowerFrame();
    /** Move the selected frame(s) to be in the front most position. */
    void bringToFront();
    /** Move the selected frame(s) to be behind all other frames */
    void sendToBack();
    void openLink();
    void changeLink();
    void editComment();
    void showDocStructure();

    void slotSoftHyphen();
    void slotLineBreak();
    void slotNonbreakingSpace();

    void refreshAllVariable();

    void slotAllowAutoFormat();

    void slotCompletion();

    void applyAutoFormat();
    void createStyleFromSelection();
    // end of public slots

protected slots:
    void spellCheckerReady();
    void spellCheckerMisspelling( const QString &, const QStringList &, unsigned int );
    void spellCheckerCorrected( const QString &, const QString &, unsigned int);
    void spellCheckerDone( const QString & );
    void spellCheckerFinished( );
    void spellCheckerIgnoreAll( const QString &);
    void spellCheckerReplaceAll( const QString &,  const QString &);
    void slotApplyFont();
    void slotApplyParag();
    void slotPageLayoutChanged( const KoPageLayout& layout );

protected:
    void addVariableActions( int type, const QStringList & texts,
                             KActionMenu * parentMenu, const QString & menuText );

    void loadexpressionActions( KActionMenu * parentMenu);

    void createExpressionActions( KActionMenu * parentMenu,const QString& filename );

    void insertPicture( const QString &filename, bool isClipart, bool makeInline, QSize pixmapSize, bool _keepRatio );

    void showParagraphDialog( int initialPage = -1, double initialTabPos = 0.0 );

    KWTextFrameSetEdit *currentTextEdit() const;
    /** The current text-edit if there is one, otherwise the selected text objects
     * This is what the "bold", "italic" etc. buttons apply to. */
    QPtrList<KoTextFormatInterface> applicableTextInterfaces() const;

    void setupActions();
    void createKWGUI();
    void doFindReplace();

    virtual void resizeEvent( QResizeEvent *e );
    virtual void guiActivateEvent( KParts::GUIActivateEvent *ev );

    virtual void updateReadWrite( bool readwrite );

    void borderShowValues();
    void borderSet();

    void tableSplitCells(int col, int row);

    void startKSpell();

private:
    KWDocument *m_doc;

    KAction *actionFileStatistics;
    KAction *actionEditCut;
    KAction *actionEditCopy;
    KAction *actionEditPaste;
    KAction *actionEditSelectAll;
    KAction *actionEditDelFrame;
    KAction *actionRaiseFrame;
    KAction *actionLowerFrame;
    KAction *actionSendBackward;
    KAction *actionBringToFront;

    KAction *actionEditCustomVars;
    KAction *actionEditFind;
    KAction *actionEditReplace;
    KAction *actionApplyAutoFormat;

    KToggleAction *actionViewTextMode;
    KToggleAction *actionViewPageMode;
    KToggleAction *actionViewPreviewMode;

    KToggleAction *actionViewFormattingChars;
    KToggleAction *actionViewFrameBorders;
    KToggleAction *actionViewHeader;
    KToggleAction *actionViewFooter;
    KToggleAction *actionViewFootNotes;
    KToggleAction *actionViewEndNotes;
    KToggleAction *actionShowDocStruct;
    KToggleAction *actionAllowAutoFormat;


    KSelectAction *actionViewZoom;

    KAction *actionInsertFrameBreak;
    KAction *actionInsertFootEndNote;
    KAction *actionInsertContents;
    KAction *actionInsertLink;
    KAction *actionInsertComment;
    KAction *actionEditComment;
    KAction *actionRemoveComment;

    KActionMenu *actionInsertVariable;
    struct VariableDef {
        int type;
        int subtype;
    };
    typedef QMap<KAction *, VariableDef> VariableDefMap;
    KActionMenu *actionInsertExpression;

    KActionMenu *actionInsertCustom;

    VariableDefMap m_variableDefMap;
    KAction *actionInsertFormula;
    KAction *actionInsertTable;
    KAction *actionAutoFormat;

    KToggleAction *actionToolsCreateText;
    KToggleAction *actionToolsCreatePix;
    KoPartSelectAction *actionToolsCreatePart;

    KAction *actionFormatFont;
    KAction *actionFormatDefault;
    KAction *actionFormatStylist;
    KAction *actionFormatPage;

    KAction *actionFontSizeIncrease;
    KAction *actionFontSizeDecrease;

    KFontSizeAction *actionFormatFontSize;
    KFontAction *actionFormatFontFamily;
    KSelectAction *actionFormatStyle;
    KActionMenu *actionFormatStyleMenu;
    KToggleAction *actionFormatBold;
    KToggleAction *actionFormatItalic;
    KToggleAction *actionFormatUnderline;
    KToggleAction *actionFormatStrikeOut;
    TKSelectColorAction *actionFormatColor;
    KToggleAction *actionFormatAlignLeft;
    KToggleAction *actionFormatAlignCenter;
    KToggleAction *actionFormatAlignRight;
    KToggleAction *actionFormatAlignBlock;
    KAction *actionFormatParag;
    KAction *actionFormatFrameSet;
    KAction *actionFormatIncreaseIndent;
    KAction *actionFormatDecreaseIndent;
    KActionMenu *actionFormatBullet;
    KActionMenu *actionFormatNumber;
    KToggleAction *actionFormatSuper;
    KToggleAction *actionFormatSub;
    KAction* actionInsertSpecialChar;

    // Text and Frame borders.
    KToggleAction *actionBorderLeft;
    KToggleAction *actionBorderRight;
    KToggleAction *actionBorderTop;
    KToggleAction *actionBorderBottom;
    KToggleAction *actionBorderOutline;
    TKSelectColorAction *actionBorderColor;
    KSelectAction *actionBorderWidth;
    KSelectAction *actionBorderStyle;
    TKSelectColorAction *actionBackgroundColor;
    struct
    {
        KoBorder left;    // Values specific to left border.
        KoBorder right;   // right.
        KoBorder top;     // top.
        KoBorder bottom;  // bottom.
        KoBorder common;  // Value common to left, right top and bottom borders.
    } m_border;

    KAction *actionTableDelRow;
    KAction *actionTableDelCol;
    KAction *actionTableInsertRow;
    KAction *actionTableInsertCol;
    KAction *actionTableResizeCol;
    KAction *actionTableJoinCells;
    KAction *actionTableSplitCells;

    KAction *actionTableUngroup;
    KAction *actionTableDelete;

    KAction *actionExtraSpellCheck;
    KAction *actionExtraCreateTemplate;

    KAction *actionChangeCase;

    KAction *actionEditPersonnalExpr;

    KAction *actionConfigure;

    KAction *actionConfigureCompletion;


    KAction *actionChangePicture;
    KAction *actionChangeClipart;
    KAction *actionConfigureHeaderFooter;
    KToggleAction *actionInlineFrame;

    KAction *actionOpenLink;
    KAction * actionChangeLink;

    KAction *actionRefreshAllVariable;
    KAction *actionCreateStyleFromSelection;

    KoCharSelectDia *m_specialCharDlg;
    KoFontDia *m_fontDlg;
    KoParagDia *m_paragDlg;

    KWGUI *m_gui;

    DCOPObject *dcop;

    KoSearchContext *m_searchEntry, *m_replaceEntry;
    KWFindReplace *m_findReplace;

    QPtrList<KAction> m_actionList; // for the kodatatools
    QPtrList<KAction> m_variableActionList;

    int m_currentPage; // 0-based current page number

    // Statusbar items
    QLabel * m_sbPageLabel; // 'Current page number and page count' label
    QLabel * m_sbFramesLabel; // Info about selected frames

    // Zoom values for each viewmode ( todo a viewmode enum and a qmap or so )
    int m_zoomViewModeNormal;
    int m_zoomViewModePreview;

    bool m_viewFrameBorders /*, m_viewTableGrid*/;

    // Spell-checking
    struct {
	KSpell *kspell;
	int spellCurrFrameSetNum;
	QPtrList<KWTextFrameSet> textFramesets;
	KMacroCommand * macroCmdSpellCheck;
        QStringList replaceAll;
     } m_spell;

    KWFrameSet *fsInline;
};

/******************************************************************/
/* Class: KWGUI						  */
/******************************************************************/

class KWGUI;
class KWLayoutWidget : public QWidget
{
    Q_OBJECT

public:
    KWLayoutWidget( QWidget *parent, KWGUI *g );

protected:
    void resizeEvent( QResizeEvent *e );
    KWGUI *gui;

};

class KWGUI : public QWidget
{
    friend class KWLayoutWidget;

    Q_OBJECT

public:
    KWGUI( QWidget *parent, KWView *_view );

    void showGUI();

    KWView *getView() { return view; }
    KWCanvas *canvasWidget() { return canvas; }
    KoRuler *getVertRuler() { return r_vert; }
    KoRuler *getHorzRuler() { return r_horz; }
    KoTabChooser *getTabChooser() { return tabChooser; }
    KWDocStruct *getDocStruct() { return docStruct; }

public slots:
    void reorganize();

protected slots:
    void unitChanged( QString  );

protected:
    void resizeEvent( QResizeEvent *e );

    KoRuler *r_vert, *r_horz;
    KWCanvas *canvas;
    KWView *view;
    KoTabChooser *tabChooser;
    KWDocStruct *docStruct;
    QSplitter *panner;
    KWLayoutWidget *left;

};

/******************************************************************/
/* Class: KWStatisticsDialog                                      */
/******************************************************************/
/**
* Is not intended to inherit from
*/

class KWStatisticsDialog : public KDialogBase
{
    Q_OBJECT

public:
    KWStatisticsDialog( QWidget *_parent, KWDocument *_doc );
    bool wasCanceled() { return m_canceled; }

private:
    KWDocument *m_doc;
    QWidget *m_parent;
    bool m_canceled;
    QLabel *resultLabelAll[6];
    QLabel *resultLabelSelected[6];

    void addBox( QFrame *page, QLabel **resultLabel );
    bool calcStats( QLabel **resultLabel, bool selection );
    bool docHasSelection();
    double calcFlesch(ulong sentences, ulong words, ulong syllables);

protected:

};

#endif
