/* This file is part of the KDE project
   Copyright (C) 2005-2006 Raphael Langerhorst <raphael.langerhorst@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

// Local
#include "SheetSelectPage.h"

// standard C/C++ includes
// Qt includes
// KDE includes
#include <kdebug.h>

// KOffice includes
// KSpread includes

using namespace KSpread;

SheetSelectPage::SheetSelectPage( QWidget *parent )
    : QWidget(parent)
{
  setupUi(this);
  setWindowTitle(i18n("Sheets"));

  //disabling automated sorting
  ListViewAvailable->setSorting(-1);
  ListViewSelected->setSorting(-1);

  //setup icons
  ButtonSelectAll->setIcon(KIcon("go-last"));
  ButtonSelect->setIcon(KIcon("go-next"));
  ButtonRemove->setIcon(KIcon("go-previous"));
  ButtonRemoveAll->setIcon(KIcon("go-first"));

  ButtonMoveTop->setIcon(KIcon("go-top"));
  ButtonMoveUp->setIcon(KIcon("go-up"));
  ButtonMoveDown->setIcon(KIcon("go-down"));
  ButtonMoveBottom->setIcon(KIcon("go-bottom"));

  //connect buttons
  connect(ButtonSelectAll,SIGNAL(clicked()),this,SLOT(selectAll()));
  connect(ButtonSelect,SIGNAL(clicked()),this,SLOT(select()));
  connect(ButtonRemove,SIGNAL(clicked()),this,SLOT(remove()));
  connect(ButtonRemoveAll,SIGNAL(clicked()),this,SLOT(removeAll()));

  connect(ButtonMoveTop,SIGNAL(clicked()),this,SLOT(moveTop()));
  connect(ButtonMoveUp,SIGNAL(clicked()),this,SLOT(moveUp()));
  connect(ButtonMoveDown,SIGNAL(clicked()),this,SLOT(moveDown()));
  connect(ButtonMoveBottom,SIGNAL(clicked()),this,SLOT(moveBottom()));

  connect(selectedSheetsButton, SIGNAL(toggled(bool)), ButtonSelectAll, SLOT(setEnabled(bool)));
  connect(selectedSheetsButton, SIGNAL(toggled(bool)), ButtonSelect, SLOT(setEnabled(bool)));
  connect(selectedSheetsButton, SIGNAL(toggled(bool)), ButtonRemove, SLOT(setEnabled(bool)));
  connect(selectedSheetsButton, SIGNAL(toggled(bool)), ButtonRemoveAll, SLOT(setEnabled(bool)));
  connect(selectedSheetsButton, SIGNAL(toggled(bool)), ButtonMoveTop, SLOT(setEnabled(bool)));
  connect(selectedSheetsButton, SIGNAL(toggled(bool)), ButtonMoveUp, SLOT(setEnabled(bool)));
  connect(selectedSheetsButton, SIGNAL(toggled(bool)), ButtonMoveDown, SLOT(setEnabled(bool)));
  connect(selectedSheetsButton, SIGNAL(toggled(bool)), ButtonMoveBottom, SLOT(setEnabled(bool)));
  connect(selectedSheetsButton, SIGNAL(toggled(bool)), ListViewAvailable, SLOT(setEnabled(bool)));
  connect(selectedSheetsButton, SIGNAL(toggled(bool)), ListViewSelected, SLOT(setEnabled(bool)));
}

// SheetSelectPage::~SheetSelectPage()
// {
// }

/*
void SheetSelectPage::getOptions( QMap<QString,QString>& opts, bool incldef )
{
  QStringList sheetlist = this->selectedSheets();
  QStringList::iterator it;
  unsigned int i = 0;
  for (it = sheetlist.begin(); it != sheetlist.end(); ++it, i++)
  {
    opts.insert(printOptionForIndex(i),*it);
  }
}

void SheetSelectPage::setOptions( const QMap<QString,QString>& opts )
{
  unsigned int i = 0;
  QStringList sheetlist;
  while (opts.contains(printOptionForIndex(i)))
  {
    sheetlist.prepend(opts[printOptionForIndex(i++)]);
  }

  QStringList::iterator it;
  for (it = sheetlist.begin(); it != sheetlist.end(); ++it)
  {
    kDebug() <<" adding sheet to list of printed sheets:" << *it;
    this->prependSelectedSheet(*it);
  }
}
*/

