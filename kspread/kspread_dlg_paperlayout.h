/* This file is part of the KDE project
   Copyright (C) 2002 Montel Laurent <lmontel@mandrakesoft.com>
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

#ifndef __kspread_dlg_paperlayout__
#define __kspread_dlg_paperlayout__

#include <kdialogbase.h>
#include <koPageLayoutDia.h>
class KSpreadTable;
class QCheckBox;
class QLineEdit;
class KSpreadView;

class KSpreadPaperLayout: public KoPageLayoutDia
{
  Q_OBJECT
public:
  KSpreadPaperLayout( QWidget* parent, const char* name,
                      const KoPageLayout& layout,
                      const KoHeadFoot& headfoot,
                      int tabs, KoUnit::Unit unit,
                      KSpreadTable *table, KSpreadView *view);
protected slots:
    virtual void slotOk();
protected:
    void initTab();
private:
    KSpreadTable *m_table;
    QCheckBox *pPrintGrid;
    QCheckBox *pPrintCommentIndicator;
    QCheckBox *pPrintFormulaIndicator;
    QLineEdit *ePrintRange;
    QLineEdit *eRepeatCols;
    QLineEdit *eRepeatRows;
};

#endif
