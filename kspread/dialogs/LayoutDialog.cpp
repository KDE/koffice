/* This file is part of the KDE project
   Copyright (C) 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
             (C) 2004 Tomas Mecir <mecirt@gmail.com>
             (C) 2002-2004 Ariya Hidayat <ariya@kde.org>
             (C) 2002-2003 Norbert Andres <nandres@web.de>
             (C) 2001-2003 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2002 John Dailey <dailey@vt.edu>
             (C) 1999-2002 Laurent Montel <montel@kde.org>
             (C) 1999-2002 Harri Porten <porten@kde.org>
             (C) 2000-2001 David Faure <faure@kde.org>
             (C) 1998-2000 Torben Weis <weis@kde.org>
             (C) 2000 Werner Trobin <trobin@kde.org>
             (C) 1999 Reginald Stadlbauer <reggie@kde.org>
             (C) 1998-1999 Stephan Kulow <coolo@kde.org>

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
#include "LayoutDialog.h"

#include <stdlib.h>
#include <math.h>

#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QFontDatabase>
#include <QPainter>
#include <QRadioButton>

//Added by qt3to4:
#include <QPaintEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPixmap>

#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <knumvalidator.h>

#include <KoCanvasBase.h>
#include <KoUnitDoubleSpinBox.h>

#include "KCCalculationSettings.h"
#include "KCCell.h"
#include "KCCellStorage.h"
#include "KCLocalization.h"
#include "KCMap.h"
#include "ui/Selection.h"
#include "KCSheet.h"
#include "KCStyle.h"
#include "KCStyleManager.h"
#include "StyleStorage.h"
#include "ValueFormatter.h"

#include "commands/MergeCommand.h"
#include "commands/StyleCommand.h"
#include "commands/RowColumnManipulators.h"

/***************************************************************************
 *
 * PatternSelect
 *
 ***************************************************************************/

PatternSelect::PatternSelect(QWidget *parent, const char *)
        : QFrame(parent)
{
    penStyle = Qt::NoPen;
    penWidth = 1;
    penColor = palette().text().color();
    selected = false;
    undefined = false;
}

void PatternSelect::setPattern(const QColor &_color, int _width, Qt::PenStyle _style)
{
    penStyle = _style;
    penColor = _color;
    penWidth = _width;
    repaint();
}

void PatternSelect::setUndefined()
{
    undefined = true;
}

void PatternSelect::paintEvent(QPaintEvent *_ev)
{
    QFrame::paintEvent(_ev);

    QPainter painter(this);

    if (!undefined) {
        QPen pen(penColor, penWidth, penStyle);
        painter.setPen(pen);
        painter.drawLine(6, height() / 2, width() - 6, height() / 2);
    } else {
        painter.fillRect(2, 2, width() - 4, height() - 4, Qt::BDiagPattern);
    }
}

void PatternSelect::mousePressEvent(QMouseEvent *)
{
    slotSelect();

    emit clicked(this);
}

void PatternSelect::slotUnselect()
{
    selected = false;

    setLineWidth(1);
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    repaint();
}

void PatternSelect::slotSelect()
{
    selected = true;

    setLineWidth(2);
    setFrameStyle(QFrame::Panel | QFrame::Plain);
    repaint();
}



/***************************************************************************
 *
 * GeneralTab
 *
 ***************************************************************************/

GeneralTab::GeneralTab(QWidget* parent, CellFormatDialog * dlg)
        : QWidget(parent),
        m_dlg(dlg)
{
    QGridLayout * layout = new QGridLayout(this);
    layout->setMargin(KDialog::marginHint());
    layout->setSpacing(KDialog::spacingHint());

    QGroupBox * groupBox = new QGroupBox(this);
    groupBox->setTitle(i18n("KCStyle"));

    QGridLayout * groupBoxLayout = new QGridLayout(groupBox);
    groupBoxLayout->setAlignment(Qt::AlignTop);
    groupBoxLayout->setSpacing(KDialog::spacingHint());
    groupBoxLayout->setMargin(KDialog::marginHint());

    QLabel * label1 = new QLabel(groupBox);
    label1->setText(i18n("Name:"));
    groupBoxLayout->addWidget(label1, 0, 0);

    m_nameEdit = new KLineEdit(groupBox);
    m_nameEdit->setText(m_dlg->styleName);
    groupBoxLayout->addWidget(m_nameEdit, 0, 1);

    m_nameStatus = new QLabel(groupBox);
    m_nameStatus->hide();
    groupBoxLayout->addWidget(m_nameStatus, 1, 1);

    QLabel * label2 = new QLabel(groupBox);
    label2->setText(i18n("Inherit style:"));
    groupBoxLayout->addWidget(label2, 2, 0);

    m_parentBox = new KComboBox(false, groupBox);
    m_parentBox->clear();
    QStringList tmp = m_dlg->getStyleManager()->styleNames();
    tmp.removeAll(m_dlg->styleName);
    // place the default style first
    tmp.removeAll(i18n("Default"));
    m_parentBox->insertItem(0, i18n("Default"));
    m_parentBox->insertItems(1, tmp);

    if (!m_dlg->getStyle()->parentName().isNull())
        m_parentBox->setCurrentIndex(m_parentBox->findText(m_dlg->getStyle()->parentName()));
    else
        m_parentBox->setCurrentIndex(m_parentBox->findText(i18n("Default")));

    connect(m_parentBox, SIGNAL(activated(const QString&)),
            this, SLOT(parentChanged(const QString&)));
    connect(m_nameEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(styleNameChanged(const QString&)));

    groupBoxLayout->addWidget(m_parentBox, 2, 1);

    m_parentStatus = new QLabel(groupBox);
    m_parentStatus->hide();
    groupBoxLayout->addWidget(m_parentStatus, 3, 1);

    QSpacerItem * spacer = new QSpacerItem(20, 260, QSizePolicy::Minimum, QSizePolicy::Expanding);

    layout->addWidget(groupBox, 0, 0);
    layout->addItem(spacer, 1, 0);

    if (m_dlg->getStyle()->type() == KCStyle::BUILTIN) {
        m_nameEdit->setEnabled(false);
        m_parentBox->setEnabled(false);
    }

    resize(QSize(534, 447).expandedTo(minimumSizeHint()));
}

GeneralTab::~GeneralTab()
{
}

void GeneralTab::styleNameChanged(const QString& name)
{
    if (!m_dlg->getStyleManager()->validateStyleName(name, m_dlg->getStyle())) {
        m_nameStatus->setText(i18n("A style with this name already exists."));
        m_nameStatus->show();
        m_dlg->enableButtonOk(false);
    } else if (name.isEmpty()) {
        m_nameStatus->setText(i18n("The style name can not be empty."));
        m_nameStatus->show();
        m_dlg->enableButtonOk(false);
    } else {
        m_nameStatus->hide();
        m_dlg->enableButtonOk(true);
    }
}

void GeneralTab::parentChanged(const QString& parentName)
{
    if (m_nameEdit->text() == parentName) {
        m_parentStatus->setText(i18n("A style cannot inherit from itself."));
        m_parentStatus->show();
        m_dlg->enableButtonOk(false);
    } else if (!m_dlg->checkCircle(m_nameEdit->text(), parentName)) {
        m_parentStatus->setText(i18n("The style cannot inherit from '%1' because of recursive references.",
                                     m_parentBox->currentText()));
        m_parentStatus->show();
        m_dlg->enableButtonOk(false);
    } else {
        m_parentStatus->hide();
        m_dlg->enableButtonOk(true);
    }

    if (parentName.isEmpty() || parentName == i18n("Default"))
        m_dlg->getStyle()->clearAttribute(KCStyle::NamedStyleKey);
    else
        m_dlg->getStyle()->setParentName(parentName);

    // Set difference to new parent, set GUI to parent values, add changes made before
    //  m_dlg->initGUI();
}

bool GeneralTab::apply(CustomStyle * style)
{
    if (m_nameEdit->isEnabled()) {
        if (style->type() != KCStyle::BUILTIN) {
            QString name(style->name());
            style->setName(m_nameEdit->text());
            if (m_parentBox->isEnabled()) {
                if (m_parentBox->currentText() == i18n("Default") || m_parentBox->currentText().isEmpty())
                    style->clearAttribute(KCStyle::NamedStyleKey);
                else
                    style->setParentName(m_parentBox->currentText());
            }
            m_dlg->getStyleManager()->changeName(name, m_nameEdit->text());
        }
    }

    if (style->type() == KCStyle::TENTATIVE)
        style->setType(KCStyle::CUSTOM);

    return true;
}



/***************************************************************************
 *
 * CellFormatDialog
 *
 ***************************************************************************/

CellFormatDialog::CellFormatDialog(QWidget* parent, Selection* selection)
        : KPageDialog(parent)
        , m_sheet(selection->activeSheet())
        , m_selection(selection)
        , m_style(0)
        , m_styleManager(0)
{
    initMembers();

    //We need both conditions quite often, so store the condition here too
    isRowSelected    = m_selection->isRowSelected();
    isColumnSelected = m_selection->isColumnSelected();

    QRect range = m_selection->lastRange();
    left = range.left();
    top = range.top();
    right = range.right();
    bottom = range.bottom();

    if (left == right)
        oneCol = true;
    else
        oneCol = false;

    if (top == bottom)
        oneRow = true;
    else
        oneRow = false;

    KCCell cell = KCCell(m_sheet, left, top);
    oneCell = (left == right && top == bottom &&
               !cell.doesMergeCells());

    isMerged = ((cell.doesMergeCells() &&
                 left + cell.mergedXCells() >= right &&
                 top + cell.mergedYCells() >= bottom));

    // Initialize with the upper left object.
    const KCStyle styleTopLeft = cell.style();
    borders[BorderType_Left].style = styleTopLeft.leftBorderPen().style();
    borders[BorderType_Left].width = styleTopLeft.leftBorderPen().width();
    borders[BorderType_Left].color = styleTopLeft.leftBorderPen().color();
    borders[BorderType_Top].style = styleTopLeft.topBorderPen().style();
    borders[BorderType_Top].width = styleTopLeft.topBorderPen().width();
    borders[BorderType_Top].color = styleTopLeft.topBorderPen().color();
    borders[BorderType_FallingDiagonal].style = styleTopLeft.fallDiagonalPen().style();
    borders[BorderType_FallingDiagonal].width = styleTopLeft.fallDiagonalPen().width();
    borders[BorderType_FallingDiagonal].color = styleTopLeft.fallDiagonalPen().color();
    borders[BorderType_RisingDiagonal].style = styleTopLeft.goUpDiagonalPen().style();
    borders[BorderType_RisingDiagonal].width = styleTopLeft.goUpDiagonalPen().width();
    borders[BorderType_RisingDiagonal].color = styleTopLeft.goUpDiagonalPen().color();

    // Look at the upper right one for the right border.
    const KCStyle styleTopRight = KCCell(m_sheet, right, top).style();
    borders[BorderType_Right].style = styleTopRight.rightBorderPen().style();
    borders[BorderType_Right].width = styleTopRight.rightBorderPen().width();
    borders[BorderType_Right].color = styleTopRight.rightBorderPen().color();

    // Look at the bottom left cell for the bottom border.
    const KCStyle styleBottomLeft = KCCell(m_sheet, left, bottom).style();
    borders[BorderType_Bottom].style = styleBottomLeft.bottomBorderPen().style();
    borders[BorderType_Bottom].width = styleBottomLeft.bottomBorderPen().width();
    borders[BorderType_Bottom].color = styleBottomLeft.bottomBorderPen().color();

    // Just an assumption
    cell = KCCell(m_sheet, right, top);
    if (cell.isPartOfMerged()) {
        cell = cell.masterCell();

        const KCStyle styleMove1 = KCCell(m_sheet, cell.column(), top).style();
        borders[BorderType_Vertical].style = styleMove1.leftBorderPen().style();
        borders[BorderType_Vertical].width = styleMove1.leftBorderPen().width();
        borders[BorderType_Vertical].color = styleMove1.leftBorderPen().color();

        const KCStyle styleMove2 = KCCell(m_sheet, right, cell.row()).style();
        borders[BorderType_Horizontal].style = styleMove2.topBorderPen().style();
        borders[BorderType_Horizontal].width = styleMove2.topBorderPen().width();
        borders[BorderType_Horizontal].color = styleMove2.topBorderPen().color();
    } else {
        borders[BorderType_Vertical].style = styleTopRight.leftBorderPen().style();
        borders[BorderType_Vertical].width = styleTopRight.leftBorderPen().width();
        borders[BorderType_Vertical].color = styleTopRight.leftBorderPen().color();
        const KCStyle styleBottomRight = KCCell(m_sheet, right, bottom).style();
        borders[BorderType_Horizontal].style = styleBottomRight.topBorderPen().style();
        borders[BorderType_Horizontal].width = styleBottomRight.topBorderPen().width();
        borders[BorderType_Horizontal].color = styleBottomRight.topBorderPen().color();
    }

    cell = KCCell(m_sheet, left, top);
    prefix = styleTopLeft.prefix();
    postfix = styleTopLeft.postfix();
    precision = styleTopLeft.precision();
    floatFormat = styleTopLeft.floatFormat();
    floatColor = styleTopLeft.floatColor();
    alignX = styleTopLeft.halign();
    alignY = styleTopLeft.valign();
    textColor = styleTopLeft.fontColor();
    bgColor = styleTopLeft.backgroundColor();
    fontSize = styleTopLeft.fontSize();
    fontFamily = styleTopLeft.fontFamily();
    fontBold = styleTopLeft.bold();
    fontItalic = styleTopLeft.italic();
    strike = styleTopLeft.strikeOut();
    underline = styleTopLeft.underline();
    // Needed to initialize the font correctly ( bug in Qt )
    font = styleTopLeft.font();
    m_currency = styleTopLeft.currency();

    brushColor = styleTopLeft.backgroundBrush().color();
    brushStyle = styleTopLeft.backgroundBrush().style();

    bMultiRow = styleTopLeft.wrapText();
    bVerticalText = styleTopLeft.verticalText();
    bShrinkToFit = styleTopLeft.shrinkToFit();
    textRotation = styleTopLeft.angle();
    formatType = styleTopLeft.formatType();

    bDontPrintText = !styleTopLeft.printText();
    bHideFormula   = styleTopLeft.hideFormula();
    bHideAll       = styleTopLeft.hideAll();
    bIsProtected   = !styleTopLeft.notProtected();

    indent = styleTopLeft.indentation();

    value = cell.value();

    const KCRowFormat *rl;
    const KCColumnFormat *cl;
    widthSize = 0.0;
    heightSize = 0.0;

    Selection::ConstIterator end(m_selection->constEnd());
    for (Selection::ConstIterator it(m_selection->constBegin()); it != end; ++it) {
        QRect range = (*it)->rect();
        KCStyle style = m_sheet->cellStorage()->style(range);   // FIXME merge
        initParameters(style);

        // left border
        range.setWidth(1);
        checkBorderLeft(m_sheet->cellStorage()->style(range));
        // right border
        range = (*it)->rect();
        range.setLeft(range.right());
        checkBorderRight(m_sheet->cellStorage()->style(range));
        // inner borders
        range = (*it)->rect();
        range = range.adjusted(1, 1, -1, -1);
        style = m_sheet->cellStorage()->style(range);
        checkBorderHorizontal(style);
        checkBorderVertical(style);
        // top border
        range = (*it)->rect();
        range.setHeight(1);
        checkBorderTop(style);
        // bottom border
        range = (*it)->rect();
        range.setBottom(range.top());
        checkBorderBottom(m_sheet->cellStorage()->style(range));
    }

    // column width
    if (!isRowSelected) {
        for (int x = left; x <= right; x++) {
            cl = m_sheet->columnFormat(x);
            widthSize = qMax(cl->width(), widthSize);
        }
    }

    // row height
    if (!isColumnSelected) {
        for (int y = top; y <= bottom; y++) {
            rl = m_sheet->rowFormat(y);
            heightSize = qMax(rl->height(), heightSize);
        }
    }

    if (!bTextRotation)
        textRotation = 0;

    init();
}

CellFormatDialog::CellFormatDialog(QWidget* parent, Selection* selection,
                                   CustomStyle* style, KCStyleManager* manager)
        : KPageDialog(parent)
        , m_sheet(selection->activeSheet())
        , m_selection(selection)
        , m_style(style)
        , m_styleManager(manager)
{
    initMembers();
    initGUI();
    init();
}

