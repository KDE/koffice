/* This file is part of the KDE project
   
   
   Copyright 2002-2003 Joseph Wenninger <jowenn@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 2001-2002 Laurent Montel <montel@kde.org>
   Copyright 2000 Simon Hausmann <hausmann@kde.org>
   Copyright 1999 Torben Weis <weis@kde.org>

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

#ifndef KSPREAD_VIEW_IFACE_H
#define KSPREAD_VIEW_IFACE_H

#include <KoViewIface.h>

#include <qstring.h>
#include <qrect.h>
#include <qcolor.h>

class KSpreadView;
class KSpreadCellProxy;

class KSpreadViewIface : public KoViewIface
{
    K_DCOP
public:
    KSpreadViewIface( KSpreadView* );
    ~KSpreadViewIface();

k_dcop:
    virtual DCOPRef doc() const;
    virtual DCOPRef map() const;
    virtual DCOPRef sheet() const;

    virtual void changeNbOfRecentFiles(int _nb);

    virtual void hide();
    virtual void show();

    virtual void setSelection(QRect selection);
    virtual QRect selection();
    virtual void find();
    virtual void replace();
    virtual void conditional();
    virtual void validity();
    virtual void insertSeries();
    virtual void insertHyperlink();
    virtual void gotoCell();
    virtual void changeAngle();
    virtual void preference();
    virtual void nextSheet();
    virtual void previousSheet();
    virtual bool showSheet(QString sheetName);
    virtual void sortList();
    virtual void setAreaName();
    virtual void showAreaName();
    virtual void mergeCell();
    virtual void dissociateCell();
    virtual void consolidate();

    virtual void deleteColumn();
    virtual void insertColumn();
    virtual void deleteRow();
    virtual void insertRow();
    virtual void hideRow();
    virtual void showRow();
    virtual void hideColumn();
    virtual void showColumn();
    virtual void upper();
    virtual void lower();

    virtual void equalizeColumn();
    virtual void equalizeRow();

    virtual void clearTextSelection();
    virtual void clearCommentSelection();
    virtual void clearValiditySelection();
    virtual void clearConditionalSelection();
    virtual void goalSeek();
    virtual void insertFromDatabase();
    virtual void insertFromTextfile();
    virtual void insertFromClipboard();
    virtual void textToColumns();
    virtual void copyAsText();

    virtual void setSelectionMoneyFormat( bool b );
    virtual void setSelectionPrecision( int delta );
    virtual void setSelectionPercent( bool b );
    virtual void setSelectionMultiRow( bool enable );
    virtual void setSelectionSize(int size);
    virtual void setSelectionUpper();
    virtual void setSelectionLower();
    virtual void setSelectionFirstLetterUpper();
    virtual void setSelectionVerticalText(bool enable);
    virtual void setSelectionComment(QString comment);
    virtual void setSelectionAngle(int value);
    virtual void setSelectionTextColor(QColor txtColor );
    virtual void setSelectionBgColor(QColor bgColor );
    virtual void setSelectionBorderColor(QColor bdColor );

    virtual void deleteSelection();
    virtual void copySelection();
    virtual void cutSelection();

    virtual void setLeftBorderColor(QColor color);
    virtual void setTopBorderColor(QColor color);
    virtual void setRightBorderColor(QColor color);
    virtual void setBottomBorderColor(QColor color);
    virtual void setAllBorderColor(QColor color);
    virtual void setOutlineBorderColor(QColor color);
    virtual void removeBorder();

    virtual void increaseIndent();
    virtual void decreaseIndent();

    void subtotals();
    void sortInc();
    void sortDec();
    void layoutDlg();
    void increaseFontSize();
    void decreaseFontSize();


private:
    KSpreadView* m_view;
    KSpreadCellProxy* m_proxy;
};

#endif