void SheetSelectPage::setAvailableSheets(const QStringList& sheetlist)
{
  foreach(QString sname, sheetlist)
  {
    kDebug() <<" adding sheet to list of printed sheets:" << sname;
    this->prependSelectedSheet(sname);
  }

}

bool SheetSelectPage::isValid(QString& /*msg*/)
{
  // we print the activeSheet() by default if no sheet is selected,
  // so we return true in any case

//   if (ListViewSelected->childCount() < 1)
//   {
//     msg = i18n("No sheets selected for printing!");
//     return false;
//   }
  return true;
}

QString SheetSelectPage::printOptionForIndex(unsigned int index)
{
  return QString("sheetprintorder%1").arg(index);
}

void SheetSelectPage::prependAvailableSheet(const QString& sheetname)
{
  new Q3ListViewItem(ListViewAvailable,sheetname);
}

void SheetSelectPage::prependSelectedSheet(const QString& sheetname)
{
  new Q3ListViewItem(ListViewSelected,sheetname);
}

QStringList SheetSelectPage::selectedSheets()
{
  QStringList list;
  Q3ListViewItem* item = ListViewSelected->firstChild();
  while (item)
  {
    list.append(item->text(0));
    item = item->nextSibling();
  }
  return list;
}

/*
QStringList SheetSelectPage::selectedSheets(QPrinter &prt)
{
  QStringList list;
  unsigned int index;
  const QMap<QString,QString>& options = prt.options();
  for (index = 0; options.contains(SheetSelectPage::printOptionForIndex(index)); index++)
  {
    list.append(options[SheetSelectPage::printOptionForIndex(index)]);
  }
  return list;
}
*/

void SheetSelectPage::clearSelection()
{
  ListViewSelected->clear();
}

void SheetSelectPage::selectAll()
{
  //we have to add all the stuff in reverse order
  // because inserted items (prependSelectedSheet) are prepended
  QStringList list;
  Q3ListViewItem* item = ListViewAvailable->firstChild();
  while (item)
  {
    list.prepend(item->text(0));
    item = item->nextSibling();
  }
  QStringList::iterator it;
  for (it = list.begin(); it != list.end(); ++it)
  {
    this->prependSelectedSheet(*it);
  }
}

void SheetSelectPage::select()
{
  //we have to add all the stuff in reverse order
  // because inserted items (prependSelectedSheet) are prepended
  QStringList list;
  Q3ListViewItem* item = ListViewAvailable->firstChild();
  while (item)
  {
    if (item->isSelected())
      list.prepend(item->text(0));
    item = item->nextSibling();
  }
  QStringList::iterator it;
  for (it = list.begin(); it != list.end(); ++it)
  {
    this->prependSelectedSheet(*it);
  }
}

void SheetSelectPage::remove()
{
  Q3ListViewItem* item = ListViewSelected->firstChild();
  Q3ListViewItem* nextitem = 0;
  while (item)
  {
    nextitem = item->nextSibling();
    if (item->isSelected())
      delete item;
    item = nextitem;
  }
}

void SheetSelectPage::removeAll()
{
  ListViewSelected->clear();
}


