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

#include "kiviooptionsdialog.h"
#include "kivio_view.h"
#include "kivio_doc.h"
#include "kivio_page.h"
#include "kivioglobal.h"
#include "kivio_config.h"
#include "kivio_icon_view.h"
#include "kivio_grid_data.h"
#include "kivio_guidelines.h"
#include "kivio_canvas.h"

#include <klocale.h>
#include <koApplication.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <koPageLayoutDia.h>
#include <kurlrequester.h>
#include <kcolorbutton.h>
#include <koUnitWidgets.h>
#include <kglobal.h>
#include <kdebug.h>

#include <qlabel.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtabwidget.h>

GuidesListViewItem::GuidesListViewItem(QListView* parent, KivioGuideLineData *gd, KoUnit::Unit u)
: KListViewItem(parent), m_data(gd)
{
  setPixmap(0, BarIcon(m_data->orientation() == Qt::Vertical ?
    "guides_vertical":"guides_horizontal"));
  QString s = KGlobal::_locale->formatNumber(KoUnit::ptToUnit(m_data->position(), u), 2);
  s += " " + KoUnit::unitName(u);
  setText(1, s);
}

GuidesListViewItem::~GuidesListViewItem()
{
  delete m_data;
}

void GuidesListViewItem::setUnit(KoUnit::Unit u)
{
  QString s = KGlobal::_locale->formatNumber(KoUnit::ptToUnit(m_data->position(), u), 2);
  s += " " + KoUnit::unitName(u);
  setText(1, s);
}

void GuidesListViewItem::setPosition(double p, KoUnit::Unit u)
{
  m_data->setPosition(KoUnit::ptFromUnit(p, u));
  QString s = KGlobal::_locale->formatNumber(p, 2);
  s += " " + KoUnit::unitName(u);
  setText(1, s);
}

void GuidesListViewItem::setOrientation(Qt::Orientation o)
{
  m_data->setOrientation(o);
  setPixmap(0, BarIcon(m_data->orientation() == Qt::Vertical ?
    "guides_vertical":"guides_horizontal"));
}

/*****************************************************************************/

KivioOptionsDialog::KivioOptionsDialog(KivioView* parent, const char* name)
  : KDialogBase(IconList, i18n("Settings"), Ok|Cancel|Apply|Default, Ok, parent, name)
{
  initPage();
  initStencil();
  initGrid();
  initGuides();
}

void KivioOptionsDialog::initPage()
{
  QFrame* page = addPage(i18n("Page"), i18n("Page Settings"),
    kapp->iconLoader()->loadIcon("empty", KIcon::Toolbar, 32));
  m_pageIndex = pageIndex(page);

  KivioView* view = static_cast<KivioView*>(parent());
  KoUnit::Unit unit = view->doc()->units();
  m_layout = view->doc()->config()->globalDefaultPageLayout();

  QLabel* unitLbl = new QLabel(i18n("Default &units:"), page);
  m_unitCombo = new QComboBox(page);
  m_unitCombo->insertStringList(KoUnit::listOfUnitName());
  m_unitCombo->setCurrentItem(unit);
  unitLbl->setBuddy(m_unitCombo);
  QLabel* layoutLbl = new QLabel(i18n("Default layout:"), page);
  m_layoutTxtLbl = new QLabel(page);
  m_layoutTxtLbl->setFrameStyle(QFrame::LineEditPanel | QFrame::Sunken);
  m_layoutTxtLbl->setSizePolicy(QSizePolicy(
    QSizePolicy::Minimum, QSizePolicy::Fixed));
  setLayoutText(m_layout);
  KPushButton* layoutBtn = new KPushButton(i18n("C&hange..."), page);
  layoutBtn->setSizePolicy(QSizePolicy(
    QSizePolicy::Fixed, QSizePolicy::Fixed));
  m_bordersChBox = new QCheckBox(i18n("Show page &borders"), page);
  m_bordersChBox->setChecked(view->isShowPageBorders());
  m_marginsChBox = new QCheckBox(i18n("Show page &margins"), page);
  m_marginsChBox->setChecked(view->isShowPageMargins());
  m_rulersChBox = new QCheckBox(i18n("Show page &rulers"), page);
  m_rulersChBox->setChecked(view->isShowRulers());

  QGridLayout* gl = new QGridLayout(page);
  gl->setSpacing(KDialog::spacingHint());
  gl->addWidget(unitLbl, 0, 0);
  gl->addMultiCellWidget(m_unitCombo, 0, 0, 1, 2);
  gl->addWidget(layoutLbl, 1, 0);
  gl->addWidget(m_layoutTxtLbl, 1, 1);
  gl->addWidget(layoutBtn, 1, 2);
  gl->addMultiCellWidget(m_bordersChBox, 2, 2, 0, 2);
  gl->addMultiCellWidget(m_marginsChBox, 3, 3, 0, 2);
  gl->addMultiCellWidget(m_rulersChBox, 4, 4, 0, 2);
  gl->addMultiCell(new QSpacerItem(0, 0), 5, 5, 0, 2);

  connect(layoutBtn, SIGNAL(clicked()), SLOT(pageLayoutDlg()));
  connect(m_unitCombo, SIGNAL(activated(int)), SLOT(unitChanged(int)));
}

