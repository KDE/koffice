/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2001 David Faure <faure@kde.org>

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
   Boston, MA 02111-1307, USA
*/

#include "kwcanvas.h"
#undef Unsorted
#include <kaccel.h>
#include <kmessagebox.h>
#include <qclipboard.h>
#include <qregexp.h>
#include <qpaintdevicemetrics.h>
#include <qprogressdialog.h>
#include <qlabel.h>
#include <qgroupbox.h>

#include <koAutoFormatDia.h>
#include "defs.h"
#include "deldia.h"
#include "docstruct.h"

#include <koFontDia.h>

#include "footnotedia.h"
#include "insdia.h"
#include <koCharSelectDia.h>
#include <koChangeCaseDia.h>
#include "kwcommand.h"
#include "kwconfig.h"
#include "kwdoc.h"
#include "kwdrag.h"
#include "kweditpersonnalexpressiondia.h"
#include "kwinsertpicdia.h"
#include "kwtableframeset.h"
#include "kwview.h"
#include "kwviewmode.h"
#include <koParagDia.h>
#include <koSearchDia.h>
#include "searchdia.h"
#include "mailmerge.h"
#include "splitcellsdia.h"
#include "stylist.h"
#include "tabledia.h"
#include <koVariable.h>
#include <koVariableDlgs.h>
#include "koInsertLink.h"
#include <koAutoFormat.h>

#include <koMainWindow.h>
#include <koTemplateCreateDia.h>
#include <koPartSelectAction.h>
#include <koFrame.h>
#include <kotextobject.h>

#include <kformulamimesource.h>

#include <ktempfile.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kstdaction.h>
#include <klocale.h>
#undef Bool
#include <kspell.h>
#include <kspelldlg.h>
#include <tkcoloractions.h>
#include <kstandarddirs.h>
#include <kparts/event.h>
#include <kformuladocument.h>

#include <stdlib.h>

#include <KWordViewIface.h>
#include <kstatusbar.h>

KWView::KWView( QWidget *_parent, const char *_name, KWDocument* _doc )
    : KoView( _doc, _parent, _name )
{
    m_doc = _doc;
    m_gui = 0;

    dcop = 0;
    dcopObject(); // build it

    fsInline=0L;
    m_spell.kspell = 0;
    m_border.left.color = white;
    m_border.left.style = KoBorder::SOLID;
    m_border.left.ptWidth = 0;
    m_border.right = m_border.left;
    m_border.top = m_border.left;
    m_border.bottom = m_border.left;
    m_border.common.color = black;
    m_border.common.style = KoBorder::SOLID;
    m_border.common.ptWidth = 1;
    m_currentPage = 0;
    m_specialCharDlg=0L;
    m_searchEntry = 0L;
    m_replaceEntry = 0L;
    m_findReplace = 0L;
    m_actionList.setAutoDelete( true );
    // Default values.
    m_zoomViewModeNormal = m_doc->zoom();
    m_zoomViewModePreview = 33;
    m_viewFrameBorders = m_doc->viewFrameBorders();
    KoView::setZoom( m_doc->zoomedResolutionY() /* KoView only supports one zoom */ ); // initial value
    //m_viewTableGrid = true;

    setInstance( KWFactory::global() );
    setXMLFile( "kword.rc" );

    QObject::connect( this, SIGNAL( embeddImage( const QString & ) ),
                      this, SLOT( slotEmbedImage( const QString & ) ) );

    setKeyCompression( TRUE );
    setAcceptDrops( TRUE );
    createKWGUI();

    connect( m_doc, SIGNAL( pageNumChanged() ),
             this, SLOT( pageNumChanged()) );

    connect( m_doc, SIGNAL( docStructureChanged(int) ),
             this, SLOT( docStructChanged(int)) );

    connect( m_doc, SIGNAL( sig_refreshMenuCustomVariable()),
             this, SLOT( refreshCustomMenu()));

    connect( m_doc, SIGNAL(sig_frameSelectedChanged()),
             this, SLOT( frameSelectedChanged()));

    connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
             this, SLOT( clipboardDataChanged() ) );

    connect( m_gui->canvasWidget(), SIGNAL(currentFrameSetEditChanged()),
             this, SLOT(slotFrameSetEditChanged()) );

    connect( m_gui->canvasWidget(), SIGNAL( currentMouseModeChanged(int) ),
             this, SLOT( setTool(int) ) );

    // Cut and copy are directly connected to the selectionChanged signal
    if ( m_doc->isReadWrite() )
    {
        connect( m_gui->canvasWidget(), SIGNAL(selectionChanged(bool)),
                 actionEditCut, SLOT(setEnabled(bool)) );
    }
    else
    {
        actionEditCut->setEnabled( false );
    }

    connect( m_gui->canvasWidget(), SIGNAL(selectionChanged(bool)),
             actionEditCopy, SLOT(setEnabled(bool)) );

    connect (m_gui->canvasWidget(), SIGNAL(selectionChanged(bool)),
             actionChangeCase, SLOT(setEnabled(bool)));

    connect( m_gui->canvasWidget(), SIGNAL(frameSelectedChanged()),
             this, SLOT(frameSelectedChanged()));

    connect( m_gui->canvasWidget(), SIGNAL(docStructChanged(int)),
             this, SLOT(docStructChanged(int)));

    connect( m_gui->canvasWidget(), SIGNAL(updateRuler()),
             this, SLOT(slotUpdateRuler()));

    if ( shell() )
    {
        connect( shell(), SIGNAL( documentSaved()), m_doc,SLOT(slotDocumentInfoModifed() ) );
        changeNbOfRecentFiles( m_doc->maxRecentFiles() );
    }

    m_gui->canvasWidget()->updateCurrentFormat();
    setFocusProxy( m_gui->canvasWidget() );

    KStatusBar * sb = statusBar();
    m_sbPageLabel = 0L;
    if ( sb ) // No statusbar in e.g. konqueror
    {
        m_sbPageLabel = new KStatusBarLabel( QString::null, 0, sb );
        addStatusBarItem( m_sbPageLabel, 0 );
    }
    m_sbFramesLabel = 0L; // Only added when frames are selected

    QString mode = m_doc->lastViewMode();
    m_gui->canvasWidget()->switchViewMode( KWViewMode::create( mode, m_doc ) );

    //when kword is embedded into konqueror apply a zoom=100
    //in konqueror we can't change zoom -- ### TODO ?
    if(!m_doc->isReadWrite())
    {
        setZoom( 100, true );
        slotUpdateRuler();
    }
}

KWView::~KWView()
{
    clearSelection();
    if ( m_findReplace )
    {
        // Abort any find/replace
        m_findReplace->abort();
    }
    // Delete gui while we still exist ( it needs documentDeleted() )
    delete m_gui;
    delete m_sbPageLabel;
    delete fsInline;
    delete dcop;
}

DCOPObject* KWView::dcopObject()
{
    if ( !dcop )
	dcop = new KWordViewIface( this );

    return dcop;
}


void KWView::clearSelection()
{
    if(m_spell.kspell)
    {
        KWTextFrameSet * fs = 0L;
        if(m_spell.spellCurrFrameSetNum!=-1)
        {
            fs=m_spell.textFramesets.at( m_spell.spellCurrFrameSetNum ) ;
            Q_ASSERT( fs );
            if ( fs )
                fs->removeHighlight();
        }
        delete m_spell.kspell;
    }
    delete m_searchEntry;
    delete m_replaceEntry;
    delete m_specialCharDlg;
}


void KWView::changeNbOfRecentFiles(int _nb)
{
    if ( shell() ) // 0 when embedded into konq !
        shell()->setMaxRecentItems( _nb );
}

void KWView::initGui()
{
    clipboardDataChanged();
    if ( m_gui )
        m_gui->showGUI();
    setTool( KWCanvas::MM_EDIT );
    initGUIButton();
    actionFormatDecreaseIndent->setEnabled(false);
    //setNoteType(m_doc->getNoteType(), false);

    actionFormatColor->setCurrentColor( Qt::black );

    //refresh zoom combobox
    changeZoomMenu( m_doc->zoom() );
    showZoom( m_doc->zoom() );

    // This is probably to emit currentMouseModeChanged and set the cursor
    m_gui->canvasWidget()->setMouseMode( m_gui->canvasWidget()->mouseMode() );

    //showFormulaToolbar( FALSE ); // not called, to avoid creating the formula-document if not necessary
    if(shell())
      shell()->showToolbar( "formula_toolbar", false );

    // Prevention against applyMainWindowSettings hiding the statusbar
    KStatusBar * sb = statusBar();
    if ( sb )
        sb->show();

    updatePageInfo();
    slotFrameSetEditChanged();
    frameSelectedChanged();
    renameButtonTOC(m_doc->isTOC());
    //at the beginning actionBackgroundColor should be active
    actionBackgroundColor->setEnabled(true);

}


void KWView::initGUIButton()
{
    actionViewFrameBorders->setChecked( viewFrameBorders() );
    actionViewFormattingChars->setChecked( m_doc->viewFormattingChars() );
    actionShowDocStruct->setChecked(m_doc->showdocStruct());

    actionViewHeader->setChecked(m_doc->isHeaderVisible());
    actionViewFooter->setChecked(m_doc->isFooterVisible());

    QString mode=m_gui->canvasWidget()->viewMode()->type();
    if(mode=="ModePreview")
        actionViewPreviewMode->setChecked(true);
    else if(mode=="ModeText")
        actionViewTextMode->setChecked(true);
    else if(mode=="ModeNormal")
        actionViewPageMode->setChecked(true);
    else
        actionViewPageMode->setChecked(true);
}