void CellFormatDialog::initGUI()
{
    isRowSelected    = false;
    isColumnSelected = false;
    styleName = m_style->name();

    borders[BorderType_Left].style = m_style->leftBorderPen().style();
    borders[BorderType_Left].width = m_style->leftBorderPen().width();
    borders[BorderType_Left].color = m_style->leftBorderPen().color();

    borders[BorderType_Top].style  = m_style->topBorderPen().style();
    borders[BorderType_Top].width  = m_style->topBorderPen().width();
    borders[BorderType_Top].color  = m_style->topBorderPen().color();

    borders[BorderType_Right].style = m_style->rightBorderPen().style();
    borders[BorderType_Right].width = m_style->rightBorderPen().width();
    borders[BorderType_Right].color = m_style->rightBorderPen().color();

    borders[BorderType_Bottom].style = m_style->bottomBorderPen().style();
    borders[BorderType_Bottom].width = m_style->bottomBorderPen().width();
    borders[BorderType_Bottom].color = m_style->bottomBorderPen().color();

    borders[BorderType_FallingDiagonal].style = m_style->fallDiagonalPen().style();
    borders[BorderType_FallingDiagonal].width = m_style->fallDiagonalPen().width();
    borders[BorderType_FallingDiagonal].color = m_style->fallDiagonalPen().color();

    borders[BorderType_RisingDiagonal].style  = m_style->goUpDiagonalPen().style();
    borders[BorderType_RisingDiagonal].width  = m_style->goUpDiagonalPen().width();
    borders[BorderType_RisingDiagonal].color  = m_style->goUpDiagonalPen().color();

    borders[BorderType_Vertical].style = m_style->leftBorderPen().style();
    borders[BorderType_Vertical].width = m_style->leftBorderPen().width();
    borders[BorderType_Vertical].color = m_style->leftBorderPen().color();
    borders[BorderType_Horizontal].style = m_style->topBorderPen().style();
    borders[BorderType_Horizontal].width = m_style->topBorderPen().width();
    borders[BorderType_Horizontal].color = m_style->topBorderPen().color();

    prefix         = m_style->prefix();
    postfix        = m_style->postfix();
    precision      = m_style->precision();
    floatFormat    = m_style->floatFormat();
    floatColor     = m_style->floatColor();
    alignX         = m_style->halign();
    alignY         = m_style->valign();
    textColor      = m_style->fontColor();
    bgColor        = m_style->backgroundColor();
    fontSize       = m_style->fontSize();
    fontFamily     = m_style->fontFamily();

    fontBold       = m_style->bold();
    fontItalic     = m_style->italic();
    strike         = m_style->strikeOut();
    underline      = m_style->underline();

    // Needed to initialize the font correctly ( bug in Qt )
    font           = m_style->font();
    m_currency     = m_style->currency();
    brushColor     = m_style->backgroundBrush().color();
    brushStyle     = m_style->backgroundBrush().style();

    bMultiRow      = m_style->wrapText();
    bVerticalText  = m_style->verticalText();
    bShrinkToFit   = m_style->shrinkToFit();
    textRotation   = m_style->angle();
    formatType     = m_style->formatType();
    indent         = m_style->indentation();

    bDontPrintText = !m_style->printText();
    bHideFormula   = m_style->hideFormula();
    bHideAll       = m_style->hideAll();
    bIsProtected   = !m_style->notProtected();

    widthSize      = defaultWidthSize;
    heightSize     = defaultHeightSize;
}

CellFormatDialog::~CellFormatDialog()
{
    delete formatOnlyNegSignedPixmap;
    delete formatRedOnlyNegSignedPixmap;
    delete formatRedNeverSignedPixmap;
    delete formatAlwaysSignedPixmap;
    delete formatRedAlwaysSignedPixmap;
}

void CellFormatDialog::initMembers()
{
    formatOnlyNegSignedPixmap    = 0;
    formatRedOnlyNegSignedPixmap = 0;
    formatRedNeverSignedPixmap   = 0;
    formatAlwaysSignedPixmap     = 0;
    formatRedAlwaysSignedPixmap  = 0;

    // We assume, that all other objects have the same values
    for (int i = 0; i < BorderType_END; ++i) {
        borders[i].bStyle = true;
        borders[i].bColor = true;
    }
    bFloatFormat    = true;
    bFloatColor     = true;
    bTextColor      = true;
    bBgColor        = true;
    bTextFontFamily = true;
    bTextFontSize   = true;
    bTextFontBold   = true;
    bTextFontItalic = true;
    bStrike         = true;
    bUnderline      = true;
    bTextRotation   = true;
    bFormatType     = true;
    bCurrency       = true;
    bDontPrintText  = false;
    bHideFormula    = false;
    bHideAll        = false;
    bIsProtected    = true;

    m_currency      = KCCurrency(); // locale default

    KCSheet* sheet = m_sheet;
    defaultWidthSize  = sheet ? sheet->map()->defaultColumnFormat()->width() : 0;
    defaultHeightSize = sheet ? sheet->map()->defaultRowFormat()->height() : 0;
}

bool CellFormatDialog::checkCircle(QString const & name, QString const & parent)
{
    return m_styleManager->checkCircle(name, parent);
}

KLocale* CellFormatDialog::locale() const
{
    return m_sheet->map()->calculationSettings()->locale();
}

void CellFormatDialog::checkBorderRight(const KCStyle& style)
{
    if (borders[BorderType_Right].style != style.rightBorderPen().style() ||
            borders[BorderType_Right].width != style.rightBorderPen().width())
        borders[BorderType_Right].bStyle = false;
    if (borders[BorderType_Right].color != style.rightBorderPen().color())
        borders[BorderType_Right].bColor = false;
}

void CellFormatDialog::checkBorderLeft(const KCStyle& style)
{
    if (borders[BorderType_Left].style != style.leftBorderPen().style() ||
            borders[BorderType_Left].width != style.leftBorderPen().width())
        borders[BorderType_Left].bStyle = false;
    if (borders[BorderType_Left].color != style.leftBorderPen().color())
        borders[BorderType_Left].bColor = false;
}

void CellFormatDialog::checkBorderTop(const KCStyle& style)
{
    if (borders[BorderType_Top].style != style.topBorderPen().style() ||
            borders[BorderType_Top].width != style.topBorderPen().width())
        borders[BorderType_Top].bStyle = false;
    if (borders[BorderType_Top].color != style.topBorderPen().color())
        borders[BorderType_Top].bColor = false;
}

void CellFormatDialog::checkBorderBottom(const KCStyle& style)
{
    if (borders[BorderType_Bottom].style != style.bottomBorderPen().style() ||
            borders[BorderType_Bottom].width != style.bottomBorderPen().width())
        borders[BorderType_Bottom].bStyle = false;
    if (borders[BorderType_Bottom].color != style.bottomBorderPen().color())
        borders[BorderType_Bottom].bColor = false;
}

void CellFormatDialog::checkBorderVertical(const KCStyle& style)
{
    if (borders[BorderType_Vertical].style != style.leftBorderPen().style() ||
            borders[BorderType_Vertical].width != style.leftBorderPen().width())
        borders[BorderType_Vertical].bStyle = false;
    if (borders[BorderType_Vertical].color != style.leftBorderPen().color())
        borders[BorderType_Vertical].bColor = false;
}

void CellFormatDialog::checkBorderHorizontal(const KCStyle& style)
{
    if (borders[BorderType_Horizontal].style != style.topBorderPen().style() ||
            borders[BorderType_Horizontal].width != style.topBorderPen().width())
        borders[BorderType_Horizontal].bStyle = false;
    if (borders[BorderType_Horizontal].color != style.topBorderPen().color())
        borders[BorderType_Horizontal].bColor = false;
}


void CellFormatDialog::initParameters(const KCStyle& style)
{
    if (borders[BorderType_FallingDiagonal].style != style.fallDiagonalPen().style())
        borders[BorderType_FallingDiagonal].bStyle = false;
    if (borders[BorderType_FallingDiagonal].width != style.fallDiagonalPen().width())
        borders[BorderType_FallingDiagonal].bStyle = false;
    if (borders[BorderType_FallingDiagonal].color != style.fallDiagonalPen().color())
        borders[BorderType_FallingDiagonal].bColor = false;

    if (borders[BorderType_RisingDiagonal].style != style.goUpDiagonalPen().style())
        borders[BorderType_RisingDiagonal].bStyle = false;
    if (borders[BorderType_RisingDiagonal].width != style.goUpDiagonalPen().width())
        borders[BorderType_RisingDiagonal].bStyle = false;
    if (borders[BorderType_RisingDiagonal].color != style.goUpDiagonalPen().color())
        borders[BorderType_RisingDiagonal].bColor = false;
    if (strike != style.strikeOut())
        bStrike = false;
    if (underline != style.underline())
        bUnderline = false;
    if (prefix != style.prefix())
        prefix.clear();
    if (postfix != style.postfix())
        postfix.clear();
    if (floatFormat != style.floatFormat())
        bFloatFormat = false;
    if (floatColor != style.floatColor())
        bFloatColor = false;
    if (textColor != style.fontColor())
        bTextColor = false;
    if (fontFamily != style.fontFamily())
        bTextFontFamily = false;
    if (fontSize != style.fontSize())
        bTextFontSize = false;
    if (fontBold != style.bold())
        bTextFontBold = false;
    if (fontItalic != style.italic())
        bTextFontItalic = false;
    if (bgColor != style.backgroundColor())
        bBgColor = false;
    if (textRotation != style.angle())
        bTextRotation = false;
    if (formatType != style.formatType())
        bFormatType = false;
    if (bMultiRow != style.wrapText())
        bMultiRow = false;
    if (bVerticalText != style.verticalText())
        bVerticalText = false;
    if (bShrinkToFit != style.shrinkToFit())
        bShrinkToFit = false;
    if (!bDontPrintText != style.printText())
        bDontPrintText = false;

    KCCurrency currency = style.currency();
    if (currency != m_currency)
        bCurrency = false;
}

void CellFormatDialog::init()
{
    // Did we initialize the bitmaps ?
    if (formatOnlyNegSignedPixmap == 0) {
        QColor black = QApplication::palette().text().color(); // not necessarily black :)
        formatOnlyNegSignedPixmap    = paintFormatPixmap("123.456", Qt::black, "-123.456", Qt::black);
        formatRedOnlyNegSignedPixmap = paintFormatPixmap("123.456", Qt::black, "-123.456", Qt::red);
        formatRedNeverSignedPixmap   = paintFormatPixmap("123.456", Qt::black, "123.456", Qt::red);
        formatAlwaysSignedPixmap     = paintFormatPixmap("+123.456", Qt::black, "-123.456", Qt::black);
        formatRedAlwaysSignedPixmap  = paintFormatPixmap("+123.456", Qt::black, "-123.456", Qt::red);
    }

    setCaption(i18n("KCCell KCFormat"));
    setButtons(KDialog::Ok | KDialog::Cancel);
    setFaceType(KPageDialog::Tabbed);
    setMinimumWidth(600);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    if (m_style) {
        generalPage = new GeneralTab(this, this);

        KPageWidgetItem* generalitem = addPage(generalPage, i18n("&General"));
        //generalitem->setHeader( i18n( "&General" ) );
        Q_UNUSED(generalitem);
    }

    floatPage = new CellFormatPageFloat(this, this);
    addPage(floatPage, i18n("&Data KCFormat"));

    fontPage = new CellFormatPageFont(this, this);
    addPage(fontPage, i18n("&Font"));

    //  miscPage = new CellFormatPageMisc( tab, this );
    //  tab->addTab( miscPage, i18n("&Misc") );

    positionPage = new CellFormatPagePosition(this, this);
    addPage(positionPage, i18n("&Position"));

    borderPage = new CellFormatPageBorder(this, this);
    addPage(borderPage, i18n("&Border"));

    patternPage = new CellFormatPagePattern(this, this);
    addPage(patternPage, i18n("Back&ground"));

    protectPage = new CellFormatPageProtection(this, this);
    addPage(protectPage, i18n("&KCCell Protection"));

    connect(this, SIGNAL(okClicked()), this, SLOT(slotApply()));
}

QPixmap * CellFormatDialog::paintFormatPixmap(const char * _string1, const QColor & _color1,
        const char *_string2, const QColor & _color2)
{
    QPixmap * pixmap = new QPixmap(150, 14);
    pixmap->fill(Qt::transparent);

    QPainter painter;
    painter.begin(pixmap);
    painter.setPen(_color1);
    painter.drawText(2, 11, _string1);
    painter.setPen(_color2);
    painter.drawText(75, 11, _string2);
    painter.end();

    return pixmap;
}

void CellFormatDialog::applyStyle()
{
    generalPage->apply(m_style);

    borderPage->apply(0);
    floatPage->apply(m_style);
    // miscPage->apply( m_style );
    fontPage->apply(m_style);
    positionPage->apply(m_style);
    patternPage->apply(m_style);
    protectPage->apply(m_style);
}

void CellFormatDialog::slotApply()
{
    if (m_style) {
        applyStyle();
        return;
    }

    // (Tomas) TODO: this will be slow !!!
    // We need to create a command that would act as macro,
    // but which would also ensure that updates are not painted until everything
    // is updated properly ...
    QUndoCommand* macroCommand = new QUndoCommand("Change KCFormat");

    if (isMerged != positionPage->getMergedCellState()) {
        MergeCommand* command = new MergeCommand(macroCommand);
        command->setSheet(m_sheet);
        command->setSelection(m_selection);
        if (!positionPage->getMergedCellState())
            //dissociate cells
            command->setReverse(true);
        command->add(*m_selection);
    }

    StyleCommand* command = new StyleCommand(macroCommand);
    command->setSheet(m_sheet);
    command->add(*m_selection);
    borderPage->apply(command);
    floatPage->apply(command);
    fontPage->apply(command);
    positionPage->apply(command);
    patternPage->apply(command);
    protectPage->apply(command);

    if (int(positionPage->getSizeHeight()) != int(heightSize)) {
        ResizeRowManipulator* command = new ResizeRowManipulator(macroCommand);
        command->setSheet(m_sheet);
        command->setSize(positionPage->getSizeHeight());
        command->add(*m_selection);
    }
    if (int(positionPage->getSizeWidth()) != int(widthSize)) {
        ResizeColumnManipulator* command = new ResizeColumnManipulator(macroCommand);
        command->setSheet(m_sheet);
        command->setSize(positionPage->getSizeWidth());
        command->add(*m_selection);
    }

    m_selection->canvas()->addCommand(macroCommand);
}



/***************************************************************************
 *
 * CellFormatPageFloat
 *
 ***************************************************************************/

