/* This file is part of the KDE project
   Copyright 2005-2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
#include "StyleManipulators.h"

#include <kdebug.h>
#include <klocale.h>

#include "Cell.h"
#include "CellStorage.h"
#include "Doc.h"
#include "Sheet.h"
#include "Style.h"
#include "StyleStorage.h"

using namespace KSpread;

StyleManipulator::StyleManipulator()
    : m_horizontalPen( QPen(QColor(), 0, Qt::NoPen) )
    , m_verticalPen( QPen(QColor(), 0, Qt::NoPen) )
    , m_style( new Style() )
{
}

StyleManipulator::~StyleManipulator()
{
}

bool StyleManipulator::process(Element* element)
{
    const QRect range = element->rect();
    if ( !m_reverse ) // (re)do
    {
        if ( m_firstrun )
        {
            const QList< QPair<QRectF,SharedSubStyle> > rawUndoData
                    = m_sheet->styleStorage()->undoData( Region(element->rect()) );
            for ( int i = 0; i < rawUndoData.count(); ++i )
            {
//                 if ( m_style->hasAttribute( rawUndoData[i].second->type() ) ||
//                      rawUndoData[i].second->type() == Style::DefaultStyleKey ||
//                      rawUndoData[i].second->type() == Style::NamedStyleKey )
                    m_undoData << rawUndoData[i];
            }
        }
        // special handling for the border
        const QPen leftPen = m_style->leftBorderPen();
        const QPen rightPen = m_style->rightBorderPen();
        const QPen topPen = m_style->topBorderPen();
        const QPen bottomPen = m_style->bottomBorderPen();
        m_style->clearAttribute( Style::LeftPen );
        m_style->clearAttribute( Style::RightPen );
        m_style->clearAttribute( Style::TopPen );
        m_style->clearAttribute( Style::BottomPen );

        // use the horizontal/vertical pens
        if ( m_horizontalPen.style() != Qt::NoPen )
        {
            m_style->setTopBorderPen( m_horizontalPen );
            m_style->setBottomBorderPen( m_horizontalPen );
        }
        if ( m_verticalPen.style() != Qt::NoPen )
        {
            m_style->setLeftBorderPen( m_verticalPen );
            m_style->setRightBorderPen( m_verticalPen );
        }

        // set the actual style
        m_sheet->cellStorage()->setStyle( Region(range), *m_style );

        // set the outer border styles
        Style style;
        if ( leftPen.style() != Qt::NoPen )
        {
            style.setLeftBorderPen( leftPen );
            m_sheet->cellStorage()->setStyle( Region(QRect(range.left(), range.top(), 1, range.height())), style );
        }
        if ( rightPen.style() != Qt::NoPen )
        {
            style.clear();
            style.setRightBorderPen( rightPen );
            m_sheet->cellStorage()->setStyle( Region(QRect(range.right(), range.top(), 1, range.height())), style );
        }
        if ( topPen.style() != Qt::NoPen )
        {
            style.clear();
            style.setTopBorderPen( topPen );
            m_sheet->cellStorage()->setStyle( Region(QRect(range.left(), range.top(), range.width(), 1)), style );
        }
        if ( bottomPen.style() != Qt::NoPen )
        {
            style.clear();
            style.setBottomBorderPen( bottomPen );
            m_sheet->cellStorage()->setStyle( Region(QRect(range.left(), range.bottom(), range.width(), 1)), style );
        }
    }
    return true;
}

bool StyleManipulator::preProcessing()
{
    if (m_firstrun)
    {
        if ( m_style->isDefault() )
            setText( i18n( "Reset Style" ) );
        else
            setText( i18n( "Change Style" ) );
    }
    return AbstractRegionCommand::preProcessing();
}

bool StyleManipulator::mainProcessing()
{
    if ( !m_reverse )
    {
        if ( m_firstrun )
        {
            m_undoData.clear();
        }
    }
    else
    {
        Style style;
        style.setDefault();
        m_sheet->cellStorage()->setStyle( *this, style );
        for ( int i = 0; i < m_undoData.count(); ++i )
        {
            m_sheet->styleStorage()->insert( m_undoData[i].first.toRect(), m_undoData[i].second );
        }
    }
    return AbstractRegionCommand::mainProcessing();
}

bool StyleManipulator::postProcessing()
{
    return true;
}


/***************************************************************************
  class IncreaseIndentManipulator
****************************************************************************/