void KWView::setupActions()
{
    // The actions here are grouped by menu, because this helps noticing
    // accelerator clashes.

    // -------------- File menu
    actionExtraCreateTemplate = new KAction( i18n( "&Create Template from Document..." ), 0,
                                             this, SLOT( extraCreateTemplate() ),
                                             actionCollection(), "extra_template" );
    actionExtraCreateTemplate->setToolTip( i18n( "Save this document and use it later as a template" ) );
    actionExtraCreateTemplate->setWhatsThis( i18n( "You can save this document as a template.<br><br>You can use this new template as a starting point for another document." ) );

    actionFileStatistics = new KAction( i18n( "Statistics" ), 0, this, SLOT( fileStatistics() ), actionCollection(), "file_statistics" );
    actionFileStatistics->setToolTip( i18n( "Sentence, word and letter counts for this document" ) );
    actionFileStatistics->setWhatsThis( i18n( "Information on the number of letters, words, syllables and sentences for this document.<p>Evaluates readability using the Flesch reading score." ) );
    // -------------- Edit actions
    actionEditCut = KStdAction::cut( this, SLOT( editCut() ), actionCollection(), "edit_cut" );
    actionEditCopy = KStdAction::copy( this, SLOT( editCopy() ), actionCollection(), "edit_copy" );
    actionEditPaste = KStdAction::paste( this, SLOT( editPaste() ), actionCollection(), "edit_paste" );
    actionEditFind = KStdAction::find( this, SLOT( editFind() ), actionCollection(), "edit_find" );
    actionEditReplace = KStdAction::replace( this, SLOT( editReplace() ), actionCollection(), "edit_replace" );
    actionEditSelectAll = KStdAction::selectAll( this, SLOT( editSelectAll() ), actionCollection(), "edit_selectall" );
    actionExtraSpellCheck = KStdAction::spelling( this, SLOT( extraSpelling() ), actionCollection(), "extra_spellcheck" );


    (void) new KAction( i18n( "Configure Mai&l Merge ..." ), 0,
                        this, SLOT( editMailMergeDataBase() ),
                        actionCollection(), "edit_sldatabase" );

    // -------------- Frame menu
    actionEditDelFrame = new KAction( i18n( "&Delete Frame" ), 0,
                                      this, SLOT( editDeleteFrame() ),
                                      actionCollection(), "edit_delframe" );
    actionEditDelFrame->setToolTip( i18n( "Delete the currently selected frame." ) ); // #### there can be more than one frame selected (DF)
    actionEditDelFrame->setWhatsThis( i18n( "Delete the currently selected frame." ) );

    actionRaiseFrame = new KAction( i18n( "Ra&ise frame" ), "raise",
                                    CTRL +SHIFT+ Key_R, this, SLOT( raiseFrame() ),
                                    actionCollection(), "raiseframe" );
    actionRaiseFrame->setToolTip( i18n( "Raise the currently selected frame so that it appears above all the other frames" ) );
    actionRaiseFrame->setWhatsThis( i18n( "Raise the currently selected frame so that it appears above all the other frames. This is only useful if frames overlap each other. If multiple frames are selected they are all raised in turn." ) );

    actionLowerFrame = new KAction( i18n( "&Lower frame" ), "lower",
                                    CTRL +SHIFT+ Key_L, this, SLOT( lowerFrame() ),
                                    actionCollection(), "lowerframe" );
    actionLowerFrame->setToolTip( i18n( "Lower the currently selected frame so that it disappears under any frame that overlaps it" ) );
    actionLowerFrame->setWhatsThis( i18n( "Lower the currently selected frame so that it disappears under any frame that overlaps it. If multiple frames are selected they are all lowered in turn." ) );

    /*
    actionToolsEdit = new KToggleAction( i18n( "Edit &Text" ), "edittool", Key_F4,
                                         this, SLOT( toolsEdit() ),
                                         actionCollection(), "tools_edit" );
    actionToolsEdit->setExclusiveGroup( "tools" );
    actionToolsEditFrames = new KToggleAction( i18n( "Edit &Frames" ),"frame_edit", Key_F5,
                                               this, SLOT( toolsEditFrame() ),
                                               actionCollection(), "tools_editframes" );
    actionToolsEditFrames->setExclusiveGroup( "tools" );
    */

    // -------------- View menu
    actionViewTextMode = new KToggleAction( i18n( "Text mode" ), 0,
                                            this, SLOT( viewTextMode() ),
                                            actionCollection(), "view_textmode" );
    actionViewTextMode->setToolTip( i18n( "Text mode." ) );
    actionViewTextMode->setWhatsThis( i18n( "Text mode." ) );

    actionViewTextMode->setExclusiveGroup( "viewmodes" );
    actionViewPageMode = new KToggleAction( i18n( "&Page mode" ), 0,
                                            this, SLOT( viewPageMode() ),
                                            actionCollection(), "view_pagemode" );
    actionViewPageMode->setWhatsThis( i18n( "Switch from preview mode to page mode.<br><br>Page mode is designed to make editing your text easy.<br><br>This function is most frequently used to return to text editing after switching to Preview mode." ) );
    actionViewPageMode->setToolTip( i18n( "Switch from preview mode to page editing mode." ) );

    actionViewPageMode->setExclusiveGroup( "viewmodes" );
    actionViewPageMode->setChecked( true );
    actionViewPreviewMode = new KToggleAction( i18n( "Pre&view mode" ), 0,
                                            this, SLOT( viewPreviewMode() ),
                                            actionCollection(), "view_previewmode" );
    actionViewPreviewMode->setWhatsThis( i18n( "Zoom out from your document to get a look at several pages of your document.<br><br>The number of pages per line can be customized." ) );
    actionViewPreviewMode->setToolTip( i18n( "Zoom out to a multiple page view." ) );

    actionViewPreviewMode->setExclusiveGroup( "viewmodes" );

    actionViewFormattingChars = new KToggleAction( i18n( "&Formatting Characters" ), 0,
                                                   this, SLOT( slotViewFormattingChars() ),
                                                   actionCollection(), "view_formattingchars" );
    actionViewFormattingChars->setToolTip( i18n( "Toggle the display of non-printing characters." ) );
    actionViewFormattingChars->setWhatsThis( i18n( "Toggle the display of non-printing characters.<br><br>When this is enabled, KWord shows you tabs, spaces, carriage returns and other non-printing characters." ) );

    actionViewFrameBorders = new KToggleAction( i18n( "Frame &Borders" ), 0,
                                                   this, SLOT( slotViewFrameBorders() ),
                                                   actionCollection(), "view_frameborders" );
    actionViewFrameBorders->setToolTip( i18n( "Turns the border display on and off." ) );
    actionViewFrameBorders->setWhatsThis( i18n( "Turns the border display on and off.<br><br>The borders are never printed. This option is useful to see how the document will appear on the printed page." ) );

    actionViewHeader = new KToggleAction( i18n( "&Header" ), 0,
                                          this, SLOT( viewHeader() ),
                                          actionCollection(), "view_header" );
    actionViewHeader->setToolTip( i18n( "Shows and hides header display." ) );
    actionViewHeader->setWhatsThis( i18n( "Selecting this option toggles the display of headers in KWord.<br><br>Headers are special frames at the top of each page which can contain page numbers or other information." ) );

    actionViewFooter = new KToggleAction( i18n( "Foo&ter" ), 0,
                                          this, SLOT( viewFooter() ),
                                          actionCollection(), "view_footer" );
    actionViewFooter->setToolTip( i18n( "Shows and hides footer display." ) );
    actionViewFooter->setWhatsThis( i18n( "Selecting this option toggles the display of footers in KWord. <br><br>Footers are special frames at the bottom of each page which can contain page numbers or other information." ) );

    actionViewFootNotes = new KToggleAction( i18n( "Foot&notes" ), 0,
                                             this, SLOT( viewFootNotes() ),
                                          actionCollection(), "view_footnotes" );

    actionViewFootNotes->setEnabled( false ); // #### TODO
    actionViewFootNotes->setExclusiveGroup( "notes" );
    actionViewEndNotes = new KToggleAction( i18n( "&Endnotes" ), 0,
                                             this, SLOT( viewEndNotes() ),
                                          actionCollection(), "view_endnotes" );

    actionViewEndNotes->setExclusiveGroup( "notes" );
    actionViewEndNotes->setEnabled( false ); // #### TODO

    actionViewZoom = new KSelectAction( i18n( "Zoom" ), "viewmag", 0,
                                        actionCollection(), "view_zoom" );

    connect( actionViewZoom, SIGNAL( activated( const QString & ) ),
             this, SLOT( viewZoom( const QString & ) ) );
    actionViewZoom->setEditable(true);
    actionViewZoom->setComboWidth( 50 );
    changeZoomMenu( );

    // -------------- Insert menu
    actionInsertSpecialChar = new KAction( i18n( "Sp&ecial Character..." ), "char",
                        ALT + SHIFT + Key_C,
                        this, SLOT( insertSpecialChar() ),
                        actionCollection(), "insert_specialchar" );
    actionInsertSpecialChar->setToolTip( i18n( "Insert one or more symbols or letters not found on the keyboard." ) );
    actionInsertSpecialChar->setWhatsThis( i18n( "Insert one or more symbols or letters not found on the keyboard." ) );

    actionInsertFrameBreak = new KAction( i18n( "&Hard Frame Break" ), CTRL + Key_Return,
                                          this, SLOT( insertFrameBreak() ),
                                          actionCollection(), "insert_framebreak" );
    actionInsertFrameBreak->setToolTip( i18n( "Force the remainder of the text into the next frame." ) );
    actionInsertFrameBreak->setWhatsThis( i18n( "This inserts a non-printing character at the current cursor position. All text after this point will be moved into the next frame in the frameset." ) );

     actionInsertLink = new KAction( i18n( "Insert link" ), 0,
                                        this, SLOT( insertLink() ),
                                        actionCollection(), "insert_link" );
    actionInsertLink->setToolTip( i18n( "Insert a web address, email address or hyperlink to a file." ) );
    actionInsertLink->setWhatsThis( i18n( "Insert a web address, email address or hyperlink to a file." ) );

    // TODO
    /*actionInsertFootEndNote = new KAction( i18n( "&Footnote or Endnote..." ), 0,
                                           this, SLOT( insertFootNoteEndNote() ),
                                           actionCollection(), "insert_footendnote" );*/

    actionInsertContents = new KAction( i18n( "Table of &Contents" ), 0,
                                        this, SLOT( insertContents() ),
                                        actionCollection(), "insert_contents" );
    actionInsertContents->setToolTip( i18n( "Insert table of contents at the current cursor position." ) );
    actionInsertContents->setWhatsThis( i18n( "Insert table of contents at the current cursor position." ) );

    m_variableDefMap.clear();
    actionInsertVariable = new KActionMenu( i18n( "&Variable" ),
                                            actionCollection(), "insert_variable" );

    // The last argument is only needed if a submenu is to be created
    addVariableActions( VT_FIELD, KoFieldVariable::actionTexts(), actionInsertVariable, i18n("&Property") );
    addVariableActions( VT_DATE, KoDateVariable::actionTexts(), actionInsertVariable, i18n("&Date") );
    addVariableActions( VT_TIME, KoTimeVariable::actionTexts(), actionInsertVariable, i18n("&Time") );

    actionInsertCustom = new KActionMenu( i18n( "&Custom" ),
                                            actionCollection(), "insert_custom" );
     actionInsertVariable->insert(actionInsertCustom);

    addVariableActions( VT_PGNUM, KoPgNumVariable::actionTexts(), actionInsertVariable, QString::null );
    /*
    addVariableActions( VT_CUSTOM, KWCustomVariable::actionTexts(), actionInsertVariable, QString::null );
    */


    // TODO at the moment serial letters don't work correctly
    addVariableActions( VT_MAILMERGE, KoMailMergeVariable::actionTexts(), actionInsertVariable, QString::null );

    actionInsertVariable->popupMenu()->insertSeparator();
    actionRefreshAllVariable = new KAction( i18n( "&Refresh all variables" ), 0,
                                    this, SLOT( refreshAllVariable() ),
                                    actionCollection(), "refresh_all_variable" );
    actionRefreshAllVariable->setToolTip( i18n( "Update all variables to current values." ) );
    actionRefreshAllVariable->setWhatsThis( i18n( "Update all variables in the document to current values.<br><br>This will update page numbers, dates or any other variables that need updating." ) );

    actionInsertVariable->insert(actionRefreshAllVariable);

    actionInsertExpression = new KActionMenu( i18n( "&Expression" ),
                                            actionCollection(), "insert_expression" );
    loadexpressionActions( actionInsertExpression);

    actionToolsCreateText = new KToggleAction( i18n( "Te&xt Frame" ), "frame_text", Key_F2,
                                               this, SLOT( toolsCreateText() ),
                                               actionCollection(), "tools_createtext" );
    actionToolsCreateText->setToolTip( i18n( "Create a new text frame." ) );
    actionToolsCreateText->setWhatsThis( i18n( "Create a new text frame." ) );

    actionToolsCreateText->setExclusiveGroup( "tools" );
    actionToolsCreatePix = new KToggleAction( i18n( "P&icture..." ), "frame_image", // or inline_image ?
                                              Key_F3 /*same as kpr*/,
                                              this, SLOT( insertPicture() ),
                                              actionCollection(), "insert_picture" );
    actionToolsCreatePix->setToolTip( i18n( "Create a new frame for a picture." ) );
    actionToolsCreatePix->setWhatsThis( i18n( "Create a new frame for a picture." ) );

    actionToolsCreatePix->setExclusiveGroup( "tools" );
    actionInsertFormula = new KAction( i18n( "For&mula" ), "frame_formula", Key_F4,
                                       this, SLOT( insertFormula() ),
                                       actionCollection(), "tools_formula" );
    actionInsertFormula->setToolTip( i18n( "Insert a formula into a new frame." ) );
    actionInsertFormula->setWhatsThis( i18n( "Insert a formula into a new frame." ) );

    actionInsertTable = new KAction( i18n( "&Table..." ), "inline_table",
                        Key_F5,
                        this, SLOT( insertTable() ),
                        actionCollection(), "insert_table" );
    actionInsertTable->setToolTip( i18n( "Create a table." ) );
    actionInsertTable->setWhatsThis( i18n( "Create a table.<br><br>The table can either exist in a frame of its own or inline." ) );

    actionToolsCreatePart = new KoPartSelectAction( i18n( "&Object Frame" ), "frame_query",
                                                    /*CTRL+Key_F2 same as kpr,*/
                                                    this, SLOT( toolsPart() ),
                                                    actionCollection(), "tools_part" );
    actionToolsCreatePart->setToolTip( i18n( "Insert an object into a new frame." ) );
    actionToolsCreatePart->setWhatsThis( i18n( "Insert an object into a new frame." ) );

    //actionToolsCreatePart->setExclusiveGroup( "tools" );

    // ------------------------- Format menu
    actionFormatFont = new KAction( i18n( "&Font..." ), ALT + CTRL + Key_F,
                                    this, SLOT( formatFont() ),
                                    actionCollection(), "format_font" );
    actionFormatFont->setToolTip( i18n( "Change character size, font, boldface, italics, etc." ) );
    actionFormatFont->setWhatsThis( i18n( "Change the attributes of the currently selected characters." ) );

    actionFormatParag = new KAction( i18n( "&Paragraph..." ), ALT + CTRL + Key_P,
                                     this, SLOT( formatParagraph() ),
                                     actionCollection(), "format_paragraph" );
    actionFormatParag->setToolTip( i18n( "Change paragraph margins, text flow, borders, bullets, numbering, etc." ) );
    actionFormatParag->setWhatsThis( i18n( "Change paragraph margins, text flow, borders, bullets, numbering, etc.<p>Select text in multiple paragraphs to change the formating of all selected paragraphs.<p>If no text is selected, the paragraph with the cursor will be changed." ) );

    actionFormatFrameSet = new KAction( i18n( "F&rame/Frameset..." ), 0,
                                     this, SLOT( formatFrameSet() ),
                                     actionCollection(), "format_frameset" );
    actionFormatFrameSet->setToolTip( i18n( "Alter frameset properties." ) );
    actionFormatFrameSet->setWhatsThis( i18n( "Alter frameset properties.<p>Currently you can change the frame background." ) );

    actionFormatPage = new KAction( i18n( "P&age..." ), 0,
                        this, SLOT( formatPage() ),
                        actionCollection(), "format_page" );
    actionFormatPage->setToolTip( i18n( "Change properties of entire page." ) );
    actionFormatPage->setWhatsThis( i18n( "Change properties of the entire page.<p>Currently you can change paper size, paper orientation, header and footer sizes, and column settings." ) );

    actionFormatStylist = new KAction( i18n( "&Stylist..." ), ALT + CTRL + Key_S,
                        this, SLOT( extraStylist() ),
                        actionCollection(), "format_stylist" );
    actionFormatStylist->setToolTip( i18n( "Change attributes of styles." ) );
    actionFormatStylist->setWhatsThis( i18n( "Change font and paragraph attributes of styles.<p>Multiple styles can be changed using the dialog box." ) );

    actionFormatFontSize = new KFontSizeAction( i18n( "Font Size" ), 0,
                                              actionCollection(), "format_fontsize" );
    connect( actionFormatFontSize, SIGNAL( fontSizeChanged( int ) ),
             this, SLOT( textSizeSelected( int ) ) );
    actionFormatFontSize->setComboWidth( 30 );

    actionFontSizeIncrease = new KAction( i18n("Increase Font Size") , CTRL + Key_Greater, this, SLOT( increaseFontSize() ), actionCollection(), "increase_fontsize" );
    actionFontSizeDecrease = new KAction( i18n("Decrease Font Size"), CTRL + Key_Less, this, SLOT( decreaseFontSize() ), actionCollection(), "decrease_fontsize" );

    actionFormatFontFamily = new KFontAction( i18n( "Font Family" ), 0,
                                              actionCollection(), "format_fontfamily" );
    connect( actionFormatFontFamily, SIGNAL( activated( const QString & ) ),
             this, SLOT( textFontSelected( const QString & ) ) );

    actionFormatStyle = new KSelectAction( i18n( "St&yle" ), 0,
                                           actionCollection(), "format_style" );
    connect( actionFormatStyle, SIGNAL( activated( int ) ),
             this, SLOT( textStyleSelected( int ) ) );
    updateStyleList();

    actionFormatDefault=new KAction( i18n( "Default Format" ), 0,
                                          this, SLOT( textDefaultFormat() ),
                                          actionCollection(), "text_default" );
    actionFormatDefault->setToolTip( i18n( "Change font and paragraph attributes to their default values." ) );
    actionFormatDefault->setWhatsThis( i18n( "Change font and paragraph attributes to their default values." ) );

    actionFormatBold = new KToggleAction( i18n( "&Bold" ), "text_bold", CTRL + Key_B,
                                           this, SLOT( textBold() ),
                                           actionCollection(), "format_bold" );
    actionFormatItalic = new KToggleAction( i18n( "&Italic" ), "text_italic", CTRL + Key_I,
                                           this, SLOT( textItalic() ),
                                           actionCollection(), "format_italic" );
    actionFormatUnderline = new KToggleAction( i18n( "&Underline" ), "text_under", CTRL + Key_U,
                                           this, SLOT( textUnderline() ),
                                           actionCollection(), "format_underline" );
    actionFormatStrikeOut = new KToggleAction( i18n( "&Strike out" ), "text_strike", 0 ,
                                           this, SLOT( textStrikeOut() ),
                                           actionCollection(), "format_strike" );

    actionFormatAlignLeft = new KToggleAction( i18n( "Align &Left" ), "text_left", CTRL + Key_L,
                                       this, SLOT( textAlignLeft() ),
                                       actionCollection(), "format_alignleft" );
    actionFormatAlignLeft->setExclusiveGroup( "align" );
    actionFormatAlignLeft->setChecked( TRUE );
    actionFormatAlignCenter = new KToggleAction( i18n( "Align &Center" ), "text_center", CTRL + ALT + Key_C,
                                         this, SLOT( textAlignCenter() ),
                                         actionCollection(), "format_aligncenter" );
    actionFormatAlignCenter->setExclusiveGroup( "align" );
    actionFormatAlignRight = new KToggleAction( i18n( "Align &Right" ), "text_right", CTRL + ALT + Key_R,
                                        this, SLOT( textAlignRight() ),
                                        actionCollection(), "format_alignright" );
    actionFormatAlignRight->setExclusiveGroup( "align" );
    actionFormatAlignBlock = new KToggleAction( i18n( "Align &Block" ), "text_block", CTRL + Key_J,
                                        this, SLOT( textAlignBlock() ),
                                        actionCollection(), "format_alignblock" );
    actionFormatAlignBlock->setExclusiveGroup( "align" );
    actionFormatList = new KToggleAction( i18n( "List" ), "enumList", 0,
                                              this, SLOT( textList() ),
                                              actionCollection(), "format_list" );
    actionFormatList->setExclusiveGroup( "style" );
    actionFormatSuper = new KToggleAction( i18n( "Superscript" ), "super", 0,
                                              this, SLOT( textSuperScript() ),
                                              actionCollection(), "format_super" );
    actionFormatSuper->setExclusiveGroup( "valign" );
    actionFormatSub = new KToggleAction( i18n( "Subscript" ), "sub", 0,
                                              this, SLOT( textSubScript() ),
                                              actionCollection(), "format_sub" );
    actionFormatSub->setExclusiveGroup( "valign" );

    actionFormatIncreaseIndent= new KAction( i18n( "Increase Indent" ), "format_increaseindent", 0,
                                      this, SLOT( textIncreaseIndent() ),
                                      actionCollection(), "format_increaseindent" );

    actionFormatDecreaseIndent= new KAction( i18n( "Decrease Indent" ),"format_decreaseindent", 0,
                                      this, SLOT( textDecreaseIndent() ),
                                      actionCollection(), "format_decreaseindent" );

    actionFormatColor = new TKSelectColorAction( i18n( "Text Color..." ), TKSelectColorAction::TextColor,
                                     this, SLOT( textColor() ),
                                     actionCollection(), "format_color",true );
    actionFormatColor->setDefaultColor(QColor());

    // ---------------------------- frame toolbar actions

    actionBorderOutline = new KToggleAction( i18n( "Border Outline" ), "borderoutline",
                            0, this, SLOT( borderOutline() ), actionCollection(), "border_outline" );
    actionBorderLeft = new KToggleAction( i18n( "Border Left" ), "borderleft",
                            0, this, SLOT( borderLeft() ), actionCollection(), "border_left" );
    actionBorderRight = new KToggleAction( i18n( "Border Right" ), "borderright",
                            0, this, SLOT( borderRight() ), actionCollection(), "border_right" );
    actionBorderTop = new KToggleAction( i18n( "Border Top" ), "bordertop",
                            0, this, SLOT( borderTop() ), actionCollection(), "border_top" );
    actionBorderBottom = new KToggleAction( i18n( "Border Bottom" ), "borderbottom",
                            0, this, SLOT( borderBottom() ),  actionCollection(), "border_bottom" );
    actionBorderStyle = new KSelectAction( i18n( "Border Style" ),
                            0,  actionCollection(), "border_style" );
    connect( actionBorderStyle, SIGNAL( activated( const QString & ) ),
             this, SLOT( borderStyle( const QString & ) ) );
    actionBorderStyle->setComboWidth( 30 );

    QStringList lst;
    lst << KoBorder::getStyle( KoBorder::SOLID );
    lst << KoBorder::getStyle( KoBorder::DASH );
    lst << KoBorder::getStyle( KoBorder::DOT );
    lst << KoBorder::getStyle( KoBorder::DASH_DOT );
    lst << KoBorder::getStyle( KoBorder::DASH_DOT_DOT );
    actionBorderStyle->setItems( lst );
    actionBorderWidth = new KSelectAction( i18n( "Border Width" ), 0,
                                                 actionCollection(), "border_width" );
    connect( actionBorderWidth, SIGNAL( activated( const QString & ) ),
             this, SLOT( borderWidth( const QString & ) ) );
    lst.clear();
    for ( unsigned int i = 1; i < 10; i++ )
        lst << QString::number( i );
    actionBorderWidth->setItems( lst );
    actionBorderWidth->setComboWidth( 30 );

    actionBorderColor = new TKSelectColorAction( i18n("Border Color"), TKSelectColorAction::LineColor, actionCollection(), "border_color",true );
    actionBorderColor->setDefaultColor(QColor());
    connect(actionBorderColor,SIGNAL(activated()),SLOT(borderColor()));


    actionBackgroundColor = new TKSelectColorAction( i18n( "Text Background Color..." ), TKSelectColorAction::FillColor, actionCollection(),"border_backgroundcolor",true);
    actionBackgroundColor->setToolTip( i18n( "Change background color for currently selected text." ) );
    actionBackgroundColor->setWhatsThis( i18n( "Change background color for currently selected text." ) );

    connect(actionBackgroundColor,SIGNAL(activated()),SLOT(backgroundColor() ));
    actionBackgroundColor->setDefaultColor(QColor());

    // ---------------------- Table menu

    actionTableInsertRow = new KAction( i18n( "&Insert Row..." ), "insert_table_row", 0,
                               this, SLOT( tableInsertRow() ),
                               actionCollection(), "table_insrow" );
    actionTableInsertRow->setToolTip( i18n( "Insert one or more rows at cursor location." ) );
    actionTableInsertRow->setWhatsThis( i18n( "Insert one or more rows at current cursor location." ) );

    actionTableInsertCol = new KAction( i18n( "I&nsert Column..." ), "insert_table_col", 0,
                               this, SLOT( tableInsertCol() ),
                               actionCollection(), "table_inscol" );
    actionTableInsertCol->setToolTip( i18n( "Insert one or more columns into the current table." ) );
    actionTableInsertCol->setWhatsThis( i18n( "Insert one or more columns into the current table." ) );

    actionTableDelRow = new KAction( i18n( "&Delete Row..." ), "delete_table_row", 0,
                                     this, SLOT( tableDeleteRow() ),
                                     actionCollection(), "table_delrow" );
    actionTableDelRow->setToolTip( i18n( "Delete one row from the current table." ) );
    actionTableDelRow->setWhatsThis( i18n( "Delete one row from the current table.<p>Your cursor does not need to be in the row to be deleted. You will be given the opportunity to specify which row to delete." ) );

    actionTableDelCol = new KAction( i18n( "D&elete Column..." ), "delete_table_col", 0,
                                     this, SLOT( tableDeleteCol() ),
                                     actionCollection(), "table_delcol" );
    actionTableDelCol->setToolTip( i18n( "Delete one column from the current table." ) );
    actionTableDelCol->setWhatsThis( i18n( "Delete one column from the current table.<p>Your cursor does not need to be in the column to be deleted. You will be given the opportunity to specify which row to delete.." ) );

    actionTableJoinCells = new KAction( i18n( "&Join Cells" ), 0,
                                        this, SLOT( tableJoinCells() ),
                                        actionCollection(), "table_joincells" );
    actionTableJoinCells->setToolTip( i18n( "Join two or more cells into one large cell." ) );
    actionTableJoinCells->setWhatsThis( i18n( "Join two or more cells into one large cell.<p>This is a good way to create titles and labels within a table." ) );

    actionTableSplitCells= new KAction( i18n( "&Split Cells..." ), 0,
                                         this, SLOT( tableSplitCells() ),
                                         actionCollection(), "table_splitcells" );
    actionTableSplitCells->setToolTip( i18n( "Split one cell into two or more cells." ) );
    actionTableSplitCells->setWhatsThis( i18n( "Split one cell into two or more cells.<p>Cells can be split horizontally, vertically or both directions at once. ." ) );

    actionTableUngroup = new KAction( i18n( "&Ungroup Table" ), 0,
                                      this, SLOT( tableUngroupTable() ),
                                      actionCollection(), "table_ungroup" );
    actionTableUngroup->setToolTip( i18n( "Break a table into individual frames." ) );
    actionTableUngroup->setWhatsThis( i18n( "Break a table into individual frames<p>Each frame can be moved independently around the page." ) );

    actionTableDelete = new KAction( i18n( "Delete &Table" ), 0,
                                     this, SLOT( tableDelete() ),
                                     actionCollection(), "table_delete" );
    actionTableDelete->setToolTip( i18n( "Delete the entire table." ) );
    actionTableDelete->setWhatsThis( i18n( "Deletes all cells and the content within the cells of the currently selected table." ) );


    // ---------------------- Tools menu


    actionAutoFormat = new KAction( i18n( "&Autocorrection..." ), 0,
                        this, SLOT( extraAutoFormat() ),
                        actionCollection(), "extra_autocorrection" );
    actionAutoFormat->setToolTip( i18n( "Change autocorrection options." ) );
    actionAutoFormat->setWhatsThis( i18n( "Change autocorrection options including:<p> <UL><LI><P>exceptions to autocorrection</P> <LI><P>add/remove autocorrection replacement text</P> <LI><P>and basic autocorrection options</P>." ) );

    actionEditPersonnalExpr=new KAction( i18n( "Edit personal expressions..." ), 0,
                                         this, SLOT( editPersonalExpr() ),
                                     actionCollection(), "personal_expr" );
    actionEditPersonnalExpr->setToolTip( i18n( "Add or change one or more personal expressions." ) );
    actionEditPersonnalExpr->setWhatsThis( i18n( "Add or change one or more personal expressions.<p>Personal expressions are a way to quickly insert commonly used phrases or text into your document." ) );

    actionChangeCase=new KAction( i18n( "Change case..." ), 0,
                                     this, SLOT( changeCaseOfText() ),
                                     actionCollection(), "change_case" );
    actionChangeCase->setToolTip( i18n( "Alter the capitalization of selected text." ) );
    actionChangeCase->setWhatsThis( i18n( "Alter the capitalization of selected text to one of three pre-defined patterns.<p>You can also switch all letters from uppercase to lower case and from lower case to upper case in one move." ) );

    //------------------------ Settings menu
    actionConfigure = KStdAction::preferences(this, SLOT(configure()), actionCollection(), "configure" );

    //------------------------ Menu frameSet
    actionChangePicture=new KAction( i18n( "Change picture..." ),"frame_image",0,
                                     this, SLOT( changePicture() ),
                                     actionCollection(), "change_picture" );

    actionChangeClipart=new KAction( i18n( "Change clipart..." ), "frame_image",0,
                                     this, SLOT( changeClipart() ),
                                     actionCollection(), "change_clipart" );
    actionConfigureHeaderFooter=new KAction( i18n( "Configure header/footer..." ), 0,
                                     this, SLOT( configureHeaderFooter() ),
                                     actionCollection(), "configure_headerfooter" );

    actionInlineFrame = new KToggleAction( i18n( "Inline Frame" ), 0,
                                            this, SLOT( inlineFrame() ),
                                            actionCollection(), "inline_frame" );
    actionOpenLink = new KAction( i18n( "Open link" ), 0,
                                     this, SLOT( openLink() ),
                                     actionCollection(), "open_link" );
    actionChangeLink=new KAction( i18n("Change Link"), 0,
                                  this,SLOT(changeLink()),
                                  actionCollection(), "change_link");


    actionShowDocStruct = new KToggleAction( i18n( "Show doc structure" ), 0,
                                            this, SLOT( showDocStructure() ),
                                            actionCollection(), "show_docstruct" );
    actionShowDocStruct->setToolTip( i18n( "Open document structure sidebar." ) );
    actionShowDocStruct->setWhatsThis( i18n( "Open document structure sidebar.<p>This sidebar helps you organize your document and quickly find pictures, tables, etc." ) );

    // ------------------- Actions with a key binding and no GUI item
    KAction* actNbsp = new KAction( i18n( "Insert non-breaking space" ), CTRL+Key_Space,
                        this, SLOT( slotNonbreakingSpace() ), actionCollection(), "nonbreaking_space" );
    KAction* actSoftHyphen = new KAction( i18n( "Insert soft hyphen" ), CTRL+Key_Minus,
                        this, SLOT( slotSoftHyphen() ), actionCollection(), "soft_hyphen" );
    KAction* actLineBreak = new KAction( i18n( "Line break" ), SHIFT+Key_Return,
                        this, SLOT( slotLineBreak() ), actionCollection(), "line_break" );
    // Necessary for the actions that are not plugged anywhere
    KAccel * accel = new KAccel( this );
    actNbsp ->plugAccel( accel );
    accel = new KAccel( this ); // needed ?
    actSoftHyphen->plugAccel( accel );
    actLineBreak->plugAccel( accel );
}

