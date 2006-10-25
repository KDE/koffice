/* This file is part of the KDE project

   Copyright 1999-2006 The KSpread Team <koffice-devel@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kspread_editors.h"
#include "kspread_canvas.h"
#include "kspread_cell.h"
#include "kspread_doc.h"
#include "selection.h"
#include "kspread_sheet.h"
#include "kspread_view.h"
#include "kspread_util.h"
#include "formula.h"
#include "functions.h"

#include <klistbox.h>

#include <qapplication.h>
#include <qlistbox.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qvaluelist.h>
#include <private/qrichtext_p.h>

//#include <klineedit.h>
#include <ktextedit.h>
#include <qapplication.h>
#include <qbutton.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qregexp.h>
#include <kdebug.h>

using namespace KSpread;



/*****************************************************************************
 *
 * FormulaEditorHighlighter
 *
 ****************************************************************************/

namespace KSpread
{

class FormulaEditorHighlighter::Private
{
public:
  Private()
  {
    canvas = 0;
    tokens = Tokens();
    rangeCount = 0;
    rangeChanged = false;
  }

  // source for cell reference checking
  Canvas* canvas;
  Tokens tokens;
  uint rangeCount;
  bool rangeChanged;
};


FormulaEditorHighlighter::FormulaEditorHighlighter(QTextEdit* textEdit, Canvas* canvas)
  : QSyntaxHighlighter(textEdit)
{
  d = new Private();
  d->canvas = canvas;
}

FormulaEditorHighlighter::~FormulaEditorHighlighter()
{
  delete d;
}

const Tokens& FormulaEditorHighlighter::formulaTokens() const
{
  return d->tokens;
}

int FormulaEditorHighlighter::highlightParagraph(const QString& text, int /* endStateOfLastPara */)
{
  // reset syntax highlighting
  setFormat(0, text.length(), Qt::black);

  // save the old ones to identify range changes
  Tokens oldTokens = d->tokens;

  // interpret the text as formula
  // we accept invalid/incomplete formulas
  Formula f;
  d->tokens = f.scan(text);

  QFont editorFont = textEdit()->currentFont();
  QFont font;

  uint oldRangeCount = d->rangeCount;

  d->rangeCount = 0;
  QValueList<QColor> colors = d->canvas->choice()->colors();
  QValueList<Range> alreadyFoundRanges;

  for (uint i = 0; i < d->tokens.count(); ++i)
  {
    Token token = d->tokens[i];
    Token::Type type = token.type();

    switch (type)
    {
      case Token::Cell:
      case Token::Range:
        {
            // don't compare, if we have already found a change
            if (!d->rangeChanged && i < oldTokens.count() && token.text() != oldTokens[i].text())
            {
                d->rangeChanged = true;
            }

            Range newRange( token.text() );

            if (!alreadyFoundRanges.contains(newRange))
            {
                alreadyFoundRanges.append(newRange);
                d->rangeCount++;
            }
            setFormat(token.pos() + 1, token.text().length(), colors[ alreadyFoundRanges.findIndex(newRange) % colors.size()] );
        }
        break;
      case Token::Boolean:     // True, False (also i18n-ized)
/*        font = QFont(editorFont);
        font.setBold(true);
        setFormat(token.pos() + 1, token.text().length(), font);*/
        break;
      case Token::Identifier:   // function name or named area*/
/*        font = QFont(editorFont);
        font.setBold(true);
        setFormat(token.pos() + 1, token.text().length(), font);*/
        break;

      case Token::Unknown:
      case Token::Integer:     // 14, 3, 1977
      case Token::Float:       // 3.141592, 1e10, 5.9e-7
      case Token::String:      // "KOffice", "The quick brown fox..."
      case Token::Operator:    // +, *, /, -
        {
            switch (token.asOperator())
            {
                case Token::LeftPar:
                case Token::RightPar:
                    //Check where this brace is in relation to the cursor and highlight it if necessary.
                    handleBrace( i );
                    break;
                default:
                    break;
            }
        }
        break;
    }
  }

  if (oldRangeCount != d->rangeCount)
    d->rangeChanged = true;

  return 0;
}

void FormulaEditorHighlighter::handleBrace( uint index )
{
  int cursorParagraph;
  int cursorPos;
  const Token& token = d->tokens.at( index );

  textEdit()->getCursorPosition( &cursorParagraph , &cursorPos );

  int distance = cursorPos-token.pos();
  int opType = token.asOperator();
  bool highlightBrace=false;

  //Check where the cursor is in relation to this left or right parenthesis token.
  //Only one pair of braces should be highlighted at a time, and if the cursor
  //is between two braces, the inner-most pair should be highlighted.

  if ( opType == Token::LeftPar )
  {
    //If cursor is directly to the left of this left brace, highlight it
    if ( distance == 1 )
      highlightBrace=true;
    else
        //Cursor is directly to the right of this left brace, highlight it unless
        //there is another left brace to the right (in which case that should be highlighted instead as it
        //is the inner-most brace)
        if (distance==2)
            if ( (index == d->tokens.count()-1) || ( d->tokens.at(index+1).asOperator() != Token::LeftPar) )
          highlightBrace=true;

  }
  else
  {
    //If cursor is directly to the right of this right brace, highlight it
    if ( distance == 2 )
      highlightBrace=true;
    else
        //Cursor is directly to the left of this right brace, so highlight it unless
        //there is another right brace to the left (in which case that should be highlighted instead as it
        //is the inner-most brace)
      if ( distance == 1 )
        if ( (index == 0) || (d->tokens.at(index-1).asOperator() != Token::RightPar) )
          highlightBrace=true;
  }

  if (highlightBrace)
  {
    QFont font = QFont( textEdit()->currentFont() );
    font.setBold(true);
    setFormat(token.pos() + 1, token.text().length(), font);

    int matching = findMatchingBrace( index );

    if (matching != -1)
    {
      Token matchingBrace = d->tokens.at(matching);
      setFormat( matchingBrace.pos() + 1 , matchingBrace.text().length() , font);
    }
  }
}

int FormulaEditorHighlighter::findMatchingBrace(int pos)
{
    int depth=0;
    int step=0;

    Tokens tokens = d->tokens;

    //If this is a left brace we need to step forwards through the text to find the matching right brace,
    //otherwise, it is a right brace so we need to step backwards through the text to find the matching left
    //brace.
    if (tokens.at(pos).asOperator() == Token::LeftPar)
        step = 1;
    else
        step = -1;

    for (int index=pos ; (index >= 0) && (index < (int) tokens.count() ) ; index += step  )
    {
        if (tokens.at(index).asOperator() == Token::LeftPar)
            depth++;
        if (tokens.at(index).asOperator() == Token::RightPar)
            depth--;

        if (depth == 0)
        {
            return index;
        }
    }

    return -1;
}

uint FormulaEditorHighlighter::rangeCount() const
{
  return d->rangeCount;
}

bool FormulaEditorHighlighter::rangeChanged() const
{
  return d->rangeChanged;
}

void FormulaEditorHighlighter::resetRangeChanged()
{
    d->rangeChanged=false;
}

} // namespace KSpread