CellFormatPageFloat::CellFormatPageFloat(QWidget* parent, CellFormatDialog *_dlg)
        : QWidget(parent),
        dlg(_dlg)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QGroupBox *grp = new QGroupBox(i18n("KCFormat"), this);
    QGridLayout *grid = new QGridLayout(grp);
    grid->setMargin(KDialog::marginHint());
    grid->setSpacing(KDialog::spacingHint());

    int fHeight = grp->fontMetrics().height();
    grid->addItem(new QSpacerItem(0, fHeight / 2), 0, 0);  // groupbox title

    generic = new QRadioButton(i18n("Generic"), grp);
    generic->setWhatsThis(i18n("This is the default format and KSpread autodetects the actual data type depending on the current cell data. By default, KSpread right justifies numbers, dates and times within a cell and left justifies anything else."));
    grid->addWidget(generic, 1, 0);

    number = new QRadioButton(i18n("KCNumber"), grp);
    number->setWhatsThis(i18n("The number notation uses the notation you globally choose in System Settings -> Common Appearance and Behavior -> Locale -> Country/KCRegion & Language -> Numbers tab. Numbers are right justified by default."));
    grid->addWidget(number, 2, 0);

    percent = new QRadioButton(i18n("Percent"), grp);
    percent->setWhatsThis(i18n("When you have a number in the current cell and you switch from the dcell format from Generic to Percent, the current cell number will be multiplied by 100%.\nFor example if you enter 12 and set the cell format to Percent, the number will then be 1,200 %. Switching back to Generic cell format will bring it back to 12.\nYou can also use the Percent icon in the KCFormat Toolbar."));
    grid->addWidget(percent, 3, 0);

    money = new QRadioButton(i18n("Money"), grp);
    money->setWhatsThis(i18n("The Money format converts your number into money notation using the settings globally fixed in System Settings -> Common Appearance and Behavior -> Locale -> Country/KCRegion & Language -> Money. The currency symbol will be displayed and the precision will be the one set in System Settings.\nYou can also use the KCCurrency icon in the KCFormat Toolbar to set the cell formatting to look like your current currency."));
    grid->addWidget(money, 4, 0);

    scientific = new QRadioButton(i18n("Scientific"), grp);
    scientific->setWhatsThis(i18n("The scientific format changes your number using the scientific notation. For example, 0.0012 will be changed to 1.2E-03. Going back using Generic cell format will display 0.0012 again."));
    grid->addWidget(scientific, 5, 0);

    fraction = new QRadioButton(i18n("Fraction"), grp);
    fraction->setWhatsThis(i18n("The fraction format changes your number into a fraction. For example, 0.1 can be changed to 1/8, 2/16, 1/10, etc. You define the type of fraction by choosing it in the field on the right. If the exact fraction is not possible in the fraction mode you choose, the nearest closest match is chosen.\n For example: when we have 1.5 as number, we choose Fraction and Sixteenths 1/16 the text displayed into cell is \"1 8/16\" which is an exact fraction. If you have 1.4 as number in your cell and you choose Fraction and Sixteenths 1/16 then the cell will display \"1 6/16\" which is the nearest closest Sixteenth fraction."));
    grid->addWidget(fraction, 6, 0);

    date = new QRadioButton(i18n("Date"), grp);
    date->setWhatsThis(i18n("To enter a date, you should enter it in one of the formats set in System Settings -> Common Appearance and Behavior -> Locale -> Country/KCRegion & Language -> Time & Dates. There are two formats set here: the date format and the short date format.\nJust like you can drag down numbers you can also drag down dates and the next cells will also get dates."));
    grid->addWidget(date, 7, 0);

    time = new QRadioButton(i18n("Time"), grp);
    time->setWhatsThis(i18n("This formats your cell content as a time. To enter a time, you should enter it in the Time format set in System Settings -> Common Appearance and Behavior -> Locale -> Country/KCRegion & Language -> Date & Time. In the KCCell KCFormat dialog box you can set how the time should be displayed by choosing one of the available time format options. The default format is the system format set in System Settings. When the number in the cell does not make sense as a time, KSpread will display 00:00 in the global format you have in System Settings."));
    grid->addWidget(time, 8, 0);

    datetime = new QRadioButton(i18n("Date and Time"), grp);
    datetime->setWhatsThis(i18n("This formats your cell content as date and time. To enter a date and a time, you should enter it in the Time format set in System Settings -> Common Appearance and Behavior -> Locale -> Country/KCRegion & Language -> Time & Dates. In the KCCell KCFormat dialog box you can set how the time should be displayed by choosing one of the available date format options. The default format is the system format set in System Settings. When the number in the cell does not make sense as a date and time, KSpread will display 00:00 in the global format you have in System Settings."));
    grid->addWidget(datetime, 9, 0);

    textFormat = new QRadioButton(i18n("Text"), grp);
    textFormat->setWhatsThis(i18n("This formats your cell content as text. This can be useful if you want a number treated as text instead as a number, for example for a ZIP code. Setting a number as text format will left justify it. When numbers are formatted as text, they cannot be used in calculations or formulas. It also change the way the cell is justified."));
    grid->addWidget(textFormat, 10, 0);

    customFormat = new QRadioButton(i18n("Custom"), grp);
    customFormat->setWhatsThis(i18n("The custom format does not work yet. To be enabled in the next release."));
    grid->addWidget(customFormat, 11, 0);
    customFormat->setEnabled(false);

    QGroupBox *box2 = new QGroupBox(grp);
    box2->setTitle(i18n("Preview"));
    QGridLayout *grid3 = new QGridLayout(box2);
    grid3->setMargin(KDialog::marginHint());
    grid3->setSpacing(KDialog::spacingHint());

    exampleLabel = new QLabel(box2);
    exampleLabel->setWhatsThis(i18n("This will display a preview of your choice so you can know what it does before clicking the OK button to validate it."));
    grid3->addWidget(exampleLabel, 0, 1);

    grid->addWidget(box2, 9, 1, 3, 1);

    customFormatEdit = new KLineEdit(grp);
    grid->addWidget(customFormatEdit, 0, 1);
    customFormatEdit->setHidden(true);

    listFormat = new QListWidget(grp);
    grid->addWidget(listFormat, 1, 1, 8, 1);
    listFormat->setWhatsThis(i18n("Displays choices of format for the fraction, date or time formats."));
    layout->addWidget(grp);

    /* *** */

    QGroupBox *box = new QGroupBox(this);

    grid = new QGridLayout(box);
    grid->setMargin(KDialog::marginHint());
    grid->setSpacing(KDialog::spacingHint());

    postfix = new KLineEdit(box);
    postfix->setWhatsThis(i18n("You can add here a Postfix such as a $HK symbol to the end of each cell content in the checked format."));
    grid->addWidget(postfix, 2, 1);
    precision = new KIntNumInput(dlg->precision, box, 10);
    precision->setSpecialValueText(i18n("variable"));
    precision->setRange(-1, 10, 1);
    precision->setSliderEnabled(false);
    precision->setWhatsThis(i18n("You can control how many digits are displayed after the decimal point for numeric values. This can also be changed using the Increase precision or Decrease precision icons in the KCFormat toolbar. "));
    grid->addWidget(precision, 1, 1);

    prefix = new KLineEdit(box);
    prefix->setWhatsThis(i18n("You can add here a Prefix such as a $ symbol at the start of each cell content in the checked format."));
    grid->addWidget(prefix, 0, 1);

    format = new KComboBox(box);
    format->setWhatsThis(i18n("You can choose whether positive values are displayed with a leading + sign and whether negative values are shown in red."));
    grid->addWidget(format, 0, 3);

    QLabel* tmpQLabel;
    tmpQLabel = new QLabel(box);
    grid->addWidget(tmpQLabel, 2, 0);
    tmpQLabel->setText(i18n("Postfix:"));

    postfix->setText(dlg->postfix);

    tmpQLabel = new QLabel(box);
    grid->addWidget(tmpQLabel, 0, 0);
    tmpQLabel->setText(i18n("Prefix:"));

    tmpQLabel = new QLabel(box);
    grid->addWidget(tmpQLabel, 1, 0);
    tmpQLabel->setText(i18n("Precision:"));

    prefix->setText(dlg->prefix);

    format->setIconSize(QSize(150, 14));
    format->insertItem(0, *_dlg->formatOnlyNegSignedPixmap, "");
    format->insertItem(1, *_dlg->formatRedOnlyNegSignedPixmap, "");
    format->insertItem(2, *_dlg->formatRedNeverSignedPixmap, "");
    format->insertItem(3, *_dlg->formatAlwaysSignedPixmap, "");
    format->insertItem(4, *_dlg->formatRedAlwaysSignedPixmap, "");

    tmpQLabel = new QLabel(box);
    grid->addWidget(tmpQLabel, 0, 2);
    tmpQLabel->setText(i18n("KCFormat:"));

    currencyLabel = new QLabel(box);
    grid->addWidget(currencyLabel, 1, 2);
    currencyLabel->setText(i18n("KCCurrency:"));

    currency = new KComboBox(box);
    grid->addWidget(currency, 1, 3);

    // fill the currency combo box
    currency->insertItem(0, i18n("Automatic"));
    int index = 2; //ignore first two in the list
    bool ok = true;
    QString text;
    while (ok) {
        text = KCCurrency::chooseString(index, ok);
        if (ok)
            currency->insertItem(index - 1, text);
        else
            break;
        ++index;
    }
    currency->setCurrentIndex(0);
    currency->hide();
    currencyLabel->hide();

    if (!dlg->bFloatFormat || !dlg->bFloatColor)
        format->setCurrentIndex(5);
    else if (dlg->floatFormat == KCStyle::OnlyNegSigned && dlg->floatColor == KCStyle::AllBlack)
        format->setCurrentIndex(0);
    else if (dlg->floatFormat == KCStyle::OnlyNegSigned && dlg->floatColor == KCStyle::NegRed)
        format->setCurrentIndex(1);
    else if (dlg->floatFormat == KCStyle::AlwaysUnsigned && dlg->floatColor == KCStyle::NegRed)
        format->setCurrentIndex(2);
    else if (dlg->floatFormat == KCStyle::AlwaysSigned && dlg->floatColor == KCStyle::AllBlack)
        format->setCurrentIndex(3);
    else if (dlg->floatFormat == KCStyle::AlwaysSigned && dlg->floatColor == KCStyle::NegRed)
        format->setCurrentIndex(4);
    layout->addWidget(box);

    cellFormatType = dlg->formatType;
    newFormatType = cellFormatType;

    if (!cellFormatType)
        generic->setChecked(true);
    else {
        if (cellFormatType == KCFormat::KCNumber)
            number->setChecked(true);
        else if (cellFormatType == KCFormat::Percentage)
            percent->setChecked(true);
        else if (cellFormatType == KCFormat::Money) {
            money->setChecked(true);
            currencyLabel->show();
            currency->show();
            if (dlg->bCurrency) {
                QString tmp;
                if (dlg->m_currency.index() == 1)   // custom currency unit
                    tmp = dlg->m_currency.symbol();
                else {
                    bool ok = true;
                    tmp = KCCurrency::chooseString(dlg->m_currency.index(), ok);
                    if (!ok)
                        tmp = dlg->m_currency.symbol();
                }
                currency->setCurrentIndex(currency->findText(tmp));
            }
        } else if (cellFormatType == KCFormat::Scientific)
            scientific->setChecked(true);
        else if (KCFormat::isDate(cellFormatType))
            date->setChecked(true);
        else if (KCFormat::isTime(cellFormatType))
            time->setChecked(true);
        else if (KCFormat::isFraction(cellFormatType))
            fraction->setChecked(true);
        else if (cellFormatType == KCFormat::Text)
            textFormat->setChecked(true);
        else if (cellFormatType == KCFormat::Custom)
            customFormat->setChecked(true);
    }

    connect(generic, SIGNAL(clicked()), this, SLOT(slotChangeState()));
    connect(fraction, SIGNAL(clicked()), this, SLOT(slotChangeState()));
    connect(money, SIGNAL(clicked()), this, SLOT(slotChangeState()));
    connect(date, SIGNAL(clicked()), this, SLOT(slotChangeState()));
    connect(datetime, SIGNAL(clicked()), this, SLOT(slotChangeState()));
    connect(scientific, SIGNAL(clicked()), this, SLOT(slotChangeState()));
    connect(number, SIGNAL(clicked()), this, SLOT(slotChangeState()));
    connect(percent, SIGNAL(clicked()), this, SLOT(slotChangeState()));
    connect(time, SIGNAL(clicked()), this, SLOT(slotChangeState()));
    connect(textFormat, SIGNAL(clicked()), this, SLOT(slotChangeState()));
    connect(customFormat, SIGNAL(clicked()), this, SLOT(slotChangeState()));

    connect(listFormat, SIGNAL(itemSelectionChanged()), this, SLOT(makeformat()));
    connect(precision, SIGNAL(valueChanged(int)), this, SLOT(slotChangeValue(int)));
    connect(prefix, SIGNAL(textChanged(const QString &)), this, SLOT(makeformat()));
    connect(postfix, SIGNAL(textChanged(const QString &)), this, SLOT(makeformat()));
    connect(currency, SIGNAL(activated(const QString &)), this, SLOT(currencyChanged(const QString &)));
    connect(format, SIGNAL(activated(int)), this, SLOT(formatChanged(int)));
    connect(format, SIGNAL(activated(int)), this, SLOT(makeformat()));
    slotChangeState();
    m_bFormatColorChanged = false;
    m_bFormatTypeChanged = false;
    this->resize(400, 400);
}

void CellFormatPageFloat::formatChanged(int)
{
    m_bFormatColorChanged = true;
}

void CellFormatPageFloat::slotChangeValue(int)
{
    makeformat();
}
void CellFormatPageFloat::slotChangeState()
{
    QStringList list;
    listFormat->clear();
    currency->hide();
    currencyLabel->hide();

    // start with enabled, they get disabled when inappropriate further down
    precision->setEnabled(true);
    prefix->setEnabled(true);
    postfix->setEnabled(true);
    format->setEnabled(true);
    if (generic->isChecked() || number->isChecked() || percent->isChecked() ||
            scientific->isChecked() || textFormat->isChecked())
        listFormat->setEnabled(false);
    else if (money->isChecked()) {
        listFormat->setEnabled(false);
        precision->setValue(2);
        currency->show();
        currencyLabel->show();
    } else if (date->isChecked()) {
        format->setEnabled(false);
        precision->setEnabled(false);
        prefix->setEnabled(false);
        postfix->setEnabled(false);
        listFormat->setEnabled(true);
        init();
    } else if (datetime->isChecked()) {
        format->setEnabled(false);
        precision->setEnabled(false);
        prefix->setEnabled(false);
        postfix->setEnabled(false);
        listFormat->setEnabled(true);
        datetimeInit();
    } else if (fraction->isChecked()) {
        precision->setEnabled(false);
        listFormat->setEnabled(true);
        list += i18n("Halves 1/2");
        list += i18n("Quarters 1/4");
        list += i18n("Eighths 1/8");
        list += i18n("Sixteenths 1/16");
        list += i18n("Tenths 1/10");
        list += i18n("Hundredths 1/100");
        list += i18n("One digit 5/9");
        list += i18n("Two digits 15/22");
        list += i18n("Three digits 153/652");
        listFormat->addItems(list);
        if (cellFormatType == KCFormat::fraction_half)
            listFormat->setCurrentRow(0);
        else if (cellFormatType == KCFormat::fraction_quarter)
            listFormat->setCurrentRow(1);
        else if (cellFormatType == KCFormat::fraction_eighth)
            listFormat->setCurrentRow(2);
        else if (cellFormatType == KCFormat::fraction_sixteenth)
            listFormat->setCurrentRow(3);
        else if (cellFormatType == KCFormat::fraction_tenth)
            listFormat->setCurrentRow(4);
        else if (cellFormatType == KCFormat::fraction_hundredth)
            listFormat->setCurrentRow(5);
        else if (cellFormatType == KCFormat::fraction_one_digit)
            listFormat->setCurrentRow(6);
        else if (cellFormatType == KCFormat::fraction_two_digits)
            listFormat->setCurrentRow(7);
        else if (cellFormatType == KCFormat::fraction_three_digits)
            listFormat->setCurrentRow(8);
        else
            listFormat->setCurrentRow(0);
    } else if (time->isChecked()) {
        precision->setEnabled(false);
        prefix->setEnabled(false);
        postfix->setEnabled(false);
        format->setEnabled(false);
        listFormat->setEnabled(true);


        list += i18n("System: ") + dlg->locale()->formatTime(QTime::currentTime(), false);
        list += i18n("System: ") + dlg->locale()->formatTime(QTime::currentTime(), true);
        QDateTime tmpTime(QDate(1, 1, 1900), QTime(10, 35, 25), Qt::UTC);


        ValueFormatter *fmt = dlg->getSheet()->map()->formatter();
        list += fmt->timeFormat(tmpTime, KCFormat::Time1);
        list += fmt->timeFormat(tmpTime, KCFormat::Time2);
        list += fmt->timeFormat(tmpTime, KCFormat::Time3);
        list += fmt->timeFormat(tmpTime, KCFormat::Time4);
        list += fmt->timeFormat(tmpTime, KCFormat::Time5);
        list += (fmt->timeFormat(tmpTime, KCFormat::Time6) + i18n(" (=[mm]:ss)"));
        list += (fmt->timeFormat(tmpTime, KCFormat::Time7) + i18n(" (=[hh]:mm:ss)"));
        list += (fmt->timeFormat(tmpTime, KCFormat::Time8) + i18n(" (=[hh]:mm)"));
        listFormat->addItems(list);

        if (cellFormatType == KCFormat::Time)
            listFormat->setCurrentRow(0);
        else if (cellFormatType == KCFormat::SecondeTime)
            listFormat->setCurrentRow(1);
        else if (cellFormatType == KCFormat::Time1)
            listFormat->setCurrentRow(2);
        else if (cellFormatType == KCFormat::Time2)
            listFormat->setCurrentRow(3);
        else if (cellFormatType == KCFormat::Time3)
            listFormat->setCurrentRow(4);
        else if (cellFormatType == KCFormat::Time4)
            listFormat->setCurrentRow(5);
        else if (cellFormatType == KCFormat::Time5)
            listFormat->setCurrentRow(6);
        else if (cellFormatType == KCFormat::Time6)
            listFormat->setCurrentRow(7);
        else if (cellFormatType == KCFormat::Time7)
            listFormat->setCurrentRow(8);
        else if (cellFormatType == KCFormat::Time8)
            listFormat->setCurrentRow(9);
        else
            listFormat->setCurrentRow(0);
    }

    if (customFormat->isChecked()) {
        customFormatEdit->setHidden(false);
        precision->setEnabled(false);
        prefix->setEnabled(false);
        postfix->setEnabled(false);
        format->setEnabled(false);
        listFormat->setEnabled(true);
    } else
        customFormatEdit->setHidden(true);

    m_bFormatTypeChanged = true;

    makeformat();
}