void KWView::refreshMenuExpression()
{
    loadexpressionActions( actionInsertExpression);
}

void KWView::loadexpressionActions( KActionMenu * parentMenu)
{
    parentMenu->popupMenu()->clear();
    QStringList files = KWFactory::global()->dirs()->findAllResources( "expression", "*.xml", TRUE );
    for( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
    {
        createExpressionActions( parentMenu,*it );
    }
}

void KWView::createExpressionActions( KActionMenu * parentMenu,const QString& filename )
{
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
        return;

    QDomDocument doc;
    doc.setContent( &file );
    file.close();

    QString group = "";

    QDomNode n = doc.documentElement().firstChild();
    bool expressionExist=false;
    for( ; !n.isNull(); n = n.nextSibling() )
        {
            if ( n.isElement() )
                {
                    QDomElement e = n.toElement();
                    if ( e.tagName() == "Type" )
                        {
                            if(!expressionExist)
                            {
                                parentMenu->popupMenu()->insertSeparator();
                                expressionExist=true;
                            }
                            group = i18n( e.namedItem( "TypeName" ).toElement().text().utf8() );
                            KActionMenu * subMenu = new KActionMenu( group, actionCollection() );
                            parentMenu->insert( subMenu );

                            QDomNode n2 = e.firstChild();
                            for( ; !n2.isNull(); n2 = n2.nextSibling() )
                                {

                                    if ( n2.isElement() )
                                        {
                                            QDomElement e2 = n2.toElement();
                                            if ( e2.tagName() == "Expression" )
                                                {
                                                    QString text = i18n( e2.namedItem( "Text" ).toElement().text().utf8() );
                                                    KAction * act = new KAction( text, 0, this, SLOT( insertExpression() ),
                                                                                 actionCollection(), "expression-action" );
                                                    subMenu->insert( act );
                                                }
                                        }
                                }

                            group = "";
                        }
                }
        }
}

void KWView::insertExpression()
{
 KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
        KAction * act = (KAction *)(sender());
        edit->insertExpression(act->text());
    }
}


void KWView::addVariableActions( int type, const QStringList & texts,
                                 KActionMenu * parentMenu, const QString & menuText )
{
    // Single items go directly into parentMenu.
    // For multiple items we create a submenu.
    if ( texts.count() > 1 && !menuText.isEmpty() )
    {
        KActionMenu * subMenu = new KActionMenu( menuText, actionCollection() );
        parentMenu->insert( subMenu );
        parentMenu = subMenu;
    }
    QStringList::ConstIterator it = texts.begin();
    for ( int i = 0; it != texts.end() ; ++it, ++i )
    {
        if ( !(*it).isEmpty() ) // in case of removed subtypes or placeholders
        {
            VariableDef v;
            v.type = type;
            v.subtype = i;
            KAction * act = new KAction( (*it), 0, this, SLOT( insertVariable() ),
                                         actionCollection(), "var-action" );
            m_variableDefMap.insert( act, v );
            parentMenu->insert( act );
        }
    }
}

void KWView::refreshCustomMenu()
{
    actionInsertCustom->popupMenu()->clear();
    QPtrListIterator<KoVariable> it( m_doc->getVariableCollection()->getVariables() );
    KAction * act=0;
    QStringList lst;
    QString varName;
    for ( ; it.current() ; ++it )
    {
        KoVariable *var = it.current();
        if ( var->type() == VT_CUSTOM )
        {
            varName=( (KoCustomVariable*) var )->name();
            if ( !lst.contains( varName) )
            {
                 lst.append( varName );
                 act = new KAction( varName, 0, this, SLOT( insertCustomVariable() ),
                                    actionCollection(), "custom-action" );
                 actionInsertCustom->insert( act );
            }
        }
    }
    bool state=!lst.isEmpty();
    if(state)
        actionInsertCustom->popupMenu()->insertSeparator();

    act = new KAction( i18n("New..."), 0, this, SLOT( insertNewCustomVariable() ), actionCollection(), "custom-action" );
    actionInsertCustom->insert( act );

    actionInsertCustom->popupMenu()->insertSeparator();

    actionEditCustomVars = new KAction( i18n( "&Custom Variables..." ), 0,
                                        this, SLOT( editCustomVars() ),
                                        actionCollection(), "edit_customvars" );
    actionEditCustomVars->setEnabled(state);
    actionInsertCustom->insert( actionEditCustomVars );
}


void KWView::insertCustomVariable()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
        KAction * act = (KAction *)(sender());
        edit->insertCustomVariable(act->text());
    }
}

void KWView::insertNewCustomVariable()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->insertVariable( VT_CUSTOM, 0 );
}

/*======================== create GUI ==========================*/
void KWView::createKWGUI()
{
    // setup GUI
    setupActions();

    m_gui = new KWGUI( this, this );
    m_gui->setGeometry( 0, 0, width(), height() );
    m_gui->show();
}

void KWView::showFormulaToolbar( bool show )
{
    // This might not be exactly the right place. But these actions
    // must be enabled when a formula is active...
    m_doc->getFormulaDocument()->getMakeGreekAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddGenericUpperAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddGenericLowerAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getRemoveEnclosingAction()->setEnabled( show );

    m_doc->getFormulaDocument()->getInsertSymbolAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddThinSpaceAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddMediumSpaceAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddThickSpaceAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddQuadSpaceAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddBracketAction()->setEnabled( show);
    m_doc->getFormulaDocument()->getAddSBracketAction()->setEnabled( show);
    m_doc->getFormulaDocument()->getAddCBracketAction()->setEnabled( show);
    m_doc->getFormulaDocument()->getAddAbsAction()->setEnabled(show);
    m_doc->getFormulaDocument()->getAddFractionAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddRootAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddSumAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddProductAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddIntegralAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddMatrixAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddOneByTwoMatrixAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddUpperLeftAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddLowerLeftAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddUpperRightAction()->setEnabled( show );
    m_doc->getFormulaDocument()->getAddLowerRightAction()->setEnabled(show );
    if(shell())
      shell()->showToolbar( "formula_toolbar", show );
}

void KWView::updatePageInfo()
{
    if ( m_sbPageLabel )
    {
        KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
        if ( edit && edit->currentFrame() )
        {
            m_currentPage = edit->currentFrame()->pageNum();
        }
        //kdDebug() << (void*)this << " KWView::updatePageInfo m_currentPage=" << m_currentPage
        //          << " m_sbPageLabel=" << m_sbPageLabel
        //          << endl;

        // ### TODO what's the current page when we have no edit object (e.g. frames are selected) ?
        // To avoid bugs, apply max page number in case a page was removed.
        m_currentPage = QMIN( m_currentPage, m_doc->getPages()-1 );

        m_sbPageLabel->setText( QString(" ")+i18n("Page %1/%2").arg(m_currentPage+1).arg(m_doc->getPages())+' ' );
    }
    slotUpdateRuler();
}

void KWView::pageNumChanged()
{
     docStructChanged(TextFrames);
     updatePageInfo();
}

void KWView::updateFrameStatusBarItem()
{
    KStatusBar * sb = statusBar();
    int nbFrame=m_doc->getSelectedFrames().count();
    if ( sb && nbFrame > 0 )
    {
        if ( !m_sbFramesLabel )
        {
            m_sbFramesLabel = sb ? new KStatusBarLabel( QString::null, 0, sb ) : 0;
            addStatusBarItem( m_sbFramesLabel );
        }
        if ( nbFrame == 1 )
        {
            KoUnit::Unit unit = m_doc->getUnit();
            QString unitName = m_doc->getUnitName();
            KWFrame * frame = m_doc->getFirstSelectedFrame();
            m_sbFramesLabel->setText( i18n( "Statusbar info", "%1. Frame: %2, %3  -  %4, %5 (width: %6, height: %7) (%8)" )
                                      .arg( frame->frameSet()->getName() )
                                      .arg( KoUnit::userValue( frame->left(), unit ) )
                                      .arg( KoUnit::userValue((frame->top() - (frame->pageNum() * m_doc->ptPaperHeight())), unit ) )
                                      .arg( KoUnit::userValue( frame->right(), unit ) )
                                      .arg( KoUnit::userValue( frame->bottom(), unit ) )
                                      .arg( KoUnit::userValue( frame->width(), unit ) )
                                      .arg( KoUnit::userValue( frame->height(), unit ) )
                                      .arg( unitName ) );
        } else
            m_sbFramesLabel->setText( i18n( "%1 frames selected" ).arg( nbFrame ) );
    }
    else if ( sb && m_sbFramesLabel )
    {
        removeStatusBarItem( m_sbFramesLabel );
        delete m_sbFramesLabel;
        m_sbFramesLabel = 0L;
    }
}

void KWView::clipboardDataChanged()
{
    if ( !m_gui || !m_doc->isReadWrite() )
    {
        actionEditPaste->setEnabled(false);
        return;
    }
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    // Is there plain text in the clipboard ?
    if ( edit && !QApplication::clipboard()->text().isEmpty() )
    {
        actionEditPaste->setEnabled(true);
        return;
    }
    QMimeSource *data = QApplication::clipboard()->data();
    bool providesImage, providesKWord, providesFormula;
    checkClipboard( data, providesImage, providesKWord, providesFormula );
    // Is there an image in the clipboard ?
    if ( providesImage )
        actionEditPaste->setEnabled( true );
    else
    {
        // Is there kword XML in the clipboard ?
        actionEditPaste->setEnabled( edit && ( providesKWord || providesFormula ) );
    }
}

void KWView::checkClipboard( QMimeSource *data, bool &providesImage, bool &providesKWord, bool &providesFormula )
{
    // QImageDrag::canDecode( data ) is very very slow in Qt 2 (n*m roundtrips)
    // Workaround....
    QValueList<QCString> formats;
    const char* fmt;
    for (int i=0; (fmt = data->format(i)); i++)
        formats.append( QCString( fmt ) );

    providesImage = false;
    QStrList fileFormats = QImageIO::inputFormats();
    for ( fileFormats.first() ; fileFormats.current() && !providesImage ; fileFormats.next() )
    {
        QCString format = fileFormats.current();
        QCString type = "image/" + format.lower();
        providesImage = ( formats.findIndex( type ) != -1 );
    }
    providesFormula = formats.findIndex( KFormula::MimeSource::selectionMimeType() ) != -1;
    providesKWord = formats.findIndex( KWTextDrag::selectionMimeType() ) != -1
                 || formats.findIndex( KWDrag::selectionMimeType() ) != -1;
}

/*=========================== file print =======================*/
void KWView::setupPrinter( KPrinter &prt )
{
    //recalc time and date variable before to print
    //it's necessary otherwise we print a document
    //with a bad date and time
    m_doc->recalcVariables(  VT_TIME );
    m_doc->recalcVariables(  VT_DATE );
    prt.setPageSelection( KPrinter::ApplicationSide );
    prt.setCurrentPage( currentPage() + 1 );
    prt.setMinMax( 1, m_doc->getPages() );

    KoPageLayout pgLayout = m_doc->pageLayout();

    prt.setPageSize( static_cast<KPrinter::PageSize>( KoPageFormat::printerPageSize( pgLayout.format ) ) );

    if ( pgLayout.orientation == PG_LANDSCAPE || pgLayout.format == PG_SCREEN )
        prt.setOrientation( KPrinter::Landscape );
    else
        prt.setOrientation( KPrinter::Portrait );
}

void KWView::print( KPrinter &prt )
{
    // Don't repaint behind the print dialog until we're done zooming/unzooming the doc
    m_gui->canvasWidget()->setUpdatesEnabled(false);
    m_gui->canvasWidget()->viewport()->setCursor( waitCursor );

    prt.setFullPage( true );

    // ### HACK: disable zooming-when-printing if embedded parts are used.
    // No koffice app supports zooming in paintContent currently.
    // Disable in ALL cases now
    bool doZoom = false;
    /*QPtrListIterator<KWFrameSet> fit = m_doc->framesetsIterator();
    for ( ; fit.current() && doZoom ; ++fit )
        if ( fit.current()->type() == FT_PART )
            doZoom = false;*/

    int oldZoom = m_doc->zoom();
    // We don't get valid metrics from the printer - and we want a better resolution
    // anyway (it's the PS driver that takes care of the printer resolution).
    QPaintDeviceMetrics metrics( &prt );

    //int dpiX = metrics.logicalDpiX();
    //int dpiY = metrics.logicalDpiY();
    int dpiX = doZoom ? 300 : QPaintDevice::x11AppDpiX();
    int dpiY = doZoom ? 300 : QPaintDevice::x11AppDpiY();
    ///////// Changing the dpiX/dpiY is very wrong nowadays. This has no effect on the font size
    ///////// that we give Qt, anymore, so it leads to minuscule fonts in the printout => doZoom==false.
    m_doc->setZoomAndResolution( 100, dpiX, dpiY );
    m_doc->newZoomAndResolution( false, true /* for printing*/ );
    //kdDebug() << "KWView::print metrics: " << metrics.logicalDpiX() << "," << metrics.logicalDpiY() << endl;
    //kdDebug() << "x11AppDPI: " << QPaintDevice::x11AppDpiX() << "," << QPaintDevice::x11AppDpiY() << endl;

    bool serialLetter = FALSE;

    QPtrList<KoVariable> vars = m_doc->getVariableCollection()->getVariables();
    KoVariable *v = 0;
    for ( v = vars.first(); v; v = vars.next() ) {
        if ( v->type() == VT_MAILMERGE ) {
            serialLetter = TRUE;
            break;
        }
    }

    if ( !m_doc->getMailMergeDataBase() ) serialLetter=FALSE;
	else
	{
		m_doc->getMailMergeDataBase()->refresh(false);
                if (m_doc->getMailMergeDataBase()->getNumRecords() == 0 )  serialLetter = FALSE;
	}

    //float left_margin = 0.0;
    //float top_margin = 0.0;

    KoPageLayout pgLayout;
    KoColumns cl;
    KoKWHeaderFooter hf;
    m_doc->getPageLayout( pgLayout, cl, hf );
    KoPageLayout oldPGLayout = pgLayout;

    if ( pgLayout.format == PG_SCREEN )
    {
        //left_margin = 25.8;
        //top_margin = 15.0;
        pgLayout.ptLeft += 25.8;         // Not sure why we need this....
        pgLayout.ptRight += 15.0;
        // TODO the other units. Well, better get rid of the multiple-unit thing.
        m_doc->setPageLayout( pgLayout, cl, hf );
    }

    QPainter painter;
    painter.begin( &prt );

    kdDebug(32001) << "KWView::print scaling by " << (double)metrics.logicalDpiX() / (double)dpiX
                   << "," << (double)metrics.logicalDpiY() / (double)dpiY << endl;
    painter.scale( (double)metrics.logicalDpiX() / (double)dpiX,
                   (double)metrics.logicalDpiY() / (double)dpiY );

    bool canceled = false;
    // Breaks wysiwyg, obviously - trying without
//#define KW_PASS_PAINTER_TO_QRT
#ifdef KW_PASS_PAINTER_TO_QRT
    int paragraphs = 0;
    fit.toFirst();
    for ( ; fit.current() ; ++fit )
        if ( fit.current()->isVisible() )
            paragraphs += fit.current()->paragraphs();
    kdDebug() << "KWView::print total paragraphs: " << paragraphs << endl;

    // This can take a lot of time (reformatting everything), so a progress dialog is needed
    QProgressDialog progress( i18n( "Printing..." ), i18n( "Cancel" ), paragraphs, this );
    progress.setProgress( 0 );
    int processedParags = 0;
    fit.toFirst();
    for ( ; fit.current() ; ++fit )
        if ( fit.current()->isVisible() )
        {
            kapp->processEvents();
            if ( progress.wasCancelled() ) {
                canceled = true;
                break;
            }

            kdDebug() << "KWView::print preparePrinting " << fit.current()->getName() << endl;
            fit.current()->preparePrinting( &painter, &progress, processedParags );
        }
#endif

    if ( !canceled )
    {
        if ( !serialLetter )
            m_gui->canvasWidget()->print( &painter, &prt );
        else
        {
            for ( int i = 0; i < m_doc->getMailMergeDataBase()->getNumRecords(); ++i ) {
                m_doc->setMailMergeRecord( i );
		m_doc->getVariableCollection()->recalcVariables(VT_MAILMERGE);
                m_gui->canvasWidget()->print( &painter, &prt );
                if ( i < m_doc->getMailMergeDataBase()->getNumRecords() - 1 )
                    prt.newPage();
            }
            m_doc->setMailMergeRecord( -1 );
        }
    }

    painter.end();

    if ( pgLayout.format == PG_SCREEN )
        m_doc->setPageLayout( oldPGLayout, cl, hf );

#ifdef KW_PASS_PAINTER_TO_QRT
    fit.toFirst();
    for ( ; fit.current() ; ++fit )
        if ( fit.current()->isVisible() )
            fit.current()->preparePrinting( 0L, 0L, processedParags );
#endif

    m_doc->setZoomAndResolution( oldZoom, QPaintDevice::x11AppDpiX(), QPaintDevice::x11AppDpiY() );
    m_doc->newZoomAndResolution( false, false );
    kdDebug() << "KWView::print zoom&res reset" << endl;

    m_gui->canvasWidget()->setUpdatesEnabled(true);
    m_gui->canvasWidget()->viewport()->setCursor( ibeamCursor );
    m_doc->repaintAllViews();
}

void KWView::showFormat( const KoTextFormat &currentFormat )
{
    // update the gui with the current format.
    //kdDebug() << "KWView::setFormat font family=" << currentFormat.font().family() << endl;
    actionFormatFontFamily->setFont( currentFormat.font().family() );
    actionFormatFontSize->setFontSize( currentFormat.font().pointSize() );
    actionFormatBold->setChecked( currentFormat.font().bold());
    actionFormatItalic->setChecked( currentFormat.font().italic() );
    actionFormatUnderline->setChecked( currentFormat.font().underline());
    actionFormatStrikeOut->setChecked( currentFormat.font().strikeOut());
    QColor col=currentFormat.textBackgroundColor();
    actionBackgroundColor->setEnabled(true);
    actionBackgroundColor->setCurrentColor( col.isValid() ? col : QApplication::palette().color( QPalette::Active, QColorGroup::Base ));
    actionBackgroundColor->setText(i18n("Text Background Color..."));
    actionFormatColor->setCurrentColor( currentFormat.color() );

    switch(currentFormat.vAlign())
      {
      case QTextFormat::AlignSuperScript:
        {
          actionFormatSub->setChecked( false );
          actionFormatSuper->setChecked( true );
          break;
        }
      case QTextFormat::AlignSubScript:
        {
          actionFormatSub->setChecked( true );
          actionFormatSuper->setChecked( false );
          break;
        }
      case QTextFormat::AlignNormal:
      default:
        {
          actionFormatSub->setChecked( false );
          actionFormatSuper->setChecked( false );
          break;
        }
      }

}

void KWView::showRulerIndent( double _leftMargin, double _firstLine, double _rightMargin )
{
  KoRuler * hRuler = m_gui ? m_gui->getHorzRuler() : 0;
  if ( hRuler )
  {
      hRuler->setFirstIndent( KoUnit::ptToUnit( _firstLine + _leftMargin, m_doc->getUnit() ) );
      hRuler->setLeftIndent( KoUnit::ptToUnit( _leftMargin, m_doc->getUnit() ) );
      hRuler->setRightIndent( KoUnit::ptToUnit( _rightMargin, m_doc->getUnit() ) );
      actionFormatDecreaseIndent->setEnabled( _leftMargin>0);
  }
}

void KWView::showAlign( int align ) {
    switch ( align ) {
        case Qt::AlignAuto: // In left-to-right mode it's align left. TODO: alignright if text->isRightToLeft()
        case Qt::AlignLeft:
            actionFormatAlignLeft->setChecked( TRUE );
            break;
        case Qt::AlignCenter:
            actionFormatAlignCenter->setChecked( TRUE );
            break;
        case Qt::AlignRight:
            actionFormatAlignRight->setChecked( TRUE );
            break;
        case Qt::AlignJustify: // Make this Qt:AlignJustify after the merge with Qt3
            actionFormatAlignBlock->setChecked( TRUE );
            break;
    }
}

void KWView::showCounter( KoParagCounter &c )
{
    actionFormatList->setChecked( c.numbering() == KoParagCounter::NUM_LIST );
}

void KWView::showFrameBorders( const KoBorder& _left, const KoBorder& _right,
                               const KoBorder& _top, const KoBorder& _bottom )
{
    showParagBorders( _left, _right, _top, _bottom );
}