void KivioOptionsDialog::initStencil()
{
  QFrame* page = addPage(i18n("Stencils Bar"), i18n("Stencils Bar Settings"),
    kapp->iconLoader()->loadIcon("stencils", KIcon::Toolbar, 32));
  m_stencilIndex = pageIndex(page);

  KivioIconViewVisual v = static_cast<KivioView*>(parent())->doc()->
    config()->stencilsBarVisual();

  m_bgColorRBtn = new QRadioButton(i18n("Background &color:"), page);
  m_bgColorRBtn->setChecked(!v.usePixmap);
  m_bgColorBtn = new KColorButton(v.color, page);
  m_bgPicRBtn = new QRadioButton(i18n("Background &picture:"), page);
  m_bgPicRBtn->setChecked(v.usePixmap);
  m_bgPicURLReq = new KURLRequester(v.pixmapFileName, page);

  QButtonGroup* grp = new QButtonGroup(page);
  grp->hide();
  grp->insert(m_bgColorRBtn);
  grp->insert(m_bgPicRBtn);

  QGridLayout* gl = new QGridLayout(page);
  gl->setSpacing(KDialog::spacingHint());
  gl->addWidget(m_bgColorRBtn, 0, 0);
  gl->addWidget(m_bgColorBtn, 0, 1);
  gl->addItem(new QSpacerItem(0, 0), 0, 2);
  gl->addWidget(m_bgPicRBtn, 1, 0);
  gl->addMultiCellWidget(m_bgPicURLReq, 1, 1, 1, 2);
  gl->addMultiCell(new QSpacerItem(0, 0), 2, 2, 0, 1);
}

void KivioOptionsDialog::initGrid()
{
  QFrame* page = addPage(i18n("Grid"), i18n("Grid Settings"),
    kapp->iconLoader()->loadIcon("grid", KIcon::Toolbar, 32));
  m_gridIndex = pageIndex(page);

  KoUnit::Unit unit = static_cast<KoUnit::Unit>(m_unitCombo->currentItem());
  KivioGridData d = static_cast<KivioView*>(parent())->doc()->grid();
  double pgw = KoUnit::ptToUnit(m_layout.ptWidth, unit);
  double pgh = KoUnit::ptToUnit(m_layout.ptHeight, unit);
  double fw = KoUnit::ptToUnit(d.freq.width(), unit);
  double fh = KoUnit::ptToUnit(d.freq.height(), unit);
  double sw = KoUnit::ptToUnit(d.snap.width(), unit);
  double sh = KoUnit::ptToUnit(d.snap.height(), unit);

  m_gridChBox = new QCheckBox(i18n("Show &grid"), page);
  m_gridChBox->setChecked(d.isShow);
  m_snapChBox = new QCheckBox(i18n("Snap to g&rid"), page);
  m_snapChBox->setChecked(d.isSnap);
  QLabel* gridColorLbl = new QLabel(i18n("Grid &color:"), page);
  m_gridColorBtn = new KColorButton(d.color, page);
  gridColorLbl->setBuddy(m_gridColorBtn);
  QGroupBox* spacingGrp = new QGroupBox(2, Qt::Horizontal, i18n("Spacing"), page);
  QLabel* spaceHorizLbl = new QLabel(i18n("&Horizontal:"), spacingGrp);
  m_spaceHorizUSpin = new KoUnitDoubleSpinBox(spacingGrp, 0.0, pgw, 0.1,
    fw, unit);
  spaceHorizLbl->setBuddy(m_spaceHorizUSpin);
  QLabel* spaceVertLbl = new QLabel(i18n("&Vertical:"), spacingGrp);
  m_spaceVertUSpin = new KoUnitDoubleSpinBox(spacingGrp, 0.0, pgh, 0.1,
    fh, unit);
  spaceVertLbl->setBuddy(m_spaceVertUSpin);
  QGroupBox* snapGrp = new QGroupBox(2, Qt::Horizontal, i18n("Snap Distance"), page);
  QLabel* snapHorizLbl = new QLabel(i18n("H&orizontal:"), snapGrp);
  m_snapHorizUSpin = new KoUnitDoubleSpinBox(snapGrp, 0.0, fw, 0.1,
    sw, unit);
  snapHorizLbl->setBuddy(m_snapHorizUSpin);
  QLabel* snapVertLbl = new QLabel(i18n("V&ertical:"), snapGrp);
  m_snapVertUSpin = new KoUnitDoubleSpinBox(snapGrp, 0.0, fh, 0.1,
    sh, unit);
  snapVertLbl->setBuddy(m_snapVertUSpin);

  QGridLayout* gl = new QGridLayout(page);
  gl->setSpacing(KDialog::spacingHint());
  gl->addMultiCellWidget(m_gridChBox, 0, 0, 0, 2);
  gl->addMultiCellWidget(m_snapChBox, 1, 1, 0, 2);
  gl->addWidget(gridColorLbl, 2, 0);
  gl->addWidget(m_gridColorBtn, 2, 1);
  gl->addItem(new QSpacerItem(0, 0), 2, 2);
  gl->addMultiCellWidget(spacingGrp, 3, 3, 0, 2);
  gl->addMultiCellWidget(snapGrp, 4, 4, 0, 2);
  gl->addMultiCell(new QSpacerItem(0, 0), 5, 5, 0, 2);

  connect(m_spaceHorizUSpin, SIGNAL(valueChanged(double)), SLOT(setMaxHorizSnap(double)));
  connect(m_spaceVertUSpin, SIGNAL(valueChanged(double)), SLOT(setMaxVertSnap(double)));
}

