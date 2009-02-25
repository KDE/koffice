/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KPrPlaceholderTextStrategy.h"

#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QPainter>

#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoParagraphStyle.h>
#include <KoShape.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeFactory.h>
#include <KoShapeRegistry.h>
#include <KoTextShapeData.h>
#include <KoTextDocument.h>
#include <KoTextDocumentLayout.h>
#include <KoStyleManager.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>

KPrPlaceholderTextStrategy::KPrPlaceholderTextStrategy( const QString & presentationClass )
: KPrPlaceholderStrategy( presentationClass )
, m_textShape( 0 )
{
}

KPrPlaceholderTextStrategy::~KPrPlaceholderTextStrategy()
{
    delete m_textShape;
}

KoShape * KPrPlaceholderTextStrategy::createShape( const QMap<QString, KoDataCenter *> & dataCenterMap )
{
    KoShape * shape = KPrPlaceholderStrategy::createShape( dataCenterMap );
    if ( m_textShape ) {
        KoTextShapeData * data = qobject_cast<KoTextShapeData*>( m_textShape->userData() );
        KoTextShapeData * newData = qobject_cast<KoTextShapeData*>( shape->userData() );
        if ( data && newData ) {
            QTextCursor cursor( data->document() );
            QTextCursor newCursor( newData->document() );
            KoTextDocument textDocument( newData->document() );
            KoStyleManager * styleManager = textDocument.styleManager();

            QTextBlockFormat blockFormat( cursor.blockFormat() );
            blockFormat.setProperty( KoParagraphStyle::StyleId, styleManager->defaultParagraphStyle()->styleId() );
            newCursor.setBlockFormat( blockFormat );

            QTextCharFormat chatFormat( cursor.blockCharFormat() );
            chatFormat.setProperty( KoCharacterStyle::StyleId, styleManager->defaultParagraphStyle()->characterStyle()->styleId() );
            newCursor.setBlockCharFormat( chatFormat );
        }
    }
    return shape;
}

void KPrPlaceholderTextStrategy::paint( QPainter & painter, const KoViewConverter &converter, const QRectF & rect )
{
    if ( m_textShape ) {
        painter.save();
        m_textShape->setSize( rect.size() );
        // this code is needed to make sure the text of the textshape is layouted before it is painted
        KoTextShapeData * shapeData = qobject_cast<KoTextShapeData*>( m_textShape->userData() );
        QTextDocument * document = shapeData->document();
        KoTextDocumentLayout * lay = dynamic_cast<KoTextDocumentLayout*>( document->documentLayout() );
        if ( lay ) {
            lay->layout();
        }
        m_textShape->paint( painter, converter );

        KoShape::applyConversion( painter, converter );
        QPen pen( Qt::gray );
        //pen.setStyle( Qt::DashLine ); // endless loop
        painter.setPen( pen );
        painter.drawRect( rect );
        painter.restore();
    }
    else {
        KPrPlaceholderStrategy::paint( painter, converter, rect );
    }
}

void KPrPlaceholderTextStrategy::saveOdf( KoShapeSavingContext & context )
{
    KPrPlaceholderStrategy::saveOdf( context );
}

bool KPrPlaceholderTextStrategy::loadOdf( const KoXmlElement & element, KoShapeLoadingContext & context )
{
    KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
    styleStack.save();

    const KoXmlElement * style = 0;
    if ( element.hasAttributeNS( KoXmlNS::draw, "text-style-name" ) ) {
        context.odfLoadingContext().fillStyleStack( element, KoXmlNS::presentation, "style-name", "presentation" );
        const QString styleName = element.attributeNS( KoXmlNS::draw, "text-style-name", QString() );
        style = context.odfLoadingContext().stylesReader().findStyle( styleName, "paragraph", context.odfLoadingContext().useStylesAutoStyles() );
    }
    else {
        const QString styleName = element.attributeNS( KoXmlNS::presentation, "style-name", QString() );
        style = context.odfLoadingContext().stylesReader().findStyle( styleName, "presentation", context.odfLoadingContext().useStylesAutoStyles() );
    }

    if ( style ) {
        KoParagraphStyle paragraphStyle;
        paragraphStyle.loadOdf( style, context.odfLoadingContext() );

        KoShapeFactory *factory = KoShapeRegistry::instance()->value( "TextShapeID" );
        Q_ASSERT( factory );
        m_textShape = factory->createDefaultShapeAndInit( context.dataCenterMap() );

        KoTextShapeData * shapeData = qobject_cast<KoTextShapeData*>(  m_textShape->userData() );
        QTextDocument * document = shapeData->document();
        QTextCursor cursor( document );
        QTextBlock block = cursor.block();
        paragraphStyle.applyStyle( block, false );
        cursor.insertText( text() );
        shapeData->foul();
    }

    styleStack.restore();
    return true;
}

void KPrPlaceholderTextStrategy::init( const QMap<QString, KoDataCenter *> & dataCenterMap )
{
    KoShapeFactory *factory = KoShapeRegistry::instance()->value( "TextShapeID" );
    Q_ASSERT( factory );
    m_textShape = factory->createDefaultShapeAndInit( dataCenterMap );

    KoTextShapeData * shapeData = qobject_cast<KoTextShapeData*>(  m_textShape->userData() );
    QTextDocument * document = shapeData->document();
    QTextCursor cursor( document );
    cursor.insertText( text() );
}

KoShapeUserData * KPrPlaceholderTextStrategy::userData() const
{
    return m_textShape ? m_textShape->userData() : 0;
}