IncreaseIndentManipulator::IncreaseIndentManipulator()
  : AbstractRegionCommand()
{
    setText( i18n( "Increase Indentation" ) );
}

bool IncreaseIndentManipulator::process( Element* element )
{
    QList< QPair<QRectF,SharedSubStyle> > indentationPairs = m_sheet->styleStorage()->undoData( Region(element->rect()) );
    for ( int i = 0; i < indentationPairs.count(); ++i )
    {
        if ( indentationPairs[i].second->type() != Style::Indentation )
            indentationPairs.removeAt( i-- );
    }

    Style style;
    if ( !m_reverse )
    {
        // increase the indentation set for the whole rectangle
        style.setIndentation( m_sheet->styleStorage()->contains( element->rect() ).indentation() + m_sheet->doc()->indentValue()  );
        m_sheet->cellStorage()->setStyle( Region(element->rect()), style );
        // increase the several indentations
        for ( int i = 0; i < indentationPairs.count(); ++i )
        {
            style.clear();
            style.insertSubStyle( indentationPairs[i].second );
            style.setIndentation( style.indentation() + m_sheet->doc()->indentValue() );
            m_sheet->cellStorage()->setStyle( Region(indentationPairs[i].first.toRect()), style );
        }
    }
    else // m_reverse
    {
        // decrease the indentation set for the whole rectangle
        style.setIndentation( m_sheet->styleStorage()->contains( element->rect() ).indentation() - m_sheet->doc()->indentValue()  );
        m_sheet->cellStorage()->setStyle( Region(element->rect()), style );
        // decrease the several indentations
        for ( int i = 0; i < indentationPairs.count(); ++i )
        {
            style.clear();
            style.insertSubStyle( indentationPairs[i].second );
            style.setIndentation( style.indentation() - m_sheet->doc()->indentValue() );
            m_sheet->cellStorage()->setStyle( Region(indentationPairs[i].first.toRect()), style );
        }
    }
    return true;
}

bool IncreaseIndentManipulator::postProcessing()
{
    return true;
}

void IncreaseIndentManipulator::setReverse(bool reverse)
{
  AbstractRegionCommand::setReverse(reverse);
  if ( !m_reverse )
    setText( i18n( "Increase Indentation" ) );
  else
    setText( i18n( "Decrease Indentation" ) );
}



/***************************************************************************
  class BorderColorManipulator
****************************************************************************/

BorderColorManipulator::BorderColorManipulator()
  : AbstractRegionCommand()
{
    setText(i18n( "Change Border Color" ));
}

bool BorderColorManipulator::preProcessing()
{
    if ( m_firstrun )
    {
        QList< QPair<QRectF,SharedSubStyle> > undoData = m_sheet->styleStorage()->undoData( *this );
        ConstIterator endOfList = constEnd();
        for (ConstIterator it = constBegin(); it != endOfList; ++it)
        {
            for ( int i = 0; i < undoData.count(); ++i )
            {
                if ( undoData[i].second->type() != Style::LeftPen ||
                     undoData[i].second->type() != Style::RightPen ||
                     undoData[i].second->type() != Style::TopPen ||
                     undoData[i].second->type() != Style::BottomPen ||
                     undoData[i].second->type() != Style::FallDiagonalPen ||
                     undoData[i].second->type() != Style::GoUpDiagonalPen )
                {
                    undoData.removeAt( i-- );
                }
            }
            m_undoData += undoData;
        }
    }
    return AbstractRegionCommand::preProcessing();
}