/*****************************************************************************
 *
 * FunctionCompletion
 *
 ****************************************************************************/

class FunctionCompletion::Private
{
public:
  CellEditor* editor;
  QVBox *completionPopup;
  KListBox *completionListBox;
  QLabel* hintLabel;
};

FunctionCompletion::FunctionCompletion( CellEditor* editor ):
QObject( editor )
{
  d = new Private;
  d->editor = editor;
  d->hintLabel = 0;

  d->completionPopup = new QVBox( editor->topLevelWidget(), 0, WType_Popup );
  d->completionPopup->setFrameStyle( QFrame::Box | QFrame::Plain );
  d->completionPopup->setLineWidth( 1 );
  d->completionPopup->installEventFilter( this );
  d->completionPopup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum);

  d->completionListBox = new KListBox( d->completionPopup );
  d->completionPopup->setFocusProxy( d->completionListBox );
  d->completionListBox->setFrameStyle( QFrame::NoFrame );
  d->completionListBox->setVariableWidth( true );
  d->completionListBox->installEventFilter( this );
  connect( d->completionListBox, SIGNAL(selected(const QString&)), this,
    SLOT(itemSelected(const QString&)) );
  connect( d->completionListBox, SIGNAL(highlighted(const QString&)), this,
    SLOT(itemSelected(const QString&)) );

  d->hintLabel = new QLabel( 0, "autocalc", Qt::WStyle_StaysOnTop |
    Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_Tool |  Qt::WX11BypassWM );
  d->hintLabel->setFrameStyle( QFrame::Plain | QFrame::Box );
  d->hintLabel->setPalette( QToolTip::palette() );
  d->hintLabel->hide();
}

FunctionCompletion::~FunctionCompletion()
{
      delete d->hintLabel;
      delete d;
}

void FunctionCompletion::itemSelected( const QString& item )
{
    KSpread::FunctionDescription* desc;
    desc = KSpread::FunctionRepository::self()->functionInfo(item);
    if(!desc)
    {
        d->hintLabel->hide();
        return;
    }

    QString helpText = desc->helpText()[0];
    if( helpText.isEmpty() )
    {
        d->hintLabel->hide();
        return;
    }

    helpText.append("</qt>").prepend("<qt>");
    d->hintLabel->setText( helpText );
    d->hintLabel->adjustSize();

    // reposition nicely
    QPoint pos = d->editor->mapToGlobal( QPoint( d->editor->width(), 0 ) );
    pos.setY( pos.y() - d->hintLabel->height() - 1 );
    d->hintLabel->move( pos );
    d->hintLabel->show();
    d->hintLabel->raise();

    // do not show it forever
    //QTimer::singleShot( 5000, d->hintLabel, SLOT( hide()) );
}

bool FunctionCompletion::eventFilter( QObject *obj, QEvent *ev )
{
    if ( obj == d->completionPopup || obj == d->completionListBox )
    {
      if ( ev->type() == QEvent::KeyPress )
      {
              QKeyEvent *ke = (QKeyEvent*)ev;
              if ( ke->key() == Key_Enter || ke->key() == Key_Return  )
              {
                  doneCompletion();
                  return true;
              }
              else if ( ke->key() == Key_Left || ke->key() == Key_Right ||
              ke->key() == Key_Up || ke->key() == Key_Down ||
              ke->key() == Key_Home || ke->key() == Key_End ||
              ke->key() == Key_Prior || ke->key() == Key_Next )
                  return false;

              d->hintLabel->hide();
              d->completionPopup->close();
              d->editor->setFocus();
              QApplication::sendEvent( d->editor, ev );
              return true;
      }

      if ( ev->type() == QEvent::MouseButtonDblClick )
      {
          doneCompletion();
          return true;
      }
  }

  return false;
}

void FunctionCompletion::doneCompletion()
{
  d->hintLabel->hide();
  d->completionPopup->close();
  d->editor->setFocus();
  emit selectedCompletion( d->completionListBox->currentText() );
}