void KWView::showParagBorders( const KoBorder& left, const KoBorder& right,
                               const KoBorder& top, const KoBorder& bottom )
{
    if ( m_border.left != left || m_border.right != right || m_border.top != top || m_border.bottom != bottom )
    {
        m_border.left = left;
        m_border.right = right;
        m_border.top = top;
        m_border.bottom = bottom;

        actionBorderLeft->setChecked( left.ptWidth > 0 );
        actionBorderRight->setChecked( right.ptWidth > 0 );
        actionBorderTop->setChecked( top.ptWidth > 0 );
        actionBorderBottom->setChecked( bottom.ptWidth > 0 );
        actionBorderOutline->setChecked(
            actionBorderLeft->isChecked() &&
            actionBorderRight->isChecked() &&
            actionBorderTop->isChecked() &&
            actionBorderBottom->isChecked());

        if ( left.ptWidth > 0 ) {
            m_border.common = left;
            borderShowValues();
        }
        if ( right.ptWidth > 0 ) {
            m_border.common = right;
            borderShowValues();
        }
        if ( top.ptWidth > 0 ) {
            m_border.common = top;
            borderShowValues();
        }
        if ( bottom.ptWidth > 0 ) {
            m_border.common = bottom;
            borderShowValues();
        }
    }
}

void KWView::updateReadWrite( bool readwrite )
{
    // Disable everything if readonly.
    if ( !readwrite )
    {
        QValueList<KAction*> actions = actionCollection()->actions();
        QValueList<KAction*>::ConstIterator aIt = actions.begin();
        QValueList<KAction*>::ConstIterator aEnd = actions.end();
        for (; aIt != aEnd; ++aIt )
            (*aIt)->setEnabled( readwrite );
        // A few harmless actions
        actionFileStatistics->setEnabled( true );
        actionExtraCreateTemplate->setEnabled( true );
        actionViewPageMode->setEnabled( true );
        actionViewPreviewMode->setEnabled( true );
        actionViewFormattingChars->setEnabled( true );
        actionViewFrameBorders->setEnabled( true );
        actionViewHeader->setEnabled( true );
        actionViewFooter->setEnabled( true );
        actionViewFootNotes->setEnabled( true );
        actionViewEndNotes->setEnabled( true );
        actionViewZoom->setEnabled( true );
        KAction* newView = actionCollection()->action("view_newview");
        if (newView) newView->setEnabled( true );
        // Well, the view menu doesn't appear in konq, so this is currently useless...
    } else
    {
        // Don't enable everything if readwrite. E.g. "undo" must be initially disabled.
        slotFrameSetEditChanged();
        // Insert
        actionInsertTable->setEnabled( true );
        actionToolsCreatePart->setEnabled( true );
        actionToolsCreatePix->setEnabled( true );
        actionToolsCreateText->setEnabled( true );
        // Format
        actionFormatStylist->setEnabled( true );
        actionFormatPage->setEnabled( true );
        // Tools
        actionExtraSpellCheck->setEnabled( true );
        actionAutoFormat->setEnabled( true );
        refreshCustomMenu();
        actionEditPersonnalExpr->setEnabled( true );
        // Settings
        actionConfigure->setEnabled( true );
    }
}

void KWView::setTool( int _mouseMode )
{
    switch ( _mouseMode ) {
    case KWCanvas::MM_EDIT:
    case KWCanvas::MM_CREATE_TABLE:
    case KWCanvas::MM_CREATE_FORMULA:
    case KWCanvas::MM_CREATE_PART:
        // No tool to activate for this mode -> deselect all the others
        actionToolsCreateText->setChecked( false );
        actionToolsCreatePix->setChecked( false );
        //actionToolsCreatePart->setChecked( false );
        break;
    case KWCanvas::MM_CREATE_TEXT:
        actionToolsCreateText->setChecked( true );
        break;
    case KWCanvas::MM_CREATE_PIX:
        actionToolsCreatePix->setChecked( true );
        break;
        //case KWCanvas::MM_CREATE_PART:
        //actionToolsCreatePart->setChecked( true );
        break;
    }

    actionTableJoinCells->setEnabled( FALSE );
    actionTableSplitCells->setEnabled( FALSE );
    actionFormatFrameSet->setEnabled(FALSE);
}

void KWView::showStyle( const QString & styleName )
{
    QPtrListIterator<KWStyle> styleIt( m_doc->styleList() );
    for ( int pos = 0 ; styleIt.current(); ++styleIt, ++pos )
    {
        if ( styleIt.current()->name() == styleName ) {
            actionFormatStyle->setCurrentItem( pos );
            return;
        }
    }
}

void KWView::updateStyleList()
{
    QString currentStyle = actionFormatStyle->currentText();
    QStringList lst;
    QPtrListIterator<KWStyle> styleIt( m_doc->styleList() );
    for (; styleIt.current(); ++styleIt ) {
        lst << styleIt.current()->translatedName();
    }
    actionFormatStyle->setItems( lst );
    showStyle( currentStyle );
}

void KWView::editCut()
{
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    if ( edit )
        edit->cut();
    else
    {
        m_gui->canvasWidget()->cutSelectedFrames();
    }
}

void KWView::editCopy()
{
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    if ( edit )
        edit->copy();
    else
    {
        m_gui->canvasWidget()->copySelectedFrames();
    }
}

void KWView::editPaste()
{
    QMimeSource *data = QApplication::clipboard()->data();
    if ( data->provides( KWDrag::selectionMimeType() ) )
        m_gui->canvasWidget()->pasteFrames();
    else {
        bool providesImage, providesKWord, providesFormula;
        checkClipboard( data, providesImage, providesKWord, providesFormula );
        // formula must be the first as a formula is also available as image
        if ( providesFormula ) {
            KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
            if ( edit && edit->frameSet()->type() == FT_FORMULA ) {
                edit->paste();
            }
            else {
                insertFormula( data );
            }
        }
        else if ( providesImage )
        {

            KoPoint docPoint( m_doc->ptLeftBorder(), m_doc->ptPageTop( m_currentPage ) + m_doc->ptTopBorder() );
            m_gui->canvasWidget()->pasteImage( data, docPoint );
        }
        else
        {
            KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
            if ( edit )
                edit->paste();
        }
    }
}

void KWView::editSelectAll()
{
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    if ( edit )
        edit->selectAll();
}


void KWView::editFind()
{
    // Already a find or replace running ?
    if ( m_findReplace )
    {
        m_findReplace->setActiveWindow();
        return;
    }

    if (!m_searchEntry)
        m_searchEntry = new KoSearchContext();
    KWTextFrameSetEdit * edit = dynamic_cast<KWTextFrameSetEdit *>(m_gui->canvasWidget()->currentFrameSetEdit());
    bool hasSelection=edit && static_cast<KWTextFrameSet *>(edit->frameSet())->hasSelection();
    KoSearchDia dialog( m_gui->canvasWidget(), "find", m_searchEntry,hasSelection );

    if ( dialog.exec() == QDialog::Accepted )
    {
        m_findReplace = new KWFindReplace( m_gui->canvasWidget(), &dialog,edit ,m_gui->canvasWidget()->kWordDocument()->frameTextObject());
        doFindReplace();
    }
}

void KWView::editReplace()
{
    // Already a find or replace running ?
    if ( m_findReplace )
    {
        m_findReplace->setActiveWindow();
        return;
    }

    if (!m_searchEntry)
        m_searchEntry = new KoSearchContext();
    if (!m_replaceEntry)
        m_replaceEntry = new KoSearchContext();

    KWTextFrameSetEdit * edit = dynamic_cast<KWTextFrameSetEdit *>(m_gui->canvasWidget()->currentFrameSetEdit());
    bool hasSelection=edit && static_cast<KWTextFrameSet *>(edit->frameSet())->hasSelection();

    KoReplaceDia dialog( m_gui->canvasWidget(), "replace", m_searchEntry, m_replaceEntry,hasSelection );
    if ( dialog.exec() == QDialog::Accepted )
    {
        m_findReplace = new KWFindReplace( m_gui->canvasWidget(), &dialog,edit ,m_gui->canvasWidget()->kWordDocument()->frameTextObject());
        doFindReplace();
    }
}

void KWView::doFindReplace()
{
    KWFindReplace* findReplace = m_findReplace; // keep a copy. "this" might be deleted before we exit this method

    findReplace->proceed();

    bool aborted = findReplace->aborted();
    delete findReplace;
    if ( !aborted ) // Only if we still exist....
        m_findReplace = 0L;
}

void KWView::raiseFrame()
{
    KMacroCommand* macroCmd = 0L;
    // For each selected frame...
    QPtrList<KWFrame> frames = m_doc->getSelectedFrames();
    QPtrListIterator<KWFrame> fIt( frames );
    for ( ; fIt.current() ; ++fIt ) {
        KWFrame* frame = fIt.current();
        // Can't raise the main frame in a WP document. Everything would go under it...
        if ( m_doc->processingType() == KWDocument::WP &&
             m_doc->frameSetNum( frame->frameSet() ) == 0 )
            continue;

        int pageNum = frame->pageNum();
        // Look for frame in same page that is most on top
        bool first = true;
        int maxZOrder = 0; // (this initialization is bogus, value is never used)
        QPtrList<KWFrame> framesInPage = m_doc->framesInPage( pageNum );
        QPtrListIterator<KWFrame> frameIt( framesInPage );
        for ( ; frameIt.current(); ++frameIt ) {
            if ( first || frameIt.current()->zOrder() > maxZOrder ) {
                maxZOrder = frameIt.current()->zOrder();
                first = false;
            }
        }
        // Now raise the frame - unless it's already the one most on top
        // That test doesn't work, due to multiple frames with the same zorder...
        //if ( !first && frame->zOrder() <= maxZOrder ) {
        if ( !first ) {
            KWFrame* frameCopy = frame->getCopy();
            frame->setZOrder( maxZOrder + 1 );

            KWFramePropertiesCommand* cmd = new KWFramePropertiesCommand( QString::null, frameCopy, frame );
            if(!macroCmd)
                macroCmd = new KMacroCommand( i18n("Raise Frame") );
            macroCmd->addCommand(cmd);
        }
    }
    if ( macroCmd )
    {
        m_doc->addCommand(macroCmd);
        // Calling updateFrames() on the selected frames' framesets isn't enough,
        // we also need other framesets to notice the new frames on top.
        m_doc->updateAllFrames();
        m_doc->layout();
        m_doc->repaintAllViews();
    }
}

void KWView::lowerFrame()
{
    KMacroCommand* macroCmd = 0L;
    // For each selected frame...
    QPtrList<KWFrame> frames = m_doc->getSelectedFrames();
    QPtrListIterator<KWFrame> fIt( frames );
    for ( ; fIt.current() ; ++fIt ) {
        KWFrame* frame = fIt.current();

        int pageNum = frame->pageNum();
        // Look for frame in same page that is most on top
        bool first = true;
        KWFrame* frameOfFirstFrameSet = 0L;
        int minZOrder = 0; // (this initialization is bogus, value is never used)
        QPtrList<KWFrame> framesInPage = m_doc->framesInPage( pageNum );
        QPtrListIterator<KWFrame> frameIt( framesInPage );
        for ( ; frameIt.current(); ++frameIt ) {
            if ( first || frameIt.current()->zOrder() < minZOrder ) {
                minZOrder = frameIt.current()->zOrder();
                first = false;
            }
            if ( !frameOfFirstFrameSet && m_doc->processingType() == KWDocument::WP &&
                 m_doc->frameSetNum( frameIt.current()->frameSet() ) == 0 )
                frameOfFirstFrameSet = frameIt.current(); // remember for below
        }
        // Now raise the frame - unless it's already the one most on top
        // That test doesn't work, due to multiple frames with the same zorder...
        //if ( !first && frame->zOrder() <= maxZOrder ) {
        if ( !first ) {
            KWFrame* frameCopy = frame->getCopy();
            frame->setZOrder( minZOrder - 1 );

            KWFramePropertiesCommand* cmd = new KWFramePropertiesCommand( QString::null, frameCopy, frame );
            if(!macroCmd)
                macroCmd = new KMacroCommand( i18n("Raise Frame") );
            macroCmd->addCommand(cmd);

            // Can't lower under the main frame in a WP document.
            // If we just did that, fix it by lowering the zorder of the main frame.
            // Hopefully no need for undo/redo for that one, the main frame remains under.
            if ( frameOfFirstFrameSet && frame->zOrder() <= frameOfFirstFrameSet->zOrder() )
                frameOfFirstFrameSet->setZOrder( frame->zOrder() - 1 );
        }
    }
    if ( macroCmd )
    {
        m_doc->addCommand(macroCmd);
        // Calling updateFrames() on the selected frames' framesets isn't enough,
        // we also need other framesets to notice the new frames on top.
        m_doc->updateAllFrames();
        m_doc->layout();
        m_doc->repaintAllViews();
    }
}

void KWView::editDeleteFrame()
{
    deleteFrame();
}

void KWView::deleteFrame( bool _warning )
{
    QPtrList<KWFrame> frames=m_doc->getSelectedFrames();
    Q_ASSERT( frames.count() >= 1 );
    if( frames.count() < 1)
        return;
    if(frames.count()==1)
    {
        KWFrame *theFrame = frames.at(0);
        KWFrameSet *fs = theFrame->frameSet();

        Q_ASSERT( !fs->isAHeader() ); // the action is disabled for such cases
        Q_ASSERT( !fs->isAFooter() );
        if ( fs->isAFooter() || fs->isAHeader() )
            return;

        // frame is part of a table?
        if ( fs->getGroupManager() )
        {
            int result = KMessageBox::warningContinueCancel(
                this,
                i18n( "You are about to delete a table.\n"
                      "Doing so will delete all the text in the table.\n"
                      "Are you sure you want to do that?"),
                i18n("Delete Table"), i18n("&Delete"),
                "DeleteTableConfirmation",
                true );
            if (result != KMessageBox::Continue)
                return;
            m_doc->deleteTable( fs->getGroupManager() );
            m_gui->canvasWidget()->emitFrameSelectedChanged();
            return;
        }

        if ( fs->getNumFrames() == 1 && fs->type() == FT_TEXT) {
            if ( m_doc->processingType() == KWDocument::WP && m_doc->frameSetNum( fs ) == 0 )
                return;

            KWTextFrameSet * textfs = dynamic_cast<KWTextFrameSet *>(m_doc->frameSet( 0 ) );
            if ( !textfs )
                return;

            KoTextDocument * textdoc = textfs->textDocument();
            Qt3::QTextParag * parag = textdoc->firstParag();
            if ( parag && parag->string()->length() > 0 )
            {
                int result = KMessageBox::warningContinueCancel(
                    this,
                    i18n( "You are about to delete the last Frame of the "
                          "Frameset '%1'. "
                          "The contents of this Frameset will not appear "
                          "anymore!\n"
                          "Are you sure you want to do that?").arg(fs->getName()),
                    i18n("Delete Frame"), i18n("&Delete"),
                    "DeleteLastFrameConfirmation",
                    true );

                if (result != KMessageBox::Continue)
                    return;

                m_doc->deleteFrame( theFrame );
                m_gui->canvasWidget()->emitFrameSelectedChanged();
                return;
            }

        }

        if(_warning)
        {
            int result = KMessageBox::warningContinueCancel(
                this,
                i18n("Do you want to delete this frame?"),
                i18n("Delete Frame"),
                i18n("&Delete"),
                "DeleteLastFrameConfirmation",
                true );
            if (result != KMessageBox::Continue)
                return;
        }
        m_doc->deleteFrame( theFrame );
        m_gui->canvasWidget()->emitFrameSelectedChanged();
    }
    else
    {
        //several frame
        if(_warning)
        {
            int result = KMessageBox::warningContinueCancel(
                this,
                i18n("Do you want to delete this frame?"),
                i18n("Delete Frame"),
                i18n("&Delete"),
                "DeleteLastFrameConfirmation",
                true );
            if (result != KMessageBox::Continue)
                return;
        }

        m_doc->deleteSeveralFrame();

        m_gui->canvasWidget()->emitFrameSelectedChanged();
    }
}

void KWView::editCustomVars()
{
    KoCustomVariablesDia dia( this, m_doc->getVariableCollection()->getVariables() );
    QStringList listOldCustomValue;
    QPtrListIterator<KoVariable> oldIt( m_doc->getVariableCollection()->getVariables() );
    for ( ; oldIt.current() ; ++oldIt )
    {
        if(oldIt.current()->type()==VT_CUSTOM)
            listOldCustomValue.append(((KoCustomVariable*)oldIt.current())->value());
    }
    if(dia.exec())
    {
        m_doc->recalcVariables( VT_CUSTOM );
        //temporaly hack, for the moment we can't undo/redo change custom variables
        QPtrListIterator<KoVariable> it( m_doc->getVariableCollection()->getVariables() );
        KMacroCommand * macroCommand = 0L;
        int i=0;
        for ( ; it.current() ; ++it )
        {
            if(it.current()->type() == VT_CUSTOM )
            {
                if(((KoCustomVariable*)it.current())->value()!=*(listOldCustomValue.at(i)))
                {
                    if(!macroCommand)
                        macroCommand = new KMacroCommand( i18n( "Change custom variable" ) );
                    KWChangeCustomVariableValue *cmd=new KWChangeCustomVariableValue(i18n( "Change custom variable" ),m_doc,*(listOldCustomValue.at(i)), ((KoCustomVariable*)it.current())->value() ,((KoCustomVariable*)it.current()));
                    macroCommand->addCommand(cmd);
                }
                i++;
            }
        }
        if(macroCommand)
            m_doc->addCommand(macroCommand);
    }
}

void KWView::editMailMergeDataBase()
{
	m_doc->getMailMergeDataBase()->showConfigDialog(this);
#if 0
    KWMailMergeEditor *dia = new KWMailMergeEditor( this, m_doc->getMailMergeDataBase() );
    dia->exec();
    // Don't know if we really need this so it's commented out (SL)
    // m_gui->canvasWidget()->repaintAll( FALSE );
    delete dia;
#endif
}

void KWView::viewTextMode()
{
    if ( actionViewTextMode->isChecked() )
    {
        if ( dynamic_cast<KWViewModePreview *>(m_gui->canvasWidget()->viewMode()) )
            m_zoomViewModePreview = m_doc->zoom();
        showZoom( m_zoomViewModeNormal ); // share the same zoom
        setZoom( m_zoomViewModeNormal, false );
        m_gui->canvasWidget()->switchViewMode( new KWViewModeText( m_doc ) );
        m_doc->setLastViewMode(m_gui->canvasWidget()->viewMode()->type());
        m_doc->updateZoomRuler();
    }
    else
        actionViewTextMode->setChecked( true ); // always one has to be checked !
}

void KWView::viewPageMode()
{
    if ( actionViewPageMode->isChecked() )
    {
        if ( dynamic_cast<KWViewModePreview *>(m_gui->canvasWidget()->viewMode()) )
            m_zoomViewModePreview = m_doc->zoom();
        showZoom( m_zoomViewModeNormal );
        setZoom( m_zoomViewModeNormal, false );
        slotUpdateRuler();
        m_gui->canvasWidget()->switchViewMode( new KWViewModeNormal( m_doc ) );
        m_doc->setLastViewMode(m_gui->canvasWidget()->viewMode()->type());
        m_doc->updateZoomRuler();
    }
    else
        actionViewPageMode->setChecked( true ); // always one has to be checked !
}

void KWView::viewPreviewMode()
{
    if ( actionViewPreviewMode->isChecked() )
    {
        m_zoomViewModeNormal = m_doc->zoom();
        showZoom( m_zoomViewModePreview );
        setZoom( m_zoomViewModePreview, false );
        slotUpdateRuler();
        m_gui->canvasWidget()->switchViewMode( new KWViewModePreview( m_doc, m_doc->nbPagePerRow() ) );
        m_doc->setLastViewMode(m_gui->canvasWidget()->viewMode()->type());
    }
    else
        actionViewPreviewMode->setChecked( true ); // always one has to be checked !
}

void KWView::changeZoomMenu( int zoom )
{
    QStringList lst;
    if(zoom>0)
    {
	if( lst.contains( i18n( "Zoom to width" ) ) == 0 )
	    lst << i18n( "Zoom to width" );
        if( lst.contains( i18n( "Zoom to Whole Page" ) )==0)
            lst << i18n( "Zoom to Whole Page" );

        QValueList<int> list;
        QString z;
        int val;
        bool ok;
        QStringList itemsList = actionViewZoom->items();
        for (QStringList::Iterator it = itemsList.begin() ; it != itemsList.end() ; ++it)
        {
            z = (*it).replace( QRegExp( "%" ), "" );
            z = z.simplifyWhiteSpace();
            val=z.toInt(&ok);
            //zoom : limit inferior=10
            if(ok && val>9  &&list.contains(val)==0)
                list.append( val );
        }
        //necessary at the beginning when we read config
        //this value is not in combo list
        if(list.contains(zoom)==0)
            list.append( zoom );

        qHeapSort( list );

        for (QValueList<int>::Iterator it = list.begin() ; it != list.end() ; ++it)
            lst.append( (QString::number(*it)+'%') );
    }
    else
    {
          lst << i18n( "Zoom to width" );
          lst << i18n( "Zoom to Whole Page" );
          lst << "33%";
          lst << "50%";
          lst << "75%";
          lst << "100%";
          lst << "125%";
          lst << "150%";
          lst << "200%";
          lst << "250%";
          lst << "350%";
          lst << "400%";
          lst << "450%";
          lst << "500%";
    }
    actionViewZoom->setItems( lst );
}