void KivioOptionsDialog::initGuides()
{
  QFrame* page = addPage(i18n("Guide Lines"), i18n("Guide Line Settings"),
    kapp->iconLoader()->loadIcon("guides", KIcon::Toolbar, 32));
  m_guidesIndex = pageIndex(page);

  KivioView* view = static_cast<KivioView*>(parent());
  QTabWidget* tabs = new QTabWidget(page);
  QWidget* managerTab = new QWidget(tabs);
  QWidget* lookTab = new QWidget(tabs);

  // Guide manager
  m_guideList = new KListView(managerTab);
  m_guideList->addColumn(i18n("Orientation"));
  m_guideList->addColumn(i18n("Position"));
  m_guideList->setColumnAlignment(1, AlignRight);
  m_guideList->setFullWidth(true);
  m_guideList->setAllColumnsShowFocus(true);
  KPushButton* newBtn = new KPushButton(kapp->iconLoader()->
    loadIcon("filenew", KIcon::Small, 16), i18n("Add"), managerTab);
  KPushButton* delBtn = new KPushButton(kapp->iconLoader()->
    loadIcon("editdelete", KIcon::Small, 16), i18n("Remove"), managerTab);
  m_propertiesGrp = new QGroupBox( 0, Qt::Vertical, i18n("Guide Line Properties"), managerTab);
  m_propertiesGrp->layout()->setSpacing(KDialog::spacingHint());
  m_propertiesGrp->layout()->setMargin(KDialog::marginHint());
  m_propertiesGrp->setEnabled(false);
  m_orientHorizRBtn = new QRadioButton(i18n("&Horizontal"), m_propertiesGrp);
  m_orientHorizRBtn->setChecked(true);
  m_orientVertRBtn = new QRadioButton(i18n("&Vertical"), m_propertiesGrp);
  QButtonGroup* orientBGrp = new QButtonGroup(m_propertiesGrp);
  orientBGrp->hide();
  orientBGrp->insert(m_orientHorizRBtn);
  orientBGrp->insert(m_orientVertRBtn);
  QLabel* posLbl = new QLabel(i18n("&Position:"), m_propertiesGrp);
  KoUnit::Unit unit = static_cast<KoUnit::Unit>(m_unitCombo->currentItem());
  m_posUSpin = new KoUnitDoubleSpinBox(m_propertiesGrp, 0.0, 0.0, 0.0, unit);
  posLbl->setBuddy(m_posUSpin);

  QGridLayout* pgl = new QGridLayout(m_propertiesGrp->layout());
  pgl->setSpacing(KDialog::spacingHint());
  pgl->setMargin(KDialog::marginHint());
  pgl->addMultiCellWidget(m_orientHorizRBtn, 0, 0, 0, 1);
  pgl->addMultiCellWidget(m_orientVertRBtn, 1, 1, 0, 1);
  pgl->addWidget(posLbl, 2, 0);
  pgl->addWidget(m_posUSpin, 2, 1);

  QGridLayout* mgl = new QGridLayout(managerTab);
  mgl->setSpacing(KDialog::spacingHint());
  mgl->setMargin(KDialog::marginHint());
  mgl->addMultiCellWidget(m_guideList, 0, 1, 0, 1);
  mgl->addWidget(newBtn, 2, 0);
  mgl->addWidget(delBtn, 2, 1);
  mgl->addWidget(m_propertiesGrp, 0, 2);
  mgl->addItem(new QSpacerItem(0, 0), 1, 2);

  // Look&Feel
  m_guidesChBox = new QCheckBox(i18n("&Show guides"), lookTab);
  m_guidesChBox->setChecked(view->isShowGuides());
  m_snapGuideChBox = new QCheckBox(i18n("S&nap to guides"), lookTab);
  m_snapGuideChBox->setChecked(view->isSnapGuides());
  QLabel* guideColorLbl = new QLabel(i18n("&Guide color:"), lookTab);
  m_guideColorBtn = new KColorButton(lookTab);
  guideColorLbl->setBuddy(m_guideColorBtn);
  QLabel* guideSelColorLbl = new QLabel(i18n("S&elected guide color:"), lookTab);
  m_guideSelColorBtn = new KColorButton(lookTab);
  guideSelColorLbl->setBuddy(m_guideSelColorBtn);

  QGridLayout* lgl = new QGridLayout(lookTab);
  lgl->setSpacing(KDialog::spacingHint());
  lgl->setMargin(KDialog::marginHint());
  lgl->addMultiCellWidget(m_guidesChBox, 0, 0, 0, 1);
  lgl->addMultiCellWidget(m_snapGuideChBox, 1, 1, 0, 1);
  lgl->addWidget(guideColorLbl, 2, 0);
  lgl->addWidget(m_guideColorBtn, 2, 1);
  lgl->addWidget(guideSelColorLbl, 3, 0);
  lgl->addWidget(m_guideSelColorBtn, 3, 1);
  lgl->addMultiCell(new QSpacerItem(0, 0), 4, 4, 0, 1);

  tabs->addTab(managerTab, i18n("&Manager"));
  tabs->addTab(lookTab, i18n("&Look && Feel"));

  QGridLayout* gl = new QGridLayout(page);
  gl->setSpacing(KDialog::spacingHint());
  gl->addWidget(tabs, 0, 0);

  connect(m_guideList, SIGNAL(selectionChanged(QListViewItem*)), SLOT(
    guideSelectionChanged(QListViewItem*)));
  connect(m_posUSpin, SIGNAL(valueChanged(double)), SLOT(changePos(double)));
  connect(m_orientHorizRBtn, SIGNAL(toggled(bool)), SLOT(guideHoriz(bool)));
  connect(newBtn, SIGNAL(clicked()), SLOT(addGuide()));
  connect(delBtn, SIGNAL(clicked()), SLOT(delGuide()));

  fillGuideList();
}

