/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006-2007 Thomas Zander <zander@kde.org>
   Copyright (C)  2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>

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

#include "CharacterHighlighting.h"

#include <KOdfText.h>
#include <KCharacterStyle.h>

CharacterHighlighting::CharacterHighlighting(bool uniqueFormat,QWidget* parent)
        : QWidget(parent),
        m_uniqueFormat(uniqueFormat)
{
    widget.setupUi(this);

    widget.underlineStyle->addItems(KOdfText::underlineTypeList());
    widget.underlineLineStyle->addItems(KOdfText::underlineStyleList());

    widget.strikethroughStyle->addItems(KOdfText::underlineTypeList()); //TODO make KOdfText consistent: either add strikethroughTypeList, or change from underlineTypeList to lineTypeList
    widget.strikethroughLineStyle->addItems(KOdfText::underlineStyleList()); //TODO idem

    connect(widget.underlineStyle, SIGNAL(activated(int)), this, SLOT(underlineTypeChanged(int)));
    connect(widget.underlineLineStyle, SIGNAL(activated(int)), this, SLOT(underlineStyleChanged(int)));
    connect(widget.underlineColor, SIGNAL(changed(QColor)), this, SLOT(underlineColorChanged(QColor)));

    connect(widget.strikethroughStyle, SIGNAL(activated(int)), this, SLOT(strikethroughTypeChanged(int)));
    connect(widget.strikethroughLineStyle, SIGNAL(activated(int)), this, SLOT(strikethroughStyleChanged(int)));
    connect(widget.strikethroughColor, SIGNAL(changed(QColor)), this, SLOT(strikethroughColorChanged(QColor)));

    connect(widget.normal, SIGNAL(toggled(bool)), this, SLOT(capitalisationChanged()));
    connect(widget.smallcaps, SIGNAL(toggled(bool)), this, SLOT(capitalisationChanged()));
    connect(widget.uppercase, SIGNAL(toggled(bool)), this, SLOT(capitalisationChanged()));
    connect(widget.lowercase, SIGNAL(toggled(bool)), this, SLOT(capitalisationChanged()));
    connect(widget.capitalize, SIGNAL(toggled(bool)), this, SLOT(capitalisationChanged()));
}

KCharacterStyle::LineType CharacterHighlighting::indexToLineType(int index)
{
    KCharacterStyle::LineType lineType;
    switch (index) {
        case 1: lineType = KCharacterStyle::SingleLine; break;
        case 2: lineType = KCharacterStyle::DoubleLine; break;
        case 0:
        default:
            lineType = KCharacterStyle::NoLineType; break;
    }
    return lineType;
}

KCharacterStyle::LineStyle CharacterHighlighting::indexToLineStyle(int index)
{
    KCharacterStyle::LineStyle lineStyle;
    switch (index) {
        case 1: lineStyle = KCharacterStyle::DashLine; break;
        case 2: lineStyle = KCharacterStyle::DottedLine; break;
        case 3: lineStyle = KCharacterStyle::DotDashLine; break;
        case 4: lineStyle = KCharacterStyle::DotDotDashLine; break;
        case 5: lineStyle = KCharacterStyle::WaveLine; break;
        case 0:
        default:
            lineStyle = KCharacterStyle::SolidLine; break;
    }
    return lineStyle;
}

int CharacterHighlighting::lineTypeToIndex(KCharacterStyle::LineType type)
{
    int index;
    switch (type) {
    case KCharacterStyle::NoLineType: index = 0; break;
    case KCharacterStyle::SingleLine: index = 1; break;
    case KCharacterStyle::DoubleLine: index = 2; break;
    default: index = 0; break;
    }
    return index;
}

int CharacterHighlighting::lineStyleToIndex(KCharacterStyle::LineStyle type)
{
    int index;
    switch (type) {
    case KCharacterStyle::SolidLine: index = 0; break;
    case KCharacterStyle::DashLine: index = 1; break;
    case KCharacterStyle::DottedLine: index = 2; break;
    case KCharacterStyle::DotDashLine: index = 3; break;
    case KCharacterStyle::DotDotDashLine: index = 4; break;
    case KCharacterStyle::WaveLine: index = 5; break;
    default: index = 0; break;
    }
    return index;
}

void CharacterHighlighting::capitalisationChanged()
{
    if (m_uniqueFormat || widget.groupBox->isChecked()) {
        if (widget.normal->isChecked())
            emit capitalizationChanged(QFont::MixedCase);
        else if (widget.smallcaps->isChecked())
            emit capitalizationChanged(QFont::SmallCaps);
        else if (widget.uppercase->isChecked())
            emit capitalizationChanged(QFont::AllUppercase);
        else if (widget.lowercase->isChecked())
            emit capitalizationChanged(QFont::AllLowercase);
        else if (widget.capitalize->isChecked())
            emit capitalizationChanged(QFont::Capitalize);
    }
}

void CharacterHighlighting::underlineTypeChanged(int item)
{
    widget.underlineLineStyle->setEnabled(item > 0);
    widget.underlineColor->setEnabled(item > 0);

    emit underlineChanged(indexToLineType(item), indexToLineStyle(widget.underlineLineStyle->currentIndex()), widget.underlineColor->color());
}