void KWView::showZoom( int zoom )
{
    QStringList list = actionViewZoom->items();
    QString zoomStr = QString::number( zoom ) + '%';
    actionViewZoom->setCurrentItem( list.findIndex(zoomStr)  );
}

void KWView::slotViewFormattingChars()
{
    m_doc->setViewFormattingChars(actionViewFormattingChars->isChecked());
    m_doc->layout(); // Due to the different formatting when this option is activated
    m_doc->repaintAllViews();
}

void KWView::slotViewFrameBorders()
{
    setViewFrameBorders(actionViewFrameBorders->isChecked());
    m_gui->canvasWidget()->repaintAll();
}

void KWView::viewHeader()
{
    bool state = actionViewHeader->isChecked();
    m_doc->setHeaderVisible( state );
    KWTextFrameSetEdit * edit = currentTextEdit();
    if(!state )
    {
        KWFrameSet *frameSet=0L;
        if(edit)
        {
            frameSet=edit->frameSet();
            if (frameSet->isAHeader())
                m_doc->terminateEditing( frameSet );
            else
            {
                KWTableFrameSet *table = frameSet->frame(0)->frameSet()->getGroupManager();
                if (table)
                {
                    if (table->isFloating() && table->anchorFrameset()->isAHeader())
                        m_doc->terminateEditing( table );
                }
            }

        }
        else
        {
            KWFormulaFrameSetEdit * editFormula = dynamic_cast<KWFormulaFrameSetEdit *>(m_gui->canvasWidget()->currentFrameSetEdit());
            if(editFormula)
            {
                frameSet= editFormula->frameSet();
                if(frameSet->type()==FT_FORMULA && frameSet->isFloating())
                    m_doc->terminateEditing( frameSet );
            }

        }
    }
    m_doc->updateResizeHandles( );
}

void KWView::viewFooter()
{
    bool state=actionViewFooter->isChecked();
    m_doc->setFooterVisible( state );
    KWTextFrameSetEdit * edit = currentTextEdit();
    if(!state )
    {
        KWFrameSet *frameSet=0L;
        if(edit)
        {
            frameSet=edit->frameSet();
            if (frameSet->isAFooter())
                m_doc->terminateEditing( frameSet );
            else
            {
                KWTableFrameSet *table = frameSet->frame(0)->frameSet()->getGroupManager();
                if (table)
                {
                    if (table->isFloating() && table->anchorFrameset()->isAFooter())
                        m_doc->terminateEditing( table );
                }
            }
        }
        else
        {
            KWFormulaFrameSetEdit * editFormula = dynamic_cast<KWFormulaFrameSetEdit *>(m_gui->canvasWidget()->currentFrameSetEdit());
            if(editFormula)
            {
                frameSet= editFormula->frameSet();
                if(frameSet->type()==FT_FORMULA && frameSet->isFloating())
                    m_doc->terminateEditing( frameSet );

            }

        }
    }
    m_doc->updateResizeHandles( );
}

void KWView::viewFootNotes()
{
#if 0
    if ( !actionViewFootNotes->isChecked() )
        return;
    setNoteType( KWFootNoteManager::FootNotes);
#endif
}

void KWView::viewEndNotes()
{
#if 0
    if ( !actionViewEndNotes->isChecked() )
        return;
    setNoteType( KWFootNoteManager::EndNotes);
#endif
}

#if 0
void KWView::setNoteType( KWFootNoteManager::NoteType nt, bool change)
{
    if (change)
        m_doc->setNoteType( nt );
    switch (nt)
    {
      case KWFootNoteManager::FootNotes:
      actionViewFootNotes->setChecked( TRUE );
      actionInsertFootEndNote->setText(i18n("&Footnote"));
      break;
    case KWFootNoteManager::EndNotes:
      default:
      actionViewEndNotes->setChecked( TRUE );
      actionInsertFootEndNote->setText(i18n("&Endnote"));
      break;
    }
}
#endif

void KWView::viewZoom( const QString &s )
{
    QString z( s );
    bool ok=false;
    KWCanvas * canvas = m_gui->canvasWidget();
    int zoom = 0;

    if ( z == i18n("Zoom to width") )
    {
        zoom = qRound( static_cast<double>(canvas->visibleWidth() * 100 ) / (m_doc->resolutionX() * m_doc->ptPaperWidth() ) );
        ok = true;
    }
    else if ( z == i18n("Zoom to Whole Page") )
    {
        double height = m_doc->resolutionY() * m_doc->ptPaperHeight();
        double width = m_doc->resolutionX() * m_doc->ptPaperWidth();
        zoom = QMIN( qRound( static_cast<double>(canvas->visibleHeight() * 100 ) / height ),
                     qRound( static_cast<double>(canvas->visibleWidth() * 100 ) / width ) );
        ok = true;
    }
    else
    {
    	z = z.replace( QRegExp( "%" ), "" );
    	z = z.simplifyWhiteSpace();
    	zoom = z.toInt(&ok);
    }
    if( !ok || zoom<10 ) //zoom should be valid and >10
        zoom = m_doc->zoom();
    //refresh menu
    changeZoomMenu( zoom );
    //refresh menu item
    showZoom(zoom);
    //apply zoom if zoom!=m_doc->zoom()
    if( zoom != m_doc->zoom() )
    {
        setZoom( zoom, true );

        m_doc->updateResizeHandles();
        KWTextFrameSetEdit * edit = currentTextEdit();
        if ( edit )
            edit->ensureCursorVisible();
    }

    m_gui->canvasWidget()->setFocus();

}

void KWView::setZoom( int zoom, bool updateViews )
{
    m_doc->setZoomAndResolution( zoom, QPaintDevice::x11AppDpiX(), QPaintDevice::x11AppDpiY());
    m_doc->newZoomAndResolution( updateViews, false );
    m_doc->updateZoomRuler();

    // Also set the zoom in KoView (for embedded views)
    //kdDebug() << "KWView::showZoom setting koview zoom to " << m_doc->zoomedResolutionY() << endl;
    KoView::setZoom( m_doc->zoomedResolutionY() /* KoView only supports one zoom */ );
}

void KWView::insertPicture()
{
    if ( !actionToolsCreatePix->isChecked() )
    {
        actionToolsCreatePix->setChecked( true ); // clicked on the already active tool!
        return;
    }
    KWInsertPicDia dia( this );
    if ( dia.exec() == QDialog::Accepted && !dia.filename().isEmpty() )
        insertPicture( dia.filename(), dia.type() == KWInsertPicDia::IPD_CLIPART, dia.makeInline(), dia.pixmapSize(),dia.keepRatio() );
    else
        setTool( KWCanvas::MM_EDIT );
}

void KWView::slotEmbedImage( const QString &filename )
{
    insertPicture( filename, false, false, QSize(),true );
}

void KWView::insertPicture( const QString &filename, bool isClipart,
                            bool makeInline, QSize pixmapSize, bool _keepRatio )
{
    if ( makeInline )
    {
        fsInline = 0L;
        uint width = 0;
        uint height = 0;
        if ( isClipart )
        {
            KWClipartFrameSet *frameset = new KWClipartFrameSet( m_doc, QString::null );
            frameset->loadClipart( filename );
            //frameset->setKeepAspectRatio( _keepRatio);
            fsInline = frameset;
            // Set an initial size
            width = m_doc->zoomItX( 100 );
            height = m_doc->zoomItY( 100 );
        }
        else
        {
            // For an inline frame we _need_ to find out the size!
            if ( pixmapSize.isEmpty() )
                pixmapSize = QPixmap( filename ).size();

            // This ensures 1-1 at 100% on screen, but allows zooming and printing with correct DPI values
            width = qRound( (double)pixmapSize.width() * m_doc->zoomedResolutionX() / POINT_TO_INCH( QPaintDevice::x11AppDpiX() ) );
            height = qRound( (double)pixmapSize.height() * m_doc->zoomedResolutionY() / POINT_TO_INCH( QPaintDevice::x11AppDpiY() ) );
            // Apply reasonable limits
            width = QMIN( width, m_doc->paperWidth() - m_doc->leftBorder() - m_doc->rightBorder() - m_doc->zoomItX( 10 ) );
            height = QMIN( height, m_doc->paperHeight() - m_doc->topBorder() - m_doc->bottomBorder() - m_doc->zoomItY( 10 ) );

            KWPictureFrameSet *frameset = new KWPictureFrameSet( m_doc, QString::null );
            frameset->loadImage( filename, QSize( width, height ) );
            frameset->setKeepAspectRatio( _keepRatio);
            fsInline = frameset;
        }
        KWFrame *frame = new KWFrame( fsInline, 0, 0, m_doc->unzoomItX( width ), m_doc->unzoomItY( height ) );
        fsInline->addFrame( frame, false );
        m_gui->canvasWidget()->inlinePictureStarted();
        setTool( KWCanvas::MM_EDIT );

        displayFrameInlineInfo();

#if 0
        edit->insertFloatingFrameSet( fs, i18n("Insert Picture Inline") );
        fs->finalize(); // done last since it triggers a redraw
        // Reset the 'tool'
        setTool( KWCanvas::MM_EDIT );
        m_doc->refreshDocStructure(Pictures);
#endif
    }
    else
    {
        m_gui->canvasWidget()->insertPicture( filename, isClipart, pixmapSize,_keepRatio );
    }
}

void KWView::insertInlinePicture()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if(edit)
    {
        m_doc->addFrameSet( fsInline, false ); // done first since the frame number is stored in the undo/redo
#if 0
        KWFrame *frame = new KWFrame( fsInline, 0, 0, m_doc->unzoomItX( width ), m_doc->unzoomItY( height ) );
        fsInline->addFrame( frame, false );
#endif
        edit->insertFloatingFrameSet( fsInline, i18n("Insert Picture Inline") );
        fsInline->finalize(); // done last since it triggers a redraw
        // Reset the 'tool'
        setTool( KWCanvas::MM_EDIT );
        m_doc->refreshDocStructure(Pictures);
        fsInline=0L;
        updateFrameStatusBarItem();
    }
    else
    {
        delete fsInline;
        fsInline=0L;
        updateFrameStatusBarItem();
    }
}

void KWView::displayFrameInlineInfo()
{
    KMessageBox::information(this,
                             i18n("Set cursor where you want to insert inline frame."),
                             i18n("Insert Inline frame"),
                             "SetCursorInsertInlineFrame",true);

    KStatusBar * sb = statusBar();
    if (sb )
    {
        if ( !m_sbFramesLabel )
        {
            m_sbFramesLabel = sb ? new KStatusBarLabel( QString::null, 0, sb ) : 0;
            addStatusBarItem( m_sbFramesLabel );
        }
        if(m_sbFramesLabel)
            m_sbFramesLabel->setText( i18n("Set cursor where you want to insert inline frame.") );
    }
}



void KWView::insertSpecialChar()
{
    KWTextFrameSetEdit *edit=currentTextEdit();
    if ( !edit )
        return;
    QString f = edit->textFontFamily();
    QChar c=' ';
    if (m_specialCharDlg==0)
    {
        m_specialCharDlg = new KoCharSelectDia( this, "insert special char", f, c, false );
        connect( m_specialCharDlg, SIGNAL(insertChar(QChar,const QString &)),
                 this, SLOT(slotSpecialChar(QChar,const QString &)));
        connect( m_specialCharDlg, SIGNAL( finished() ),
                 this, SLOT( slotSpecialCharDlgClosed() ) );
    }
    m_specialCharDlg->show();
}

void KWView::slotSpecialCharDlgClosed()
{
    m_specialCharDlg = 0L;
}

void KWView::slotSpecialChar(QChar c, const QString &_font)
{
    KWTextFrameSetEdit *edit=currentTextEdit();
    if ( !edit )
        return;
    edit->setFamily( _font );
    edit->insertSpecialChar(c);

}

void KWView::insertFrameBreak()
{
    KWTextFrameSetEdit *edit=currentTextEdit();
    if ( !edit )
        return;
    edit->insertFrameBreak();
}

void KWView::insertLink()
{
    KWTextFrameSetEdit *edit=currentTextEdit();
    if ( !edit )
        return;
    QString link;
    QString ref;
    if(KoInsertLinkDia::createLinkDia(link, ref))
    {
        if(!link.isEmpty() && !ref.isEmpty())
            edit->insertLink(link, ref);
    }
}

void KWView::insertVariable()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
        KAction * act = (KAction *)(sender());
        VariableDefMap::Iterator it = m_variableDefMap.find( act );
        if ( it == m_variableDefMap.end() )
            kdWarning() << "Action not found in m_variableDefMap." << endl;
        else
        {
            edit->insertVariable( (*it).type, (*it).subtype );
        }
    }
}

void KWView::insertFootNoteEndNote()
{
#if 0
    int start = m_doc->getFootNoteManager().findStart( m_gui->canvasWidget()->getCursor() );

    if ( start == -1 )
    {
        KMessageBox::sorry( this,
                            i18n( "Sorry, you can only insert footnotes or "
                                  "endnotes into the first frameset."),
                            i18n("Insert Footnote/Endnote"));
    } else {
        KWFootNoteDia dia( 0L, "", m_doc, m_gui->canvasWidget(), start,
                 m_doc->getNoteType() == KWFootNoteManager::FootNotes );
        dia.show();
    }
#endif
}

void KWView::renameButtonTOC(bool b)
{
   KActionCollection * coll = actionCollection();
   QString name= b ? i18n("Update Table of &Contents"):i18n("Table of &Contents");
   coll->action("insert_contents")->setText(name);
}

void KWView::insertContents()
{
    KWTextFrameSetEdit *edit = currentTextEdit();
    if (edit)
        edit->insertTOC();
}

void KWView::formatFont()
{
    //kdDebug(32002) << "KWView::formatFont" << endl;
    KWTextFrameSetEdit *edit = currentTextEdit();
    if (edit)
    {
        QColor col=edit->textBackgroundColor();
        col=col.isValid() ? col : QApplication::palette().color( QPalette::Active, QColorGroup::Base );
        KoFontDia *fontDia = new KoFontDia( this, "", edit->textFont(),
                                            actionFormatSub->isChecked(), actionFormatSuper->isChecked(),
                                            edit->textColor(), col );
        fontDia->exec();
        int flags = fontDia->changedFlags();
        kdDebug() << "KWView::formatFont changedFlags = " << flags << endl;
        if ( flags )
        {
            // The "change all the format" call
            edit->setFont(fontDia->getNewFont(),
                          fontDia->getSubScript(), fontDia->getSuperScript(),
                          fontDia->color(),fontDia->backGroundColor(),
                          flags);
        }

        delete fontDia;
    }
    m_gui->canvasWidget()->setFocus();
}

void KWView::formatParagraph()
{
    showParagraphDialog();
}

void KWView::showParagraphDialog( int initialPage, double initialTabPos )
{
    KWTextFrameSetEdit *edit = currentTextEdit();
    if (edit)
    {
        KoParagDia *paragDia = new KoParagDia( this, "",
                                               KoParagDia::PD_SPACING | KoParagDia::PD_ALIGN |
                                               KoParagDia::PD_BORDERS |
                                               KoParagDia::PD_NUMBERING | KoParagDia::PD_TABS | KoParagDia::PD_SHADOW, m_doc->getUnit(),edit->textFrameSet()->frame(0)->width() );
        paragDia->setCaption( i18n( "Paragraph settings" ) );

        // Initialize the dialog from the current paragraph's settings
        KWParagLayout lay = static_cast<KWTextParag *>(edit->cursor()->parag())->paragLayout();
        paragDia->setParagLayout( lay );

        // Set initial page and initial tabpos if necessary
        if ( initialPage != -1 )
        {
            paragDia->setCurrentPage( initialPage );
            if ( initialPage == KoParagDia::PD_TABS )
                paragDia->tabulatorsWidget()->setCurrentTab( initialTabPos );
        }

        if(!paragDia->exec())
            return;
        KMacroCommand * macroCommand = new KMacroCommand( i18n( "Paragraph settings" ) );
        KCommand *cmd=0L;
        bool changed=false;
        if(paragDia->isLeftMarginChanged())
        {
            cmd=edit->setMarginCommand( QStyleSheetItem::MarginLeft, paragDia->leftIndent() );
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
            m_gui->getHorzRuler()->setLeftIndent( KoUnit::ptToUnit( paragDia->leftIndent(), m_doc->getUnit() ) );

        }

        if(paragDia->isRightMarginChanged())
        {
            cmd=edit->setMarginCommand( QStyleSheetItem::MarginRight, paragDia->rightIndent() );
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
            m_gui->getHorzRuler()->setRightIndent( KoUnit::ptToUnit( paragDia->rightIndent(), m_doc->getUnit() ) );
        }
        if(paragDia->isSpaceBeforeChanged())
        {
            cmd=edit->setMarginCommand( QStyleSheetItem::MarginTop, paragDia->spaceBeforeParag() );
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
        }
        if(paragDia->isSpaceAfterChanged())
        {
            cmd=edit->setMarginCommand( QStyleSheetItem::MarginBottom, paragDia->spaceAfterParag() );
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
        }
        if(paragDia->isFirstLineChanged())
        {
            cmd=edit->setMarginCommand( QStyleSheetItem::MarginFirstLine, paragDia->firstLineIndent());
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
            m_gui->getHorzRuler()->setFirstIndent(
                KoUnit::ptToUnit( paragDia->leftIndent() + paragDia->firstLineIndent(), m_doc->getUnit() ) );
        }

        if(paragDia->isAlignChanged())
        {
            cmd=edit->setAlignCommand( paragDia->align() );
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
        }
        if(paragDia->isCounterChanged())
        {
            cmd=edit->setCounterCommand( paragDia->counter() );
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
        }
        if(paragDia->listTabulatorChanged())
        {
            cmd=edit->setTabListCommand( paragDia->tabListTabulator() );
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
        }

        if(paragDia->isLineSpacingChanged())
        {
            cmd=edit->setLineSpacingCommand( paragDia->lineSpacing() );
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
        }
        if(paragDia->isBorderChanged())
        {
            cmd=edit->setBordersCommand( paragDia->leftBorder(), paragDia->rightBorder(),
                              paragDia->topBorder(), paragDia->bottomBorder() );
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
        }
        if ( paragDia->isPageBreakingChanged() )
        {
            cmd=edit->setPageBreakingCommand( paragDia->pageBreaking() );
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
        }

        if( paragDia->isShadowChanged())
        {
            cmd=edit->setShadowCommand( paragDia->shadowDistance(),paragDia->shadowDirection(), paragDia->shadowColor() );
            if(cmd)
            {
                macroCommand->addCommand(cmd);
                changed=true;
            }
        }
        if(changed)
            m_doc->addCommand(macroCommand);
        else
            delete macroCommand;
        delete paragDia;
    }

}

void KWView::slotHRulerDoubleClicked( double ptpos )
{
    showParagraphDialog( KoParagDia::PD_TABS, ptpos );
}

void KWView::slotHRulerDoubleClicked()
{
    if ( m_gui->getHorzRuler()->flags() & KoRuler::F_TABS )
        formatParagraph();
    else
        formatPage();
}

void KWView::formatPage()
{
    if( !m_doc->isReadWrite())
        return;
    KoPageLayout pgLayout;
    KoColumns cl;
    KoKWHeaderFooter kwhf;
    m_doc->getPageLayout( pgLayout, cl, kwhf );

    pageLayout tmpOldLayout;
    tmpOldLayout._pgLayout=pgLayout;
    tmpOldLayout._cl=cl;
    tmpOldLayout._hf=kwhf;

    KoHeadFoot hf;
    int flags = FORMAT_AND_BORDERS | KW_HEADER_AND_FOOTER | DISABLE_UNIT;
    if ( m_doc->processingType() == KWDocument::WP )
        flags |= COLUMNS;
    else
        flags |= DISABLE_BORDERS;

    KoUnit::Unit unit = m_doc->getUnit();
    KoUnit::Unit oldUnit = unit;

    if ( KoPageLayoutDia::pageLayout( pgLayout, hf, cl, kwhf, flags, unit ) ) {
        if( !(tmpOldLayout._pgLayout==pgLayout)||
            tmpOldLayout._cl.columns!=cl.columns ||
            tmpOldLayout._cl.ptColumnSpacing!=cl.ptColumnSpacing||
            tmpOldLayout._hf.header!=kwhf.header||
            tmpOldLayout._hf.footer!=kwhf.footer||
            tmpOldLayout._hf.ptHeaderBodySpacing != kwhf.ptHeaderBodySpacing ||
            tmpOldLayout._hf.ptFooterBodySpacing != kwhf.ptFooterBodySpacing)
        {
            pageLayout tmpNewLayout;
            tmpNewLayout._pgLayout=pgLayout;
            tmpNewLayout._cl=cl;
            tmpNewLayout._hf=kwhf;

            KWTextFrameSetEdit *edit = currentTextEdit();
            if (edit)
                edit->textFrameSet()->clearUndoRedoInfo();
            KWPageLayoutCommand *cmd =new KWPageLayoutCommand( i18n("Change Layout"),m_doc,tmpOldLayout,tmpNewLayout ) ;
            m_doc->addCommand(cmd);

            m_doc->setPageLayout( pgLayout, cl, kwhf );
            m_doc->updateRuler();

            m_doc->updateResizeHandles();
            m_doc->updateContentsSize();
#if 0
            m_gui->canvasWidget()->frameSizeChanged( pgLayout );
#endif
        }
        if ( unit != oldUnit )
            m_doc->setUnit( unit ); // needs undo/redo support
    }
}