void CellFormatPageFloat::init()
{
    QStringList list;
    QString tmp;
    QString tmp2;
    QDate tmpDate(2000, 2, 18);
    list += i18n("System: ") + dlg->locale()->formatDate(QDate::currentDate(), KLocale::ShortDate);
    list += i18n("System: ") + dlg->locale()->formatDate(QDate::currentDate(), KLocale::LongDate);

    ValueFormatter *fmt = dlg->getSheet()->map()->formatter();

    /*18-Feb-00*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date1);
    /*18-Feb-1999*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date2);
    /*18-Feb*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date3);
    /*18-2*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date4);
    /*18/2/00*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date5);
    /*18/5/1999*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date6);
    /*Feb-99*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date7);
    /*February-99*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date8);
    /*February-1999*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date9);
    /*F-99*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date10);
    /*18/Feb*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date11);
    /*18/2*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date12);
    /*18/Feb/1999*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date13);
    /*2000/Feb/18*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date14);
    /*2000-Feb-18*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date15);
    /*2000-2-18*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date16);
    /*2 february 2000*/
    list += fmt->dateFormat(tmpDate, KCFormat::Date17);
    list += fmt->dateFormat(tmpDate, KCFormat::Date18);
    list += fmt->dateFormat(tmpDate, KCFormat::Date19);
    list += fmt->dateFormat(tmpDate, KCFormat::Date20);
    list += fmt->dateFormat(tmpDate, KCFormat::Date21);
    list += fmt->dateFormat(tmpDate, KCFormat::Date22);
    list += fmt->dateFormat(tmpDate, KCFormat::Date23);
    list += fmt->dateFormat(tmpDate, KCFormat::Date24);
    list += fmt->dateFormat(tmpDate, KCFormat::Date25);
    list += fmt->dateFormat(tmpDate, KCFormat::Date26);
    list += fmt->dateFormat(tmpDate, KCFormat::Date27);
    list += fmt->dateFormat(tmpDate, KCFormat::Date28);
    list += fmt->dateFormat(tmpDate, KCFormat::Date29);
    list += fmt->dateFormat(tmpDate, KCFormat::Date30);
    list += fmt->dateFormat(tmpDate, KCFormat::Date31);
    list += fmt->dateFormat(tmpDate, KCFormat::Date32);
    list += fmt->dateFormat(tmpDate, KCFormat::Date33);
    list += fmt->dateFormat(tmpDate, KCFormat::Date34);
    list += fmt->dateFormat(tmpDate, KCFormat::Date35);

    listFormat->addItems(list);
    if (cellFormatType == KCFormat::ShortDate)
        listFormat->setCurrentRow(0);
    else if (cellFormatType == KCFormat::TextDate)
        listFormat->setCurrentRow(1);
    else if (cellFormatType == KCFormat::Date1)
        listFormat->setCurrentRow(2);
    else if (cellFormatType == KCFormat::Date2)
        listFormat->setCurrentRow(3);
    else if (cellFormatType == KCFormat::Date3)
        listFormat->setCurrentRow(4);
    else if (cellFormatType == KCFormat::Date4)
        listFormat->setCurrentRow(5);
    else if (cellFormatType == KCFormat::Date5)
        listFormat->setCurrentRow(6);
    else if (cellFormatType == KCFormat::Date6)
        listFormat->setCurrentRow(7);
    else if (cellFormatType == KCFormat::Date7)
        listFormat->setCurrentRow(8);
    else if (cellFormatType == KCFormat::Date8)
        listFormat->setCurrentRow(9);
    else if (cellFormatType == KCFormat::Date9)
        listFormat->setCurrentRow(10);
    else if (cellFormatType == KCFormat::Date10)
        listFormat->setCurrentRow(11);
    else if (cellFormatType == KCFormat::Date11)
        listFormat->setCurrentRow(12);
    else if (cellFormatType == KCFormat::Date12)
        listFormat->setCurrentRow(13);
    else if (cellFormatType == KCFormat::Date13)
        listFormat->setCurrentRow(14);
    else if (cellFormatType == KCFormat::Date14)
        listFormat->setCurrentRow(15);
    else if (cellFormatType == KCFormat::Date15)
        listFormat->setCurrentRow(16);
    else if (cellFormatType == KCFormat::Date16)
        listFormat->setCurrentRow(17);
    else if (cellFormatType == KCFormat::Date17)
        listFormat->setCurrentRow(18);
    else if (cellFormatType == KCFormat::Date18)
        listFormat->setCurrentRow(19);
    else if (cellFormatType == KCFormat::Date19)
        listFormat->setCurrentRow(20);
    else if (cellFormatType == KCFormat::Date20)
        listFormat->setCurrentRow(21);
    else if (cellFormatType == KCFormat::Date21)
        listFormat->setCurrentRow(22);
    else if (cellFormatType == KCFormat::Date22)
        listFormat->setCurrentRow(23);
    else if (cellFormatType == KCFormat::Date23)
        listFormat->setCurrentRow(24);
    else if (cellFormatType == KCFormat::Date24)
        listFormat->setCurrentRow(25);
    else if (cellFormatType == KCFormat::Date25)
        listFormat->setCurrentRow(26);
    else if (cellFormatType == KCFormat::Date26)
        listFormat->setCurrentRow(27);
    else if (cellFormatType == KCFormat::Date27)
        listFormat->setCurrentRow(28);
    else if (cellFormatType == KCFormat::Date28)
        listFormat->setCurrentRow(29);
    else if (cellFormatType == KCFormat::Date29)
        listFormat->setCurrentRow(30);
    else if (cellFormatType == KCFormat::Date30)
        listFormat->setCurrentRow(31);
    else if (cellFormatType == KCFormat::Date31)
        listFormat->setCurrentRow(32);
    else if (cellFormatType == KCFormat::Date32)
        listFormat->setCurrentRow(33);
    else if (cellFormatType == KCFormat::Date33)
        listFormat->setCurrentRow(34);
    else if (cellFormatType == KCFormat::Date34)
        listFormat->setCurrentRow(35);
    else if (cellFormatType == KCFormat::Date35)
        listFormat->setCurrentRow(36);
    else
        listFormat->setCurrentRow(0);
}

void CellFormatPageFloat::datetimeInit()
{
    QStringList list;
    list += i18n("System: ") + dlg->locale()->formatDateTime(QDateTime::currentDateTime(), KLocale::ShortDate);
    list += i18n("System: ") + dlg->locale()->formatDateTime(QDateTime::currentDateTime(), KLocale::LongDate);
    listFormat->addItems(list);
}

void CellFormatPageFloat::currencyChanged(const QString &)
{
    int index = currency->currentIndex();
    if (index > 0)
        ++index;
    dlg->m_currency = KCCurrency(index);

    makeformat();
}

void CellFormatPageFloat::updateFormatType()
{
    if (generic->isChecked())
        newFormatType = KCFormat::Generic;
    else if (number->isChecked())
        newFormatType = KCFormat::KCNumber;
    else if (percent->isChecked())
        newFormatType = KCFormat::Percentage;
    else if (date->isChecked()) {
        newFormatType = KCFormat::ShortDate;
        switch (listFormat->currentRow()) {
        case 0: newFormatType = KCFormat::ShortDate; break;
        case 1: newFormatType = KCFormat::TextDate; break;
        case 2: newFormatType = KCFormat::Date1; break; /*18-Feb-99*/
        case 3: newFormatType = KCFormat::Date2; break; /*18-Feb-1999*/
        case 4: newFormatType = KCFormat::Date3; break; /*18-Feb*/
        case 5: newFormatType = KCFormat::Date4; break; /*18-05*/
        case 6: newFormatType = KCFormat::Date5; break; /*18/05/00*/
        case 7: newFormatType = KCFormat::Date6; break; /*18/05/1999*/
        case 8: newFormatType = KCFormat::Date7; break;/*Feb-99*/
        case 9: newFormatType = KCFormat::Date8; break; /*February-99*/
        case 10: newFormatType = KCFormat::Date9; break; /*February-1999*/
        case 11: newFormatType = KCFormat::Date10; break; /*F-99*/
        case 12: newFormatType = KCFormat::Date11; break; /*18/Feb*/
        case 13: newFormatType = KCFormat::Date12; break; /*18/02*/
        case 14: newFormatType = KCFormat::Date13; break; /*18/Feb/1999*/
        case 15: newFormatType = KCFormat::Date14; break; /*2000/Feb/18*/
        case 16: newFormatType = KCFormat::Date15; break;/*2000-Feb-18*/
        case 17: newFormatType = KCFormat::Date16; break;/*2000-02-18*/
        case 18: newFormatType = KCFormat::Date17; break; /*2000-02-18*/
        case 19: newFormatType = KCFormat::Date18; break;
        case 20: newFormatType = KCFormat::Date19; break;
        case 21: newFormatType = KCFormat::Date20; break;
        case 22: newFormatType = KCFormat::Date21; break;
        case 23: newFormatType = KCFormat::Date22; break;
        case 24: newFormatType = KCFormat::Date23; break;
        case 25: newFormatType = KCFormat::Date24; break;
        case 26: newFormatType = KCFormat::Date25; break;
        case 27: newFormatType = KCFormat::Date26; break;
        case 28: newFormatType = KCFormat::Date27; break;
        case 29: newFormatType = KCFormat::Date28; break;
        case 30: newFormatType = KCFormat::Date29; break;
        case 31: newFormatType = KCFormat::Date30; break;
        case 32: newFormatType = KCFormat::Date31; break;
        case 33: newFormatType = KCFormat::Date32; break;
        case 34: newFormatType = KCFormat::Date33; break;
        case 35: newFormatType = KCFormat::Date34; break;
        case 36: newFormatType = KCFormat::Date35; break;
        }
    } else if (money->isChecked())
        newFormatType = KCFormat::Money;
    else if (scientific->isChecked())
        newFormatType = KCFormat::Scientific;
    else if (fraction->isChecked()) {
        newFormatType = KCFormat::fraction_half;
        switch (listFormat->currentRow()) {
        case 0: newFormatType = KCFormat::fraction_half; break;
        case 1: newFormatType = KCFormat::fraction_quarter; break;
        case 2: newFormatType = KCFormat::fraction_eighth; break;
        case 3: newFormatType = KCFormat::fraction_sixteenth; break;
        case 4: newFormatType = KCFormat::fraction_tenth; break;
        case 5: newFormatType = KCFormat::fraction_hundredth; break;
        case 6: newFormatType = KCFormat::fraction_one_digit; break;
        case 7: newFormatType = KCFormat::fraction_two_digits; break;
        case 8: newFormatType = KCFormat::fraction_three_digits; break;
        }
    } else if (time->isChecked()) {
        newFormatType = KCFormat::Time;
        switch (listFormat->currentRow()) {
        case 0: newFormatType = KCFormat::Time; break;
        case 1: newFormatType = KCFormat::SecondeTime; break;
        case 2: newFormatType = KCFormat::Time1; break;
        case 3: newFormatType = KCFormat::Time2; break;
        case 4: newFormatType = KCFormat::Time3; break;
        case 5: newFormatType = KCFormat::Time4; break;
        case 6: newFormatType = KCFormat::Time5; break;
        case 7: newFormatType = KCFormat::Time6; break;
        case 8: newFormatType = KCFormat::Time7; break;
        case 9: newFormatType = KCFormat::Time8; break;
        }
    } else if (textFormat->isChecked())
        newFormatType = KCFormat::Text;
    else if (customFormat->isChecked())
        newFormatType = KCFormat::Custom;
}

void CellFormatPageFloat::makeformat()
{
    m_bFormatTypeChanged = true;
    QString tmp;

    updateFormatType();
    QColor color;
    KCStyle::FloatFormat floatFormat = KCStyle::OnlyNegSigned;
    switch (format->currentIndex()) {
    case 0:
        floatFormat = KCStyle::OnlyNegSigned;
        color = Qt::black;
        break;
    case 1:
        floatFormat =  KCStyle::OnlyNegSigned;
        color = Qt::red;
        break;
    case 2:
        floatFormat =  KCStyle::AlwaysUnsigned;
        color = Qt::red;
        break;
    case 3:
        floatFormat =  KCStyle::AlwaysSigned;
        color = Qt::black;
        break;
    case 4:
        floatFormat =  KCStyle::AlwaysSigned;
        color = Qt::red;
        break;
    }
    if (!dlg->value.isNumber() || dlg->value.asFloat() >= 0 || !format->isEnabled()) {
        color = Qt::black;
    }
    ValueFormatter *fmt = dlg->getSheet()->map()->formatter();
    tmp = fmt->formatText(dlg->value, newFormatType, precision->value(),
                          floatFormat,
                          prefix->isEnabled() ? prefix->text() : QString(),
                          postfix->isEnabled() ? postfix->text() : QString(),
                          newFormatType == KCFormat::Money ? dlg->m_currency.symbol() : QString()).asString();
    if (tmp.length() > 50)
        tmp = tmp.left(50);
    exampleLabel->setText(tmp.prepend("<font color=" + color.name() + '>').append("</font>"));
}

void CellFormatPageFloat::apply(CustomStyle * style)
{
    if (postfix->text() != dlg->postfix) {
        if (postfix->isEnabled())
            style->setPostfix(postfix->text());
        else
            style->setPostfix("");
    }
    if (prefix->text() != dlg->prefix) {
        if (prefix->isEnabled())
            style->setPrefix(prefix->text());
        else
            style->setPrefix("");
    }

    if (dlg->precision != precision->value())
        style->setPrecision(precision->value());

    if (m_bFormatColorChanged) {
        switch (format->currentIndex()) {
        case 0:
            style->setFloatFormat(KCStyle::OnlyNegSigned);
            style->setFloatColor(KCStyle::AllBlack);
            break;
        case 1:
            style->setFloatFormat(KCStyle::OnlyNegSigned);
            style->setFloatColor(KCStyle::NegRed);
            break;
        case 2:
            style->setFloatFormat(KCStyle::AlwaysUnsigned);
            style->setFloatColor(KCStyle::NegRed);
            break;
        case 3:
            style->setFloatFormat(KCStyle::AlwaysSigned);
            style->setFloatColor(KCStyle::AllBlack);
            break;
        case 4:
            style->setFloatFormat(KCStyle::AlwaysSigned);
            style->setFloatColor(KCStyle::NegRed);
            break;
        }
    }
    if (m_bFormatTypeChanged) {
        style->setFormatType(newFormatType);
        if (money->isChecked()) {
            KCCurrency currency;
            int index = this->currency->currentIndex();
            if (index == 0) {
                if (this->currency->currentText() == i18n("Automatic"))
                    currency = KCCurrency();
                else
                    currency = KCCurrency(this->currency->currentText());
            } else {
                currency = KCCurrency(++index);
            }
            style->setCurrency(currency);
        }
    }
}

void CellFormatPageFloat::apply(StyleCommand* _obj)
{
    if (postfix->text() != dlg->postfix)
        if (postfix->isEnabled()) {
            // If we are in here it *never* can be disabled - FIXME (Werner)!
            if (postfix->isEnabled())
                _obj->setPostfix(postfix->text());
            else
                _obj->setPostfix("");
        }
    if (prefix->text() != dlg->prefix) {
        if (prefix->isEnabled())
            _obj->setPrefix(prefix->text());
        else
            _obj->setPrefix("");
    }

    if (dlg->precision != precision->value())
        _obj->setPrecision(precision->value());

    if (m_bFormatColorChanged) {
        switch (format->currentIndex()) {
        case 0:
            _obj->setFloatFormat(KCStyle::OnlyNegSigned);
            _obj->setFloatColor(KCStyle::AllBlack);
            break;
        case 1:
            _obj->setFloatFormat(KCStyle::OnlyNegSigned);
            _obj->setFloatColor(KCStyle::NegRed);
            break;
        case 2:
            _obj->setFloatFormat(KCStyle::AlwaysUnsigned);
            _obj->setFloatColor(KCStyle::NegRed);
            break;
        case 3:
            _obj->setFloatFormat(KCStyle::AlwaysSigned);
            _obj->setFloatColor(KCStyle::AllBlack);
            break;
        case 4:
            _obj->setFloatFormat(KCStyle::AlwaysSigned);
            _obj->setFloatColor(KCStyle::NegRed);
            break;
        }
    }
    if (m_bFormatTypeChanged) {
        _obj->setFormatType(newFormatType);
        if (money->isChecked()) {
            KCCurrency currency;
            int index = this->currency->currentIndex();
            if (index == 0) {
                if (this->currency->currentText() == i18n("Automatic"))
                    currency = KCCurrency();
                else
                    currency = KCCurrency(this->currency->currentText());
            } else {
                currency = KCCurrency(++index);
            }
            _obj->setCurrency(currency);
        }
    }
}



/***************************************************************************
 *
 * CellFormatPageProtection
 *
 ***************************************************************************/

CellFormatPageProtection::CellFormatPageProtection(QWidget* parent, CellFormatDialog * _dlg)
        : QWidget(parent),
        m_dlg(_dlg)
{
    setupUi(this);
    connect(m_bHideAll, SIGNAL(toggled(bool)), m_bIsProtected, SLOT(setDisabled(bool)));
    connect(m_bHideAll, SIGNAL(toggled(bool)), m_bHideFormula, SLOT(setDisabled(bool)));

    m_bDontPrint->setChecked(m_dlg->bDontPrintText);
    m_bHideAll->setChecked(m_dlg->bHideAll);
    m_bHideFormula->setChecked(m_dlg->bHideFormula);
    m_bIsProtected->setChecked(m_dlg->bIsProtected);
}

CellFormatPageProtection::~CellFormatPageProtection()
{
}

