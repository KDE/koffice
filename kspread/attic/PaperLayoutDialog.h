/* This file is part of the KDE project
   Copyright (C) 2002-2003 Ariya Hidayat <ariya@kde.org>
             (C) 2002-2003 Norbert Andres <nandres@web.de>
             (C) 2002-2003 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2002 Laurent Montel <montel@kde.org>
   
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

#ifndef KSPREAD_PAPER_LAYOUT_DIALOG
#define KSPREAD_PAPER_LAYOUT_DIALOG

#include <kdialog.h>
#include <KoPageLayoutDia.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QEvent>
#include <QCloseEvent>

class QCheckBox;
class KComboBox;
class QRadioButton;
class KLineEdit;

namespace KSpread
{
class Sheet;
class View;

class PaperLayout: public KoPageLayoutDia
{
  Q_OBJECT
public:
  PaperLayout( QWidget* parent, const char* name,
                      const KoPageLayout& layout,
                      const KoHeadFoot& headfoot,
                      int tabs, KoUnit unit,
                      Sheet * sheet, View *view);

  bool eventFilter( QObject* obj, QEvent* ev );

protected slots:
  virtual void slotOk();
  virtual void slotCancel();
  void slotSelectionChanged();

protected:
  void initTab();

protected:
  virtual void closeEvent ( QCloseEvent * );

private slots:
    void slotChooseZoom( int index );
    void slotChoosePageLimit( int index );

private:
  void initGeneralOptions( QWidget * tab, QVBoxLayout * vbox );
  void initRanges( QWidget * tab, QVBoxLayout * vbox );
  void initScaleOptions( QWidget * tab, QVBoxLayout * vbox );

  Sheet * m_pSheet;
  View  * m_pView;

  QCheckBox * pApplyToAll;
  QCheckBox * pPrintGrid;
  QCheckBox * pPrintCommentIndicator;
  QCheckBox * pPrintFormulaIndicator;
  QCheckBox * pPrintObjects;
  QCheckBox * pPrintCharts;
  QCheckBox * pPrintGraphics;
  KLineEdit * ePrintRange;
  KLineEdit * eRepeatCols;
  KLineEdit * eRepeatRows;
  KLineEdit * m_focus;
  KComboBox * m_cZoom;
  KComboBox * m_cLimitPagesX;
  KComboBox * m_cLimitPagesY;
  QRadioButton * m_rScalingZoom;
  QRadioButton * m_rScalingLimitPages;
};

} // namespace KSpread

#endif // KSPREAD_PAPER_LAYOUT_DIALOG