void KWView::formatFrameSet()
{
    if ( m_doc->getFirstSelectedFrame() )
    {
        m_gui->canvasWidget()->editFrameProperties();
    }
    else // Should never happen, the action is disabled
        KMessageBox::sorry( this,
                            i18n("Sorry, you have to select a frame first."),
                            i18n("Format Frameset"));
}

void KWView::extraSpelling()
{
    if (m_spell.kspell) return; // Already in progress
    m_doc->setReadWrite(false); // prevent editing text
    m_spell.spellCurrFrameSetNum = -1;
    m_spell.macroCmdSpellCheck=0L;

    m_spell.textFramesets.clear();
     for ( unsigned int i = 0; i < m_doc->getNumFrameSets(); i++ ) {
        KWFrameSet *frameset = m_doc->frameSet( i );
        frameset->addTextFrameSets(m_spell.textFramesets);
     }
    startKSpell();
}

void KWView::extraAutoFormat()
{
    m_doc->getAutoFormat()->readConfig();
    KoAutoFormatDia dia( this, 0, m_doc->getAutoFormat() );
    dia.exec();
}

void KWView::extraStylist()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->hideCursor();
    KWStyleManager * styleManager = new KWStyleManager( this, m_doc->getUnit(),m_doc, m_doc->styleList());
    styleManager->exec();
    delete styleManager;
    if ( edit )
        edit->showCursor();
}

void KWView::extraCreateTemplate()
{
    int width = 60;
    int height = 60;
    double ratio = m_doc->ptPaperHeight() / m_doc->ptPaperWidth();
    if ( ratio > 1 )
        width = qRound( 60 / ratio );
    else
        height = qRound( 60 / ratio );
    double zoom = (double)width / m_doc->ptPaperWidth();
    int oldZoom = m_doc->zoom();
    setZoom( qRound( 100 * zoom ), false );
    kdDebug() << "KWView::extraCreateTemplate ratio=" << ratio << " preview size: " << width << "," << height
              << " zoom:" << zoom << endl;
    QPixmap pix( width, height );
    pix.fill( Qt::white );
    QPainter painter;
    painter.begin( &pix );
    QRect pageRect( 0, 0, m_doc->paperWidth(), m_doc->paperHeight() );

    KWViewModeNormal * viewMode = new KWViewModeNormal( m_doc );
    QColorGroup cg = QApplication::palette().active();

    // Draw all framesets contents
    QPtrListIterator<KWFrameSet> fit = m_doc->framesetsIterator();
    for ( ; fit.current() ; ++fit )
    {
        KWFrameSet * frameset = fit.current();
        if ( frameset->isVisible() && !frameset->isFloating() )
            frameset->drawContents( &painter, pageRect, cg, false, false, 0L, viewMode, 0L );
    }

    painter.end();
    delete viewMode;
    setZoom( oldZoom, false );

    KTempFile tempFile( QString::null, ".kwt" );
    tempFile.setAutoDelete(true);

    m_doc->saveNativeFormat( tempFile.name() );

    KoTemplateCreateDia::createTemplate( "kword_template", KWFactory::global(),
                                         tempFile.name(), pix, this );

    KWFactory::global()->dirs()->addResourceType("kword_template",
                                                    KStandardDirs::kde_default( "data" ) +
                                                    "kword/templates/");
}

void KWView::toolsCreateText()
{
    if ( actionToolsCreateText->isChecked() )
        m_gui->canvasWidget()->setMouseMode( KWCanvas::MM_CREATE_TEXT );
    else
        actionToolsCreateText->setChecked( true ); // always one has to be checked !
}

void KWView::insertTable()
{
    KWCanvas * canvas = m_gui->canvasWidget();
    KWTableDia *tableDia = new KWTableDia( this, 0, canvas, m_doc,
                                           canvas->tableRows(),
                                           canvas->tableCols(),
                                           canvas->tableWidthMode(),
                                           canvas->tableHeightMode(),
                                           canvas->tableIsFloating() );
    tableDia->setCaption( i18n( "Insert Table" ) );
    if ( tableDia->exec() == QDialog::Rejected )
        canvas->setMouseMode( KWCanvas::MM_EDIT );
    delete tableDia;
}

void KWView::insertFormula( QMimeSource* source )
{
    KWTextFrameSetEdit *edit = currentTextEdit();
    if (edit)
    {
        KWFormulaFrameSet *frameset = new KWFormulaFrameSet( m_doc, QString::null );
        m_doc->addFrameSet( frameset, false ); // done first since the frame number is stored in the undo/redo
        KWFrame *frame = new KWFrame(frameset, 0, 0, 10, 10 );
        frameset->addFrame( frame, false );
        if ( source ) {
            QByteArray data = source->encodedData( KFormula::MimeSource::selectionMimeType() );
            QDomDocument formula;
            formula.setContent( data );
            frameset->paste( formula );
        }
        edit->insertFloatingFrameSet( frameset, i18n("Insert Formula") );
        frameset->finalize(); // done last since it triggers a redraw

        // Strange, seems we need this - hmm, do we, still ?
        // There was a bug in KWFormulaFrameSet::slotFormulaChanged that could
        // have been the cause of this. Maybe we don't need this any longer.
        //edit->cursor()->parag()->invalidate( 0 ); // and that's done by KWTextParag::setCustomItem. Hmm.
        //edit->cursor()->parag()->setChanged( true );
        //m_doc->slotRepaintChanged( edit->frameSet() );
        m_doc->refreshDocStructure(FT_FORMULA);
    }
}

void KWView::toolsPart()
{
    /*if ( !actionToolsCreatePart->isChecked() )
    {
        actionToolsCreatePart->setChecked( true ); // always one has to be checked !
        return;
        }*/
    m_gui->canvasWidget()->insertPart( actionToolsCreatePart->documentEntry() );
}

void KWView::tableInsertRow()
{
    m_gui->canvasWidget()->setMouseMode( KWCanvas::MM_EDIT );
    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    Q_ASSERT(table);
    KWInsertDia dia( this, "", table, m_doc, KWInsertDia::ROW, m_gui->canvasWidget() );
    dia.setCaption( i18n( "Insert Row" ) );
    dia.exec();
}

void KWView::tableInsertCol()
{
    m_gui->canvasWidget()->setMouseMode( KWCanvas::MM_EDIT );
    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    Q_ASSERT(table);
    // value = 62 because a insert column = 60 +2 (border )see kwtableframeset.cc
    if ( table->boundingRect().right() + 62 > static_cast<int>( m_doc->ptPaperWidth() ) )
    {
        KMessageBox::sorry( this,
                            i18n( "There is not enough space at the right of the table "
                                  "to insert a new column." ),
                            i18n( "Insert Column" ) );
    }
    else
    {
        KWInsertDia dia( this, "", table, m_doc, KWInsertDia::COL, m_gui->canvasWidget() );
        dia.setCaption( i18n( "Insert Column" ) );
        dia.exec();
    }
}

void KWView::tableDeleteRow()
{
    m_gui->canvasWidget()->setMouseMode( KWCanvas::MM_EDIT );

    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    Q_ASSERT(table);
    if ( table->getRows() == 1 )
    {
        int result;
        result = KMessageBox::warningContinueCancel(this,
                                                    i18n("The table has only one row. "
                                                         "Deleting this row will delete the table.\n"
                                                         "Do you want to delete the table?"),
                                                    i18n("Delete Row"),
                                                    i18n("&Delete"));
        if (result == KMessageBox::Continue)
        {
            m_doc->deleteTable( table );
            m_gui->canvasWidget()->emitFrameSelectedChanged();
        }
    }
    else
    {
        KWDeleteDia dia( this, "", table, m_doc, KWDeleteDia::ROW, m_gui->canvasWidget() );
        dia.setCaption( i18n( "Delete Row" ) );
        dia.exec();
    }

}

void KWView::tableDeleteCol()
{
    m_gui->canvasWidget()->setMouseMode( KWCanvas::MM_EDIT );

    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    Q_ASSERT(table);
    if ( table->getCols() == 1 )
    {
        int result;
        result = KMessageBox::warningContinueCancel(this,
                                                    i18n("The table has only one column. "
                                                         "Deleting this column will delete the table.\n"
                                                         "Do you want to delete the table?"),
                                                    i18n("Delete Column"),
                                                    i18n("&Delete"));
        if (result == KMessageBox::Continue)
        {
            m_doc->deleteTable( table );
            m_gui->canvasWidget()->emitFrameSelectedChanged();
        }
    }
    else
    {
        KWDeleteDia dia( this, "", table, m_doc, KWDeleteDia::COL, m_gui->canvasWidget() );
        dia.setCaption( i18n( "Delete Column" ) );
        dia.exec();
    }
}

void KWView::tableJoinCells()
{
    //m_gui->canvasWidget()->setMouseMode( KWCanvas::MM_EDIT_FRAME );

    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    Q_ASSERT(table);
    KCommand * cmd=table->joinCells();
    if ( !cmd )
    {
        KMessageBox::sorry( this,
                            i18n( "You have to select some cells which are next to each other "
                                  "and are not already joined." ),
                            i18n( "Join Cells" ) );
        return;
    }
    m_doc->addCommand(cmd);
    m_doc->layout();
    //KoRect r = m_doc->zoomRect( table->boundingRect() );
    //m_gui->canvasWidget()->repaintScreen( r, TRUE );
    m_gui->canvasWidget()->repaintAll();
    m_gui->canvasWidget()->emitFrameSelectedChanged();
}

void KWView::tableSplitCells() {
    KWSplitCellDia *splitDia=new KWSplitCellDia( this,"split cell",m_gui->canvasWidget()->splitCellRows(),m_gui->canvasWidget()->splitCellCols() );
    if(splitDia->exec()) {
        unsigned int nbCols=splitDia->cols();
        unsigned int nbRows=splitDia->rows();
        m_gui->canvasWidget()->setSplitCellRows(nbRows);
        m_gui->canvasWidget()->setSplitCellCols(nbCols);
        tableSplitCells(nbCols, nbRows);
    }
    delete splitDia;
}

void KWView::tableSplitCells(int cols, int rows)
{
    //m_gui->canvasWidget()->setMouseMode( KWCanvas::MM_EDIT_FRAME );

    QPtrList <KWFrame> selectedFrames = m_doc->getSelectedFrames();
    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    if ( !table && selectedFrames.count() > 0) {
        table=selectedFrames.at(0)->frameSet()->getGroupManager();
    }

    if(selectedFrames.count() >1 || table == 0) {
        KMessageBox::sorry( this,
                            i18n( "You have to put the cursor into a table "
                                  "before splitting cells." ),
                            i18n( "Split Cells" ) );
        return;
    }

    //int rows=1, cols=2;
    KCommand *cmd=table->splitCell(rows,cols);
    if ( !cmd ) {
        KMessageBox::sorry( this,
                            i18n("There is not enough space to split the cell into that many parts, make it bigger first"),
                            i18n("Split Cells") );
        return;
    }
    m_doc->addCommand(cmd);
    //KoRect r = m_doc->zoomRect( table->boundingRect() );
    //m_gui->canvasWidget()->repaintScreen( r, TRUE );
    m_doc->updateAllFrames();
    m_doc->layout();
    m_gui->canvasWidget()->repaintAll();
    m_doc->frameSelectedChanged();
}

void KWView::tableUngroupTable()
{
    m_gui->canvasWidget()->setMouseMode( KWCanvas::MM_EDIT );

    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    Q_ASSERT(table);

    // Use a macro command because we may have to make the table non-floating first
    KMacroCommand * macroCmd = new KMacroCommand( i18n( "Ungroup Table" ) );

    if ( table->isFloating() )
    {
        KWFrameSetPropertyCommand *cmd = new KWFrameSetPropertyCommand( QString::null, table, KWFrameSetPropertyCommand::FSP_FLOATING, "false" );
        macroCmd->addCommand(cmd);
    }

    KWUngroupTableCommand *cmd = new KWUngroupTableCommand( QString::null, table );
    macroCmd->addCommand( cmd );
    m_doc->addCommand( macroCmd );
    macroCmd->execute(); // do it all
}

void KWView::tableDelete()
{
    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    Q_ASSERT(table);
    m_doc->deleteTable( table );
    m_gui->canvasWidget()->emitFrameSelectedChanged();
}

void KWView::textStyleSelected( int index )
{
    if(m_gui->canvasWidget()->currentFrameSetEdit())
    {
        KWTextFrameSetEdit * edit = dynamic_cast<KWTextFrameSetEdit *>(m_gui->canvasWidget()->currentFrameSetEdit()->currentTextEdit());
        if ( edit )
            edit->applyStyle( m_doc->styleAt( index ) );
        m_gui->canvasWidget()->setFocus(); // the combo keeps focus...
    }
}

void KWView::increaseFontSize()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    KoTextFormat *format = edit->currentFormat();
    if ( edit )
        edit->setPointSize( format->font().pointSize() + 1 );
}

void KWView::decreaseFontSize()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    KoTextFormat *format = edit->currentFormat();
    if ( edit && format->font().pointSize() > 1)
        edit->setPointSize( format->font().pointSize() - 1 );
}

void KWView::textSizeSelected( int size )
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setPointSize( size );
    m_gui->canvasWidget()->setFocus(); // the combo keeps focus...
}

void KWView::textFontSelected( const QString & font )
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setFamily( font );
    m_gui->canvasWidget()->setFocus(); // the combo keeps focus...
}

void KWView::textBold()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setBold(actionFormatBold->isChecked());
}

void KWView::textItalic()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setItalic(actionFormatItalic->isChecked());

}

void KWView::textUnderline()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setUnderline(actionFormatUnderline->isChecked());
}

void KWView::textStrikeOut()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setStrikeOut(actionFormatStrikeOut->isChecked());
}

void KWView::textColor()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
/*        QColor color = edit->textColor();
        if ( KColorDialog::getColor( color ) ) {
            actionFormatColor->setColor( color );
            edit->setTextColor( color );
        }
*/
        edit->setTextColor( actionFormatColor->color() );
    }
}

void KWView::textAlignLeft()
{
    if ( actionFormatAlignLeft->isChecked() )
    {
        KWTextFrameSetEdit * edit = currentTextEdit();
        if ( edit )
        {
            KCommand *cmd=edit->setAlignCommand(Qt::AlignLeft);
            if(cmd)
                m_doc->addCommand(cmd);
        }
    } else
        actionFormatAlignLeft->setChecked( true );
}

void KWView::textAlignCenter()
{
    if ( actionFormatAlignCenter->isChecked() )
    {
        KWTextFrameSetEdit * edit = currentTextEdit();
        if ( edit )
        {
            KCommand *cmd=edit->setAlignCommand(Qt::AlignCenter);
            if(cmd)
                m_doc->addCommand(cmd);
        }
    } else
        actionFormatAlignCenter->setChecked( true );
}

void KWView::textAlignRight()
{
    if ( actionFormatAlignRight->isChecked() )
    {
        KWTextFrameSetEdit * edit = currentTextEdit();
        if ( edit )
        {
            KCommand *cmd=edit->setAlignCommand(Qt::AlignRight);
            if(cmd)
                m_doc->addCommand(cmd);
        }
    } else
        actionFormatAlignRight->setChecked( true );
}

void KWView::textAlignBlock()
{
    if ( actionFormatAlignBlock->isChecked() )
    {
        KWTextFrameSetEdit * edit = currentTextEdit();
        if ( edit )
        {
            KCommand *cmd=edit->setAlignCommand(Qt::AlignJustify);
            if(cmd)
                m_doc->addCommand(cmd);
        }
    } else
        actionFormatAlignBlock->setChecked( true );
}

void KWView::textList()
{
    KoParagCounter c;
    if ( actionFormatList->isChecked() )
    {
        c.setNumbering( KoParagCounter::NUM_LIST );
        c.setStyle( KoParagCounter::STYLE_NUM );
    }
    else
    {
        c.setNumbering( KoParagCounter::NUM_NONE );
    }
    KWTextFrameSetEdit * edit = currentTextEdit();
    Q_ASSERT(edit);
    if ( edit )
    {
        KCommand *cmd=edit->setCounterCommand( c );
        if(cmd)
            m_doc->addCommand(cmd);
    }
}

void KWView::textSuperScript()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setTextSuperScript(actionFormatSuper->isChecked());
}

void KWView::textSubScript()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setTextSubScript(actionFormatSub->isChecked());
}

void KWView::changeCaseOfText()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if(!edit)
        return;
    KoChangeCaseDia *caseDia=new KoChangeCaseDia( this,"change case" );
    if(caseDia->exec())
    {
        edit->changeCaseOfText(caseDia->getTypeOfCase());
    }
    delete caseDia;
}

void KWView::editPersonalExpr()
{
   KWEditPersonnalExpression *personalDia=new KWEditPersonnalExpression( this );
   if(personalDia->exec())
       m_doc->refreshMenuExpression();
   delete personalDia;
}


void KWView::textIncreaseIndent()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
        double leftMargin = edit->currentParagLayout().margins[QStyleSheetItem::MarginLeft];
        double indent = m_doc->indentValue();
        double newVal = leftMargin + indent;
        // Test commented out. This breaks with the DTP case... The user can put
        // a frame anywhere, even closer to the edges than left/right border allows (DF).
        //if( newVal <= (m_doc->ptPaperWidth()-m_doc->ptRightBorder()-m_doc->ptLeftBorder()))
        {
            KCommand *cmd=edit->setMarginCommand( QStyleSheetItem::MarginLeft, newVal );
            if(cmd)
                m_doc->addCommand(cmd);
        }
    }
}

void KWView::textDecreaseIndent()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
        double leftMargin = edit->currentParagLayout().margins[QStyleSheetItem::MarginLeft];
        if ( leftMargin > 0 )
        {
            double indent = m_doc->indentValue();
            double newVal = leftMargin - indent;
            KCommand *cmd=edit->setMarginCommand( QStyleSheetItem::MarginLeft, QMAX( newVal, 0 ) );
            if(cmd)
                m_doc->addCommand(cmd);
        }
    }
}


void KWView::textDefaultFormat()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->setDefaultFormat();
}


void KWView::borderOutline()
{
    bool b = actionBorderOutline->isChecked();

    actionBorderLeft->setChecked(b);
    actionBorderRight->setChecked(b);
    actionBorderTop->setChecked(b);
    actionBorderBottom->setChecked(b);

    borderSet();
}

void KWView::borderLeft()
{
    actionBorderOutline->setChecked(
        actionBorderLeft->isChecked() &&
        actionBorderRight->isChecked() &&
        actionBorderTop->isChecked() &&
        actionBorderBottom->isChecked());

    borderSet();
}

void KWView::borderRight()
{
    actionBorderOutline->setChecked(
        actionBorderLeft->isChecked() &&
        actionBorderRight->isChecked() &&
        actionBorderTop->isChecked() &&
        actionBorderBottom->isChecked());

    borderSet();
}

void KWView::borderTop()
{
    actionBorderOutline->setChecked(
        actionBorderLeft->isChecked() &&
        actionBorderRight->isChecked() &&
        actionBorderTop->isChecked() &&
        actionBorderBottom->isChecked());

    borderSet();
}

void KWView::borderBottom()
{
    actionBorderOutline->setChecked(
        actionBorderLeft->isChecked() &&
        actionBorderRight->isChecked() &&
        actionBorderTop->isChecked() &&
        actionBorderBottom->isChecked());

    borderSet();
}

void KWView::borderColor()
{
    m_border.common.color = actionBorderColor->color();
    m_border.left.color = m_border.common.color;
    m_border.right.color = m_border.common.color;
    m_border.top.color = m_border.common.color;
    m_border.bottom.color = m_border.common.color;
    borderSet();
}

void KWView::borderWidth( const QString &width )
{
    m_border.common.ptWidth = width.toInt();
    m_border.left.ptWidth = m_border.common.ptWidth;
    m_border.right.ptWidth = m_border.common.ptWidth;
    m_border.top.ptWidth = m_border.common.ptWidth;
    m_border.bottom.ptWidth = m_border.common.ptWidth;
    borderSet();
    m_gui->canvasWidget()->setFocus();
}