void CellFormatPageProtection::apply(CustomStyle * style)
{
    if (m_dlg->bDontPrintText != m_bDontPrint->isChecked()) {
        style->setDontPrintText(m_bDontPrint->isChecked());
    }

    if (m_dlg->bIsProtected != m_bIsProtected->isChecked()) {
        style->setNotProtected(!m_bIsProtected->isChecked());
    }

    if (m_dlg->bHideAll != m_bHideAll->isChecked()) {
        style->setHideAll(m_bHideAll->isChecked());
    }

    if (m_dlg->bHideFormula != m_bHideFormula->isChecked()) {
        style->setHideFormula(m_bHideFormula->isChecked());
    }
}

void CellFormatPageProtection::apply(StyleCommand* _obj)
{
    if (m_dlg->bDontPrintText != m_bDontPrint->isChecked())
        _obj->setDontPrintText(m_bDontPrint->isChecked());

    if (m_dlg->bIsProtected != m_bIsProtected->isChecked())
        _obj->setNotProtected(!m_bIsProtected->isChecked());

    if (m_dlg->bHideAll != m_bHideAll->isChecked())
        _obj->setHideAll(m_bHideAll->isChecked());

    if (m_dlg->bHideFormula != m_bHideFormula->isChecked())
        _obj->setHideFormula(m_bHideFormula->isChecked());
}



/***************************************************************************
 *
 * CellFormatPageFont
 *
 ***************************************************************************/

CellFormatPageFont::CellFormatPageFont(QWidget* parent, CellFormatDialog *_dlg)
        : QWidget(parent)
{
    setupUi(this);

    dlg = _dlg;

    bTextColorUndefined = !dlg->bTextColor;

    connect(textColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotSetTextColor(const QColor &)));


    QStringList tmpListFont;
    QFontDatabase *fontDataBase = new QFontDatabase();
    tmpListFont = fontDataBase->families();
    delete fontDataBase;

    family_combo->addItems(tmpListFont);
    selFont = dlg->font;

    if (dlg->bTextFontFamily) {
        selFont.setFamily(dlg->fontFamily);
        // kDebug(36001) <<"Family =" << dlg->fontFamily;

        if (family_combo->findItems(dlg->fontFamily, Qt::MatchExactly).size() == 0) {
            family_combo->insertItem(0, "");
            family_combo->setCurrentRow(0);
        } else
            family_combo->setCurrentItem(family_combo->findItems(dlg->fontFamily, Qt::MatchExactly)[0]);
    } else {
        family_combo->insertItem(0, "");
        family_combo->setCurrentRow(0);
    }

    connect(family_combo, SIGNAL(currentTextChanged(const QString &)),
            SLOT(family_chosen_slot(const QString &)));

    QStringList lst;
    lst.append("");
    for (unsigned int i = 1; i < 100; ++i)
        lst.append(QString("%1").arg(i));

    size_combo->insertItems(0, lst);


    size_combo->setInsertPolicy(KComboBox::NoInsert);

    connect(size_combo, SIGNAL(activated(const QString &)),
            SLOT(size_chosen_slot(const QString &)));
    connect(size_combo , SIGNAL(textChanged(const QString &)),
            this, SLOT(size_chosen_slot(const QString &)));

    connect(weight_combo, SIGNAL(activated(const QString &)),
            SLOT(weight_chosen_slot(const QString &)));

    connect(style_combo, SIGNAL(activated(const QString &)),
            SLOT(style_chosen_slot(const QString &)));

    strike->setChecked(dlg->strike);
    connect(strike, SIGNAL(clicked()),
            SLOT(strike_chosen_slot()));

    underline->setChecked(dlg->underline);
    connect(underline, SIGNAL(clicked()),
            SLOT(underline_chosen_slot()));

    example_label->setText(i18n("Dolor Ipse"));

    connect(this, SIGNAL(fontSelected(const QFont&)),
            this, SLOT(display_example(const QFont&)));

    setCombos();
    display_example(selFont);
    fontChanged = false;
    this->resize(400, 400);
}

void CellFormatPageFont::slotSetTextColor(const QColor &_color)
{
    textColor = _color;
    bTextColorUndefined = false;
}

void CellFormatPageFont::apply(CustomStyle * style)
{
    if (!bTextColorUndefined && textColor != dlg->textColor)
        style->setFontColor(textColor);

    if ((size_combo->currentIndex() != 0)
            && (dlg->fontSize != selFont.pointSize()))
        style->setFontSize(selFont.pointSize());

    if ((selFont.family() != dlg->fontFamily)
            && family_combo->currentItem() != 0 && !family_combo->currentItem()->text().isEmpty())
        style->setFontFamily(selFont.family());

    style->setFontBold(weight_combo->currentIndex() != 0 && selFont.bold());
    style->setFontItalic(style_combo->currentIndex() != 0 && selFont.italic());
    style->setFontStrikeOut(strike->isChecked());
    style->setFontUnderline(underline->isChecked());
}

void CellFormatPageFont::apply(StyleCommand* _obj)
{
    if (!bTextColorUndefined && textColor != dlg->textColor)
        _obj->setFontColor(textColor);
    if (fontChanged) {
        if ((size_combo->currentIndex() != 0)
                && (dlg->fontSize != selFont.pointSize()))
            _obj->setFontSize(selFont.pointSize());
        if ((selFont.family() != dlg->fontFamily) && (family_combo->currentItem() != 0 && !family_combo->currentItem()->text().isEmpty()))
            _obj->setFontFamily(selFont.family());
        if (weight_combo->currentIndex() != 0)
            _obj->setFontBold(selFont.bold());
        if (style_combo->currentIndex() != 0)
            _obj->setFontItalic(selFont.italic());
        _obj->setFontStrike(strike->isChecked());
        _obj->setFontUnderline(underline->isChecked());
    }
}

void CellFormatPageFont::underline_chosen_slot()
{
    selFont.setUnderline(underline->isChecked());
    emit fontSelected(selFont);
}

void CellFormatPageFont::strike_chosen_slot()
{
    selFont.setStrikeOut(strike->isChecked());
    emit fontSelected(selFont);
}

void CellFormatPageFont::family_chosen_slot(const QString & family)
{
    selFont.setFamily(family);
    emit fontSelected(selFont);
}

void CellFormatPageFont::size_chosen_slot(const QString & size)
{
    QString size_string = size;

    if (size_string.toInt() > 0) selFont.setPointSize(size_string.toInt());
    emit fontSelected(selFont);
}

void CellFormatPageFont::weight_chosen_slot(const QString & weight)
{
    QString weight_string = weight;

    if (weight_string == i18n("Normal"))
        selFont.setBold(false);
    if (weight_string == i18n("Bold"))
        selFont.setBold(true);
    emit fontSelected(selFont);
}

void CellFormatPageFont::style_chosen_slot(const QString & style)
{
    QString style_string = style;

    if (style_string == i18n("Roman"))
        selFont.setItalic(false);
    if (style_string == i18n("Italic"))
        selFont.setItalic(true);
    emit fontSelected(selFont);
}


void CellFormatPageFont::display_example(const QFont& font)
{
    QString string;
    fontChanged = true;
    example_label->setFont(font);
    example_label->repaint();
}

void CellFormatPageFont::setCombos()
{
    QString string;
    KComboBox* combo;
    int number_of_entries;
    bool found;

    if (dlg->bTextColor)
        textColor = dlg->textColor;
    else
        textColor = palette().text().color();

    if (!textColor.isValid())
        textColor = palette().text().color();

    textColorButton->setColor(textColor);


    combo = size_combo;
    if (dlg->bTextFontSize) {
//      kDebug(36001) <<"SIZE=" << dlg->fontSize;
        selFont.setPointSize(dlg->fontSize);
        number_of_entries = size_combo->count();
        string.setNum(dlg->fontSize);
        found = false;

        for (int i = 0; i < number_of_entries ; i++) {
            if (string == (QString) combo->itemText(i)) {
                combo->setCurrentIndex(i);
                found = true;
                // kDebug(36001) <<"Found Size" << string.data() <<" setting to item" i;
                break;
            }
        }
    } else
        combo->setCurrentIndex(0);

    if (!dlg->bTextFontBold)
        weight_combo->setCurrentIndex(0);
    else if (dlg->fontBold) {
        selFont.setBold(dlg->fontBold);
        weight_combo->setCurrentIndex(2);
    } else {
        selFont.setBold(dlg->fontBold);
        weight_combo->setCurrentIndex(1);
    }

    if (!dlg->bTextFontItalic)
        weight_combo->setCurrentIndex(0);
    else if (dlg->fontItalic) {
        selFont.setItalic(dlg->fontItalic);
        style_combo->setCurrentIndex(2);
    } else {
        selFont.setItalic(dlg->fontItalic);
        style_combo->setCurrentIndex(1);
    }
}



/***************************************************************************
 *
 * CellFormatPagePosition
 *
 ***************************************************************************/

CellFormatPagePosition::CellFormatPagePosition(QWidget* parent, CellFormatDialog *_dlg)
        : QWidget(parent),
        dlg(_dlg)
{
    setupUi(this);
    connect(angleRotation, SIGNAL(valueChanged(int)), spinBox3, SLOT(setValue(int)));
    connect(spinBox3, SIGNAL(valueChanged(int)), angleRotation, SLOT(setValue(int)));

    if (dlg->alignX == KCStyle::Left)
        left->setChecked(true);
    else if (dlg->alignX == KCStyle::Center)
        center->setChecked(true);
    else if (dlg->alignX == KCStyle::Right)
        right->setChecked(true);
    else if (dlg->alignX == KCStyle::HAlignUndefined)
        standard->setChecked(true);

    QButtonGroup* horizontalGroup = new QButtonGroup(this);
    horizontalGroup->addButton(left);
    horizontalGroup->addButton(center);
    horizontalGroup->addButton(right);
    horizontalGroup->addButton(standard);
    connect(horizontalGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotStateChanged(int)));

    if (dlg->alignY == KCStyle::Top)
        top->setChecked(true);
    else if (dlg->alignY == KCStyle::Middle)
        middle->setChecked(true);
    else if (dlg->alignY == KCStyle::Bottom)
        bottom->setChecked(true);

    multi->setChecked(dlg->bMultiRow);

    vertical->setChecked(dlg->bVerticalText);
    
    shrinkToFit->setChecked(dlg->bShrinkToFit);

    angleRotation->setValue(-dlg->textRotation);//annma
    spinBox3->setValue(-dlg->textRotation);
    if (dlg->textRotation != 0) {
        multi->setEnabled(false);
        vertical->setEnabled(false);
        shrinkToFit->setEnabled(false);
    }

    mergeCell->setChecked(dlg->isMerged);
    mergeCell->setEnabled(!dlg->oneCell && ((!dlg->isRowSelected) && (!dlg->isColumnSelected)));

    QGridLayout *grid2 = new QGridLayout(indentGroup);
    grid2->setMargin(KDialog::marginHint());
    grid2->setSpacing(KDialog::spacingHint());
    grid2->addItem(new QSpacerItem(0, indentGroup->fontMetrics().height() / 8), 0, 0);  // groupbox title
    m_indent = new KoUnitDoubleSpinBox(indentGroup);
    m_indent->setMinimum(0.0);
    m_indent->setMaximum(400.0);
    m_indent->setLineStepPt(10.0);
    m_indent->setUnit(dlg->selection()->canvas()->unit());
    m_indent->changeValue(dlg->indent);
    grid2->addWidget(m_indent, 0, 0);

    width = new KoUnitDoubleSpinBox(m_widthPanel);
    QGridLayout *gridWidth = new QGridLayout(m_widthPanel);
    gridWidth->setMargin(0);
    gridWidth->setSpacing(0);
    gridWidth->addWidget(width, 0, 0);
    width->setValue(dlg->widthSize);
    width->setUnit(dlg->selection()->canvas()->unit());
    //to ensure, that we don't get rounding problems, we store the displayed value (for later check for changes)
    dlg->widthSize = width->value();

    if (dlg->isRowSelected)
        width->setEnabled(false);

    double dw = dlg->selection()->canvas()->unit().toUserValue(dlg->defaultWidthSize);
    defaultWidth->setText(i18n("Default width (%1 %2)", dw, KoUnit::unitName(dlg->selection()->canvas()->unit())));
    if (dlg->isRowSelected)
        defaultWidth->setEnabled(false);

    height = new KoUnitDoubleSpinBox(m_heightPanel);
    QGridLayout *gridHeight = new QGridLayout(m_heightPanel);
    gridHeight->setMargin(0);
    gridHeight->setSpacing(0);
    gridHeight->addWidget(height, 0, 0);
    height->setValue(dlg->heightSize);
    height->setUnit(dlg->selection()->canvas()->unit());
    //to ensure, that we don't get rounding problems, we store the displayed value (for later check for changes)
    dlg->heightSize = height->value();

    if (dlg->isColumnSelected)
        height->setEnabled(false);

    double dh =  dlg->selection()->canvas()->unit().toUserValue(dlg->defaultHeightSize);
    defaultHeight->setText(i18n("Default height (%1 %2)", dh, KoUnit::unitName(dlg->selection()->canvas()->unit())));
    if (dlg->isColumnSelected)
        defaultHeight->setEnabled(false);

    // in case we're editing a style, we disable the cell size settings
    if (dlg->getStyle()) {
        defaultHeight->setEnabled(false);
        defaultWidth->setEnabled(false);
    }

    connect(defaultWidth , SIGNAL(clicked()), this, SLOT(slotChangeWidthState()));
    connect(defaultHeight , SIGNAL(clicked()), this, SLOT(slotChangeHeightState()));
    connect(vertical , SIGNAL(clicked()), this, SLOT(slotChangeVerticalState()));
    connect(shrinkToFit, SIGNAL(clicked()), this, SLOT(slotChangeShrinkToFitState()));
    connect(multi , SIGNAL(clicked()), this, SLOT(slotChangeMultiState()));
    connect(angleRotation, SIGNAL(valueChanged(int)), this, SLOT(slotChangeAngle(int)));

    slotStateChanged(0);
    m_bOptionText = false;
    this->resize(400, 400);
}

void CellFormatPagePosition::slotChangeMultiState()
{
    m_bOptionText = true;
    if (vertical->isChecked()) {
        vertical->setChecked(false);
    }
}

void CellFormatPagePosition::slotChangeVerticalState()
{
    m_bOptionText = true;
    if (multi->isChecked()) {
        multi->setChecked(false);
    }
    if (shrinkToFit->isChecked()) {
        shrinkToFit->setChecked(false);
    }
}


void CellFormatPagePosition::slotChangeShrinkToFitState()
{
    m_bOptionText = true;
    if (vertical->isChecked()) {
        vertical->setChecked(false);
    }
    if (multi->isChecked()) {
        multi->setChecked(false);
    }
}

void CellFormatPagePosition::slotStateChanged(int)
{
    if (right->isChecked() || center->isChecked())
        m_indent->setEnabled(false);
    else
        m_indent->setEnabled(true);
}

bool CellFormatPagePosition::getMergedCellState() const
{
    return  mergeCell->isChecked();
}

void CellFormatPagePosition::slotChangeWidthState()
{
    if (defaultWidth->isChecked())
        width->setEnabled(false);
    else
        width->setEnabled(true);
}

void CellFormatPagePosition::slotChangeHeightState()
{
    if (defaultHeight->isChecked())
        height->setEnabled(false);
    else
        height->setEnabled(true);
}

void CellFormatPagePosition::slotChangeAngle(int _angle)
{
    if (_angle == 0) {
        multi->setEnabled(true);
        vertical->setEnabled(true);
    } else {
        multi->setEnabled(false);
        vertical->setEnabled(false);
    }
}

void CellFormatPagePosition::apply(CustomStyle * style)
{
    if (top->isChecked() && dlg->alignY != KCStyle::Top)
        style->setVAlign(KCStyle::Top);
    else if (bottom->isChecked() && dlg->alignY != KCStyle::Bottom)
        style->setVAlign(KCStyle::Bottom);
    else if (middle->isChecked() && dlg->alignY != KCStyle::Middle)
        style->setVAlign(KCStyle::Middle);

    if (left->isChecked() && dlg->alignX != KCStyle::Left)
        style->setHAlign(KCStyle::Left);
    else if (right->isChecked() && dlg->alignX != KCStyle::Right)
        style->setHAlign(KCStyle::Right);
    else if (center->isChecked() && dlg->alignX != KCStyle::Center)
        style->setHAlign(KCStyle::Center);
    else if (standard->isChecked() && dlg->alignX != KCStyle::HAlignUndefined)
        style->setHAlign(KCStyle::HAlignUndefined);

    if (m_bOptionText) {
        if (multi->isEnabled()) {
            style->setWrapText(multi->isChecked());
        }
        if (vertical->isEnabled()) {
            style->setVerticalText(vertical->isChecked());
        }
        if (shrinkToFit->isEnabled()) {
            style->setShrinkToFit(shrinkToFit->isChecked());
        }
    }

    if (dlg->textRotation != angleRotation->value())
        style->setAngle((-angleRotation->value()));

    if (m_indent->isEnabled()
            && dlg->indent != m_indent->value())
        style->setIndentation(m_indent->value());

    // setting the default column width and row height
    if (dlg->getStyle()->type() == KCStyle::BUILTIN && dlg->getStyle()->name() == "Default") {
        if ((int) height->value() != (int) dlg->heightSize) {
            dlg->getSheet()->map()->setDefaultRowHeight(height->value());
        }
        if ((int) width->value() != (int) dlg->widthSize) {
            dlg->getSheet()->map()->setDefaultColumnWidth(width->value());
        }
    }
}