void FunctionCompletion::showCompletion( const QStringList &choices )
{
  if( !choices.count() ) return;

  d->completionListBox->clear();
  for( unsigned i = 0; i < choices.count(); i++ )
    new QListBoxText( (QListBox*)d->completionListBox, choices[i] );
  d->completionListBox->setCurrentItem( 0 );

  // size of the pop-up
  d->completionPopup->setMaximumHeight( 100 );
  d->completionPopup->resize( d->completionListBox->sizeHint() +
    QSize( d->completionListBox->verticalScrollBar()->width() + 4,
        d->completionListBox->horizontalScrollBar()->height() + 4 ) );
  int h = d->completionListBox->height();
  int w = d->completionListBox->width();

  QPoint pos = d->editor->globalCursorPosition();

  // if popup is partially invisible, move to other position
  // FIXME check it if it works in Xinerama multihead
  int screen_num = QApplication::desktop()->screenNumber( d->completionPopup );
  QRect screen = QApplication::desktop()->screenGeometry( screen_num );
  if( pos.y() + h > screen.y()+screen.height() )
    pos.setY( pos.y() - h - d->editor->height() );
  if( pos.x() + w > screen.x()+screen.width() )
    pos.setX(  screen.x()+screen.width() - w );

  d->completionPopup->move( pos );
  d->completionListBox->setFocus();
  d->completionPopup->show();
}



/****************************************************************************
 *
 * CellEditor
 *
 ****************************************************************************/

class CellEditor::Private
{
public:
  Cell*                     cell;
  Canvas*                   canvas;
  KTextEdit*                textEdit;
  FormulaEditorHighlighter* highlighter;
  FunctionCompletion*       functionCompletion;
  QTimer*                   functionCompletionTimer;

  QPoint globalCursorPos;

  bool captureAllKeyEvents : 1;
  bool checkChoice         : 1;
  bool updateChoice        : 1;
  bool updatingChoice      : 1;

  uint length;
  uint fontLength;
  uint length_namecell;
  uint length_text;
  uint currentToken;
  uint rangeCount;
};


CellEditor::CellEditor( Cell* _cell, Canvas* _parent, bool captureAllKeyEvents, const char* _name )
  : QWidget( _parent, _name )
{
  d = new Private();
  d->cell = _cell;
  d->canvas = _parent;
  d->textEdit = new KTextEdit(this);
  d->globalCursorPos = QPoint();
  d->captureAllKeyEvents = captureAllKeyEvents;
  d->checkChoice = true;
  d->updateChoice = true;
  d->updatingChoice = false;
  d->length = 0;
  d->fontLength = 0;
  d->length_namecell = 0;
  d->length_text = 0;
  d->currentToken = 0;
  d->rangeCount = 0;

//TODO - Get rid of QTextEdit margins, this doesn't seem easily possible in Qt 3.3, so a job for Qt 4 porting.

  d->textEdit->setHScrollBarMode(QScrollView::AlwaysOff);
  d->textEdit->setVScrollBarMode(QScrollView::AlwaysOff);
  d->textEdit->setFrameStyle(QFrame::NoFrame);
  d->textEdit->setLineWidth(0);
  d->textEdit->installEventFilter( this );

  d->highlighter = new FormulaEditorHighlighter(d->textEdit, _parent);

  d->functionCompletion = new FunctionCompletion( this );
  d->functionCompletionTimer = new QTimer( this );
  connect( d->functionCompletion, SIGNAL( selectedCompletion( const QString& ) ),
    SLOT( functionAutoComplete( const QString& ) ) );
  connect( d->textEdit, SIGNAL( textChanged() ), SLOT( checkFunctionAutoComplete() ) );
  connect( d->functionCompletionTimer, SIGNAL( timeout() ),
    SLOT( triggerFunctionAutoComplete() ) );

  if (!cell()->format()->multiRow(cell()->column(),cell()->row()))
    d->textEdit->setWordWrap(QTextEdit::NoWrap);
  else
	d->textEdit->setWrapPolicy(QTextEdit::AtWordOrDocumentBoundary);

//TODO - Custom KTextEdit class which supports text completion
/*
  d->textEdit->setFrame( false );
  d->textEdit->setCompletionMode((KGlobalSettings::Completion)canvas()->view()->doc()->completionMode()  );
  d->textEdit->setCompletionObject( &canvas()->view()->doc()->completion(),true );
*/
  setFocusProxy( d->textEdit );

  connect( d->textEdit, SIGNAL( cursorPositionChanged(int,int) ), this, SLOT (slotCursorPositionChanged(int,int)));
  connect( d->textEdit, SIGNAL( cursorPositionChanged(QTextCursor*) ), this, SLOT (slotTextCursorChanged(QTextCursor*)));
  connect( d->textEdit, SIGNAL( textChanged() ), this, SLOT( slotTextChanged() ) );

// connect( d->textEdit, SIGNAL(completionModeChanged( KGlobalSettings::Completion )),this,SLOT (slotCompletionModeChanged(KGlobalSettings::Completion)));

  // A choose should always start at the edited cell
//  canvas()->setChooseMarkerRow( canvas()->markerRow() );
//  canvas()->setChooseMarkerColumn( canvas()->markerColumn() );

  // set font size according to zoom factor
  QFont font( _cell->format()->font() );
  font.setPointSizeFloat( 0.01 * _parent->doc()->zoom() * font.pointSizeFloat() );
  d->textEdit->setFont( font );

  if (d->fontLength == 0)
  {
    QFontMetrics fm( d->textEdit->font() );
    d->fontLength = fm.width('x');
  }
}

CellEditor::~CellEditor()
{
  canvas()->endChoose();

  delete d->highlighter;
  delete d->functionCompletion;
  delete d->functionCompletionTimer;
  delete d;
}

Cell* CellEditor::cell() const
{
  return d->cell;
}

Canvas* CellEditor::canvas() const
{
  return d->canvas;
}

QPoint CellEditor::globalCursorPosition() const
{
  return d->globalCursorPos;
}

void CellEditor::checkFunctionAutoComplete()
{
  d->functionCompletionTimer->stop();
  d->functionCompletionTimer->start( 2000, true );
}