void KWView::borderStyle( const QString &style )
{
    m_border.common.style = KoBorder::getStyle( style );
    m_border.left.style = m_border.common.style;
    m_border.right.style = m_border.common.style;
    m_border.top.style = m_border.common.style;
    m_border.bottom.style = m_border.common.style;
    borderSet();
    m_gui->canvasWidget()->setFocus();
}

void KWView::backgroundColor()
{
    QColor backColor = actionBackgroundColor->color();
    KWTextFrameSetEdit *edit = currentTextEdit();
    if ( m_gui)
    {
        if(edit)
            edit->setTextBackgroundColor(backColor);
        else
            m_gui->canvasWidget()->setFrameBackgroundColor( backColor );
    }
}


void KWView::borderSet()
{
    // The effect of this action depends on if we are in Edit Text or Edit Frame mode.

    m_border.left = m_border.common;
    m_border.right = m_border.common;
    m_border.top = m_border.common;
    m_border.bottom = m_border.common;
    if ( !actionBorderLeft->isChecked() )
    {
        m_border.left.ptWidth = 0;
    }
    if ( !actionBorderRight->isChecked() )
    {
        m_border.right.ptWidth = 0;
    }
    if ( !actionBorderTop->isChecked() )
    {
        m_border.top.ptWidth = 0;
    }
    if ( !actionBorderBottom->isChecked() )
    {
        m_border.bottom.ptWidth = 0;
    }
    KWTextFrameSetEdit *edit = currentTextEdit();
    if ( edit )
    {
        KCommand *cmd=edit->setBordersCommand( m_border.left, m_border.right, m_border.top, m_border.bottom );
        if(cmd)
            m_doc->addCommand(cmd);
    }
    else
    {
        m_gui->canvasWidget()->setLeftFrameBorder( m_border.common, actionBorderLeft->isChecked() );
        m_gui->canvasWidget()->setRightFrameBorder( m_border.common, actionBorderRight->isChecked() );
        m_gui->canvasWidget()->setTopFrameBorder( m_border.common, actionBorderTop->isChecked() );
        m_gui->canvasWidget()->setBottomFrameBorder( m_border.common, actionBorderBottom->isChecked() );
    }
}

void KWView::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    if ( m_gui ) m_gui->resize( width(), height() );
}

void KWView::guiActivateEvent( KParts::GUIActivateEvent *ev )
{
    if ( ev->activated() )
    {
        initGui();
    }
    KoView::guiActivateEvent( ev );
}

void KWView::borderShowValues()
{
    actionBorderWidth->setCurrentItem( (int)m_border.common.ptWidth - 1 );
    actionBorderStyle->setCurrentItem( (int)m_border.common.style );
}

void KWView::tabListChanged( const KoTabulatorList & tabList )
{
    if(!m_doc->isReadWrite())
        return;
    KWTextFrameSetEdit * edit = currentTextEdit();
    if (!edit)
        return;
    KCommand *cmd=edit->setTabListCommand( tabList );
    if(cmd)
        m_doc->addCommand(cmd);
}

void KWView::newPageLayout( KoPageLayout _layout )
{
    KoPageLayout pgLayout;
    KoColumns cl;
    KoKWHeaderFooter hf;
    m_doc->getPageLayout( pgLayout, cl, hf );

    if(_layout==pgLayout)
        return;

    pageLayout tmpOldLayout;
    tmpOldLayout._pgLayout=pgLayout;
    tmpOldLayout._cl=cl;
    tmpOldLayout._hf=hf;

    m_doc->setPageLayout( _layout, cl, hf );

    pageLayout tmpNewLayout;
    tmpNewLayout._pgLayout=_layout;
    tmpNewLayout._cl=cl;
    tmpNewLayout._hf=hf;

    KWTextFrameSetEdit *edit = currentTextEdit();
    if (edit)
        edit->textFrameSet()->clearUndoRedoInfo();
    KWPageLayoutCommand *cmd = new KWPageLayoutCommand( i18n("Change Layout"),m_doc,tmpOldLayout,tmpNewLayout ) ;
    m_doc->addCommand(cmd);

    m_doc->updateRuler();

    m_doc->updateResizeHandles();
}

void KWView::newFirstIndent( double _firstIndent )
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if (!edit) return;
    double val = _firstIndent - edit->currentParagLayout().margins[QStyleSheetItem::MarginLeft];
    KCommand *cmd=edit->setMarginCommand( QStyleSheetItem::MarginFirstLine, val );
    if(cmd)
        m_doc->addCommand(cmd);
}

void KWView::newLeftIndent( double _leftIndent)
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if (edit)
    {
        KCommand *cmd=edit->setMarginCommand( QStyleSheetItem::MarginLeft, _leftIndent );
        if(cmd)
            m_doc->addCommand(cmd);
    }
}

void KWView::newRightIndent( double _rightIndent)
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if (edit)
    {
        KCommand *cmd=edit->setMarginCommand( QStyleSheetItem::MarginRight, _rightIndent );
        if(cmd)
            m_doc->addCommand(cmd);
    }
}

QPopupMenu * KWView::popupMenu( const QString& name )
{
    Q_ASSERT(factory());
    if ( factory() )
        return ((QPopupMenu*)factory()->container( name, this ));
    return 0L;
}

void KWView::openPopupMenuInsideFrame( KWFrame* frame, const QPoint & _point )
{
    frame->frameSet()->showPopup( frame, m_gui->canvasWidget()->currentFrameSetEdit(),
                                     this, _point );
}

void KWView::openPopupMenuChangeAction( const QPoint & _point )
{
    if(!koDocument()->isReadWrite() )
        return;
    ((QPopupMenu*)factory()->container("action_popup",this))->popup(_point);
}

// ### DF: I don't see the purpose of that method. frameSelectedChanged() does this already.
void KWView::updatePopupMenuChangeAction()
{
    KWFrame *frame=m_doc->getFirstSelectedFrame();
    // Warning, frame can be 0L !

    // if a header/footer etc. Dont show the popup.
    if(frame && frame->frameSet() && frame->frameSet()->frameSetInfo() != KWFrameSet::FI_BODY)
        return;
    actionEditDelFrame->setEnabled(true );

    actionInlineFrame->setEnabled(true);

    // if text frame,
    if(frame && frame->frameSet() && frame->frameSet()->type() == FT_TEXT)
        {
            // if frameset 0 disable delete
            if(m_doc->processingType()  == KWDocument::WP && frame->frameSet() == m_doc->frameSet(0))
                {
                    actionEditDelFrame->setEnabled(false);
                    actionInlineFrame->setEnabled(false);
                }
        }
}

void KWView::openPopupMenuEditFrame( const QPoint & _point )
{
    if(!koDocument()->isReadWrite() )
        return;
    updatePopupMenuChangeAction();
    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    if(!table)
    {
        QPtrList<KAction> actionList= QPtrList<KAction>();

        int nbFrame=m_doc->getSelectedFrames().count();
        KActionSeparator *separator=new KActionSeparator();
        KActionSeparator *separator2=new KActionSeparator();
        if(nbFrame ==1)
        {
            KWFrame *frame=m_doc->getFirstSelectedFrame();
            KWFrameSet *frameSet=frame->frameSet();
            if(frameSet->type()==FT_PICTURE)
            {
                actionList.append(separator);
                actionList.append(actionChangePicture);
            }
            else if(frameSet->type()==FT_CLIPART)
            {
                actionList.append(separator);
                actionList.append(actionChangeClipart);
            }
            else if(frameSet->isHeaderOrFooter())
            {
                actionList.append(separator);
                actionList.append(actionConfigureHeaderFooter);
            }

            bool state = !frameSet->isHeaderOrFooter();
            state = state && (m_doc->processingType() == KWDocument::WP &&frameSet!=m_doc->frameSet( 0 ));
            if(state)
            {
                actionList.append(separator2);
                KWFrameSet * parentFs = frameSet->getGroupManager() ? frameSet->getGroupManager() : frameSet;
                actionInlineFrame->setChecked(parentFs->isFloating());
                actionList.append(actionInlineFrame);
            }
        }
        plugActionList( "picture_action", actionList );
        ((QPopupMenu*)factory()->container("frame_popup",this))->exec(_point);
        unplugActionList( "picture_action" );
        delete separator;
        delete separator2;
    }
    else
        ((QPopupMenu*)factory()->container("frame_popup_table",this))->popup(_point);
}

void KWView::startKSpell()
{
    // m_spellCurrFrameSetNum is supposed to be set by the caller of this method
    if(m_doc->getKSpellConfig() && !m_ignoreWord.isEmpty())
        m_doc->getKSpellConfig()->setIgnoreList(m_ignoreWord);
    m_spell.kspell = new KSpell( this, i18n( "Spell Checking" ), this, SLOT( spellCheckerReady() ), m_doc->getKSpellConfig() );


     m_spell.kspell->setIgnoreUpperWords(m_doc->dontCheckUpperWord());
     m_spell.kspell->setIgnoreTitleCase(m_doc->dontCheckTitleCase());

    QObject::connect( m_spell.kspell, SIGNAL( death() ),
                      this, SLOT( spellCheckerFinished() ) );
    QObject::connect( m_spell.kspell, SIGNAL( misspelling( const QString &, const QStringList &, unsigned int ) ),
                      this, SLOT( spellCheckerMisspelling( const QString &, const QStringList &, unsigned int ) ) );
    QObject::connect( m_spell.kspell, SIGNAL( corrected( const QString &, const QString &, unsigned int ) ),
                      this, SLOT( spellCheckerCorrected( const QString &, const QString &, unsigned int ) ) );
    QObject::connect( m_spell.kspell, SIGNAL( done( const QString & ) ),
                      this, SLOT( spellCheckerDone( const QString & ) ) );
}

void KWView::spellCheckerReady()
{
    for ( unsigned int i = m_spell.spellCurrFrameSetNum + 1; i < m_spell.textFramesets.count(); i++ ) {
        KWTextFrameSet *textfs = m_spell.textFramesets.at( i );
        if(!textfs->isVisible())
            continue;
        m_spell.spellCurrFrameSetNum = i; // store as number, not as pointer, to implement "go to next frameset" when done
        //kdDebug() << "KWView::spellCheckerReady spell-checking frameset " << m_spellCurrFrameSetNum << endl;

        Qt3::QTextParag * p = textfs->textDocument()->firstParag();
        QString text;
        bool textIsEmpty=true;
        while ( p ) {
            QString str = p->string()->toString();
            str.truncate( str.length() - 1 ); // damn trailing space
            if(textIsEmpty)
                textIsEmpty=str.isEmpty();
            text += str + '\n';
            p = p->next();
        }
        if(textIsEmpty)
            continue;
        text += '\n';
        m_spell.kspell->check( text );
        return;
    }
    //kdDebug() << "KWView::spellCheckerReady done" << endl;

    // Done
    m_doc->setReadWrite(true);
    m_spell.kspell->cleanUp();
    delete m_spell.kspell;
    m_spell.kspell = 0;
    m_spell.textFramesets.clear();
    m_spell.ignoreWord.clear();
    if(m_spell.macroCmdSpellCheck)
        m_doc->addCommand(m_spell.macroCmdSpellCheck);
}

void KWView::spellCheckerMisspelling( const QString &old, const QStringList &, unsigned int pos )
{
    //kdDebug() << "KWView::spellCheckerMisspelling old=" << old << " pos=" << pos << endl;
    KWTextFrameSet * fs = m_spell.textFramesets.at( m_spell.spellCurrFrameSetNum ) ;
    Q_ASSERT( fs );
    if ( !fs ) return;
    Qt3::QTextParag * p = fs->textDocument()->firstParag();
    while ( p && (int)pos >= p->length() )
    {
        pos -= p->length();
        p = p->next();
    }
    Q_ASSERT( p );
    if ( !p ) return;
    //kdDebug() << "KWView::spellCheckerMisspelling p=" << p->paragId() << " pos=" << pos << " length=" << old.length() << endl;
    fs->highlightPortion( p, pos, old.length(), m_gui->canvasWidget() );
}

void KWView::spellCheckerCorrected( const QString &old, const QString &corr, unsigned int pos )
{
    //kdDebug() << "KWView::spellCheckerCorrected old=" << old << " corr=" << corr << " pos=" << pos << endl;

    KWTextFrameSet * fs = m_spell.textFramesets.at( m_spell.spellCurrFrameSetNum ) ;
    Q_ASSERT( fs );
    if ( !fs ) return;
    Qt3::QTextParag * p = fs->textDocument()->firstParag();
    while ( p && (int)pos >= p->length() )
    {
        pos -= p->length();
        p = p->next();
    }
    Q_ASSERT( p );
    if ( !p ) return;
    fs->highlightPortion( p, pos, old.length(), m_gui->canvasWidget() );

    QTextCursor cursor( fs->textDocument() );
    cursor.setParag( p );
    cursor.setIndex( pos );
    if(!m_spell.macroCmdSpellCheck)
        m_spell.macroCmdSpellCheck=new KMacroCommand(i18n("Correct misspelled word"));
    m_spell.macroCmdSpellCheck->addCommand(fs->textObject()->replaceSelectionCommand(
        &cursor, corr, KoTextObject::HighlightSelection, QString::null ));
}

void KWView::spellCheckerDone( const QString & )
{
    KWTextFrameSet * fs = m_spell.textFramesets.at( m_spell.spellCurrFrameSetNum ) ;
    Q_ASSERT( fs );
    if ( fs )
        fs->removeHighlight();

    int result = m_spell.kspell->dlgResult();

    //store ignore word
    m_spell.ignoreWord=m_spell.kspell->ksConfig().ignoreList ();

    m_spell.kspell->cleanUp();
    delete m_spell.kspell;
    m_spell.kspell = 0;

    if ( result != KS_CANCEL && result != KS_STOP )
    {
        // Try to check another frameset
        startKSpell();
    }
    else
    {
        m_doc->setReadWrite(true);
        m_spell.textFramesets.clear();
        m_ignoreWord.clear();
        if(m_spell.macroCmdSpellCheck)
            m_doc->addCommand(m_spell.macroCmdSpellCheck);
    }
}

void KWView::spellCheckerFinished()
{
    KSpell::spellStatus status = m_spell.kspell->status();
    delete m_spell.kspell;
    m_spell.kspell = 0;
    bool kspellNoConfigured=false;
    if (status == KSpell::Error)
    {
        KMessageBox::sorry(this, i18n("ISpell could not be started.\n"
                                      "Please make sure you have ISpell properly configured and in your PATH."));
        kspellNoConfigured=true;
    }
    else if (status == KSpell::Crashed)
    {
        KMessageBox::sorry(this, i18n("ISpell seems to have crashed."));
    }
    KWTextFrameSet * fs = 0L;
    if(m_spell.spellCurrFrameSetNum!=-1)
    {
        fs = m_spell.textFramesets.at( m_spell.spellCurrFrameSetNum ) ;
        Q_ASSERT( fs );
        if ( fs )
            fs->removeHighlight();
    }
    m_doc->setReadWrite(true);
    m_spell.textFramesets.clear();
    m_ignoreWord.clear();
    if(m_spell.macroCmdSpellCheck)
        m_doc->addCommand(m_spell.macroCmdSpellCheck);

    KWTextFrameSetEdit * edit = currentTextEdit();
    if (edit)
        edit->drawCursor( TRUE );
    if(kspellNoConfigured)
    {
        KWConfig configDia( this );
        configDia.openPage( KWConfig::KW_KSPELL);
        configDia.exec();
    }
}

void KWView::configure()
{
    KWConfig configDia( this );
    configDia.exec();
}

KWTextFrameSetEdit *KWView::currentTextEdit()
{
    if (!m_gui)
        return 0L;
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    if ( edit )
        return dynamic_cast<KWTextFrameSetEdit *>(edit->currentTextEdit());
    return 0L;
}

void KWView::slotFrameSetEditChanged()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    bool rw = koDocument()->isReadWrite();
    bool hasSelection = false;
    if ( edit ) {
        hasSelection = edit->textFrameSet()->hasSelection();
    }
    else {
        KWFrameSetEdit * e = m_gui->canvasWidget()->currentFrameSetEdit();
        if ( e && e->frameSet()->type() == FT_FORMULA ) {
            hasSelection = true;
        }
    }
    actionEditCut->setEnabled( hasSelection && rw );
    actionEditCopy->setEnabled( hasSelection );
    actionEditFind->setEnabled( edit && rw );
    actionEditReplace->setEnabled( edit && rw );
    //actionFormatDefault->setEnabled( hasSelection && rw);
    clipboardDataChanged(); // for paste

    bool state = (edit != 0L) && rw;
    actionEditSelectAll->setEnabled(state);
    actionFormatDefault->setEnabled( state);
    actionFormatFont->setEnabled(state);
    actionFormatFontSize->setEnabled(state);
    actionFormatFontFamily->setEnabled(state);
    actionFormatStyle->setEnabled(state);
    actionFormatBold->setEnabled(state);
    actionFormatItalic->setEnabled(state);
    actionFormatUnderline->setEnabled(state);
    actionFormatStrikeOut->setEnabled(state);
    actionFormatColor->setEnabled(state);
    actionFormatAlignLeft->setEnabled(state);
    actionFormatAlignCenter->setEnabled(state);
    actionFormatAlignRight->setEnabled(state);
    actionFormatAlignBlock->setEnabled(state);
    actionFormatIncreaseIndent->setEnabled(state);
    actionChangeCase->setEnabled( edit && hasSelection && state);
    actionInsertLink->setEnabled(state);

    bool goodleftMargin=false;
    if(state)
        goodleftMargin=(edit->currentParagLayout().margins[QStyleSheetItem::MarginLeft]>0);

    actionFormatDecreaseIndent->setEnabled(goodleftMargin && state);

    actionFormatList->setEnabled(state);
    actionFormatSuper->setEnabled(state);
    actionFormatSub->setEnabled(state);
    actionFormatParag->setEnabled(state);
    actionInsertSpecialChar->setEnabled(state);
    actionInsertFormula->setEnabled(state);
    actionInsertVariable->setEnabled(state);
    actionInsertExpression->setEnabled(state);

    //frameset different of header/footer
    state= state && edit && edit->frameSet() && !edit->frameSet()->isHeaderOrFooter() && !edit->frameSet()->getGroupManager();

    actionInsertContents->setEnabled(state);
    actionInsertFrameBreak->setEnabled( state );

    slotUpdateRuler();
}

void KWView::slotUpdateRuler()
{
    // Set the "frame start" in the ruler (tabs are relative to that position)
    KWFrameSetEdit * edit = m_gui->canvasWidget()->currentFrameSetEdit();
    KWFrame * frame = 0L;
    // Use the currently edited (fallback: the first selected) frame
    if ( edit && edit->currentFrame() )
        frame = edit->currentFrame();
    else
        frame = m_doc->getFirstSelectedFrame();
    if( !frame)
        frame=m_doc->frameSet(0)->frame(0);
    if ( frame )
    {
        QRect r = m_doc->zoomRect( *frame );
        r = m_gui->canvasWidget()->viewMode()->normalToView( r );
        // Now we need to make those coordinates relative to the page corner
        QPoint pc = m_gui->canvasWidget()->pageCorner();
        m_gui->getHorzRuler()->setFrameStartEnd( r.left() - pc.x(), r.right() - pc.x() );
        m_gui->getVertRuler()->setFrameStartEnd( r.top() - pc.y(), r.bottom() - pc.y() );
    }
    m_gui->canvasWidget()->updateRulerOffsets();
}