void KivioOptionsDialog::applyPage()
{
  KivioView* view = static_cast<KivioView*>(parent());
  view->doc()->setUnits(static_cast<KoUnit::Unit>(m_unitCombo->currentItem()));
  view->doc()->config()->setGlobalDefaultPageLayout(m_layout);
  view->togglePageBorders(m_bordersChBox->isChecked());
  view->togglePageMargins(m_marginsChBox->isChecked());
  view->toggleShowRulers(m_rulersChBox->isChecked());
}

void KivioOptionsDialog::applyStencil()
{
  KivioIconViewVisual v;
  v.usePixmap = m_bgPicRBtn->isChecked();
  v.color = m_bgColorBtn->color();
  v.pixmapFileName = m_bgPicURLReq->url();
  static_cast<KivioView*>(parent())->doc()->config()->
    setGlobalStencilsBarVisual(v);
}

void KivioOptionsDialog::applyGrid()
{
  KivioGridData d;
  KoUnit::Unit unit = static_cast<KoUnit::Unit>(m_unitCombo->currentItem());
  d.freq.setWidth(KoUnit::ptFromUnit(m_spaceHorizUSpin->value(), unit));
  d.freq.setHeight(KoUnit::ptFromUnit(m_spaceVertUSpin->value(), unit));
  d.snap.setWidth(KoUnit::ptFromUnit(m_snapHorizUSpin->value(), unit));
  d.snap.setHeight(KoUnit::ptFromUnit(m_snapVertUSpin->value(), unit));
  d.isShow = m_gridChBox->isChecked();
  d.isSnap = m_snapChBox->isChecked();
  d.color = m_gridColorBtn->color();
  KivioView* view = static_cast<KivioView*>(parent());
  view->doc()->setGrid(d);
  view->doc()->updateView(0, true);
}