bool BorderColorManipulator::mainProcessing()
{
    if ( !m_reverse )
    {
        // change colors
        Style style;
        for ( int i = 0; i < m_undoData.count(); ++i )
        {
            style.clear();
            style.insertSubStyle( m_undoData[i].second );
            QPen pen;
            if ( m_undoData[i].second->type() == Style::LeftPen )
            {
                pen = style.leftBorderPen();
                pen.setColor( m_color );
                style.setLeftBorderPen( pen );
            }
            if ( m_undoData[i].second->type() == Style::RightPen )
            {
                pen = style.rightBorderPen();
                pen.setColor( m_color );
                style.setRightBorderPen( pen );
            }
            if ( m_undoData[i].second->type() == Style::TopPen )
            {
                pen = style.topBorderPen();
                pen.setColor( m_color );
                style.setTopBorderPen( pen );
            }
            if ( m_undoData[i].second->type() == Style::BottomPen )
            {
                pen = style.bottomBorderPen();
                pen.setColor( m_color );
                style.setBottomBorderPen( pen );
            }
            if ( m_undoData[i].second->type() == Style::FallDiagonalPen )
            {
                pen = style.fallDiagonalPen();
                pen.setColor( m_color );
                style.setFallDiagonalPen( pen );
            }
            if ( m_undoData[i].second->type() == Style::GoUpDiagonalPen )
            {
                pen = style.goUpDiagonalPen();
                pen.setColor( m_color );
                style.setGoUpDiagonalPen( pen );
            }
            m_sheet->cellStorage()->setStyle( Region(m_undoData[i].first.toRect()), style );
        }
    }
    else // m_reverse
    {
        for ( int i = 0; i < m_undoData.count(); ++i )
        {
            Style style;
            style.insertSubStyle( m_undoData[i].second );
            m_sheet->cellStorage()->setStyle( Region(m_undoData[i].first.toRect()), style );
        }
    }
    return true;
}

bool BorderColorManipulator::postProcessing()
{
    return true;
}


/***************************************************************************
  class IncreasePrecisionManipulator
****************************************************************************/

IncreasePrecisionManipulator::IncreasePrecisionManipulator()
  : AbstractRegionCommand()
{
    setText( i18n( "Increase Precision" ) );
}

bool IncreasePrecisionManipulator::process( Element* element )
{
    QList< QPair<QRectF,SharedSubStyle> > precisionPairs = m_sheet->styleStorage()->undoData( Region(element->rect()) );
    for ( int i = 0; i < precisionPairs.count(); ++i )
    {
        if ( precisionPairs[i].second->type() != Style::Precision )
            precisionPairs.removeAt( i-- );
    }

    Style style;
    if ( !m_reverse )
    {
        // increase the precision set for the whole rectangle
        Style style;
        int precision = m_sheet->styleStorage()->contains( element->rect() ).precision() + 1;
        style.setPrecision( precision );
        if ( precision <= 10 )
            m_sheet->cellStorage()->setStyle( Region(element->rect()), style );
        // increase the several precisions
        for ( int i = 0; i < precisionPairs.count(); ++i )
        {
            style.clear();
            style.insertSubStyle( precisionPairs[i].second );
            precision = style.precision() + 1;
            style.setPrecision( precision );
            if ( precision <= 10 )
                m_sheet->cellStorage()->setStyle( Region(precisionPairs[i].first.toRect()), style );
        }
    }
    else // m_reverse
    {
        // decrease the precision set for the whole rectangle
        Style style;
        int precision = m_sheet->styleStorage()->contains( element->rect() ).precision() - 1;
        style.setPrecision( precision );
        if ( precision >= 0 )
            m_sheet->cellStorage()->setStyle( Region(element->rect()), style );
        // decrease the several precisions
        for ( int i = 0; i < precisionPairs.count(); ++i )
        {
            style.clear();
            style.insertSubStyle( precisionPairs[i].second );
            precision = style.precision() - 1;
            if ( precision >= 0 )
                style.setPrecision( precision );
            m_sheet->cellStorage()->setStyle( Region(precisionPairs[i].first.toRect()), style );
        }
    }
    return true;
}

bool IncreasePrecisionManipulator::postProcessing()
{
    return true;
}

void IncreasePrecisionManipulator::setReverse(bool reverse)
{
    AbstractRegionCommand::setReverse(reverse);
    if ( !m_reverse )
        setText( i18n( "Increase Precision" ) );
    else
        setText( i18n( "Decrease Precision" ) );
}