void CellEditor::triggerFunctionAutoComplete()
{
  // tokenize the expression (don't worry, this is very fast)
  int para = 0, curPos = 0;
  d->textEdit->getCursorPosition( &para, &curPos );
  QString subtext = d->textEdit->text().left( curPos );

  KSpread::Formula f;
  KSpread::Tokens tokens = f.scan( subtext );
  if( !tokens.valid() ) return;
  if( tokens.count()<1 ) return;

  KSpread::Token lastToken = tokens[ tokens.count()-1 ];

  // last token must be an identifier
  if( !lastToken.isIdentifier() ) return;
  QString id = lastToken.text();
  if( id.length() < 1 ) return;

  // find matches in function names
  QStringList fnames = KSpread::FunctionRepository::self()->functionNames();
  QStringList choices;
  for( unsigned i=0; i<fnames.count(); i++ )
    if( fnames[i].startsWith( id, false ) )
      choices.append( fnames[i] );
  choices.sort();

  // no match, don't bother with completion
  if( !choices.count() ) return;

  // single perfect match, no need to give choices
  if( choices.count()==1 )
    if( choices[0].lower() == id.lower() )
      return;

  // present the user with completion choices
  d->functionCompletion->showCompletion( choices );
}

void CellEditor::functionAutoComplete( const QString& item )
{
  if( item.isEmpty() ) return;

  int para = 0, curPos = 0;
  d->textEdit->getCursorPosition( &para, &curPos );
  QString subtext = text().left( curPos );

  KSpread::Formula f;
  KSpread::Tokens tokens = f.scan( subtext );
  if( !tokens.valid() ) return;
  if( tokens.count()<1 ) return;

  KSpread::Token lastToken = tokens[ tokens.count()-1 ];
  if( !lastToken.isIdentifier() ) return;

  d->textEdit->blockSignals( true );
  d->textEdit->setSelection( 0, lastToken.pos()+1, 0, lastToken.pos()+lastToken.text().length()+1 );
  d->textEdit->insert( item );
  d->textEdit->blockSignals( false );
}

void CellEditor::slotCursorPositionChanged(int /* para */, int pos)
{
//   kdDebug() << k_funcinfo << endl;

  // TODO Stefan: optimize this function!

  // turn choose mode on/off
  if (!checkChoice())
    return;

  d->highlighter->rehighlight();

  Tokens tokens = d->highlighter->formulaTokens();
  uint rangeCounter = 0;
  uint currentRange = 0;
  uint regionStart = 0;
  uint regionEnd = 0;
  bool lastWasASemicolon = false;
  d->currentToken = 0;
  uint rangeCount = d->highlighter->rangeCount();
  d->rangeCount = rangeCount;

  Token token;
  Token::Type type;
  // search the current token
  // determine the subregion number, btw
  for (uint i = 0; i < tokens.count(); ++i)
  {
    if (tokens[i].pos() >= pos - 1) // without '='
    {
/*      kdDebug() << "token.pos >= cursor.pos" << endl;*/
      type = tokens[i].type();
      if (type == Token::Cell || type == Token::Range)
      {
        if (lastWasASemicolon)
        {
          regionEnd = rangeCounter++;
          lastWasASemicolon = false;
          continue;
        }
      }
      if (type == Token::Operator && tokens[i].asOperator() == Token::Semicolon)
      {
        lastWasASemicolon = true;
        continue;
      }
      lastWasASemicolon = false;
      break;
    }
    token = tokens[i];
    d->currentToken = i;

    type = token.type();
    if (type == Token::Cell || type == Token::Range)
    {
      if (!lastWasASemicolon)
      {
        regionStart = rangeCounter;
      }
      regionEnd = rangeCounter;
      currentRange = rangeCounter++;
    }
    // semicolons are use as deliminiters in regions
    if (type == Token::Operator)
    {
      if (token.asOperator() == Token::Semicolon)
      {
        lastWasASemicolon = true;
      }
      else
      {
        lastWasASemicolon = false;
        // set the region start to the next element
        regionStart = currentRange + 1;
        regionEnd = regionStart - 1; // len = 0
      }
    }
  }

//   kdDebug() << "regionStart = " << regionStart/* << endl*/
//             << ", regionEnd = " << regionEnd/* << endl*/
//             << ", currentRange = " << currentRange << endl;

  d->canvas->choice()->setActiveElement(currentRange);
  d->canvas->choice()->setActiveSubRegion(regionStart, regionEnd-regionStart+1);

  // triggered by keyboard action?
  if (!d->updatingChoice)
  {
    if (d->highlighter->rangeChanged())
    {
      d->highlighter->resetRangeChanged();

      disconnect( d->canvas->choice(), SIGNAL(changed(const Region&)),
                  d->canvas->view(), SLOT(slotScrollChoice(const Region&)) );
      d->canvas->doc()->emitBeginOperation();
      setUpdateChoice(false);

      Tokens tokens = d->highlighter->formulaTokens();
      d->canvas->choice()->update(); // set the old one dirty
      d->canvas->choice()->clear();
      Region tmpRegion;
      Region::ConstIterator it;

      //A list of regions which have already been highlighted on the spreadsheet.
      //This is so that we don't end up highlighting the same region twice in two different
      //colours.
      QValueList<Region> alreadyUsedRegions;

      for (uint i = 0; i < tokens.count(); ++i)
      {
        Token token = tokens[i];
        Token::Type type = token.type();
        if (type == Token::Cell || type == Token::Range)
        {
          Region region(d->canvas->view(), token.text());
          it = region.constBegin();

          if (!alreadyUsedRegions.contains(region))
          {
            QRect r=(*it)->rect();

            if (d->canvas->choice()->isEmpty())
                d->canvas->choice()->initialize((*it)->rect(), (*it)->sheet());
            else
                d->canvas->choice()->extend((*it)->rect(), (*it)->sheet());

            alreadyUsedRegions.append(region);
          }
        }
      }
      setUpdateChoice(true);
      d->canvas->doc()->emitEndOperation(*d->canvas->choice());
      connect( d->canvas->choice(), SIGNAL(changed(const Region&)),
               d->canvas->view(), SLOT(slotScrollChoice(const Region&)) );
    }
  }
}