void CellFormatPagePosition::apply(StyleCommand* _obj)
{
    KCStyle::HAlign  ax;
    KCStyle::VAlign ay;

    if (top->isChecked())
        ay = KCStyle::Top;
    else if (bottom->isChecked())
        ay = KCStyle::Bottom;
    else if (middle->isChecked())
        ay = KCStyle::Middle;
    else
        ay = KCStyle::Middle; // Default, just in case

    if (left->isChecked())
        ax = KCStyle::Left;
    else if (right->isChecked())
        ax = KCStyle::Right;
    else if (center->isChecked())
        ax = KCStyle::Center;
    else if (standard->isChecked())
        ax = KCStyle::HAlignUndefined;
    else
        ax = KCStyle::HAlignUndefined; //Default, just in case

    if (top->isChecked() && ay != dlg->alignY)
        _obj->setVerticalAlignment(KCStyle::Top);
    else if (bottom->isChecked() && ay != dlg->alignY)
        _obj->setVerticalAlignment(KCStyle::Bottom);
    else if (middle->isChecked() && ay != dlg->alignY)
        _obj->setVerticalAlignment(KCStyle::Middle);

    if (left->isChecked() && ax != dlg->alignX)
        _obj->setHorizontalAlignment(KCStyle::Left);
    else if (right->isChecked() && ax != dlg->alignX)
        _obj->setHorizontalAlignment(KCStyle::Right);
    else if (center->isChecked() && ax != dlg->alignX)
        _obj->setHorizontalAlignment(KCStyle::Center);
    else if (standard->isChecked() && ax != dlg->alignX)
        _obj->setHorizontalAlignment(KCStyle::HAlignUndefined);

    if (m_bOptionText) {
        if (multi->isEnabled())
            _obj->setMultiRow(multi->isChecked());
        else
            _obj->setMultiRow(false);
    }

    if (m_bOptionText) {
        if (vertical->isEnabled())
            _obj->setVerticalText(vertical->isChecked());
        else
            _obj->setVerticalText(false);
    }

    if (m_bOptionText) {
        if (shrinkToFit->isEnabled())
            _obj->setShrinkToFit(shrinkToFit->isChecked());
        else
            _obj->setShrinkToFit(false);
    }

    if (dlg->textRotation != angleRotation->value())
        _obj->setAngle((-angleRotation->value()));
    if (m_indent->isEnabled()
            && dlg->indent != m_indent->value())
        _obj->setIndentation(m_indent->value());
}

double CellFormatPagePosition::getSizeHeight() const
{
    if (defaultHeight->isChecked())
        return dlg->defaultHeightSize; // guess who calls this!
    else
        return height->value();
}

double CellFormatPagePosition::getSizeWidth() const
{
    if (defaultWidth->isChecked())
        return dlg->defaultWidthSize; // guess who calls this!
    else
        return width->value();
}



/***************************************************************************
 *
 * BorderButton
 *
 ***************************************************************************/

BorderButton::BorderButton(QWidget *parent, const char * /*_name*/) : QPushButton(parent)
{
    penStyle = Qt::NoPen;
    penWidth = 1;
    penColor = palette().text().color();
    setCheckable(true);
    setChecked(false);
    setChanged(false);
}
void BorderButton::mousePressEvent(QMouseEvent *)
{

    this->setChecked(!isChecked());
    emit clicked(this);
}

void BorderButton::setUndefined()
{
    setPenStyle(Qt::SolidLine);
    setPenWidth(1);
    setColor(palette().midlight().color());
}


void BorderButton::unselect()
{
    setChecked(false);
    setPenWidth(1);
    setPenStyle(Qt::NoPen);
    setColor(palette().text().color());
    setChanged(true);
}



/***************************************************************************
 *
 * Border
 *
 ***************************************************************************/

Border::Border(QWidget *parent, const char * /*_name*/, bool _oneCol, bool _oneRow)
        : QFrame(parent)
{
    setAutoFillBackground(true);

    oneCol = _oneCol;
    oneRow = _oneRow;
}


#define OFFSETX 5
#define OFFSETY 5
void Border::paintEvent(QPaintEvent *_ev)
{
    QFrame::paintEvent(_ev);
    QPen pen;
    QPainter painter;
    painter.begin(this);
    pen = QPen(palette().midlight(), 2, Qt::SolidLine).color();
    painter.setPen(pen);

    painter.drawLine(OFFSETX - 5, OFFSETY, OFFSETX , OFFSETY);
    painter.drawLine(OFFSETX, OFFSETY - 5, OFFSETX , OFFSETY);
    painter.drawLine(width() - OFFSETX, OFFSETY, width() , OFFSETY);
    painter.drawLine(width() - OFFSETX, OFFSETY - 5, width() - OFFSETX , OFFSETY);

    painter.drawLine(OFFSETX, height() - OFFSETY, OFFSETX , height());
    painter.drawLine(OFFSETX - 5, height() - OFFSETY, OFFSETX , height() - OFFSETY);

    painter.drawLine(width() - OFFSETX, height() - OFFSETY, width() , height() - OFFSETY);
    painter.drawLine(width() - OFFSETX, height() - OFFSETY, width() - OFFSETX , height());
    if (oneCol == false) {
        painter.drawLine(width() / 2, OFFSETY - 5, width() / 2 , OFFSETY);
        painter.drawLine(width() / 2 - 5, OFFSETY, width() / 2 + 5 , OFFSETY);
        painter.drawLine(width() / 2, height() - OFFSETY, width() / 2 , height());
        painter.drawLine(width() / 2 - 5, height() - OFFSETY, width() / 2 + 5 , height() - OFFSETY);
    }
    if (oneRow == false) {
        painter.drawLine(OFFSETX - 5, height() / 2, OFFSETX , height() / 2);
        painter.drawLine(OFFSETX, height() / 2 - 5, OFFSETX , height() / 2 + 5);
        painter.drawLine(width() - OFFSETX, height() / 2, width(), height() / 2);
        painter.drawLine(width() - OFFSETX, height() / 2 - 5, width() - OFFSETX , height() / 2 + 5);
    }
    painter.end();
    emit redraw();
}

void Border::mousePressEvent(QMouseEvent* _ev)
{
    emit choosearea(_ev);
}



/***************************************************************************
 *
 * CellFormatPageBorder
 *
 ***************************************************************************/

CellFormatPageBorder::CellFormatPageBorder(QWidget* parent, CellFormatDialog *_dlg)
        : QWidget(parent),
        dlg(_dlg)
{
    sheet = dlg->getSheet();

    InitializeGrids();
    InitializeBorderButtons();
    InitializePatterns();
    SetConnections();

    preview->slotSelect();
    pattern[2]->slotSelect();

    style->setEnabled(false);
    size->setEnabled(false);
    preview->setPattern(Qt::black , 1, Qt::SolidLine);
    this->resize(400, 400);
}

void CellFormatPageBorder::InitializeGrids()
{
    QGridLayout *grid = new QGridLayout(this);
    grid->setMargin(KDialog::marginHint());
    grid->setSpacing(KDialog::spacingHint());
    QGridLayout *grid2 = 0;
    QGroupBox* tmpQGroupBox = 0;

    /***********************/
    /* here is the data to initialize all the border buttons with */
    const char borderButtonNames[BorderType_END][20] = {"top", "bottom", "left", "right", "vertical", "fall", "go", "horizontal"};

    const char shortcutButtonNames[BorderShortcutType_END][20] = {"remove", "all", "outline"};

    QString borderButtonIconNames[BorderType_END] = {"border_top", "border_bottom", "border_left", "border_right",
            "border_vertical", "border_horizontal", "border_fall", "border_up"
                                                    };

    QString shortcutButtonIconNames[BorderShortcutType_END] = { "border_remove", "", "border_outline"};

    int borderButtonPositions[BorderType_END][2] = {{0, 2}, {4, 2}, {2, 0}, {2, 4}, {4, 4}, {4, 0}, {0, 0}, {0, 4}};

    int shortcutButtonPositions[BorderShortcutType_END][2] = { {0, 0}, {0, 1}, {0, 2} };
    /***********************/

    /* set up a layout box for most of the border setting buttons */
    tmpQGroupBox = new QGroupBox(this);
    tmpQGroupBox->setTitle(i18n("Border"));
    tmpQGroupBox->setAlignment(Qt::AlignLeft);
    grid2 = new QGridLayout(tmpQGroupBox);
    grid2->setMargin(KDialog::marginHint());
    grid2->setSpacing(KDialog::spacingHint());
    int fHeight = tmpQGroupBox->fontMetrics().height();
    grid2->addItem(new QSpacerItem(0, fHeight / 2), 0, 0);  // groupbox title

    area = new Border(tmpQGroupBox, "area", dlg->oneCol, dlg->oneRow);
    grid2->addWidget(area, 2, 1, 3, 3);
    QPalette palette = area->palette();
    palette.setColor(area->backgroundRole(), this->palette().base().color());
    area->setPalette(palette);

    /* initailize the buttons that are in this box */
    for (int i = BorderType_Top; i < BorderType_END; i++) {
        borderButtons[i] = new BorderButton(tmpQGroupBox,
                                            borderButtonNames[i]);
        loadIcon(borderButtonIconNames[i], borderButtons[i]);
        grid2->addWidget(borderButtons[i], borderButtonPositions[i][0] + 1,
                         borderButtonPositions[i][1]);
    }

    grid->addWidget(tmpQGroupBox, 0, 0, 3, 1);

    /* the remove, all, and outline border buttons are in a second box down
       below.*/

    tmpQGroupBox = new QGroupBox(this);
    tmpQGroupBox->setTitle(i18n("Preselect"));
    tmpQGroupBox->setAlignment(Qt::AlignLeft);

    grid2 = new QGridLayout(tmpQGroupBox);
    grid2->setMargin(KDialog::marginHint());
    grid2->setSpacing(KDialog::spacingHint());

    /* the "all" button is different depending on what kind of region is currently
       selected */
    if ((dlg->oneRow == true) && (dlg->oneCol == false)) {
        shortcutButtonIconNames[BorderShortcutType_All] = "border_vertical";
    } else if ((dlg->oneRow == false) && (dlg->oneCol == true)) {
        shortcutButtonIconNames[BorderShortcutType_All] = "border_horizontal";
    } else {
        shortcutButtonIconNames[BorderShortcutType_All] = "border_inside";
    }

    for (int i = BorderShortcutType_Remove; i < BorderShortcutType_END; i++) {
        shortcutButtons[i] = new BorderButton(tmpQGroupBox,
                                              shortcutButtonNames[i]);
        loadIcon(shortcutButtonIconNames[i], shortcutButtons[i]);
        grid2->addWidget(shortcutButtons[i], shortcutButtonPositions[i][0],
                         shortcutButtonPositions[i][1]);
    }

    if (dlg->oneRow && dlg->oneCol) {
        shortcutButtons[BorderShortcutType_All]->setEnabled(false);
    }

    grid->addWidget(tmpQGroupBox, 3, 0, 2, 1);

    /* now set up the group box with the pattern selector */
    tmpQGroupBox = new QGroupBox(this);
    tmpQGroupBox->setTitle(i18n("Pattern"));
    tmpQGroupBox->setAlignment(Qt::AlignLeft);

    grid2 = new QGridLayout(tmpQGroupBox);
    grid2->setMargin(KDialog::marginHint());
    grid2->setSpacing(KDialog::spacingHint());
    fHeight = tmpQGroupBox->fontMetrics().height();
    grid2->addItem(new QSpacerItem(0, fHeight / 2), 0, 0);  // groupbox title

    char name[] = "PatternXX";
    Q_ASSERT(NUM_BORDER_PATTERNS < 100);

    for (int i = 0; i < NUM_BORDER_PATTERNS; i++) {
        name[7] = '0' + (i + 1) / 10;
        name[8] = '0' + (i + 1) % 10;
        pattern[i] = new PatternSelect(tmpQGroupBox, name);
        pattern[i]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        grid2->addWidget(pattern[i], i % 5, i / 5);
        /* this puts them in the pattern:
           1  6
           2  7
           3  8
           4  9
           5  10
        */
    }

    color = new KColorButton(tmpQGroupBox);
    grid2->addWidget(color, 8, 1);

    QLabel *tmpQLabel = new QLabel(tmpQGroupBox);
    tmpQLabel->setText(i18n("Color:"));
    grid2->addWidget(tmpQLabel, 8, 0);

    /* tack on the 'customize' border pattern selector */
    customize  = new QCheckBox(i18n("Customize"), tmpQGroupBox);
    grid2->addWidget(customize, 6, 0);
    connect(customize, SIGNAL(clicked()), SLOT(cutomize_chosen_slot()));

    size = new KComboBox(tmpQGroupBox);
    size->setEditable(true);
    grid2->addWidget(size, 7, 1);
    size->setValidator(new KIntValidator(size));
    QString tmp;
    for (int i = 0; i < 10; i++) {
        tmp = tmp.setNum(i);
        size->insertItem(i, tmp);
    }
    size->setCurrentIndex(1);

    style = new KComboBox(tmpQGroupBox);
    grid2->addWidget(style, 7, 0);
    style->setIconSize(QSize(100, 14));
    style->insertItem(0, paintFormatPixmap(Qt::DotLine), "");
    style->insertItem(1, paintFormatPixmap(Qt::DashLine), "");
    style->insertItem(2, paintFormatPixmap(Qt::DashDotLine), "");
    style->insertItem(3, paintFormatPixmap(Qt::DashDotDotLine), "");
    style->insertItem(4, paintFormatPixmap(Qt::SolidLine), "");
    palette = style->palette();
    palette.setColor(style->backgroundRole(), this->palette().window().color());
    style->setPalette(palette);

    grid->addWidget(tmpQGroupBox, 0, 1, 4, 1);

    /* Now the preview box is put together */
    tmpQGroupBox = new QGroupBox(this);
    tmpQGroupBox->setTitle(i18n("Preview"));
    tmpQGroupBox->setAlignment(Qt::AlignLeft);

    grid2 = new QGridLayout(tmpQGroupBox);
    grid2->setMargin(KDialog::marginHint());
    grid2->setSpacing(KDialog::spacingHint());
    fHeight = tmpQGroupBox->fontMetrics().height();
    grid2->addItem(new QSpacerItem(0, fHeight / 2), 0, 0);  // groupbox title

    preview = new PatternSelect(tmpQGroupBox, "Pattern_preview");
    preview->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(preview, 1, 0);

    grid->addWidget(tmpQGroupBox, 4, 1);
}

void CellFormatPageBorder::InitializeBorderButtons()
{
    for (int i = BorderType_Top; i < BorderType_END; i++) {
        if (dlg->borders[i].style != Qt::NoPen ||
                !dlg->borders[i].bStyle) {
            /* the horozontil and vertical buttons might be disabled depending on what
               kind of area is selected so check that first. */
            if ((dlg->oneRow == true && i == BorderType_Horizontal) ||
                    (dlg->oneCol == true && i == BorderType_Vertical)) {
                borderButtons[i]->setEnabled(false);
            } else if (dlg->borders[i].bColor && dlg->borders[i].bStyle) {
                borderButtons[i]->setPenStyle(dlg->borders[i].style);
                borderButtons[i]->setPenWidth(dlg->borders[i].width);
                borderButtons[i]->setColor(dlg->borders[i].color);
                borderButtons[i]->setChecked(true);
            } else {
                borderButtons[i]->setUndefined();
            }
        }
    }


}

void CellFormatPageBorder::InitializePatterns()
{
    pattern[0]->setPattern(Qt::black, 1, Qt::DotLine);
    pattern[1]->setPattern(Qt::black, 1, Qt::DashLine);
    pattern[2]->setPattern(Qt::black, 1, Qt::SolidLine);
    pattern[3]->setPattern(Qt::black, 1, Qt::DashDotLine);
    pattern[4]->setPattern(Qt::black, 1, Qt::DashDotDotLine);
    pattern[5]->setPattern(Qt::black, 2, Qt::SolidLine);
    pattern[6]->setPattern(Qt::black, 3, Qt::SolidLine);
    pattern[7]->setPattern(Qt::black, 4, Qt::SolidLine);
    pattern[8]->setPattern(Qt::black, 5, Qt::SolidLine);
    pattern[9]->setPattern(Qt::black, 1, Qt::NoPen);

    slotSetColorButton(Qt::black);
}

