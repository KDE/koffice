/* This file is part of the KDE project

   Copyright 1999-2004 The KSpread Team <koffice-devel@kde.org>

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
#include <qvbox.h>
#include <private/qrichtext_p.h>

//#include <klineedit.h>
#include <ktextedit.h>
#include <qapplication.h>
#include <qbutton.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qregexp.h>
#include <kdebug.h>

#include "highlight_range.h"

using namespace KSpread;

/********************************************
 *
 * CellEditor
 *
 ********************************************/

CellEditor::CellEditor( Cell* _cell, Canvas* _parent, const char* _name )
  : QWidget( _parent, _name )
{
  m_pCell = _cell;
  m_pCanvas = _parent;
  setFocusPolicy( QWidget::StrongFocus );
}

CellEditor::~CellEditor()
{
}


/********************************************
 *
 * FormulaEditorHighlighter
 *
 ********************************************/

namespace KSpread
{
FormulaEditorHighlighter::FormulaEditorHighlighter(QTextEdit* textEdit, Canvas* canvas)

  : QSyntaxHighlighter(textEdit) , m_pCanvas(canvas), _refsChanged(true)
{
  m_colors.push_back(Qt::red);
  m_colors.push_back(Qt::blue);
  m_colors.push_back(Qt::magenta);
  m_colors.push_back(Qt::darkRed);
  m_colors.push_back(Qt::darkGreen);
  m_colors.push_back(Qt::darkMagenta);
  m_colors.push_back(Qt::darkCyan);
  m_colors.push_back(Qt::darkYellow);
}


bool FormulaEditorHighlighter::referencesChanged()
{
  bool result=_refsChanged;
  _refsChanged=false;
  return result;
}

/*
Cell* TextEditorHighlighter::cellRefAt(int position, QColor& outCellColor)
{
  outCellColor=Qt::black; //outCellColor is black if there is no cell ref at the specified position

  QString t=textEdit()->text();

  if ( (t.length() < 1) || (t[0] != '=') )
    return 0;

  int startPos=-1,endPos=t.length();

  //Walk backwards to get start of ref name
  for (int i=position;i>=0;i--)
  {
    QChar ch=t[i];
    if ( (!ch.isLetterOrNumber()) && (ch != '$') && (ch != '!') )
    {
      //Replace the 'QChar(0)' bit below with '0' and compile with gcc 3.3 -
      //it seems that someone doesn't think too much of the C++ standard
      if (i != position)
      {
        startPos=i+1;
        break;
      }
      else
        //If the cursor is positioned at the end of a cell reference,
        //move the search starting position backwards
        position=i;
    }
  }

  //Walk forwards to get end of ref name
  for (unsigned int i=position;i<t.length();i++)
  {
    QChar ch=t[i];
    if ( (!ch.isLetterOrNumber()) && (ch != '$') && (ch != '!') )
    {
      endPos=i;
      break;
    }
  }

  if (startPos == -1)
    return 0;

  QString ref=t.mid(startPos,endPos-startPos);

  KSpreadPoint pt(ref,_sheet->workbook(),_sheet);

  if (pt.isValid())
  {
    //Get colour at position
    int oldPara,oldIndex;

    //setCursorPosition emits a cursorPositionChanged signal, we don't want that to happen in case
    //this function is called as a result of a cursor position change.
    textEdit()->blockSignals(true);
    textEdit()->getCursorPosition(&oldPara,&oldIndex);
    textEdit()->setCursorPosition(oldPara,position);

    outCellColor=textEdit()->color();

    textEdit()->setCursorPosition(oldPara,oldIndex);
    textEdit()->blockSignals(false);

//    kdDebug() << "Cell ref color at " << position << " - Red (" << outCellColor.red() << ") Green (" << outCellColor.green() <<
//        ") Blue (" << outCellColor.blue() << ")" << endl;

    return pt.cell();
  }

  return 0;
}*/

void FormulaEditorHighlighter::getReferences(std::vector< KSharedPtr<HighlightRange> >* /*cellRefs*/)
{
/*  if (!cellRefs)
    return;

  for (unsigned int i=0;i<_refs.size();i++)
  {
    QString rangeReference=_refs[i];
    
    //If this reference is given as a single point (eg. A1 or Sheet1!A1) it must be transformed
    //into a range reference (ie. A1 -> A1:A1)
    if (_refs[i].find(':') == -1)
    {
        int sheetNameSeparator=_refs[i].find('!') + 1;
        
        rangeReference += ":"+_refs[i].mid(sheetNameSeparator);
    }
      
    HighlightRange* hr=new EditorHighlightRange(rangeReference,_sheet->workbook(),_sheet,textEdit());
    hr->setColor(m_colors[i%m_colors.size()]);
    cellRefs->push_back(KSharedPtr<HighlightRange>(hr));
  }*/
}


int FormulaEditorHighlighter::highlightParagraph(const QString& text, int /* endStateOfLastPara */)
{
  _refs.clear();
  _refsChanged=true;

  int wordStart=-1;

  setFormat(0,text.length(),Qt::black);

  for (unsigned int i=0;i<text.length();i++)
  {
    const QChar ch=text[i];

    //Formulas must start with an equals sign ,
    //if this is not a formula then don't highlight the text
    if ( (i==0) && (ch != '='))
      return 0;



    if ( (ch.isLetterOrNumber()) || (ch=='!') ||(ch=='$') || (ch==':'))
    {
      if (wordStart == -1)
        wordStart=i;
      //setFormat(i,1,QColor(255,0,0));
    }
    else
    {
      if (wordStart != -1)
      {
        const QString cellRef=text.mid(wordStart,i-wordStart);
        QString relativeRef=cellRef.lower();

        //relativeRef.remove('$');

        if (relativeRef.find('!') == -1)
        {
          QString sheetName;
          if (m_pCanvas->chooseMode())
          {
            sheetName = m_pCanvas->choice()->sheet()->sheetName().lower();
          }
          else
          {
            sheetName = m_pCanvas->selectionInfo()->sheet()->sheetName().lower();
          }
          relativeRef.prepend(sheetName + "!");
        }

        bool cellRefValid=false;

        Region rg(m_pCanvas->view(),cellRef);
        cellRefValid = rg.isValid();
//         if (cellRef.find(':' ) != -1)
//         {
//           Range rg(cellRef,m_pCanvas->activeSheet()->workbook(),m_pCanvas->activeSheet());
//           //Check that the syntax of the range is valid and that
//           //the range is not an invalid rectangle.
//           cellRefValid=(rg.isValid());
//         }
//         else
//         {
//           Point pt(cellRef,m_pCanvas->activeSheet()->workbook(),m_pCanvas->activeSheet());
//           cellRefValid=pt.isValid();
//         }

        if (cellRefValid)
        {
          int refIndex=-1;

          //Check to see if this cell has been referenced earlier in the formula
          //if it has been, we should use the same colour as previously
          for (unsigned int k=0;k<_refs.size();k++)
            if (_refs[k]==relativeRef)
              refIndex=k;


          if (refIndex == -1)
          {
            _refs.push_back(relativeRef);
            //_refsChanged=true;
            refIndex=_refs.size()-1;

            Region region(m_pCanvas->view(), relativeRef);
            m_pCanvas->choice()->extend(region);
          }

          QColor clr = m_colors[refIndex % m_colors.size()];

          setFormat(wordStart,i-wordStart,clr);
        }

        wordStart = -1;
      }
    }
  }

  return 0;
}
}