void CellEditor::slotTextCursorChanged(QTextCursor* cursor)
{
  QTextStringChar *chr = cursor->paragraph()->at( cursor->index() );
  int h = cursor->paragraph()->lineHeightOfChar( cursor->index() );
  int x = cursor->paragraph()->rect().x() + chr->x;
  int y, dummy;
  cursor->paragraph()->lineHeightOfChar( cursor->index(), &dummy, &y );
  y += cursor->paragraph()->rect().y();

  d->globalCursorPos = d->textEdit->mapToGlobal( d->textEdit-> contentsToViewport( QPoint( x, y + h ) ) );
}

void CellEditor::cut()
{
  d->textEdit->cut();
}

void CellEditor::paste()
{
  d->textEdit->paste();
}

void CellEditor::copy()
{
  d->textEdit->copy();
}

void CellEditor::setEditorFont(QFont const & font, bool updateSize)
{
  QFont tmpFont( font );
  tmpFont.setPointSizeFloat( 0.01 * canvas()->doc()->zoom() * tmpFont.pointSizeFloat() );
  d->textEdit->setFont( tmpFont );

  if (updateSize)
  {
    QFontMetrics fm( d->textEdit->font() );
    d->fontLength = fm.width('x');

    int mw = fm.width( d->textEdit->text() ) + d->fontLength;
    // don't make it smaller: then we would have to repaint the obscured cells
    if (mw < width())
      mw = width();

    int mh = fm.height();
    if (mh < height())
      mh = height();

    setGeometry(x(), y(), mw, mh);
  }
}

void CellEditor::slotCompletionModeChanged(KGlobalSettings::Completion _completion)
{
  canvas()->view()->doc()->setCompletionMode( _completion );
}

void CellEditor::slotTextChanged()
{
//   kdDebug() << k_funcinfo << endl;

  //FIXME - text() may return richtext?
  QString t = text();

  if (t.length() > d->length)
  {
    d->length = t.length();

  QFontMetrics fm(d->textEdit->font());
  // - requiredWidth = width of text plus some spacer characters
  int requiredWidth = fm.width(t) + (2*fm.width('x'));

  //For normal single-row cells, the text editor must be expanded horizontally to
  //allow the text to fit if the new text is too wide
  //For multi-row (word-wrap enabled) cells, the text editor must expand vertically to
  //allow for new rows of text & the width of the text editor is not affected
  if (d->textEdit->wordWrap() == QTextEdit::NoWrap)
  {
    if (requiredWidth > width())
    {
      if (t.isRightToLeft())
      {
        setGeometry(x() - requiredWidth + width(), y(), requiredWidth,height());
      }
      else
      {
        setGeometry(x(), y(), requiredWidth,height());
      }
    }
  }
  else
  {
    int requiredHeight = d->textEdit->heightForWidth(width());

    if (requiredHeight > height())
    {
      setGeometry(x(), y(), width(), requiredHeight);
    }
  }

   /* // allocate more space than needed. Otherwise it might be too slow
    d->length = t.length();

    // Too slow for long texts
    // QFontMetrics fm( d->textEdit->font() );
    //  int mw = fm.width( t ) + fm.width('x');
    int mw = d->fontLength * d->length;

    if (mw < width())
      mw = width();

    if (t.isRightToLeft())
      setGeometry(x() - mw + width(), y(), mw, height());
    else
      setGeometry(x(), y(), mw, height());

    d->length -= 2; */
  }

  if ( (cell()->formatType()) == Percentage_format )
  {
    if ( (t.length() == 1) && t[0].isDigit() )
    {
      QString tmp = t + " %";
      d->textEdit->setText(tmp);
      d->textEdit->setCursorPosition(0,1);
      return;
    }
  }

  canvas()->view()->editWidget()->setText( t );
  // canvas()->view()->editWidget()->setCursorPosition( d->textEdit->cursorPosition() );
}

void CellEditor::setCheckChoice(bool state)
{
  d->checkChoice = state;
}

bool CellEditor::checkChoice()
{
  if (!d->checkChoice)
    return false;

//   // prevent recursion
//   d->checkChoice = false; // TODO nescessary?

  d->length_namecell = 0;
  d->currentToken = 0;

  QString text = d->textEdit->text();
  if ( text[0] != '=' )
  {
    canvas()->setChooseMode(false);
  }
  else
  {
    int para, cur;
    d->textEdit->getCursorPosition(&para, &cur);

    Tokens tokens = d->highlighter->formulaTokens();

    // empty formula?
    if (tokens.count() < 1)
    {
      canvas()->startChoose();
    }
    else
    {
      Token token;
      for (uint i = 0; i < tokens.count(); ++i)
      {
        if (tokens[i].pos() >= cur - 1) // without '='
        {
          break;
        }
        token = tokens[i];
        d->currentToken = i;
      }

      Token::Type type = token.type();
      if (type == Token::Operator && token.asOperator() != Token::RightPar)
      {
        canvas()->setChooseMode(true);
      }
      else if (type == Token::Cell || type == Token::Range)
      {
        d->length_namecell = token.text().length();
        canvas()->setChooseMode(true);
      }
      else
      {
        canvas()->setChooseMode(false);
      }
    }
  }

//   d->checkChoice = true;

  return true;
}

void CellEditor::setUpdateChoice(bool state)
{
  d->updateChoice = state;
}

