/* This file is part of the KDE project
   Copyright (C) 2003 Peter Simonsson <psn@linux.se>

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

#include "kivioarrowheadformatdlg.h"

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qbitmap.h>
#include <qpixmap.h>
#include <qpainter.h>

#include <kcombobox.h>
#include <klocale.h>

#include <koUnitWidgets.h>

#include "kivio_view.h"
#include "kivio_lineendspix.h"

KivioArrowHeadFormatDlg::KivioArrowHeadFormatDlg(KivioView* parent, const char* name)
  : KDialogBase(parent, name, true, i18n("Arrowhead Format"), Ok|Cancel|Default, Ok)
{
  m_unit = KoUnit::U_PT;
  init();
}

void KivioArrowHeadFormatDlg::init()
{
  QWidget* mainWidget = new QWidget(this);
  setMainWidget(mainWidget);
  QGridLayout* gl = new QGridLayout(mainWidget, 2, 1, KDialog::marginHint(), KDialog::spacingHint());

  QGroupBox* startGBox = new QGroupBox(2, Qt::Horizontal, i18n("Start Arrowhead"), mainWidget);
  QLabel* startAHTypeLbl = new QLabel(i18n("&Type:"), startGBox);
  m_startAHTypeCBox = new KComboBox(startGBox);
  loadArrowHeads(m_startAHTypeCBox, false);
  startAHTypeLbl->setBuddy(m_startAHTypeCBox);
  QLabel* startAHWidthLbl = new QLabel(i18n("&Width:"), startGBox);
  m_startAHWidthUSBox = new KoUnitDoubleSpinBox(startGBox, 0.0, 1000.0, 0.1, 1.0, m_unit, 2);
  startAHWidthLbl->setBuddy(m_startAHWidthUSBox);
  QLabel* startAHHeightLbl = new QLabel(i18n("&Height:"), startGBox);
  m_startAHHeightUSBox = new KoUnitDoubleSpinBox(startGBox, 0.0, 1000.0, 0.1, 1.0, m_unit, 2);
  startAHHeightLbl->setBuddy(m_startAHHeightUSBox);

  QGroupBox* endGBox = new QGroupBox(2, Qt::Horizontal, i18n("End Arrowhead"), mainWidget);
  QLabel* endAHTypeLbl = new QLabel(i18n("&Type:"), endGBox);
  m_endAHTypeCBox = new KComboBox(endGBox);
  loadArrowHeads(m_endAHTypeCBox, true);
  endAHTypeLbl->setBuddy(m_endAHTypeCBox);
  QLabel* endAHWidthLbl = new QLabel(i18n("&Width:"), endGBox);
  m_endAHWidthUSBox = new KoUnitDoubleSpinBox(endGBox, 0.0, 1000.0, 0.1, 1.0, m_unit, 2);
  endAHWidthLbl->setBuddy(m_endAHWidthUSBox);
  QLabel* endAHHeightLbl = new QLabel(i18n("&Height:"), endGBox);
  m_endAHHeightUSBox = new KoUnitDoubleSpinBox(endGBox, 0.0, 1000.0, 0.1, 1.0, m_unit, 2);
  endAHHeightLbl->setBuddy(m_endAHHeightUSBox);

  gl->addWidget(startGBox, 0, 0);
  gl->addWidget(endGBox, 1, 0);
}

void KivioArrowHeadFormatDlg::loadArrowHeads(KComboBox* combo, bool inverted)
{
  QBitmap mask;
  QPixmap pixAll(lineends);
  int tw = combo->fontMetrics().width(" 99:");

  if (inverted) {
    QPixmap tpix(pixAll.width(),pixAll.height());
    QPainter tp(&tpix);
    tp.scale(-1,1);
    tp.drawPixmap(-pixAll.width()+1,0,pixAll);
    tp.end();
    pixAll = tpix;
  }

  QPixmap pix(pixAll.width() + tw + 3, 17);
  QPainter p(&pix, combo);
  int cindex = 0;

  // insert item "0: None"
  pix.fill(white);
  p.drawText(0,0,tw,pix.height(),AlignRight|AlignVCenter,QString("%1:").arg(cindex));
  p.drawText(tw+3,0,pix.width()-tw-3,pix.height(),AlignLeft|AlignVCenter,i18n("no line end", "None"));
  mask = pix;
  pix.setMask(mask);
  combo->insertItem(pix,cindex++);

  for (int y = 0; y < pixAll.height(); y += 17 ) {
    pix.fill(white);
    p.drawText(0,0,tw,pix.height(),AlignRight|AlignVCenter,QString("%1:").arg(cindex));
    p.drawPixmap(tw+3,0,pixAll,0,y,pix.width(),pix.height());

    mask = pix;
    pix.setMask(mask);
    combo->insertItem(pix,cindex++);
  }

  p.end();
}

int KivioArrowHeadFormatDlg::startAHType()
{
  return m_startAHTypeCBox->currentItem();
}

double KivioArrowHeadFormatDlg::startAHWidth()
{
  return KoUnit::ptFromUnit(m_startAHWidthUSBox->value(), m_unit);
}

double KivioArrowHeadFormatDlg::startAHHeight()
{
  return KoUnit::ptFromUnit(m_startAHHeightUSBox->value(), m_unit);
}

int KivioArrowHeadFormatDlg::endAHType()
{
  return m_endAHTypeCBox->currentItem();
}

double KivioArrowHeadFormatDlg::endAHWidth()
{
  return KoUnit::ptFromUnit(m_endAHWidthUSBox->value(), m_unit);
}

double KivioArrowHeadFormatDlg::endAHHeight()
{
  return KoUnit::ptFromUnit(m_endAHHeightUSBox->value(), m_unit);
}

void KivioArrowHeadFormatDlg::setUnit(KoUnit::Unit u)
{
  m_unit = u;
  m_startAHWidthUSBox->setUnit(u);
  m_startAHHeightUSBox->setUnit(u);
  m_endAHWidthUSBox->setUnit(u);
  m_endAHHeightUSBox->setUnit(u);
}

void KivioArrowHeadFormatDlg::setStartAHType(int t)
{
  m_startAHTypeCBox->setCurrentItem(t);
}

void KivioArrowHeadFormatDlg::setStartAHWidth(double w)
{
  m_startAHWidthUSBox->setValue(KoUnit::ptToUnit(w, m_unit));
}

void KivioArrowHeadFormatDlg::setStartAHHeight(double h)
{
  m_startAHHeightUSBox->setValue(KoUnit::ptToUnit(h, m_unit));
}

void KivioArrowHeadFormatDlg::setEndAHType(int t)
{
  m_endAHTypeCBox->setCurrentItem(t);
}

void KivioArrowHeadFormatDlg::setEndAHWidth(double w)
{
  m_endAHWidthUSBox->setValue(KoUnit::ptToUnit(w, m_unit));
}

void KivioArrowHeadFormatDlg::setEndAHHeight(double h)
{
  m_endAHHeightUSBox->setValue(KoUnit::ptToUnit(h, m_unit));
}

void KivioArrowHeadFormatDlg::slotDefault()
{
  setStartAHType(0);
  setEndAHType(0);
  setStartAHWidth(10.0);
  setStartAHHeight(10.0);
  setEndAHWidth(10.0);
  setEndAHHeight(10.0);
}

#include "kivioarrowheadformatdlg.moc"