void KivioOptionsDialog::applyGuides()
{
  KivioView* view = static_cast<KivioView*>(parent());
  view->toggleShowGuides(m_guidesChBox->isChecked());
  view->toggleSnapGuides(m_snapGuideChBox->isChecked());

  view->canvasWidget()->eraseGuides();
  KivioGuideLines* gl = view->activePage()->guideLines();
  gl->selectAll();
  gl->removeSelected();
  QListViewItemIterator it(m_guideList);

  while (it.current()) {
    KivioGuideLineData* data = static_cast<GuidesListViewItem*>(it.current())->
      guideData();
    gl->add(data->position(), data->orientation());
    ++it;
  }

  view->canvasWidget()->paintGuides();
}

void KivioOptionsDialog::defaultPage()
{
  m_layout = KoPageLayoutDia::standardLayout();
  m_unitCombo->setCurrentItem(KoUnit::U_MM);
  setLayoutText(m_layout);
  m_bordersChBox->setChecked(true);
  m_marginsChBox->setChecked(true);
  m_rulersChBox->setChecked(true);
}

void KivioOptionsDialog::defaultStencil()
{
  m_bgColorRBtn->setChecked(true);
  m_bgColorBtn->setColor(QColor(0x4BD2FF));
  m_bgPicRBtn->setChecked(false);
  m_bgPicURLReq->clear();
}

void KivioOptionsDialog::defaultGrid()
{
  KoUnit::Unit unit = static_cast<KoUnit::Unit>(m_unitCombo->currentItem());
  m_spaceHorizUSpin->setValue(KoUnit::ptToUnit(10.0, unit));
  m_spaceVertUSpin->setValue(KoUnit::ptToUnit(10.0, unit));
  m_snapHorizUSpin->setValue(KoUnit::ptToUnit(10.0, unit));
  m_snapVertUSpin->setValue(KoUnit::ptToUnit(10.0, unit));
  m_gridChBox->setChecked(true);
  m_snapChBox->setChecked(true);
  m_gridColorBtn->setColor(QColor(228,228,228));
}

void KivioOptionsDialog::defaultGuides()
{
  m_guidesChBox->setChecked(true);
  m_snapGuideChBox->setChecked(true);
  m_guideList->clear();
}

void KivioOptionsDialog::setLayoutText(const KoPageLayout& l)
{
  KoUnit::Unit unit = static_cast<KoUnit::Unit>(m_unitCombo->currentItem());
  QString txt = i18n("Format: %1 Width: %2 %4 Height: %3 %5").arg(
    KoPageFormat::formatString(l.format)).arg(KoUnit::ptToUnit(l.ptWidth, unit))
    .arg(KoUnit::ptToUnit(l.ptHeight, unit)).arg(KoUnit::unitName(unit)).arg(
    KoUnit::unitName(unit));
  m_layoutTxtLbl->setText(txt);
}

void KivioOptionsDialog::pageLayoutDlg()
{
  KoHeadFoot headfoot;
  int tabs = FORMAT_AND_BORDERS | DISABLE_UNIT;
  KoUnit::Unit unit = static_cast<KoUnit::Unit>(m_unitCombo->currentItem());

  if(KoPageLayoutDia::pageLayout(m_layout, headfoot, tabs, unit))
  {
    setLayoutText(m_layout);
  }
}

void KivioOptionsDialog::unitChanged(int u)
{
  KoUnit::Unit unit = static_cast<KoUnit::Unit>(u);
  setLayoutText(m_layout);
  m_snapHorizUSpin->setUnit(unit);
  m_snapVertUSpin->setUnit(unit);
  m_spaceHorizUSpin->setUnit(unit);
  m_spaceVertUSpin->setUnit(unit);
  m_posUSpin->setUnit(unit);
  QListViewItemIterator it(m_guideList);

  while (it.current()) {
    GuidesListViewItem* item = static_cast<GuidesListViewItem*>(it.current());
    ++it;
    item->setUnit(unit);
  }
}