void CellEditor::updateChoice()
{
//   kdDebug() << k_funcinfo << endl;

  if (!d->updateChoice)
    return;

//   // prevent recursion
//   d->updateChoice = false; // TODO nescessary?
  d->updatingChoice = true;

  Selection* choice = d->canvas->choice();

  if (choice->isEmpty())
    return;

  if (!choice->activeElement())
    return;

  // only one element TODO
  if (++choice->constBegin() == choice->constEnd())
  {
  }

  QString name_cell = choice->activeSubRegionName();

  Tokens tokens = d->highlighter->formulaTokens();
  uint start = 1;
  uint length = 0;
  if (!tokens.empty())
  {
    Token token = tokens[d->currentToken];
    Token::Type type = token.type();
    if (type == Token::Cell || type == Token::Range)
    {
      start = token.pos() + 1; // don't forget the '='!
      length = token.text().length();
    }
    else
    {
      start = token.pos() + token.text().length() + 1;
    }
  }

  d->length_namecell = name_cell.length();
  d->length_text = text().length();
    //kdDebug(36001) << "updateChooseMarker2 len=" << d->length_namecell << endl;

  QString oldText = text();
  QString newText = oldText.left(start) + name_cell + oldText.right(d->length_text - start - length);

  setCheckChoice( false );
  setText( newText );
  setCheckChoice( true );
  setCursorPosition( start + d->length_namecell );

  d->canvas->view()->editWidget()->setText( newText );
    //kdDebug(36001) << "old=" << old << " len=" << d->length_namecell << " pos=" << pos << endl;

//   d->updateChoice = false;
  d->updatingChoice = false;
}

void CellEditor::resizeEvent( QResizeEvent* )
{
    d->textEdit->setGeometry( 0, 0, width(), height() );
}

void CellEditor::handleKeyPressEvent( QKeyEvent * _ev )
{
  if (_ev->key() == Qt::Key_F4)
  {
    if (d->textEdit == 0)
    {
      QApplication::sendEvent( d->textEdit, _ev );
      return;
    }

    QRegExp exp("(\\$?)([a-zA-Z]+)(\\$?)([0-9]+)$");

    int para,cur;
    d->textEdit->getCursorPosition(&para,&cur);
   // int cur = d->textEdit->cursorPosition();
    QString tmp, tmp2;
    int n = -1;

    // this is ugly, and sort of hack
    // FIXME rewrite to use the real Tokenizer
    unsigned i;
    for( i = 0; i < 10; i++ )
    {
      tmp =  d->textEdit->text().left( cur+i );
      tmp2 = d->textEdit->text().right( d->textEdit->text().length() - cur - i );

      n = exp.search(tmp);
      if( n >= 0 ) break;
    }

    if (n == -1) return;

    QString newPart;
    if ((exp.cap(1) == "$") && (exp.cap(3) == "$"))
      newPart = "$" + exp.cap(2) + exp.cap(4);
    else if ((exp.cap(1) != "$") && (exp.cap(3) != "$"))
      newPart = "$" + exp.cap(2) + "$" + exp.cap(4);
    else if ((exp.cap(1) == "$") && (exp.cap(3) != "$"))
      newPart = exp.cap(2) + "$" + exp.cap(4);
    else if ((exp.cap(1) != "$") && (exp.cap(3) == "$"))
      newPart = exp.cap(2) + exp.cap(4);

    QString newString = tmp.left(n);
    newString += newPart;
    cur = newString.length() - i;
    newString += tmp2;

    d->textEdit->setText(newString);
    d->textEdit->setCursorPosition( 0, cur );

    _ev->accept();

    return;
  }

  // Send the key event to the KLineEdit
  QApplication::sendEvent( d->textEdit, _ev );
}

void CellEditor::handleIMEvent( QIMEvent * _ev )
{
    // send the IM event to the KLineEdit
    QApplication::sendEvent( d->textEdit, _ev );
}

QString CellEditor::text() const
{
    return d->textEdit->text();
}

void CellEditor::setText(QString text)
{
  d->textEdit->setText(text);
  //Usability : It is usually more convenient if the cursor is positioned at the end of the text so it can
  //be quickly deleted using the backspace key

  //This also ensures that the caret is sized correctly for the text
  d->textEdit->setCursorPosition(0,text.length());

    if (d->fontLength == 0)
    {
      QFontMetrics fm( d->textEdit->font() );
      d->fontLength = fm.width('x');
    }
}

int CellEditor::cursorPosition() const
{
  int para,cur;
  d->textEdit->getCursorPosition(&para,&cur);
  return cur;
   // return d->textEdit->cursorPosition();
}

void CellEditor::setCursorPosition( int pos )
{
    d->textEdit->setCursorPosition(0,pos);
    canvas()->view()->editWidget()->setCursorPosition( pos );
}

bool CellEditor::eventFilter( QObject* o, QEvent* e )
{
    // Only interested in KTextEdit
    if ( o != d->textEdit )
        return false;
    if ( e->type() == QEvent::FocusOut )
    {
        canvas()->setLastEditorWithFocus( Canvas::CellEditor );
        return false;
    }

    if ( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease )
    {
        QKeyEvent* k = (QKeyEvent*)e;
        if (!(k->state() & Qt::ShiftButton) || canvas()->chooseMode())
        {
          //If the user presses the return key to finish editing this cell, choose mode must be turned off first
          //otherwise it will merely select a different cell
          if (k->key() == Key_Return || k->key() == Key_Enter)
          {
            kdDebug() << "CellEditor::eventFilter: canvas()->endChoose();" << endl;
            canvas()->endChoose();
          }

          //NB - Added check for Key_Return when migrating text edit from KLineEdit to KTextEdit, since
          //normal behaviour for KTextEdit is to swallow return key presses
          if ( k->key() == Key_Up || k->key() == Key_Down ||
                k->key() == Key_Next || k->key() == Key_Prior ||
                k->key() == Key_Escape || k->key() == Key_Tab ||
                k->key() == Key_Return || k->key() == Key_Enter)
          {
              // Send directly to canvas
              QApplication::sendEvent( parent(), e );
              return true;
          }
        }
        else if ( k->state() & Qt::ShiftButton && ( k->key() == Key_Return || k->key() == Key_Enter ) )
        {
            // enable content wrapping
            d->cell->format()->setMultiRow( true );
        }
        // End choosing. May be restarted by CellEditor::slotTextChanged
        if ( e->type() == QEvent::KeyPress && !k->text().isEmpty() )
        {
          canvas()->setChooseMode(false);
        }
        // forward Left/Right keys - so that pressing left/right in this
        // editor leaves editing mode ... otherwise editing is annoying
        // left/right arrows still work with the F2-editor.

        // Forward left & right arrows to parent, unless this editor has been set to capture arrow key events
        // Changed to this behaviour for consistancy with OO Calc & MS Office.
        if ( ((k->key() == Qt::Key_Left) || (k->key() == Qt::Key_Right)) && (!d->captureAllKeyEvents)) {
          QApplication::sendEvent (parent(), e);
          return true;
        }
    }

    return false;
}