void CharacterHighlighting::underlineStyleChanged(int item)
{
    if (widget.underlineStyle->currentIndex())
        emit underlineChanged(indexToLineType(widget.underlineStyle->currentIndex()), indexToLineStyle(item), widget.underlineColor->color());
}

void CharacterHighlighting::underlineColorChanged(QColor color)
{
    if (widget.underlineStyle->currentIndex())
        emit underlineChanged(indexToLineType(widget.underlineStyle->currentIndex()), indexToLineStyle(widget.underlineLineStyle->currentIndex()), color);
}

void CharacterHighlighting::strikethroughTypeChanged(int item)
{
    widget.strikethroughLineStyle->setEnabled(item > 0);
    widget.strikethroughColor->setEnabled(item > 0);

    emit strikethroughChanged(indexToLineType(item), indexToLineStyle(widget.strikethroughLineStyle->currentIndex()), widget.strikethroughColor->color());
}

void CharacterHighlighting::strikethroughStyleChanged(int item)
{
    if (widget.strikethroughStyle->currentIndex())
        emit strikethroughChanged(indexToLineType(widget.strikethroughStyle->currentIndex()), indexToLineStyle(item), widget.strikethroughColor->color());
}

void CharacterHighlighting::strikethroughColorChanged(QColor color)
{
    if (widget.strikethroughStyle->currentIndex())
        emit strikethroughChanged(indexToLineType(widget.strikethroughStyle->currentIndex()), indexToLineStyle(widget.strikethroughLineStyle->currentIndex()), color);
}

void CharacterHighlighting::setDisplay(KCharacterStyle *style)
{
    if (style == 0)
        return;

//set the underline up
    widget.underlineStyle->setCurrentIndex(1);
    widget.underlineLineStyle->setCurrentIndex(lineStyleToIndex(style->underlineStyle()));
    if (m_uniqueFormat)
        widget.underlineStyle->setCurrentIndex(lineTypeToIndex(style->underlineType()));
    else
        widget.underlineStyle->setCurrentIndex(-1);

    underlineTypeChanged(widget.underlineStyle->currentIndex());
    widget.underlineColor->setColor(style->underlineColor());

//set the strikethrough up
    widget.strikethroughStyle->setCurrentIndex(1);
    widget.strikethroughLineStyle->setCurrentIndex(lineStyleToIndex(style->strikeOutStyle()));
    if (m_uniqueFormat)
        widget.strikethroughStyle->setCurrentIndex(lineTypeToIndex(style->strikeOutType()));
    else
        widget.strikethroughStyle->setCurrentIndex(-1);
    strikethroughTypeChanged(widget.strikethroughStyle->currentIndex());
    widget.strikethroughColor->setColor(style->strikeOutColor());

//Now set the capitalisation
    switch (style->fontCapitalization()) {
    case QFont::MixedCase: widget.normal->setChecked(true); break;
    case QFont::SmallCaps: widget.smallcaps->setChecked(true); break;
    case QFont::AllUppercase: widget.uppercase->setChecked(true); break;
    case QFont::AllLowercase: widget.lowercase->setChecked(true); break;
    case QFont::Capitalize: widget.capitalize->setChecked(true); break;
    }

    widget.groupBox->setCheckable(!m_uniqueFormat);
    widget.groupBox->setChecked(m_uniqueFormat);

    capitalisationChanged();
}

void CharacterHighlighting::save(KCharacterStyle *style)
{
    if (style == 0)
        return;

    if (widget.underlineStyle->currentIndex() == 0) {
        style->setUnderlineType(KCharacterStyle::NoLineType);
        style->setUnderlineStyle(KCharacterStyle::NoLineStyle);
    } else if (widget.underlineStyle->currentIndex() > 0) {
        style->setUnderlineType(indexToLineType(widget.underlineStyle->currentIndex()));
        style->setUnderlineStyle(indexToLineStyle(widget.underlineLineStyle->currentIndex()));
        style->setUnderlineColor(widget.underlineColor->color());
    }

    if (widget.strikethroughStyle->currentIndex() == 0) {
        style->setStrikeOutType(KCharacterStyle::NoLineType);
        style->setStrikeOutStyle(KCharacterStyle::NoLineStyle);
    } else if (widget.strikethroughStyle->currentIndex() > 0) {
        style->setStrikeOutType(indexToLineType(widget.strikethroughStyle->currentIndex()));
        style->setStrikeOutStyle(indexToLineStyle(widget.strikethroughLineStyle->currentIndex()));
        style->setStrikeOutColor(widget.strikethroughColor->color());
    }

    if (m_uniqueFormat || widget.groupBox->isChecked()) {
        if (widget.normal->isChecked())
            style->setFontCapitalization(QFont::MixedCase);
        else if (widget.smallcaps->isChecked())
            style->setFontCapitalization(QFont::SmallCaps);
        else if (widget.uppercase->isChecked())
            style->setFontCapitalization(QFont::AllUppercase);
        else if (widget.lowercase->isChecked())
            style->setFontCapitalization(QFont::AllLowercase);
        else if (widget.capitalize->isChecked())
            style->setFontCapitalization(QFont::Capitalize);
    }
}

#include <CharacterHighlighting.moc>