void KivioOptionsDialog::slotOk()
{
  applyPage();
  applyStencil();
  applyGrid();
  applyGuides();
  accept();
}

void KivioOptionsDialog::slotApply()
{
  if(activePageIndex() == m_pageIndex) {
    applyPage();
  }
  if(activePageIndex() == m_stencilIndex) {
    applyStencil();
  }
  if(activePageIndex() == m_gridIndex) {
    applyGrid();
  }
  if(activePageIndex() == m_guidesIndex) {
    applyGuides();
  }
}

void KivioOptionsDialog::slotDefault()
{
  if(activePageIndex() == m_pageIndex) {
    defaultPage();
  }
  if(activePageIndex() == m_stencilIndex) {
    defaultStencil();
  }
  if(activePageIndex() == m_gridIndex) {
    defaultGrid();
  }
  if(activePageIndex() == m_guidesIndex) {
    defaultGuides();
  }
}

void KivioOptionsDialog::setMaxHorizSnap(double v)
{
  m_snapHorizUSpin->setMaxValue(v);
}

void KivioOptionsDialog::setMaxVertSnap(double v)
{
  m_snapVertUSpin->setMaxValue(v);
}

void KivioOptionsDialog::fillGuideList()
{
  KoUnit::Unit unit = static_cast<KoUnit::Unit>(m_unitCombo->currentItem());
  KivioGuidesList list = static_cast<KivioView*>(parent())->activePage()->
    guideLines()->guides();
  list.sort();
  m_guideList->clear();

  for(KivioGuideLineData* d = list.first(); d; d = list.next()) {
    (void) new GuidesListViewItem(m_guideList, new KivioGuideLineData(*d), unit);
  }
}

void KivioOptionsDialog::guideSelectionChanged(QListViewItem* li)
{
  if(!li) {
    m_propertiesGrp->setEnabled(false);
    m_orientHorizRBtn->setChecked(true);
    m_orientVertRBtn->setChecked(false);
    m_posUSpin->setValue(0.0);
    return;
  }

  m_propertiesGrp->setEnabled(true);
  KoUnit::Unit unit = static_cast<KoUnit::Unit>(m_unitCombo->currentItem());
  KivioGuideLineData* data = static_cast<GuidesListViewItem*>(li)->guideData();

  m_orientHorizRBtn->setChecked(data->orientation() == Qt::Horizontal);
  m_orientVertRBtn->setChecked(data->orientation() != Qt::Horizontal);

  double max = KoUnit::ptToUnit(m_layout.ptWidth, unit);

  if(data->orientation() == Qt::Horizontal) {
    max = KoUnit::ptToUnit(m_layout.ptHeight, unit);
  }

  m_posUSpin->setMaxValue(max);
  m_posUSpin->setValue(KoUnit::ptToUnit(data->position(), unit));
}

void KivioOptionsDialog::changePos(double p)
{
  KoUnit::Unit unit = static_cast<KoUnit::Unit>(m_unitCombo->currentItem());
  QListViewItemIterator it(m_guideList);

  while (it.current()) {
    GuidesListViewItem* li = static_cast<GuidesListViewItem*>(it.current());

    if(li->isSelected()) {
      li->setPosition(p, unit);
    }

    ++it;
  }
}

void KivioOptionsDialog::guideHoriz(bool h)
{
  Qt::Orientation o;

  if(h) {
    o = Qt::Horizontal;
  } else {
    o = Qt::Vertical;
  }

  QListViewItemIterator it(m_guideList);

  while (it.current()) {
    GuidesListViewItem* li = static_cast<GuidesListViewItem*>(it.current());

    if(li->isSelected()) {
      li->setOrientation(o);
    }

    ++it;
  }
}

void KivioOptionsDialog::addGuide()
{
  KoUnit::Unit unit = static_cast<KoUnit::Unit>(m_unitCombo->currentItem());
  GuidesListViewItem* it = new GuidesListViewItem(m_guideList, new KivioGuideLineData(Qt::Horizontal), unit);
  m_guideList->clearSelection();
  m_guideList->setSelected(it, true);
}

void KivioOptionsDialog::delGuide()
{
  QListViewItemIterator it(m_guideList);

  while (it.current()) {
    QListViewItem *item = it.current();
    ++it;

    if (item->isSelected()) {
      delete item;
      item = 0;
    }
  }
}

#include "kiviooptionsdialog.moc"