void CellEditor::setCursorToRange(uint pos)
{
//   kdDebug() << k_funcinfo << endl;

  d->updatingChoice = true;
  uint counter = 0;
  Tokens tokens = d->highlighter->formulaTokens();
  for (uint i = 0; i < tokens.count(); ++i)
  {
    Token token = tokens[i];
    Token::Type type = token.type();
    if (type == Token::Cell || type == Token::Range)
    {
      if (counter == pos)
      {
        setCursorPosition(token.pos() + token.text().length() + 1);
      }
      counter++;
    }
  }
  d->updatingChoice = false;
}



/*****************************************************************************
 *
 * ComboboxLocationEditWidget
 *
 ****************************************************************************/

ComboboxLocationEditWidget::ComboboxLocationEditWidget( QWidget * _parent,
                                                      View * _view )
    : KComboBox( _parent, "ComboboxLocationEditWidget" )
{
    m_locationWidget = new LocationEditWidget( _parent, _view );
    setLineEdit( m_locationWidget );
    insertItem( "" );

    QValueList<Reference>::Iterator it;
    QValueList<Reference> area = _view->doc()->listArea();
    for ( it = area.begin(); it != area.end(); ++it )
        slotAddAreaName( (*it).ref_name);
    connect( this, SIGNAL( activated ( const QString & ) ), m_locationWidget, SLOT( slotActivateItem() ) );
}


void ComboboxLocationEditWidget::slotAddAreaName( const QString &_name)
{
    insertItem( _name );
    m_locationWidget->addCompletionItem( _name );
}

void ComboboxLocationEditWidget::slotRemoveAreaName( const QString &_name )
{
    for ( int i = 0; i<count(); i++ )
    {
        if ( text(i)==_name )
        {
            removeItem( i );
            break;
        }
    }
    m_locationWidget->removeCompletionItem( _name );
}



/*****************************************************************************
 *
 * LocationEditWidget
 *
 ****************************************************************************/

LocationEditWidget::LocationEditWidget( QWidget * _parent,
                                                      View * _view )
    : KLineEdit( _parent, "LocationEditWidget" ),
      m_pView(_view)
{
    setCompletionObject( &completionList,true );
    setCompletionMode(KGlobalSettings::CompletionAuto  );
}

void LocationEditWidget::addCompletionItem( const QString &_item )
{
    kdDebug()<<"  LocationEditWidget::addCompletionItem add :"<<_item<<endl;
    if ( completionList.items().contains( _item) == 0 )
    {
        completionList.addItem( _item );
        kdDebug()<<" _utem :"<<_item<<endl;
        kdDebug()<<" completionList.items().count()"<<completionList.items().count()<<endl;
    }
}

void LocationEditWidget::removeCompletionItem( const QString &_item )
{
    completionList.removeItem( _item );
}

void LocationEditWidget::slotActivateItem()
{
    activateItem();
}

bool LocationEditWidget::activateItem()
{
    QString ltext = text();
    QString tmp = ltext.lower();
    QValueList<Reference>::Iterator it;
    QValueList<Reference> area = m_pView->doc()->listArea();
    for ( it = area.begin(); it != area.end(); ++it )
    {
        if ((*it).ref_name == tmp)
        {
            QString tmp = (*it).sheet_name;
            tmp += "!";
            tmp += util_rangeName((*it).rect);
            m_pView->selectionInfo()->initialize( Region(m_pView,tmp) );
            return true;
        }
    }

    // Set the cell component to uppercase:
    // Sheet1!a1 -> Sheet1!A2
    int pos = ltext.find('!');
    if ( pos !=- 1 )
        tmp = ltext.left(pos)+ltext.mid(pos).upper();
    else
        tmp = ltext.upper();

    // Selection entered in location widget
    if ( ltext.contains( ':' ) )
      m_pView->selectionInfo()->initialize( Region(m_pView,tmp) );
    // Location entered in location widget
    else
    {
      Region region(m_pView,tmp);
        bool validName = true;
        for (unsigned int i = 0; i < ltext.length(); ++i)
        {
            if (!ltext[i].isLetter())
            {
                validName = false;
                break;
            }
        }
        if ( !region.isValid() && validName)
        {
            QRect rect( m_pView->selectionInfo()->selection() );
            Sheet * t = m_pView->activeSheet();
            // set area name on current selection/cell

            m_pView->doc()->addAreaName(rect, ltext.lower(), t->sheetName());
        }

        if (!validName)
        {
          m_pView->selectionInfo()->initialize(region);
        }
    }

    // Set the focus back on the canvas.
    m_pView->canvasWidget()->setFocus();
    return false;
}