void SheetSelectPage::moveTop()
{
  //this creates a temporary new list (selected first, then rest)
  // which replaces the existing one, to avoid the need of an additional sort column

  QList<Q3ListViewItem*> newlist;
  Q3ListViewItem* item = ListViewSelected->firstChild();
  Q3ListViewItem* nextitem = 0;
//   kDebug() <<"Filling new list with selected items first";
  while (item)
  {
    nextitem = item->nextSibling();
    if (item->isSelected())
    {
      newlist.prepend(item);
      ListViewSelected->takeItem(item);
    }
    item = nextitem;
  }
//   kDebug() <<"Appending the rest";
  item = ListViewSelected->firstChild();
  while (item)
  {
//     kDebug() <<" processing item" << item->text(0);
    nextitem = item->nextSibling();
    if (!item->isSelected())
    {
      newlist.prepend(item);
      ListViewSelected->takeItem(item);
    }
    item = nextitem;
  }

//   kDebug() <<"Refill the view with the correctly ordered list";
  //the view is empty now, refill in correct order (reversed!!)
  QList<Q3ListViewItem*>::iterator it;
  for (it = newlist.begin(); it != newlist.end(); ++it)
  {
//     kDebug() <<" adding" << (*it)->text(0);
    ListViewSelected->insertItem(*it);
  }
}

void SheetSelectPage::moveUp()
{
  //this creates a temporary new list
  // which replaces the existing one, to avoid the need of an additional sort column

  QList<Q3ListViewItem*> newlist;
  Q3ListViewItem* item = ListViewSelected->firstChild();
  Q3ListViewItem* nextitem = 0;
  while (item)
  {
    nextitem = item->nextSibling();
    if (!item->isSelected())
    {
      while (nextitem && nextitem->isSelected())
      {
        Q3ListViewItem* nextnextitem = nextitem->nextSibling();
        newlist.prepend(nextitem);
        ListViewSelected->takeItem(nextitem);
        nextitem = nextnextitem;
      }
    }

    newlist.prepend(item);
    ListViewSelected->takeItem(item);
    item = nextitem;
  }

//   kDebug() <<"Refill the view with the correctly ordered list";
  //the view is empty now, refill in correct order (reversed!!)
  QList<Q3ListViewItem*>::iterator it;
  for (it = newlist.begin(); it != newlist.end(); ++it)
  {
//     kDebug() <<" adding" << (*it)->text(0);
    ListViewSelected->insertItem(*it);
  }
}

void SheetSelectPage::moveDown()
{
  Q3ListViewItem* item = ListViewSelected->lastItem();
//   while (item)
//   {
//     nextitem = item->nextSibling();
//     if (previousitem && previousitem->isSelected())
//     {
//       previousitem->moveItem(item);
//     }
//     previousitem = item;
//     item = nextitem;
//   }
  while (item)
  {
    while (item && !item->isSelected() && item->itemAbove() && item->itemAbove()->isSelected())
    {
      Q3ListViewItem* tempitem = item->itemAbove();
      tempitem->moveItem(item);
    }
    if (item)
      item = item->itemAbove();
  }
}

void SheetSelectPage::moveBottom()
{
  //this creates a temporary new list (unselected first, then rest)
  // which replaces the existing one, to avoid the need of an additional sort column

  QList<Q3ListViewItem*> newlist;
  Q3ListViewItem* item = ListViewSelected->firstChild();
  Q3ListViewItem* nextitem = 0;
//   kDebug() <<"Filling new list with unselected items first";
  while (item)
  {
//     kDebug() <<" processing item" << item->text(0);
    nextitem = item->nextSibling();
    if (!item->isSelected())
    {
      newlist.prepend(item);
      ListViewSelected->takeItem(item);
    }
    item = nextitem;
  }
//   kDebug() <<"Appending the rest";
  item = ListViewSelected->firstChild();
  while (item)
  {
    nextitem = item->nextSibling();
    if (item->isSelected())
    {
      newlist.prepend(item);
      ListViewSelected->takeItem(item);
    }
    item = nextitem;
  }

//   kDebug() <<"Refill the view with the correctly ordered list";
  //the view is empty now, refill in correct order (reversed!!)
  QList<Q3ListViewItem*>::iterator it;
  for (it = newlist.begin(); it != newlist.end(); ++it)
  {
//     kDebug() <<" adding" << (*it)->text(0);
    ListViewSelected->insertItem(*it);
  }
}

#include "SheetSelectPage.moc"
