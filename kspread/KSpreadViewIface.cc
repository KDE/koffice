#include "KSpreadViewIface.h"

#include "kspread_view.h"
#include "kspread_doc.h"
#include "kspread_map.h"

#include <kapplication.h>
#include <dcopclient.h>
#include <dcopref.h>

/************************************************
 *
 * KSpreadViewIface
 *
 ************************************************/

KSpreadViewIface::KSpreadViewIface( KSpreadView* t )
    : KoViewIface( t )
{
    m_view = t;
}

KSpreadViewIface::~KSpreadViewIface()
{
}

DCOPRef KSpreadViewIface::doc() const
{
    return DCOPRef( kapp->dcopClient()->appId(), m_view->doc()->dcopObject()->objId() );
}

DCOPRef KSpreadViewIface::map() const
{
    return DCOPRef( kapp->dcopClient()->appId(), m_view->doc()->map()->dcopObject()->objId() );
}

DCOPRef KSpreadViewIface::table() const
{
    return DCOPRef( kapp->dcopClient()->appId(), m_view->activeTable()->dcopObject()->objId() );
}

void KSpreadViewIface::changeNbOfRecentFiles(int _nb)
{
    if(_nb<0)
        return;
    m_view->changeNbOfRecentFiles(_nb);
}


void KSpreadViewIface::hide()
{
    m_view->hide();
}

void KSpreadViewIface::show()
{
    m_view->show();
}

void KSpreadViewIface::find()
{
    m_view->find();
}

void KSpreadViewIface::replace()
{
    m_view->replace();
}

void KSpreadViewIface::conditional()
{
    m_view->conditional();
}

void KSpreadViewIface::validity()
{
    m_view->validity();
}

void KSpreadViewIface::insertSeries()
{
    m_view->insertSeries();
}

void KSpreadViewIface::insertHyperlink()
{
    m_view->insertHyperlink();
}

void KSpreadViewIface::gotoCell()
{
    m_view->gotoCell();
}

void KSpreadViewIface::changeAngle()
{
    m_view->changeAngle();
}

void KSpreadViewIface::preference()
{
    m_view->preference();
}

void KSpreadViewIface::nextTable()
{
    m_view->nextTable();
}

void KSpreadViewIface::previousTable()
{
    m_view->previousTable();
}

void KSpreadViewIface::sortList()
{
    m_view->sortList();
}

void KSpreadViewIface::setAreaName()
{
    m_view->setAreaName();
}

void KSpreadViewIface::showAreaName()
{
    m_view->showAreaName();
}

void KSpreadViewIface::mergeCell()
{
    m_view->mergeCell();
}

void KSpreadViewIface::dissociateCell()
{
    m_view->dissociateCell();
}

void KSpreadViewIface::consolidate()
{
    m_view->consolidate();
}

void KSpreadViewIface::deleteColumn()
{
    m_view->deleteColumn();
}

void KSpreadViewIface::insertColumn()
{
    m_view->insertColumn();
}

void KSpreadViewIface::deleteRow()
{
    m_view->deleteRow();
}

void KSpreadViewIface::insertRow()
{
    m_view->insertRow();
}

void KSpreadViewIface::hideRow()
{
    m_view->hideRow();
}

void KSpreadViewIface::showRow()
{
    m_view->showRow();
}

void KSpreadViewIface::hideColumn()
{
    m_view->hideColumn();
}

void KSpreadViewIface::showColumn()
{
    m_view->showColumn();
}

void KSpreadViewIface::upper()
{
    m_view->upper();
}

void KSpreadViewIface::lower()
{
    m_view->lower();
}

void KSpreadViewIface::equalizeColumn()
{
    m_view->equalizeColumn();
}

void KSpreadViewIface::equalizeRow()
{
    m_view->equalizeRow();
}

void KSpreadViewIface::clearTextSelection()
{
    m_view->clearTextSelection();
}

void KSpreadViewIface::clearCommentSelection()
{
    m_view->clearCommentSelection();
}

void KSpreadViewIface::clearValiditySelection()
{
    m_view->clearValiditySelection();
}

void KSpreadViewIface::clearConditionalSelection()
{
    m_view->clearConditionalSelection();
}

void KSpreadViewIface::goalSeek()
{
    m_view->goalSeek();
}