void CellFormatPageBorder::SetConnections()
{
    connect(color, SIGNAL(changed(const QColor &)),
            this, SLOT(slotSetColorButton(const QColor &)));

    for (int i = 0; i < NUM_BORDER_PATTERNS; i++) {
        connect(pattern[i], SIGNAL(clicked(PatternSelect*)),
                this, SLOT(slotUnselect2(PatternSelect*)));
    }

    for (int i = BorderType_Top; i < BorderType_END; i++) {
        connect(borderButtons[i], SIGNAL(clicked(BorderButton *)),
                this, SLOT(changeState(BorderButton *)));
    }

    for (int i = BorderShortcutType_Remove; i < BorderShortcutType_END; i++) {
        connect(shortcutButtons[i], SIGNAL(clicked(BorderButton *)),
                this, SLOT(preselect(BorderButton *)));
    }

    connect(area , SIGNAL(redraw()), this, SLOT(draw()));
    connect(area , SIGNAL(choosearea(QMouseEvent *)),
            this, SLOT(slotPressEvent(QMouseEvent *)));

    connect(style, SIGNAL(activated(int)), this, SLOT(slotChangeStyle(int)));
    connect(size, SIGNAL(textChanged(const QString &)),
            this, SLOT(slotChangeStyle(const QString &)));
    connect(size , SIGNAL(activated(int)), this, SLOT(slotChangeStyle(int)));
}

void CellFormatPageBorder::cutomize_chosen_slot()
{
    if (customize->isChecked()) {
        style->setEnabled(true);
        size->setEnabled(true);
        slotUnselect2(preview);
    } else {
        style->setEnabled(false);
        size->setEnabled(false);
        pattern[2]->slotSelect();
        preview->setPattern(Qt::black , 1, Qt::SolidLine);
    }
}

void CellFormatPageBorder::slotChangeStyle(const QString &)
{
    /* if they try putting text in the size box, then erase the line */
    slotChangeStyle(0);
}

void CellFormatPageBorder::slotChangeStyle(int)
{
    int index = style->currentIndex();
    QString tmp;
    int penSize = size->currentText().toInt();
    if (!penSize) {
        preview->setPattern(preview->getColor(), penSize, Qt::NoPen);
    } else {
        switch (index) {
        case 0:
            preview->setPattern(preview->getColor(), penSize, Qt::DotLine);
            break;
        case 1:
            preview->setPattern(preview->getColor(), penSize, Qt::DashLine);
            break;
        case 2:
            preview->setPattern(preview->getColor(), penSize, Qt::DashDotLine);
            break;
        case 3:
            preview->setPattern(preview->getColor(), penSize, Qt::DashDotDotLine);
            break;
        case 4:
            preview->setPattern(preview->getColor(), penSize, Qt::SolidLine);
            break;
        default:
            kDebug(36001) << "Error in combobox";
            break;
        }
    }
    slotUnselect2(preview);
}

QPixmap CellFormatPageBorder::paintFormatPixmap(Qt::PenStyle _style)
{
    QPixmap pixmap(100, 14);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setPen(QPen(palette().text().color(), 5, _style));
    painter.drawLine(0, 7, 100, 7);
    painter.end();
    return pixmap;
}

void CellFormatPageBorder::loadIcon(const QString& _pix, BorderButton *_button)
{
    _button->setIcon(KIcon(_pix));
}

void CellFormatPageBorder::apply(StyleCommand* obj)
{
    if (borderButtons[BorderType_Horizontal]->isChanged())
        applyHorizontalOutline(obj);

    if (borderButtons[BorderType_Vertical]->isChanged())
        applyVerticalOutline(obj);

    if (borderButtons[BorderType_Left]->isChanged())
        applyLeftOutline(obj);

    if (borderButtons[BorderType_Right]->isChanged())
        applyRightOutline(obj);

    if (borderButtons[BorderType_Top]->isChanged())
        applyTopOutline(obj);

    if (borderButtons[BorderType_Bottom]->isChanged())
        applyBottomOutline(obj);

    if (borderButtons[BorderType_RisingDiagonal]->isChanged() ||
            borderButtons[BorderType_FallingDiagonal]->isChanged())
        applyDiagonalOutline(obj);
}

void CellFormatPageBorder::applyTopOutline(StyleCommand* obj)
{
    BorderButton * top = borderButtons[BorderType_Top];

    QPen tmpPen(top->getColor(), top->getPenWidth(), top->getPenStyle());

    if (dlg->getStyle()) {
        dlg->getStyle()->setTopBorderPen(tmpPen);
    } else {
        if (borderButtons[BorderType_Top]->isChanged())
            obj->setTopBorderPen(tmpPen);
    }
}

void CellFormatPageBorder::applyBottomOutline(StyleCommand* obj)
{
    BorderButton * bottom = borderButtons[BorderType_Bottom];

    QPen tmpPen(bottom->getColor(), bottom->getPenWidth(), bottom->getPenStyle());

    if (dlg->getStyle()) {
        dlg->getStyle()->setBottomBorderPen(tmpPen);
    } else {
        if (borderButtons[BorderType_Bottom]->isChanged())
            obj->setBottomBorderPen(tmpPen);
    }
}

void CellFormatPageBorder::applyLeftOutline(StyleCommand* obj)
{
    BorderButton * left = borderButtons[BorderType_Left];
    QPen tmpPen(left->getColor(), left->getPenWidth(), left->getPenStyle());

    if (dlg->getStyle()) {
        dlg->getStyle()->setLeftBorderPen(tmpPen);
    } else {
        if (borderButtons[BorderType_Left]->isChanged())
            obj->setLeftBorderPen(tmpPen);
    }
}

void CellFormatPageBorder::applyRightOutline(StyleCommand* obj)
{
    BorderButton* right = borderButtons[BorderType_Right];
    QPen tmpPen(right->getColor(), right->getPenWidth(), right->getPenStyle());

    if (dlg->getStyle()) {
        dlg->getStyle()->setRightBorderPen(tmpPen);
    } else {
        if (borderButtons[BorderType_Right]->isChanged())
            obj->setRightBorderPen(tmpPen);
    }
}

void CellFormatPageBorder::applyDiagonalOutline(StyleCommand* obj)
{
    BorderButton * fallDiagonal = borderButtons[BorderType_FallingDiagonal];
    BorderButton * goUpDiagonal = borderButtons[BorderType_RisingDiagonal];
    QPen tmpPenFall(fallDiagonal->getColor(), fallDiagonal->getPenWidth(),
                    fallDiagonal->getPenStyle());
    QPen tmpPenGoUp(goUpDiagonal->getColor(), goUpDiagonal->getPenWidth(),
                    goUpDiagonal->getPenStyle());

    if (dlg->getStyle()) {
        if (fallDiagonal->isChanged())
            dlg->getStyle()->setFallDiagonalPen(tmpPenFall);
        if (goUpDiagonal->isChanged())
            dlg->getStyle()->setGoUpDiagonalPen(tmpPenGoUp);
    } else {
        if (fallDiagonal->isChanged())
            obj->setFallDiagonalPen(tmpPenFall);
        if (goUpDiagonal->isChanged())
            obj->setGoUpDiagonalPen(tmpPenGoUp);
    }
}

void CellFormatPageBorder::applyHorizontalOutline(StyleCommand* obj)
{
    QPen tmpPen(borderButtons[BorderType_Horizontal]->getColor(),
                borderButtons[BorderType_Horizontal]->getPenWidth(),
                borderButtons[BorderType_Horizontal]->getPenStyle());

    if (dlg->getStyle()) {
        dlg->getStyle()->setTopBorderPen(tmpPen);
    } else {
        if (borderButtons[BorderType_Horizontal]->isChanged())
            obj->setHorizontalPen(tmpPen);
    }
}

void CellFormatPageBorder::applyVerticalOutline(StyleCommand* obj)
{
    BorderButton* vertical = borderButtons[BorderType_Vertical];
    QPen tmpPen(vertical->getColor(), vertical->getPenWidth(),
                vertical->getPenStyle());

    if (dlg->getStyle()) {
        dlg->getStyle()->setLeftBorderPen(tmpPen);
    } else {
        if (borderButtons[BorderType_Vertical]->isChanged())
            obj->setVerticalPen(tmpPen);
    }
}


void CellFormatPageBorder::slotSetColorButton(const QColor &_color)
{
    currentColor = _color;

    for (int i = 0; i < NUM_BORDER_PATTERNS; ++i) {
        pattern[i]->setColor(currentColor);
    }
    preview->setColor(currentColor);
}

void CellFormatPageBorder::slotUnselect2(PatternSelect *_p)
{
    for (int i = 0; i < NUM_BORDER_PATTERNS; ++i) {
        if (pattern[i] != _p) {
            pattern[i]->slotUnselect();
        }
    }
    preview->setPattern(_p->getColor(), _p->getPenWidth(), _p->getPenStyle());
}

void CellFormatPageBorder::preselect(BorderButton *_p)
{
    BorderButton* top = borderButtons[BorderType_Top];
    BorderButton* bottom = borderButtons[BorderType_Bottom];
    BorderButton* left = borderButtons[BorderType_Left];
    BorderButton* right = borderButtons[BorderType_Right];
    BorderButton* vertical = borderButtons[BorderType_Vertical];
    BorderButton* horizontal = borderButtons[BorderType_Horizontal];
    BorderButton* remove = shortcutButtons[BorderShortcutType_Remove];
    BorderButton* outline = shortcutButtons[BorderShortcutType_Outline];
    BorderButton* all = shortcutButtons[BorderShortcutType_All];

    _p->setChecked(false);
    if (_p == remove) {
        for (int i = BorderType_Top; i < BorderType_END; i++) {
            if (borderButtons[i]->isChecked()) {
                borderButtons[i]->unselect();
            }
        }
    }
    if (_p == outline) {
        top->setChecked(true);
        top->setPenWidth(preview->getPenWidth());
        top->setPenStyle(preview->getPenStyle());
        top->setColor(currentColor);
        top->setChanged(true);
        bottom->setChecked(true);
        bottom->setPenWidth(preview->getPenWidth());
        bottom->setPenStyle(preview->getPenStyle());
        bottom->setColor(currentColor);
        bottom->setChanged(true);
        left->setChecked(true);
        left->setPenWidth(preview->getPenWidth());
        left->setPenStyle(preview->getPenStyle());
        left->setColor(currentColor);
        left->setChanged(true);
        right->setChecked(true);
        right->setPenWidth(preview->getPenWidth());
        right->setPenStyle(preview->getPenStyle());
        right->setColor(currentColor);
        right->setChanged(true);
    }
    if (_p == all) {
        if (dlg->oneRow == false) {
            horizontal->setChecked(true);
            horizontal->setPenWidth(preview->getPenWidth());
            horizontal->setPenStyle(preview->getPenStyle());
            horizontal->setColor(currentColor);
            horizontal->setChanged(true);
        }
        if (dlg->oneCol == false) {
            vertical->setChecked(true);
            vertical->setPenWidth(preview->getPenWidth());
            vertical->setPenStyle(preview->getPenStyle());
            vertical->setColor(currentColor);
            vertical->setChanged(true);
        }
    }
    area->repaint();
}

void CellFormatPageBorder::changeState(BorderButton *_p)
{
    _p->setChanged(true);

    if (_p->isChecked()) {
        _p->setPenWidth(preview->getPenWidth());
        _p->setPenStyle(preview->getPenStyle());
        _p->setColor(currentColor);
    } else {
        _p->setPenWidth(1);
        _p->setPenStyle(Qt::NoPen);
        _p->setColor(palette().text().color());
    }

    area->repaint();
}

void CellFormatPageBorder::draw()
{
    BorderButton* top = borderButtons[BorderType_Top];
    BorderButton* bottom = borderButtons[BorderType_Bottom];
    BorderButton* left = borderButtons[BorderType_Left];
    BorderButton* right = borderButtons[BorderType_Right];
    BorderButton* risingDiagonal = borderButtons[BorderType_RisingDiagonal];
    BorderButton* fallingDiagonal = borderButtons[BorderType_FallingDiagonal];
    BorderButton* vertical = borderButtons[BorderType_Vertical];
    BorderButton* horizontal = borderButtons[BorderType_Horizontal];
    QPen pen;
    QPainter painter;
    painter.begin(area);

    if ((bottom->getPenStyle()) != Qt::NoPen) {
        pen = QPen(bottom->getColor(), bottom->getPenWidth(), bottom->getPenStyle());
        painter.setPen(pen);
        painter.drawLine(OFFSETX, area->height() - OFFSETY, area->width() - OFFSETX , area->height() - OFFSETY);
    }
    if ((top->getPenStyle()) != Qt::NoPen) {
        pen = QPen(top->getColor(), top->getPenWidth(), top->getPenStyle());
        painter.setPen(pen);
        painter.drawLine(OFFSETX, OFFSETY, area->width() - OFFSETX, OFFSETY);
    }
    if ((left->getPenStyle()) != Qt::NoPen) {
        pen = QPen(left->getColor(), left->getPenWidth(), left->getPenStyle());
        painter.setPen(pen);
        painter.drawLine(OFFSETX, OFFSETY, OFFSETX , area->height() - OFFSETY);
    }
    if ((right->getPenStyle()) != Qt::NoPen) {
        pen = QPen(right->getColor(), right->getPenWidth(), right->getPenStyle());
        painter.setPen(pen);
        painter.drawLine(area->width() - OFFSETX, OFFSETY, area->width() - OFFSETX,
                         area->height() - OFFSETY);

    }
    if ((fallingDiagonal->getPenStyle()) != Qt::NoPen) {
        pen = QPen(fallingDiagonal->getColor(), fallingDiagonal->getPenWidth(),
                   fallingDiagonal->getPenStyle());
        painter.setPen(pen);
        painter.drawLine(OFFSETX, OFFSETY, area->width() - OFFSETX,
                         area->height() - OFFSETY);
        if (dlg->oneCol == false && dlg->oneRow == false) {
            painter.drawLine(area->width() / 2, OFFSETY, area->width() - OFFSETX,
                             area->height() / 2);
            painter.drawLine(OFFSETX, area->height() / 2 , area->width() / 2,
                             area->height() - OFFSETY);
        }
    }
    if ((risingDiagonal->getPenStyle()) != Qt::NoPen) {
        pen = QPen(risingDiagonal->getColor(), risingDiagonal->getPenWidth(),
                   risingDiagonal->getPenStyle());
        painter.setPen(pen);
        painter.drawLine(OFFSETX, area->height() - OFFSETY , area->width() - OFFSETX ,
                         OFFSETY);
        if (dlg->oneCol == false && dlg->oneRow == false) {
            painter.drawLine(area->width() / 2, OFFSETY, OFFSETX, area->height() / 2);
            painter.drawLine(area->width() / 2, area->height() - OFFSETY ,
                             area->width() - OFFSETX, area->height() / 2);
        }

    }
    if ((vertical->getPenStyle()) != Qt::NoPen) {
        pen = QPen(vertical->getColor(), vertical->getPenWidth(),
                   vertical->getPenStyle());
        painter.setPen(pen);
        painter.drawLine(area->width() / 2, 5 , area->width() / 2 , area->height() - 5);
    }
    if ((horizontal->getPenStyle()) != Qt::NoPen) {
        pen = QPen(horizontal->getColor(), horizontal->getPenWidth(),
                   horizontal->getPenStyle());
        painter.setPen(pen);
        painter.drawLine(OFFSETX, area->height() / 2, area->width() - OFFSETX,
                         area->height() / 2);
    }
    painter.end();
}

void CellFormatPageBorder::invertState(BorderButton *_p)
{
    if (_p->isChecked()) {
        _p->unselect();
    } else {
        _p->setChecked(true);
        _p->setPenWidth(preview->getPenWidth());
        _p->setPenStyle(preview->getPenStyle());
        _p->setColor(currentColor);
        _p->setChanged(true);
    }
}