/********************************************
 *
 * EditorHighlightRange
 *
 ********************************************/

/*EditorHighlightRange::EditorHighlightRange(const QString& rangeReference, Map* workbook, Sheet* sheet, QTextEdit* editor)
: HighlightRange(rangeReference,workbook,sheet) , _editor(editor) {}

void EditorHighlightRange::setRange(QRect& newArea)
{
    QRect oldArea=range();

    if (oldArea == newArea)
        return;

    QString firstCellName = Cell::name(range().left(),range().top());
    QString lastCellName = Cell::name(range().right(),range().bottom());


    Range::setRange( newArea );

    //Replace references in associated KTextEdit
    QRegExp searchExpression;

    QString expr;

    //Does this old range represent a single cell or multiple cells?
    if ( (oldArea.width() == 1) && (oldArea.height() == 1) )
    {
        expr=QString("\\b(%1!|)(%2:%3|%2)\\b")
                                        .arg(sheet()->sheetName())
                                        .arg(firstCellName)
                                        .arg(lastCellName);
        searchExpression=QRegExp( expr
                                        ,false);
    }
    else
    {
        expr=QString("\\b(%1!|)(%2:%3)\\b")
                                        .arg(sheet()->sheetName())
                                        .arg(firstCellName)
                                        .arg(lastCellName);
        searchExpression=QRegExp( expr
                                        ,false);
    }

    QString replacementText=toString();

    QString newText=_editor->text();
    newText=newText.replace(searchExpression,replacementText);

    _editor->setText(newText);
} */