void KSpreadViewIface::insertFromDatabase()
{
    m_view->insertFromDatabase();
}

void KSpreadViewIface::insertFromTextfile()
{
    m_view->insertFromTextfile();
}

void KSpreadViewIface::insertFromClipboard()
{
    m_view->insertFromClipboard();
}

void KSpreadViewIface::textToColumns()
{
    m_view->textToColumns();
}

void KSpreadViewIface::copyAsText()
{
    m_view->copyAsText();
}

void KSpreadViewIface::setSelection(QRect selection)
{
  m_view->selectionInfo()->setSelection(selection.topLeft(), selection.bottomRight(),
                                        m_view->activeTable());
}

QRect KSpreadViewIface::selection()
{
  return m_view->selectionInfo()->selection();
}

void KSpreadViewIface::setSelectionMoneyFormat( bool b )
{
  m_view->moneyFormat(b);
}

void KSpreadViewIface::setSelectionPrecision( int delta )
{
  m_view->setSelectionPrecision(delta);
}

void KSpreadViewIface::setSelectionPercent( bool b )
{
  m_view->percent(b);
}

void KSpreadViewIface::setSelectionMultiRow( bool enable )
{
  m_view->multiRow(enable);
}

void KSpreadViewIface::setSelectionSize(int size)
{
  m_view->setSelectionFontSize(size);
}

void KSpreadViewIface::setSelectionUpper()
{
  m_view->upper();
}

void KSpreadViewIface::setSelectionLower()
{
  m_view->lower();
}

void KSpreadViewIface::setSelectionFirstLetterUpper()
{
  m_view->firstLetterUpper();
}

void KSpreadViewIface::setSelectionVerticalText(bool enable)
{
  m_view->verticalText(enable);
}

void KSpreadViewIface::setSelectionComment(QString comment)
{
  m_view->setSelectionComment(comment);
}

void KSpreadViewIface::setSelectionAngle(int value)
{
  m_view->setSelectionAngle(value);
}

void KSpreadViewIface::setSelectionTextColor( QColor txtColor )
{
  m_view->setSelectionTextColor(txtColor);
}

void KSpreadViewIface::setSelectionBgColor( QColor bgColor )
{
  m_view->setSelectionBackgroundColor(bgColor);
}

void KSpreadViewIface::setSelectionBorderColor( QColor bdColor )
{
  m_view->setSelectionBorderColor(bdColor);
}

void KSpreadViewIface::deleteSelection()
{
  m_view->clearTextSelection();
}

void KSpreadViewIface::copySelection()
{
  m_view->copySelection();
}

void KSpreadViewIface::cutSelection()
{
  m_view->cutSelection();
}

void KSpreadViewIface::setLeftBorderColor(QColor color)
{
  m_view->setSelectionLeftBorderColor(color);
}

void KSpreadViewIface::setTopBorderColor(QColor color)
{
  m_view->setSelectionTopBorderColor(color);
}

void KSpreadViewIface::setRightBorderColor(QColor color)
{
  m_view->setSelectionRightBorderColor(color);
}

void KSpreadViewIface::setBottomBorderColor(QColor color)
{
  m_view->setSelectionBottomBorderColor(color);
}

void KSpreadViewIface::setAllBorderColor(QColor color)
{
  m_view->setSelectionAllBorderColor(color);
}

void KSpreadViewIface::setOutlineBorderColor(QColor color)
{
  m_view->setSelectionOutlineBorderColor(color);
}

void KSpreadViewIface::removeBorder()
{
  m_view->borderRemove();
}

void KSpreadViewIface::increaseIndent()
{
  m_view->increaseIndent();
}

void KSpreadViewIface::decreaseIndent()
{
  m_view->increaseIndent();
}

void KSpreadViewIface::subtotals()
{
    m_view->subtotals();
}

void KSpreadViewIface::sortInc()
{
    m_view->sortInc();
}

void KSpreadViewIface::sortDec()
{
    m_view->sortDec();
}

void KSpreadViewIface::layoutDlg()
{
    m_view->layoutDlg();
}


void KSpreadViewIface::increaseFontSize()
{
    m_view->increaseFontSize();
}

void KSpreadViewIface::decreaseFontSize()
{
    m_view->decreaseFontSize();
}