void KWView::frameSelectedChanged()
{
    bool rw = koDocument()->isReadWrite();
    QPtrList<KWFrame> selectedFrames = m_doc->getSelectedFrames();
    int nbFrame = selectedFrames.count();

    actionFormatFrameSet->setEnabled( nbFrame>=1 );
    if ( rw && nbFrame >= 1 )
    {
        bool okForDelete = true;
        bool okForLowerRaise = true;
        // Check we didn't select the main text frame (in WP mode)
        QPtrListIterator<KWFrame> it( selectedFrames );
        for ( ; it.current() && ( okForDelete || okForLowerRaise ) ; ++it )
        {
            // Check we selected no footer nor header
            okForDelete &= !it.current()->frameSet()->isHeaderOrFooter();
            bool isMainWPFrame = ( m_doc->processingType() == KWDocument::WP )
                                   && it.current()->frameSet() == m_doc->frameSet( 0 );
            okForDelete &= !isMainWPFrame;
            okForLowerRaise &= !isMainWPFrame;
        }
        actionEditDelFrame->setEnabled( okForDelete );
        actionEditCut->setEnabled( okForDelete );
        actionLowerFrame->setEnabled( okForLowerRaise );
        actionRaiseFrame->setEnabled( okForLowerRaise );
    } else
    {   // readonly document, or no frame selected -> disable
        actionEditDelFrame->setEnabled( false );
        actionInlineFrame->setEnabled(false);
        actionEditCut->setEnabled( false );
        actionLowerFrame->setEnabled( false );
        actionRaiseFrame->setEnabled( false );
    }
    bool frameDifferentOfPart=false;
    if(nbFrame >= 1)
    {
        QPtrListIterator<KWFrame> it( selectedFrames );
        for ( ; it.current(); ++it )
        {
            if ( it.current()->frameSet()->type()!=FT_PART && it.current()->frameSet()->type()!=FT_CLIPART && it.current()->frameSet()->type()!= FT_PICTURE)
            {
                frameDifferentOfPart=true;
                break;
            }
        }
    }

    actionBackgroundColor->setEnabled( (nbFrame >= 1) && frameDifferentOfPart);

    if ( frameDifferentOfPart ) {
        KWFrame *frame = m_doc->getFirstSelectedFrame();

        if ( frame )
        {
            QColor frameCol=frame->backgroundColor().color();
            actionBackgroundColor->setText(i18n("Frame Background Color"));
            actionBackgroundColor->setCurrentColor( frameCol.isValid()? frame->backgroundColor().color() :  QApplication::palette().color( QPalette::Active, QColorGroup::Base ));
        }
    }

    actionEditCopy->setEnabled( nbFrame >= 1 );

    KWTableFrameSet *table = m_gui->canvasWidget()->getCurrentTable();
    actionTableJoinCells->setEnabled( table && (nbFrame>1));

    bool state=(table && nbFrame==1);

    actionTableSplitCells->setEnabled( state );

    state=(table && nbFrame>0);

    actionTableInsertRow->setEnabled( state);
    actionTableInsertCol->setEnabled( state);
    actionTableDelRow->setEnabled( state );
    actionTableDelCol->setEnabled( state );
    actionTableDelete->setEnabled( state );
    actionTableUngroup->setEnabled( state );

    m_doc->refreshFrameBorderButton();

    updateFrameStatusBarItem();
    slotUpdateRuler();
}

void KWView::docStructChanged(int _type)
{
    KWDocStruct *m_pDocStruct=m_gui->getDocStruct();
    if(m_pDocStruct)
        m_pDocStruct->getDocStructTree()->refreshTree(_type);
}

void KWView::setViewFrameBorders(bool b)
{
    m_viewFrameBorders = b;
    // Store setting in doc, for further views and for saving
    m_doc->setViewFrameBorders( b );
}

bool KWView::doubleClickActivation() const
{
    return TRUE;
}

QWidget* KWView::canvas()
{
    return m_gui->canvasWidget()->viewport();
}

int KWView::canvasXOffset() const
{
    return m_gui->canvasWidget()->contentsX();
}

int KWView::canvasYOffset() const
{
    return m_gui->canvasWidget()->contentsY();
}

void KWView::canvasAddChild( KoViewChild *child )
{
    m_gui->canvasWidget()->addChild( child->frame() );
}

void KWView::changePicture()
{
    QString file,oldFile;
    KWFrame * frame = m_doc->getFirstSelectedFrame();
    KWPictureFrameSet *frameset = static_cast<KWPictureFrameSet *>(frame->frameSet());
    oldFile=frameset->image().key().filename();
    if ( KWInsertPicDia::selectPictureDia(file, KWInsertPicDia::SelectImage , oldFile ) )
    {
         KWFrameChangePictureClipartCommand *cmd= new KWFrameChangePictureClipartCommand( i18n("Change picture"), FrameIndex(frame), oldFile, file, true ) ;

        frameset->loadImage( file, m_doc->zoomRect( *frame ).size() );
        m_doc->frameChanged( frame );
        m_doc->addCommand(cmd);
    }
}

void KWView::changeClipart()
{
    QString file,oldFile;
    KWFrame * frame = m_doc->getFirstSelectedFrame();

    KWClipartFrameSet *frameset = static_cast<KWClipartFrameSet *>(frame->frameSet());
    oldFile=frameset->key().filename();
    if ( KWInsertPicDia::selectPictureDia(file, KWInsertPicDia::SelectClipart, oldFile ) )
    {

        KWFrameChangePictureClipartCommand *cmd= new KWFrameChangePictureClipartCommand( i18n("Change clipart"), FrameIndex( frame ), oldFile, file, false ) ;

        frameset->loadClipart( file );
        m_doc->frameChanged( frame );
        m_doc->addCommand(cmd);
    }
}

void KWView::configureHeaderFooter()
{
    KoPageLayout pgLayout;
    KoColumns cl;
    KoKWHeaderFooter kwhf;
    m_doc->getPageLayout( pgLayout, cl, kwhf );

    pageLayout tmpOldLayout;
    tmpOldLayout._pgLayout=pgLayout;
    tmpOldLayout._cl=cl;
    tmpOldLayout._hf=kwhf;

    KoHeadFoot hf;
    int flags = KW_HEADER_AND_FOOTER;
    KoUnit::Unit unit = m_doc->getUnit();
    KoUnit::Unit oldUnit = unit;

    if ( KoPageLayoutDia::pageLayout( pgLayout, hf, cl, kwhf, flags, unit ) ) {
        if( tmpOldLayout._hf.header!=kwhf.header||
            tmpOldLayout._hf.footer!=kwhf.footer||
            tmpOldLayout._hf.ptHeaderBodySpacing != kwhf.ptHeaderBodySpacing ||
            tmpOldLayout._hf.ptFooterBodySpacing != kwhf.ptFooterBodySpacing)
        {
            pageLayout tmpNewLayout;
            tmpNewLayout._pgLayout=pgLayout;
            tmpNewLayout._cl=cl;
            tmpNewLayout._hf=kwhf;

            KWPageLayoutCommand *cmd =new KWPageLayoutCommand( i18n("Change Layout"),m_doc,tmpOldLayout,tmpNewLayout ) ;
            m_doc->addCommand(cmd);

            m_doc->setPageLayout( pgLayout, cl, kwhf );
            m_doc->updateRuler();

            m_doc->updateResizeHandles();
        }
        if ( unit != oldUnit )
            m_doc->setUnit( unit ); // needs undo/redo support
    }

}

void KWView::inlineFrame()
{
    KWFrame * frame = m_doc->getFirstSelectedFrame();
    KWFrameSet * fs = frame->frameSet();
    KWFrameSet * parentFs = fs->getGroupManager() ? fs->getGroupManager() : fs;

    if(actionInlineFrame->isChecked())
    {

        KMacroCommand* macroCmd = new KMacroCommand( i18n("Make FrameSet Inline") );
        QPtrList<FrameIndex> frameindexList;
        QPtrList<FrameResizeStruct> frameindexMove;

        FrameIndex *index=new FrameIndex( frame );
        FrameResizeStruct *move=new FrameResizeStruct;

        move->sizeOfBegin=frame->normalize();
        // turn non-floating frame into floating frame
        KWFrameSetPropertyCommand *cmd = new KWFrameSetPropertyCommand( i18n("Make FrameSet Inline"), parentFs, KWFrameSetPropertyCommand::FSP_FLOATING, "true" );
        cmd->execute();

        move->sizeOfEnd=frame->normalize();

        frameindexList.append(index);
        frameindexMove.append(move);

        KWFrameMoveCommand *cmdMoveFrame = new KWFrameMoveCommand( i18n("Move Frame"), frameindexList, frameindexMove );

        macroCmd->addCommand(cmdMoveFrame);
        macroCmd->addCommand(cmd);
        m_doc->addCommand(macroCmd);
    }
    else
    {
        KWFrameSetPropertyCommand *cmd = new KWFrameSetPropertyCommand( i18n("Make FrameSet Non-Inline"), parentFs, KWFrameSetPropertyCommand::FSP_FLOATING, "false" );
        m_doc->addCommand(cmd);
        cmd->execute();
    }
}


void KWView::openLink()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->openLink();
}

void KWView::changeLink()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
    {
        KoLinkVariable * var=edit->linkVariable();
        if(var)
        {
            QString oldhref= var->url();
            QString oldLinkName=var->value();
            QString link=oldLinkName;
            QString ref=oldhref;
            if(KoInsertLinkDia::createLinkDia(link, ref))
            {
                if(!link.isEmpty() && !ref.isEmpty())
                {
                    KWChangeLinkVariable*cmd=new KWChangeLinkVariable( i18n("Change link"), m_doc,oldhref, ref, oldLinkName,link, var);
                    cmd->execute();
                    m_doc->addCommand(cmd);
                }
            }
        }
    }
}

void KWView::showDocStructure()
{
    m_doc->setShowDocStruct(actionShowDocStruct->isChecked());
    m_doc->reorganizeGUI();
}

void KWView::slotSoftHyphen()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->insertSoftHyphen();
}

void KWView::slotNonbreakingSpace()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->insertNonbreakingSpace();
}

void KWView::slotLineBreak()
{
    KWTextFrameSetEdit * edit = currentTextEdit();
    if ( edit )
        edit->insertLineBreak();
}

void KWView::refreshAllVariable()
{
    m_doc->recalcVariables( VT_ALL );
}

/******************************************************************/
/* Class: KWLayoutWidget                                          */
/******************************************************************/

KWLayoutWidget::KWLayoutWidget( QWidget *parent, KWGUI *g )
    : QWidget( parent )
{
    gui = g;
}

void KWLayoutWidget::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    gui->reorganize();
}

/******************************************************************/
/* Class: KWGUI                                                */
/******************************************************************/
KWGUI::KWGUI( QWidget *parent, KWView *_view )
    : QWidget( parent, "" )
{
    view = _view;

    r_horz = r_vert = 0;
    KWDocument * doc = view->kWordDocument();

    panner = new QSplitter( Qt::Horizontal, this );
    docStruct = new KWDocStruct( panner, doc, this );
    docStruct->setMinimumWidth( 0 );
    left = new KWLayoutWidget( panner, this );
    left->show();
    canvas = new KWCanvas( left, doc, this );

    QValueList<int> l;
    l << 0;
    panner->setSizes( l );

    KoPageLayout layout = doc->pageLayout();

    tabChooser = new KoTabChooser( left, KoTabChooser::TAB_ALL );
    tabChooser->setReadWrite(doc->isReadWrite());

    r_horz = new KoRuler( left, canvas->viewport(), Qt::Horizontal, layout,
                          KoRuler::F_INDENTS | KoRuler::F_TABS, doc->getUnit(), tabChooser );
    r_horz->setReadWrite(doc->isReadWrite());
    r_vert = new KoRuler( left, canvas->viewport(), Qt::Vertical, layout, 0, doc->getUnit() );
    connect( r_horz, SIGNAL( newPageLayout( KoPageLayout ) ), view, SLOT( newPageLayout( KoPageLayout ) ) );
    r_vert->setReadWrite(doc->isReadWrite());

    r_horz->setZoom( doc->zoomedResolutionX() );
    r_vert->setZoom( doc->zoomedResolutionY() );

    r_horz->setGridSize(doc->gridX());

    connect( r_horz, SIGNAL( newLeftIndent( double ) ), view, SLOT( newLeftIndent( double ) ) );
    connect( r_horz, SIGNAL( newFirstIndent( double ) ), view, SLOT( newFirstIndent( double ) ) );
    connect( r_horz, SIGNAL( newRightIndent( double ) ), view, SLOT( newRightIndent( double ) ) );

    connect( r_horz, SIGNAL( doubleClicked() ), view, SLOT( slotHRulerDoubleClicked() ) );
    connect( r_horz, SIGNAL( doubleClicked(double) ), view, SLOT( slotHRulerDoubleClicked(double) ) );
    connect( r_horz, SIGNAL( unitChanged( QString ) ), this, SLOT( unitChanged( QString ) ) );
    connect( r_vert, SIGNAL( newPageLayout( KoPageLayout ) ), view, SLOT( newPageLayout( KoPageLayout ) ) );
    connect( r_vert, SIGNAL( doubleClicked() ), view, SLOT( formatPage() ) );
    connect( r_vert, SIGNAL( unitChanged( QString ) ), this, SLOT( unitChanged( QString ) ) );

    r_horz->setUnit( doc->getUnit() );
    r_vert->setUnit( doc->getUnit() );

    r_horz->hide();
    r_vert->hide();

    canvas->show();

    reorganize();

#if 0
    if ( doc->processingType() == KWDocument::DTP )   // ???
        canvas->setRuler2Frame( 0, 0 );
#endif

    connect( r_horz, SIGNAL( tabListChanged( const KoTabulatorList & ) ), view,
             SLOT( tabListChanged( const KoTabulatorList & ) ) );

    setKeyCompression( TRUE );
    setAcceptDrops( TRUE );
    setFocusPolicy( QWidget::NoFocus );

    canvas->setContentsPos( 0, 0 );
}

void KWGUI::showGUI()
{
    reorganize();
}

void KWGUI::resizeEvent( QResizeEvent *e )
{
    QWidget::resizeEvent( e );
    reorganize();
}

void KWGUI::reorganize()
{
    int space=20;
    if(view->kWordDocument()->showRuler())
    {
        r_vert->show();
        r_horz->show();
        tabChooser->show();
        tabChooser->setGeometry( 0, 0, 20, 20 );
    }
    else
    {
        r_vert->hide();
        r_horz->hide();
        tabChooser->hide();
        space=0;
    }

    if(view->kWordDocument()->showdocStruct())
        docStruct->show();
    else
        docStruct->hide();

    panner->setGeometry( 0, 0, width(), height() );
    canvas->setGeometry( space, space, left->width() - space, left->height() - space );
    r_horz->setGeometry( space, 0, left->width() - space, space );
    r_vert->setGeometry( 0, space, space, left->height() - space );
}

void KWGUI::unitChanged( QString u )
{
    view->kWordDocument()->setUnit( KoUnit::unit( u ) );
}

void KWView::fileStatistics()
{
    KWStatisticsDialog *statisticsDialog = new KWStatisticsDialog( this, m_doc );
    if ( !statisticsDialog->wasCanceled() )
        statisticsDialog->exec();
    delete statisticsDialog;
}

// Implementation of KWStatisticsDialog
KWStatisticsDialog::KWStatisticsDialog( QWidget *_parent, KWDocument *_doc )
    : KDialogBase( KJanusWidget::Tabbed,
                   i18n("Statistics"),
                   KDialogBase::Ok,
                   KDialogBase::Ok,
                   _parent,
                   "statistics",
                   true,
                   false )
{
    QFrame *pageAll = 0;
    QFrame *pageSelected = 0;
    for (int i=0; i < 6; ++i) {
        resultLabelAll[i] = 0;
        resultLabelSelected[i] = 0;
    }
    m_doc = _doc;
    m_parent = _parent;
    m_canceled = true;

    // add Tab "All"
    pageAll = addPage( i18n( "All" ) );
    addBox( pageAll, resultLabelAll );

    // let's see if there's selected text
    bool b = docHasSelection();
    if ( b ) {
        // add Tab "Selected"
        pageSelected = addPage( i18n( "Selected" ) );
        addBox( pageSelected, resultLabelSelected);
        // assign results
        if ( !calcStats( resultLabelSelected, true ) )
            return;
        if ( !calcStats( resultLabelAll, false ) )
            return;
        showPage( 1 );
    } else {
        // assign results
        if ( !calcStats( resultLabelAll, false ) )
            return;
        //if ( !calcStats( resultLabelAll, false ) ) return;
        showPage( 0 );
    }
    m_canceled = false;
}

bool KWStatisticsDialog::calcStats( QLabel **resultLabel, bool selection )
{
    ulong charsWithSpace = 0L;
    ulong charsWithoutSpace = 0L;
    ulong words = 0L;
    ulong sentences = 0L;
    ulong syllables = 0L;

    // safety check result labels
    for (int i=0; i < 6; ++i) {
        if ( !resultLabel[i] ) {
            kdDebug() << "Warning: KWStatisticsDiaolog::calcStats result table not initialized." << endl;
            return false;
        }
    }

    // count paragraphs for progress dialog:
    ulong paragraphs = 0L;
    QPtrListIterator<KWFrameSet> framesetIt( m_doc->framesetsIterator() );
    for ( framesetIt.toFirst(); framesetIt.current(); ++framesetIt ) {
        KWFrameSet *frameSet = framesetIt.current();
        if ( frameSet->frameSetInfo() == KWFrameSet::FI_BODY && frameSet->isVisible() ) {
            if ( selection && false )
                paragraphs += frameSet->paragraphsSelected();
            else
                paragraphs += frameSet->paragraphs();
        }
    }
    QProgressDialog progress( i18n( "Counting..." ), i18n( "Cancel" ), paragraphs, this, "count", true );
    progress.setMinimumDuration( 1000 );
    progress.setProgress( 0 );

    // do the actual counting
    for ( framesetIt.toFirst(); framesetIt.current(); ++framesetIt ) {
        KWFrameSet *frameSet = framesetIt.current();
        // Exclude headers and footers
        if ( frameSet->frameSetInfo() == KWFrameSet::FI_BODY && frameSet->isVisible() ) {
            if( ! frameSet->statistics( &progress, charsWithSpace, charsWithoutSpace,
                                        words, sentences, syllables, selection ) ) {
                // someone pressed "Cancel"
                return false;
            }
        }
    }

    // assign results
    KLocale *locale = KGlobal::locale();
    resultLabel[0]->setText( locale->formatNumber( charsWithSpace, 0) );
    resultLabel[1]->setText( locale->formatNumber( charsWithoutSpace, 0 ) );
    resultLabel[2]->setText( locale->formatNumber( syllables, 0 ) );
    resultLabel[3]->setText( locale->formatNumber( words, 0 ) );
    resultLabel[4]->setText( locale->formatNumber( sentences, 0 ) );
    // add flesch
    double f = calcFlesch( sentences, words, syllables );
    QString flesch;
    QString flesch_score_string;
    flesch_score_string = locale->formatNumber( f , 1 );
    if( words < 200 ) {
        // a kind of warning if too few words:
        flesch = i18n("approximately %1").arg( flesch_score_string );
    } else {
        flesch = flesch_score_string;
    }
    resultLabel[5]->setText( flesch );
    return true;
}

double KWStatisticsDialog::calcFlesch( ulong sentences, ulong words, ulong syllables )
{
    // calculate Flesch reading ease score:
    float flesch_score = 0;
    if( words > 0 && sentences > 0 )
        flesch_score = 206.835 - (1.015 * (words / sentences)) - (84.6 * syllables / words);
    return flesch_score;
}

void KWStatisticsDialog::addBox( QFrame *page, QLabel **resultLabel )
{
    // Layout Managers
    QVBoxLayout *topLayout = new QVBoxLayout( page, 0, 6 );
    QGroupBox *box = new QGroupBox( i18n( "Statistics" ), page );
    QGridLayout *grid = new QGridLayout( box, 7, 3, KDialog::marginHint(), KDialog::spacingHint() );

    // margins
    int fHeight = box->fontMetrics().height();
    grid->setMargin( fHeight );
    grid->addColSpacing( 1, fHeight );
    grid->addRowSpacing( 0, fHeight );

    //maximum size for result column (don't know how to do this better..)
    QString init = i18n("approximately %1").arg( "00000000" );

    // insert labels
    QLabel *label1 = new QLabel( i18n( "Characters including spaces:" ), box );
    grid->addWidget( label1, 1, 0, 1 );
    resultLabel[0] = new QLabel( "", box );
    grid->addWidget( resultLabel[0], 1, 2, 2 );

    QLabel *label2 = new QLabel( i18n( "Characters without spaces:" ), box );
    grid->addWidget( label2, 2, 0, 1 );
    resultLabel[1] = new QLabel( "", box );
    grid->addWidget( resultLabel[1], 2, 2, 2 );

    QLabel *label3 = new QLabel( i18n( "Syllables:" ), box );
    grid->addWidget( label3, 3, 0, 1 );
    resultLabel[2] = new QLabel( "", box );
    grid->addWidget( resultLabel[2], 3, 2, 2 );

    QLabel *label4 = new QLabel( i18n( "Words:" ), box );
    grid->addWidget( label4, 4, 0, 1 );
    resultLabel[3] = new QLabel( "", box );
    grid->addWidget( resultLabel[3], 4, 2, 2 );

    QLabel *label5 = new QLabel( i18n( "Sentences:" ), box );
    grid->addWidget( label5, 5, 0, 1 );
    resultLabel[4] = new QLabel( "", box );
    grid->addWidget( resultLabel[4], 5, 2, 2 );

    QLabel *label6 = new QLabel( i18n( "Flesch reading ease:" ), box );
    grid->addWidget( label6, 6, 0, 1 );
    resultLabel[5] = new QLabel( init, box );
    grid->addWidget( resultLabel[5], 6, 2, 2 );

    topLayout->addWidget( box );
}

bool KWStatisticsDialog::docHasSelection()
{
    QPtrListIterator<KWFrameSet> fsIt( m_doc->framesetsIterator() );

    for ( ; fsIt.current(); ++fsIt ) {
        KWFrameSet *fs = fsIt.current();
        if ( fs->paragraphsSelected() ) {
            return true;
        }
    }
    return false;
}
#include "kwview.moc"