void CellFormatPageBorder::slotPressEvent(QMouseEvent *_ev)
{
    BorderButton* top = borderButtons[BorderType_Top];
    BorderButton* bottom = borderButtons[BorderType_Bottom];
    BorderButton* left = borderButtons[BorderType_Left];
    BorderButton* right = borderButtons[BorderType_Right];
    BorderButton* vertical = borderButtons[BorderType_Vertical];
    BorderButton* horizontal = borderButtons[BorderType_Horizontal];


    QRect rect(OFFSETX, OFFSETY - 8, area->width() - OFFSETX, OFFSETY + 8);
    if (rect.contains(QPoint(_ev->x(), _ev->y()))) {
        if (((top->getPenWidth() != preview->getPenWidth()) ||
                (top->getColor() != currentColor) ||
                (top->getPenStyle() != preview->getPenStyle()))
                && top->isChecked()) {
            top->setPenWidth(preview->getPenWidth());
            top->setPenStyle(preview->getPenStyle());
            top->setColor(currentColor);
            top->setChanged(true);
        } else
            invertState(top);
    }
    rect.setCoords(OFFSETX, area->height() - OFFSETY - 8, area->width() - OFFSETX,
                   area->height() - OFFSETY + 8);
    if (rect.contains(QPoint(_ev->x(), _ev->y()))) {
        if (((bottom->getPenWidth() != preview->getPenWidth()) ||
                (bottom->getColor() != currentColor) ||
                (bottom->getPenStyle() != preview->getPenStyle()))
                && bottom->isChecked()) {
            bottom->setPenWidth(preview->getPenWidth());
            bottom->setPenStyle(preview->getPenStyle());
            bottom->setColor(currentColor);
            bottom->setChanged(true);
        } else
            invertState(bottom);
    }

    rect.setCoords(OFFSETX - 8, OFFSETY, OFFSETX + 8, area->height() - OFFSETY);
    if (rect.contains(QPoint(_ev->x(), _ev->y()))) {
        if (((left->getPenWidth() != preview->getPenWidth()) ||
                (left->getColor() != currentColor) ||
                (left->getPenStyle() != preview->getPenStyle()))
                && left->isChecked()) {
            left->setPenWidth(preview->getPenWidth());
            left->setPenStyle(preview->getPenStyle());
            left->setColor(currentColor);
            left->setChanged(true);
        } else
            invertState(left);
    }
    rect.setCoords(area->width() - OFFSETX - 8, OFFSETY, area->width() - OFFSETX + 8,
                   area->height() - OFFSETY);
    if (rect.contains(QPoint(_ev->x(), _ev->y()))) {
        if (((right->getPenWidth() != preview->getPenWidth()) ||
                (right->getColor() != currentColor) ||
                (right->getPenStyle() != preview->getPenStyle()))
                && right->isChecked()) {
            right->setPenWidth(preview->getPenWidth());
            right->setPenStyle(preview->getPenStyle());
            right->setColor(currentColor);
            right->setChanged(true);
        } else
            invertState(right);
    }

//don't work because I don't know how create a rectangle
//for diagonal
    /*rect.setCoords(OFFSETX,OFFSETY,XLEN-OFFSETX,YHEI-OFFSETY);
    if (rect.contains(QPoint(_ev->x(),_ev->y())))
            {
             invertState(fallDiagonal);
            }
    rect.setCoords(OFFSETX,YHEI-OFFSETY,XLEN-OFFSETX,OFFSETY);
    if (rect.contains(QPoint(_ev->x(),_ev->y())))
            {
             invertState(goUpDiagonal);
            } */

    if (dlg->oneCol == false) {
        rect.setCoords(area->width() / 2 - 8, OFFSETY, area->width() / 2 + 8,
                       area->height() - OFFSETY);

        if (rect.contains(QPoint(_ev->x(), _ev->y()))) {
            if (((vertical->getPenWidth() != preview->getPenWidth()) ||
                    (vertical->getColor() != currentColor) ||
                    (vertical->getPenStyle() != preview->getPenStyle()))
                    && vertical->isChecked()) {
                vertical->setPenWidth(preview->getPenWidth());
                vertical->setPenStyle(preview->getPenStyle());
                vertical->setColor(currentColor);
                vertical->setChanged(true);
            } else
                invertState(vertical);
        }
    }
    if (dlg->oneRow == false) {
        rect.setCoords(OFFSETX, area->height() / 2 - 8, area->width() - OFFSETX,
                       area->height() / 2 + 8);
        if (rect.contains(QPoint(_ev->x(), _ev->y()))) {
            if (((horizontal->getPenWidth() != preview->getPenWidth()) ||
                    (horizontal->getColor() != currentColor) ||
                    (horizontal->getPenStyle() != preview->getPenStyle()))
                    && horizontal->isChecked()) {
                horizontal->setPenWidth(preview->getPenWidth());
                horizontal->setPenStyle(preview->getPenStyle());
                horizontal->setColor(currentColor);
                horizontal->setChanged(true);
            } else
                invertState(horizontal);
        }
    }

    area->repaint();
}



/***************************************************************************
 *
 * BrushSelect
 *
 ***************************************************************************/

BrushSelect::BrushSelect(QWidget *parent, const char *) : QFrame(parent)
{
    brushStyle = Qt::NoBrush;
    brushColor = Qt::red;
    selected = false;
}

void BrushSelect::setPattern(const QColor &_color, Qt::BrushStyle _style)
{
    brushStyle = _style;
    brushColor = _color;
    repaint();
}


void BrushSelect::paintEvent(QPaintEvent *_ev)
{
    QFrame::paintEvent(_ev);

    QPainter painter;
    QBrush brush(brushColor, brushStyle);
    painter.begin(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(brush);
    painter.drawRect(2, 2, width() - 4, height() - 4);
    painter.end();
}

void BrushSelect::mousePressEvent(QMouseEvent *)
{
    slotSelect();

    emit clicked(this);
}

void BrushSelect::slotUnselect()
{
    selected = false;

    setLineWidth(1);
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    repaint();
}

void BrushSelect::slotSelect()
{
    selected = true;

    setLineWidth(2);
    setFrameStyle(QFrame::Panel | QFrame::Plain);
    repaint();
}



/***************************************************************************
 *
 * CellFormatPagePattern
 *
 ***************************************************************************/

CellFormatPagePattern::CellFormatPagePattern(QWidget* parent, CellFormatDialog *_dlg) : QWidget(parent)
{
    dlg = _dlg;

    QGridLayout *grid = new QGridLayout(this);
    grid->setMargin(KDialog::marginHint());
    grid->setSpacing(KDialog::spacingHint());

    QGroupBox* tmpQGroupBox;
    tmpQGroupBox = new QGroupBox(this);
    tmpQGroupBox->setTitle(i18n("Pattern"));
    tmpQGroupBox->setAlignment(Qt::AlignLeft);

    QGridLayout *grid2 = new QGridLayout(tmpQGroupBox);
    grid2->setMargin(KDialog::marginHint());
    grid2->setSpacing(KDialog::spacingHint());
    int fHeight = tmpQGroupBox->fontMetrics().height();
    grid2->addItem(new QSpacerItem(0, fHeight / 2), 0, 0);  // groupbox title


    brush1 = new BrushSelect(tmpQGroupBox, "Frame_1");
    brush1->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush1, 1, 0);

    brush2 = new BrushSelect(tmpQGroupBox, "Frame_2");
    brush2->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush2, 1, 1);

    brush3 = new BrushSelect(tmpQGroupBox, "Frame_3");
    brush3->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush3, 1, 2);

    brush4 = new BrushSelect(tmpQGroupBox, "Frame_4");
    brush4->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush4, 2, 0);

    brush5 = new BrushSelect(tmpQGroupBox, "Frame_5");
    brush5->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush5, 2, 1);

    brush6 = new BrushSelect(tmpQGroupBox, "Frame_6");
    brush6->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush6, 2, 2);

    brush7 = new BrushSelect(tmpQGroupBox, "Frame_7");
    brush7->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush7, 3, 0);

    brush8 = new BrushSelect(tmpQGroupBox, "Frame_8");
    brush8->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush8, 3, 1);

    brush9 = new BrushSelect(tmpQGroupBox, "Frame_9");
    brush9->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush9, 3, 2);

    brush10 = new BrushSelect(tmpQGroupBox, "Frame_10");
    brush10->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush10, 4, 0);

    brush11 = new BrushSelect(tmpQGroupBox, "Frame_11");
    brush11->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush11, 4, 1);

    brush12 = new BrushSelect(tmpQGroupBox, "Frame_12");
    brush12->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush12, 4, 2);

    brush13 = new BrushSelect(tmpQGroupBox, "Frame_13");
    brush13->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush13, 5, 0);

    brush14 = new BrushSelect(tmpQGroupBox, "Frame_14");
    brush14->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush14, 5, 1);

    brush15 = new BrushSelect(tmpQGroupBox, "Frame_15");
    brush15->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(brush15, 5, 2);

    QGridLayout *grid3 = new QGridLayout();
    color = new KColorButton(tmpQGroupBox);
    grid3->addWidget(color, 0, 1);

    QLabel *tmpQLabel = new QLabel(tmpQGroupBox);
    tmpQLabel->setText(i18n("Color:"));
    grid3->addWidget(tmpQLabel, 0, 0);

    grid2->addItem(grid3, 6, 0, 1, 3);

    grid3 = new QGridLayout();
    grid3->setSpacing(KDialog::spacingHint());

    tmpQLabel = new QLabel(tmpQGroupBox);
    grid3->addWidget(tmpQLabel, 0, 0);
    tmpQLabel->setText(i18n("Background color:"));

    bgColorButton = new KColorButton(tmpQGroupBox);
    grid3->addWidget(bgColorButton, 0, 1);
    if (dlg->bBgColor)
        bgColor = dlg->bgColor;
    else
        bgColor = palette().base().color();

    if (!bgColor.isValid())
        bgColor = palette().base().color();

    bgColorButton->setColor(bgColor);
    connect(bgColorButton, SIGNAL(changed(const QColor &)),
            this, SLOT(slotSetBackgroundColor(const QColor &)));

    notAnyColor = new QPushButton(i18n("No Color"), tmpQGroupBox);
    grid3->addWidget(notAnyColor, 0, 2);
    connect(notAnyColor, SIGNAL(clicked()),
            this, SLOT(slotNotAnyColor()));
    b_notAnyColor = true;

    grid2->addItem(grid3, 7, 0, 1, 3);

    grid->addWidget(tmpQGroupBox, 0, 0, 4, 1);

    tmpQGroupBox = new QGroupBox(this);
    tmpQGroupBox->setTitle(i18n("Preview"));
    tmpQGroupBox->setAlignment(Qt::AlignLeft);

    grid2 = new QGridLayout(tmpQGroupBox);
    grid2->setMargin(KDialog::marginHint());
    grid2->setSpacing(KDialog::spacingHint());
    fHeight = tmpQGroupBox->fontMetrics().height();
    grid2->addItem(new QSpacerItem(0, fHeight / 2), 0, 0);  // groupbox title

    current = new BrushSelect(tmpQGroupBox, "Current");
    current->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    grid2->addWidget(current, 1, 0);
    grid->addWidget(tmpQGroupBox, 4, 0);

    connect(brush1, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush2, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush3, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush4, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush5, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush6, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush7, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush8, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush9, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush10, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush11, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush12, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush13, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush14, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));
    connect(brush15, SIGNAL(clicked(BrushSelect*)),
            this, SLOT(slotUnselect2(BrushSelect*)));

    brush1->setPattern(Qt::red, Qt::VerPattern);
    brush2->setPattern(Qt::red, Qt::HorPattern);
    brush3->setPattern(Qt::red, Qt::Dense1Pattern);
    brush4->setPattern(Qt::red, Qt::Dense2Pattern);
    brush5->setPattern(Qt::red, Qt::Dense3Pattern);
    brush6->setPattern(Qt::red, Qt::Dense4Pattern);
    brush7->setPattern(Qt::red, Qt::Dense5Pattern);
    brush8->setPattern(Qt::red, Qt::Dense6Pattern);
    brush9->setPattern(Qt::red, Qt::Dense7Pattern);
    brush10->setPattern(Qt::red, Qt::CrossPattern);
    brush11->setPattern(Qt::red, Qt::BDiagPattern);
    brush12->setPattern(Qt::red, Qt::FDiagPattern);
    brush13->setPattern(Qt::red, Qt::VerPattern);
    brush14->setPattern(Qt::red, Qt::DiagCrossPattern);
    brush15->setPattern(Qt::red, Qt::NoBrush);

    current->setPattern(dlg->brushColor, dlg->brushStyle);
    current->slotSelect();
    selectedBrush = current;
    color->setColor(dlg->brushColor);
    QPalette palette = current->palette();
    palette.setColor(current->backgroundRole(), bgColor);
    current->setPalette(palette);

    connect(color, SIGNAL(changed(const QColor &)),
            this, SLOT(slotSetColorButton(const QColor &)));

    slotSetColorButton(dlg->brushColor);
    init();
    this->resize(400, 400);
}

void CellFormatPagePattern::slotNotAnyColor()
{
    b_notAnyColor = true;
    bgColorButton->setColor(palette().base().color());
    QPalette palette = current->palette();
    palette.setColor(current->backgroundRole(), this->palette().base().color());
    current->setPalette(palette);
}

void CellFormatPagePattern::slotSetBackgroundColor(const QColor &_color)
{
    bgColor = _color;
    QPalette palette = current->palette();
    palette.setColor(current->backgroundRole(), bgColor);
    current->setPalette(palette);
    b_notAnyColor = false;
}

void CellFormatPagePattern::init()
{
    if (dlg->brushStyle == Qt::VerPattern) {
        brush1->slotSelect();
    } else if (dlg->brushStyle == Qt::HorPattern) {
        brush2->slotSelect();
    } else if (dlg->brushStyle == Qt::Dense1Pattern) {
        brush3->slotSelect();
    } else if (dlg->brushStyle == Qt::Dense2Pattern) {
        brush4->slotSelect();
    } else if (dlg->brushStyle == Qt::Dense3Pattern) {
        brush5->slotSelect();
    } else if (dlg->brushStyle == Qt::Dense4Pattern) {
        brush6->slotSelect();
    } else if (dlg->brushStyle == Qt::Dense5Pattern) {
        brush7->slotSelect();
    } else if (dlg->brushStyle == Qt::Dense6Pattern) {
        brush8->slotSelect();
    } else if (dlg->brushStyle == Qt::Dense7Pattern) {
        brush9->slotSelect();
    } else if (dlg->brushStyle == Qt::CrossPattern) {
        brush10->slotSelect();
    } else if (dlg->brushStyle == Qt::BDiagPattern) {
        brush11->slotSelect();
    } else if (dlg->brushStyle == Qt::FDiagPattern) {
        brush12->slotSelect();
    } else if (dlg->brushStyle == Qt::VerPattern) {
        brush13->slotSelect();
    } else if (dlg->brushStyle == Qt::DiagCrossPattern) {
        brush14->slotSelect();
    } else if (dlg->brushStyle == Qt::NoBrush) {
        brush15->slotSelect();
    } else
        kDebug(36001) << "Error in brushStyle";
}

void CellFormatPagePattern::slotSetColorButton(const QColor &_color)
{
    currentColor = _color;

    brush1->setBrushColor(currentColor);
    brush2->setBrushColor(currentColor);
    brush3->setBrushColor(currentColor);
    brush4->setBrushColor(currentColor);
    brush5->setBrushColor(currentColor);
    brush6->setBrushColor(currentColor);
    brush7->setBrushColor(currentColor);
    brush8->setBrushColor(currentColor);
    brush9->setBrushColor(currentColor);
    brush10->setBrushColor(currentColor);
    brush11->setBrushColor(currentColor);
    brush12->setBrushColor(currentColor);
    brush13->setBrushColor(currentColor);
    brush14->setBrushColor(currentColor);
    brush15->setBrushColor(currentColor);
    current->setBrushColor(currentColor);
}

void CellFormatPagePattern::slotUnselect2(BrushSelect *_p)
{
    selectedBrush = _p;

    if (brush1 != _p)
        brush1->slotUnselect();
    if (brush2 != _p)
        brush2->slotUnselect();
    if (brush3 != _p)
        brush3->slotUnselect();
    if (brush4 != _p)
        brush4->slotUnselect();
    if (brush5 != _p)
        brush5->slotUnselect();
    if (brush6 != _p)
        brush6->slotUnselect();
    if (brush7 != _p)
        brush7->slotUnselect();
    if (brush8 != _p)
        brush8->slotUnselect();
    if (brush9 != _p)
        brush9->slotUnselect();
    if (brush10 != _p)
        brush10->slotUnselect();
    if (brush11 != _p)
        brush11->slotUnselect();
    if (brush12 != _p)
        brush12->slotUnselect();
    if (brush13 != _p)
        brush13->slotUnselect();
    if (brush14 != _p)
        brush14->slotUnselect();
    if (brush15 != _p)
        brush15->slotUnselect();

    current->setBrushStyle(selectedBrush->getBrushStyle());
}

void CellFormatPagePattern::apply(CustomStyle * style)
{
    if (selectedBrush != 0
            && (dlg->brushStyle != selectedBrush->getBrushStyle()
                || dlg->brushColor != selectedBrush->getBrushColor()))
        style->setBackgroundBrush(QBrush(selectedBrush->getBrushColor(), selectedBrush->getBrushStyle()));

    if (!b_notAnyColor && bgColor != dlg->getStyle()->backgroundColor())
        style->setBackgroundColor(bgColor);
}

void CellFormatPagePattern::apply(StyleCommand *_obj)
{
    if (selectedBrush != 0
            && (dlg->brushStyle != selectedBrush->getBrushStyle()
                || dlg->brushColor != selectedBrush->getBrushColor()))
        _obj->setBackgroundBrush(QBrush(selectedBrush->getBrushColor(), selectedBrush->getBrushStyle()));

    if (bgColor == dlg->bgColor)
        return;

    if (!b_notAnyColor)
        _obj->setBackgroundColor(bgColor);
}

#include "LayoutDialog.moc"