void LocationEditWidget::keyPressEvent( QKeyEvent * _ev )
{
    // Do not handle special keys and accelerators. This is
    // done by QLineEdit.
    if ( _ev->state() & ( Qt::AltButton | Qt::ControlButton ) )
    {
        QLineEdit::keyPressEvent( _ev );
        // Never allow that keys are passed on to the parent.
        _ev->accept();

        return;
    }

    // Handle some special keys here. Eve
    switch( _ev->key() )
    {
    case Key_Return:
    case Key_Enter:
    {
        if ( activateItem() )
            return;
        _ev->accept();
    }
    break;
    // Escape pressed, restore original value
    case Key_Escape:
        // #### Torben says: This is duplicated code. Bad.
        if ( m_pView->selectionInfo()->isSingular() ) {
            setText( Cell::columnName( m_pView->canvasWidget()->markerColumn() )
                     + QString::number( m_pView->canvasWidget()->markerRow() ) );
        } else {
            setText( Cell::columnName( m_pView->selectionInfo()->lastRange().left() )
                     + QString::number( m_pView->selectionInfo()->lastRange().top() )
                     + ":"
                     + Cell::columnName( m_pView->selectionInfo()->lastRange().right() )
                     + QString::number( m_pView->selectionInfo()->lastRange().bottom() ) );
        }
        m_pView->canvasWidget()->setFocus();
        _ev->accept();
        break;
    default:
        QLineEdit::keyPressEvent( _ev );
        // Never allow that keys are passed on to the parent.
        _ev->accept();
    }
}



/****************************************************************
 *
 * EditWidget
 * The line-editor that appears above the sheet and allows to
 * edit the cells content.
 *
 ****************************************************************/

EditWidget::EditWidget( QWidget *_parent, Canvas *_canvas,
                                      QButton *cancelButton, QButton *okButton )
  : QLineEdit( _parent, "EditWidget" )
{
  m_pCanvas = _canvas;
  Q_ASSERT(m_pCanvas != NULL);
  // Those buttons are created by the caller, so that they are inserted
  // properly in the layout - but they are then managed here.
  m_pCancelButton = cancelButton;
  m_pOkButton = okButton;
  isArray = false;

  installEventFilter(m_pCanvas);

  if ( !m_pCanvas->doc()->isReadWrite() || !m_pCanvas->activeSheet() )
    setEnabled( false );

  QObject::connect( m_pCancelButton, SIGNAL( clicked() ),
                    this, SLOT( slotAbortEdit() ) );
  QObject::connect( m_pOkButton, SIGNAL( clicked() ),
                    this, SLOT( slotDoneEdit() ) );

  setEditMode( false ); // disable buttons
}

void EditWidget::showEditWidget(bool _show)
{
    if (_show)
  {
      m_pCancelButton->show();
      m_pOkButton->show();
      show();
  }
    else
  {
      m_pCancelButton->hide();
      m_pOkButton->hide();
      hide();
  }
}

void EditWidget::slotAbortEdit()
{
    m_pCanvas->deleteEditor( false /*discard changes*/ );
    // will take care of the buttons
}

void EditWidget::slotDoneEdit()
{
  m_pCanvas->deleteEditor( true /*keep changes*/, isArray);
  isArray = false;
  // will take care of the buttons
}

void EditWidget::keyPressEvent ( QKeyEvent* _ev )
{
    // Dont handle special keys and accelerators, except Enter ones
    if (( ( _ev->state() & ( Qt::AltButton | Qt::ControlButton ) )
         || ( _ev->state() & Qt::ShiftButton )
         || ( _ev->key() == Key_Shift )
         || ( _ev->key() == Key_Control ) )
      && (_ev->key() != Key_Return) && (_ev->key() != Key_Enter))
    {
        QLineEdit::keyPressEvent( _ev );
        _ev->accept();
        return;
    }

  if ( !m_pCanvas->doc()->isReadWrite() )
    return;

  if ( !m_pCanvas->editor() )
  {
    // Start editing the current cell
    m_pCanvas->createEditor( Canvas::CellEditor,false );
  }
  CellEditor * cellEditor = (CellEditor*) m_pCanvas->editor();

  switch ( _ev->key() )
  {
    case Key_Down:
    case Key_Up:
    case Key_Return:
    case Key_Enter:
      cellEditor->setText( text());
      // Don't allow to start a chooser when pressing the arrow keys
      // in this widget, since only up and down would work anyway.
      // This is why we call slotDoneEdit now, instead of sending
      // to the canvas.
      //QApplication::sendEvent( m_pCanvas, _ev );
      isArray = (_ev->state() & Qt::AltButton) &&
          (_ev->state() & Qt::ControlButton);
      slotDoneEdit();
      m_pCanvas->view()->updateEditWidget();
      _ev->accept();
      break;
    case Key_F2:
      cellEditor->setFocus();
      cellEditor->setText( text());
      cellEditor->setCursorPosition(cursorPosition());
      break;
    default:

      QLineEdit::keyPressEvent( _ev );

      setFocus();
      cellEditor->setCheckChoice( false );
      cellEditor->setText( text() );
      cellEditor->setCheckChoice( true );
      cellEditor->setCursorPosition( cursorPosition() );
  }
}

void EditWidget::setEditMode( bool mode )
{
  m_pCancelButton->setEnabled(mode);
  m_pOkButton->setEnabled(mode);
}

void EditWidget::focusOutEvent( QFocusEvent* ev )
{
  //kdDebug(36001) << "EditWidget lost focus" << endl;
  // See comment about setLastEditorWithFocus
  m_pCanvas->setLastEditorWithFocus( Canvas::EditWidget );

  QLineEdit::focusOutEvent( ev );
}

void EditWidget::setText( const QString& t )
{
  if ( t == text() ) // Why this? (David)
    return;

  QLineEdit::setText( t );
}



#include "kspread_editors.moc"