/********************************************
 *
 * FunctionCompletion
 *
 ********************************************/

class FunctionCompletion::Private
{
public:
  TextEditor* editor;
  QVBox *completionPopup;
  KListBox *completionListBox;
};

FunctionCompletion::FunctionCompletion( TextEditor* editor ): 
QObject( editor )
{
  d = new Private;
  d->editor = editor;
  
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
}

FunctionCompletion::~FunctionCompletion()
{
      delete d;
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

/********************************************
 *
 * TextEditor
 *
 ********************************************/


TextEditor::TextEditor( Cell* _cell, Canvas* _parent, bool captureAllKeyEvents, const char* _name )
  : CellEditor( _cell, _parent, _name ),
    m_captureAllKeyEvents(captureAllKeyEvents),
    m_sizeUpdate(false),
    m_length(0),
    m_fontLength(0)
{
 // m_pEdit = new KLineEdit( this );
  m_pEdit = new KTextEdit(this);

//TODO - Get rid of QTextEdit margins, this doesn't seem easily possible in Qt 3.3, so a job for Qt 4 porting.

  m_pEdit->setHScrollBarMode(QScrollView::AlwaysOff);
  m_pEdit->setVScrollBarMode(QScrollView::AlwaysOff);
  m_pEdit->setFrameStyle(QFrame::NoFrame);
  m_pEdit->setLineWidth(0);
  m_pEdit->installEventFilter( this );

  m_highlighter = new FormulaEditorHighlighter(m_pEdit, _parent);

  functionCompletion = new FunctionCompletion( this );
  functionCompletionTimer = new QTimer( this );
  connect( functionCompletion, SIGNAL( selectedCompletion( const QString& ) ),
    SLOT( functionAutoComplete( const QString& ) ) );
  connect( m_pEdit, SIGNAL( textChanged() ), SLOT( checkFunctionAutoComplete() ) );    
  connect( functionCompletionTimer, SIGNAL( timeout() ), 
    SLOT( triggerFunctionAutoComplete() ) );

  if (!cell()->format()->multiRow(cell()->column(),cell()->row()))
  {
    m_pEdit->setWordWrap(QTextEdit::NoWrap);
  }

//TODO - Custom KTextEdit class which supports text completion
/*
  m_pEdit->setFrame( false );
  m_pEdit->setCompletionMode((KGlobalSettings::Completion)canvas()->view()->doc()->completionMode()  );
  m_pEdit->setCompletionObject( &canvas()->view()->doc()->completion(),true );
*/
  setFocusProxy( m_pEdit );

  connect( m_pEdit, SIGNAL( cursorPositionChanged(int,int) ), this, SLOT (slotCursorPositionChanged(int,int)));
  connect( m_pEdit, SIGNAL( cursorPositionChanged(QTextCursor*) ), this, SLOT (slotTextCursorChanged(QTextCursor*)));
  connect( m_pEdit, SIGNAL( textChanged() ), this, SLOT( slotTextChanged() ) );

// connect( m_pEdit, SIGNAL(completionModeChanged( KGlobalSettings::Completion )),this,SLOT (slotCompletionModeChanged(KGlobalSettings::Completion)));

  // A choose should always start at the edited cell
//  canvas()->setChooseMarkerRow( canvas()->markerRow() );
//  canvas()->setChooseMarkerColumn( canvas()->markerColumn() );

  m_blockCheck = false;

  // set font size according to zoom factor
  QFont font( _cell->format()->font() );
  font.setPointSizeFloat( 0.01 * _parent->doc()->zoom() * font.pointSizeFloat() );
  m_pEdit->setFont( font );

  if (m_fontLength == 0)
  {
    QFontMetrics fm( m_pEdit->font() );
    m_fontLength = fm.width('x');
  }
}

TextEditor::~TextEditor()
{
  delete m_highlighter;
  delete functionCompletion;
  delete functionCompletionTimer;
  canvas()->endChoose();
}

QPoint TextEditor::globalCursorPosition() const
{
  return globalCursorPos;
}


void TextEditor::checkFunctionAutoComplete()
{  
  functionCompletionTimer->stop();
  functionCompletionTimer->start( 500, true );
}

void TextEditor::triggerFunctionAutoComplete()
{  
  // tokenize the expression (don't worry, this is very fast)
  int para = 0, curPos = 0;
  m_pEdit->getCursorPosition( &para, &curPos );
  QString subtext = m_pEdit->text().left( curPos );

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
  functionCompletion->showCompletion( choices );
}

void TextEditor::functionAutoComplete( const QString& item )
{
  if( item.isEmpty() ) return;

  int para = 0, curPos = 0;
  m_pEdit->getCursorPosition( &para, &curPos );
  QString subtext = text().left( curPos );

  KSpread::Formula f;
  KSpread::Tokens tokens = f.scan( subtext );
  if( !tokens.valid() ) return;
  if( tokens.count()<1 ) return;

  KSpread::Token lastToken = tokens[ tokens.count()-1 ];
  if( !lastToken.isIdentifier() ) return;
  
  m_pEdit->blockSignals( true );
  m_pEdit->setSelection( 0, lastToken.pos()+1, 0, lastToken.pos()+lastToken.text().length()+1 );
  m_pEdit->insert( item );
  m_pEdit->blockSignals( false );
}

void TextEditor::slotCursorPositionChanged(int /* para */,int /* pos */)
{
//  m_highlighter->cellRefAt(pos);

/*  if (m_highlighter->referencesChanged())
  {
    std::vector< KSharedPtr<HighlightRange> >* highlightedCells=
        new std::vector< KSharedPtr<HighlightRange> >;

    m_highlighter->getReferences(highlightedCells);

    canvas()->setHighlightedRanges(highlightedCells);

    delete highlightedCells;
  }*/
}

void TextEditor::slotTextCursorChanged(  QTextCursor* cursor )
{
  QTextStringChar *chr = cursor->paragraph()->at( cursor->index() );
  int h = cursor->paragraph()->lineHeightOfChar( cursor->index() );
  int x = cursor->paragraph()->rect().x() + chr->x;
  int y, dummy;
  cursor->paragraph()->lineHeightOfChar( cursor->index(), &dummy, &y );
  y += cursor->paragraph()->rect().y();

  globalCursorPos = m_pEdit->mapToGlobal( m_pEdit-> contentsToViewport( QPoint( x, y + h ) ) );

}

void TextEditor::cut()
{
    if(m_pEdit)
        m_pEdit->cut();
}

void TextEditor::paste()
{
    if( m_pEdit)
        m_pEdit->paste();
}

void TextEditor::copy()
{
    if( m_pEdit)
        m_pEdit->copy();
}

void TextEditor::setEditorFont(QFont const & font, bool updateSize)
{
  if (!m_pEdit)
    return;

  QFont tmpFont( font );
  tmpFont.setPointSizeFloat( 0.01 * canvas()->doc()->zoom() * tmpFont.pointSizeFloat() );
  m_pEdit->setFont( tmpFont );

  if (updateSize)
  {
    QFontMetrics fm( m_pEdit->font() );
    m_fontLength = fm.width('x');

    int mw = fm.width( m_pEdit->text() ) + m_fontLength;
    // don't make it smaller: then we would have to repaint the obscured cells
    if (mw < width())
      mw = width();

    int mh = fm.height();
    if (mh < height())
      mh = height();

    setGeometry(x(), y(), mw, mh);
    m_sizeUpdate = true;
  }
}

void TextEditor::slotCompletionModeChanged(KGlobalSettings::Completion _completion)
{
  canvas()->view()->doc()->setCompletionMode( _completion );
}

void TextEditor::slotTextChanged( /*const QString& t*/ )
{
   //FIXME - text() may return richtext?
  QString t=text();

  // if ( canvas->chooseCursorPosition() >= 0 )
  // m_pEdit->setCursorPosition( canvas->chooseCursorPosition() );
  if (!checkChoose())
    return;

  if (t.length() > m_length)
  {
  m_length = t.length();

  QFontMetrics fm(m_pEdit->font());
  int requiredWidth=fm.width(t) + (2*fm.width('x')); // - requiredWidth = width of text plus some spacer characters

  //For normal single-row cells, the text editor must be expanded horizontally to
  //allow the text to fit if the new text is too wide
  //For multi-row (word-wrap enabled) cells, the text editor must expand vertically to
  //allow for new rows of text & the width of the text editor is not affected
  if (m_pEdit->wordWrap() == QTextEdit::NoWrap)
  {


    if (requiredWidth > width())
    {
      if (t.isRightToLeft())
      {

        setGeometry(x() - requiredWidth + width(),y(),requiredWidth,height());
      }
      else
      {
        setGeometry(x(),y(),requiredWidth,height());
      }
    }
  }
  else
  {
    int requiredHeight=m_pEdit->heightForWidth(width());

    if (requiredHeight > height())
      setGeometry(x(),y(),width(),requiredHeight);

  }

   /* // allocate more space than needed. Otherwise it might be too slow
    m_length = t.length();

    // Too slow for long texts
    // QFontMetrics fm( m_pEdit->font() );
    //  int mw = fm.width( t ) + fm.width('x');
    int mw = m_fontLength * m_length;

    if (mw < width())
      mw = width();

    if (t.isRightToLeft())
      setGeometry(x() - mw + width(), y(), mw, height());
    else
      setGeometry(x(), y(), mw, height());

    m_length -= 2; */
  }

  if ( (cell()->formatType()) == Percentage_format )
  {
    if ( (t.length() == 1) && t[0].isDigit() )
    {
      QString tmp = t + " %";
      m_pEdit->setText(tmp);
      m_pEdit->setCursorPosition(0,1);
      return;
    }
  }

  canvas()->view()->editWidget()->setText( t );
  // canvas()->view()->editWidget()->setCursorPosition( m_pEdit->cursorPosition() );
}

bool TextEditor::checkChoose()
{
    if ( m_blockCheck )
        return false;

    QString t = m_pEdit->text();
    if ( t[0] != '=' )
        canvas()->endChoose();
    else
    {
  int para,cur;
  m_pEdit->getCursorPosition(&para,&cur);

      QChar r = t[ cur - 1 - canvas()->chooseTextLen() ];
      if ( ( r == '*' || r == '|' || r == '&' || r == '-' ||
             r == '+' || r == '/' || r == '!' || r == '(' ||
             r == '^' || r == ',' || r == '%' || r == '[' ||
             r == '{' || r == '~' || r == '=' || r == ';' ||
       r == '>' || r == '<') )
  {

          canvas()->startChoose();
      }
      else
      {
        //kdDebug() << "canvas()->endChoose(); 1" << endl;
        //  canvas()->endChoose();
      }
    }
    return true;
}

void TextEditor::resizeEvent( QResizeEvent* )
{
    m_pEdit->setGeometry( 0, 0, width(), height() );
}

void TextEditor::handleKeyPressEvent( QKeyEvent * _ev )
{
  if (_ev->key() == Qt::Key_F4)
  {
    if (m_pEdit == 0)
    {
      QApplication::sendEvent( m_pEdit, _ev );
      return;
    }

    QRegExp exp("(\\$?)([a-zA-Z]+)(\\$?)([0-9]+)$");

    int para,cur;
    m_pEdit->getCursorPosition(&para,&cur);
   // int cur = m_pEdit->cursorPosition();
    QString tmp, tmp2;
    int n = -1;

    // this is ugly, and sort of hack
    // FIXME rewrite to use the real Tokenizer
    unsigned i;
    for( i = 0; i < 10; i++ )
    {
      tmp =  m_pEdit->text().left( cur+i );
      tmp2 = m_pEdit->text().right( m_pEdit->text().length() - cur - i );

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

    m_pEdit->setText(newString);
    m_pEdit->setCursorPosition( 0, cur );

    _ev->accept();

    return;
  }

  // Send the key event to the KLineEdit
  QApplication::sendEvent( m_pEdit, _ev );
}

void TextEditor::handleIMEvent( QIMEvent * _ev )
{
    // send the IM event to the KLineEdit
    QApplication::sendEvent( m_pEdit, _ev );
}

QString TextEditor::text() const
{
    return m_pEdit->text();
}

void TextEditor::setText(QString text)
{
    if (m_pEdit != 0)
  {
          m_pEdit->setText(text);
    //Usability : It is usually more convenient if the cursor is positioned at the end of the text so it can
    //be quickly deleted using the backspace key

    //This also ensures that the caret is sized correctly for the text
    m_pEdit->setCursorPosition(0,text.length());
  }

    if (m_fontLength == 0)
    {
      QFontMetrics fm( m_pEdit->font() );
      m_fontLength = fm.width('x');
    }
}

int TextEditor::cursorPosition() const
{
  int para,cur;
  m_pEdit->getCursorPosition(&para,&cur);
  return cur;
   // return m_pEdit->cursorPosition();
}

void TextEditor::setCursorPosition( int pos )
{
    m_pEdit->setCursorPosition(0,pos);
    canvas()->view()->editWidget()->setCursorPosition( pos );
    checkChoose();
}

void TextEditor::insertFormulaChar(int /*c*/)
{
}

bool TextEditor::eventFilter( QObject* o, QEvent* e )
{
    // Only interested in KTextEdit
    if ( o != m_pEdit )
        return false;
    if ( e->type() == QEvent::FocusOut )
    {
        canvas()->setLastEditorWithFocus( Canvas::CellEditor );
        return false;
    }

    if ( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease )
    {
        QKeyEvent* k = (QKeyEvent*)e;
        if ( !( k->state() & Qt::ShiftButton )|| canvas()->chooseFormulaArea())
        {
          //If the user presses the return key to finish editing this cell, choose mode must be turned off first
          //otherwise it will merely select a different cell
          if (k->key() == Key_Return || k->key() == Key_Enter)
          {
            kdDebug() << "canvas()->endChoose(); 2" << endl;
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
        // End choosing. May be restarted by TextEditor::slotTextChanged
        if ( e->type() == QEvent::KeyPress && !k->text().isEmpty() )
        {
            //kdDebug(36001) << "eventFilter End Choose" << endl;
//           kdDebug() << "canvas()->endChoose(); 3" << endl;
//           canvas()->endChoose();

            //kdDebug(36001) << "Cont" << endl;
        }
        // forward Left/Right keys - so that pressing left/right in this
        // editor leaves editing mode ... otherwise editing is annoying
        // left/right arrows still work with the F2-editor.
        
        // Forward left & right arrows to parent, unless this editor has been set to capture arrow key events
        // Changed to this behaviour for consistancy with OO Calc & MS Office.
        if ( ((k->key() == Qt::Key_Left) || (k->key() == Qt::Key_Right)) && (!m_captureAllKeyEvents)) {
          QApplication::sendEvent (parent(), e);
          return true;
        }
    }

    return false;
}

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
  TextEditor * cellEditor = (TextEditor*) m_pCanvas->editor();

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
      cellEditor->blockCheckChoose( true );
      cellEditor->setText( text() );
      cellEditor->blockCheckChoose( false );
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
