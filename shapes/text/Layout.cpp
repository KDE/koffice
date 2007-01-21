/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "Layout.h"
#include "ListItemsHelper.h"

#include <KoTextDocumentLayout.h>
#include <KoTextShapeData.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>
#include <KoListStyle.h>
#include <KoStyleManager.h>
#include <KoTextBlockData.h>
#include <KoTextBlockBorderData.h>
#include <KoInsets.h>
#include <KoShape.h>

#include <kdebug.h>
#include <QTextList>

// TODO  get rid of this Private.  A dpointer in a plugin doesn't make sense.
class LayoutPrivate {
public:
    LayoutPrivate() : blockData(0), data(0), reset(true), isRtl(false)  { }
    KoStyleManager *styleManager;

    double y;
    QTextBlock block;
    KoTextBlockData *blockData;
    QTextBlockFormat format;
    QTextBlock::Iterator fragmentIterator;
    KoTextShapeData *data;
    bool newShape, newParag, reset, isRtl;
    KoInsets borderInsets;
    KoInsets shapeBorder;
    KoTextDocumentLayout *parent;
};

// ---------------- layout helper ----------------
Layout::Layout(KoTextDocumentLayout *parent) {
    d = new LayoutPrivate();
    d->parent = parent;
    layout = 0;
}

bool Layout::start() {
    if(d->reset)
        resetPrivate();
    else if(shape)
        nextParag();
    d->reset = false;
    return !(layout == 0 || d->parent->shapes().count() <= shapeNumber);
}

void Layout::end() {
    if(layout)
        layout->endLayout();
    layout = 0;
}

void Layout::reset() {
    d->reset = true;
}

bool Layout::interrupted() {
    return d->reset;
}

void Layout::setStyleManager(KoStyleManager *sm) {
    d->styleManager = sm;
}

double Layout::width() {
    Q_ASSERT(shape);
    double ptWidth = shape->size().width() - d->format.leftMargin() - d->format.rightMargin();
    if(d->newParag)
        ptWidth -= d->format.textIndent();
    if(d->newParag && d->blockData)
        ptWidth -= d->blockData->counterWidth() + d->blockData->counterSpacing();
    ptWidth -= d->borderInsets.left + d->borderInsets.right + d->shapeBorder.right;
    return ptWidth;
}

double Layout::x() {
    double result = d->newParag?d->format.textIndent():0.0;
    result += d->isRtl ? d->format.rightMargin() : d->format.leftMargin();
    result += listIndent();
    result += d->borderInsets.left + d->shapeBorder.left;
    return result;
}

double Layout::y() {
    return d->y;
}

double Layout::docOffsetInShape() const {
    return d->data->documentOffset();
}

bool Layout::addLine(QTextLine &line) {
    double height = d->format.doubleProperty(KoParagraphStyle::FixedLineHeight);
    bool useFixedLineHeight = height != 0.0;
    bool useFontProperties = d->format.boolProperty(KoParagraphStyle::LineSpacingFromFont);
    if(! useFixedLineHeight) {
        if(useFontProperties)
            height = line.height();
        else {
            if(d->fragmentIterator.atEnd()) // no text in parag.
                height = d->block.charFormat().fontPointSize();
            else {
                // read max font height
                height = qMax(height,
                        d->fragmentIterator.fragment().charFormat().fontPointSize());
                while(! (d->fragmentIterator.atEnd() || d->fragmentIterator.fragment().contains(
                               d->block.position() + line.textStart() + line.textLength() -1))) {
                    d->fragmentIterator++;
                    height = qMax(height, d->fragmentIterator.fragment().charFormat().fontPointSize());
                }
            }
            if(height < 0.01) height = 12; // default size for uninitialized styles.
        }
    }

    if(d->data->documentOffset() + shape->size().height() < d->y + height + d->shapeBorder.bottom) {
//kDebug() << "   NEXT shape" << endl;
        // line does not fit.
        d->data->setEndPosition(d->block.position() + line.textStart()-1);
        nextShape();
        if(d->data)
            d->data->setPosition(d->block.position() + line.textStart());
        return true;
    }

    // add linespacing
    if(! useFixedLineHeight) {
        double linespacing = d->format.doubleProperty(KoParagraphStyle::LineSpacing);;
        if(linespacing == 0.0) { // unset
            int percent = d->format.intProperty(KoParagraphStyle::FixedLineHeight);
            if(percent != 0)
                linespacing = height * ((percent - 100) / 100.0);
            else if(linespacing == 0.0)
                linespacing = height * 0.2; // default
        }
        height += linespacing;
    }

    double minimum = d->format.doubleProperty(KoParagraphStyle::MinimumLineHeight);
    if(minimum > 0.0)
        height = qMax(height, minimum);
    if(qAbs(d->y - line.y()) < 0.126) // rounding problems due to Qt-scribe internally using ints.
        d->y += height;
    else
        d->y = line.y() + height; // The line got a pos <> from y(), follow that lead.
    d->newShape = false;
    d->newParag = false;
    return false;
}

bool Layout::nextParag() {
    if(layout) { // guard against first time
        layout->endLayout();
        d->block = d->block.next();
        double borderBottom = d->y;
        if(!d->newShape) { // only add bottom of prev parag if we did not go to a new shape for this parag.
            if(d->format.pageBreakPolicy() == QTextFormat::PageBreak_AlwaysAfter ||
                    d->format.boolProperty(KoParagraphStyle::BreakAfter)) {
                d->data->setEndPosition(d->block.position()-1);
                nextShape();
                if(d->data)
                    d->data->setPosition(d->block.position());
            }
            d->y += d->borderInsets.bottom;
            borderBottom = d->y; // don't inlude the bottom margin!
            d->y += d->format.bottomMargin();
        }
        if(d->blockData && d->blockData->border())
            d->blockData->border()->setParagraphBottom(borderBottom);
    }
    layout = 0;
    d->blockData = 0;
    if(! d->block.isValid()) {
        QTextBlock block = d->block.previous(); // last correct one.
        d->data->setEndPosition(block.position() + block.length());

        // cleanupShapes();
        return false;
    }
    d->format = d->block.blockFormat();
    d->blockData = dynamic_cast<KoTextBlockData*> (d->block.userData());
    d->isRtl = d->block.text().isRightToLeft();

    // initialize list item stuff for this parag.
    QTextList *textList = d->block.textList();
    if(textList) {
        QTextListFormat format = textList->format();
        int styleId = format.intProperty(KoListStyle::CharacterStyleId);
        KoCharacterStyle *charStyle = 0;
        if(styleId > 0 && d->styleManager)
            charStyle = d->styleManager->characterStyle(styleId);
        if(!charStyle && d->styleManager) { // try the one from paragraph style
            KoParagraphStyle *ps = d->styleManager->paragraphStyle(
                    d->format.intProperty(KoParagraphStyle::StyleId));
            if(ps)
                charStyle = ps->characterStyle();
        }

        if(! (d->blockData && d->blockData->hasCounterData())) {
            QFont font;
            if(charStyle)
                font = QFont(charStyle->fontFamily(), qRound(charStyle->fontPointSize()),
                        charStyle->fontWeight(), charStyle->fontItalic());
            else {
                QTextCursor cursor(d->block);
                font = cursor.charFormat().font();
            }
            ListItemsHelper lih(textList, font);
            lih.recalculate();
            d->blockData = dynamic_cast<KoTextBlockData*> (d->block.userData());
        }
    }

    updateBorders(); // fill the border inset member vars.
    d->y += d->borderInsets.top;

    if(!d->newShape && (d->format.pageBreakPolicy() == QTextFormat::PageBreak_AlwaysBefore ||
            d->format.boolProperty(KoParagraphStyle::BreakBefore))) {
        d->data->setEndPosition(d->block.position()-1);
        nextShape();
        if(d->data)
            d->data->setPosition(d->block.position());
    }
    d->y += topMargin();
    layout = d->block.layout();
    QTextOption options = layout->textOption();
    options.setAlignment(d->format.alignment());
    if(d->isRtl)
        options.setTextDirection(Qt::RightToLeft);
    layout->setTextOption(options);

    layout->beginLayout();
    d->fragmentIterator = d->block.begin();
    d->newParag = true;

    if(textList) {
        // if list set list-indent. Do this after borders init to we can account for them.
        // Also after we account for indents etc so the y() pos is correct.
        if(d->isRtl)
            d->blockData->setCounterPosition(QPointF(shape->size().width() - d->borderInsets.right -
                d->shapeBorder.right - d->format.leftMargin() - d->blockData->counterWidth(), y()));
        else
            d->blockData->setCounterPosition(QPointF(d->borderInsets.left + d->shapeBorder.left +
                        d->format.textIndent() + d->format.leftMargin() , y()));
    }

    return true;
}

double Layout::documentOffsetInShape() {
    return d->data->documentOffset();
}

void Layout::nextShape() {
    d->newShape = true;

    if(d->data) {
// TODO add weduwen wezen algoritm here. May require me to relayout a parag..
        Q_ASSERT(d->data->endPosition() >= d->data->position());
        d->y = d->data->documentOffset() + shape->size().height() + 10.0;
        d->data->wipe();
    }

    shape = 0;
    d->data = 0;

    QList<KoShape *> shapes = d->parent->shapes();
    for(shapeNumber++; shapeNumber < shapes.count(); shapeNumber++) {
        shape = shapes[shapeNumber];
        d->data = dynamic_cast<KoTextShapeData*> (shape->userData());
        if(d->data != 0)
            break;
        shape = 0;
        d->data = 0;
    }

    if(shape == 0)
        return;
    d->data->setDocumentOffset(d->y);
    d->data->faul(); // make dirty since this one needs relayout at this point.
    d->shapeBorder = shape->borderInsets();
    d->y += d->shapeBorder.top;
}

// and the end of text, make sure the rest of the frames have something sane to show.
void Layout::cleanupShapes() {
    int i = shapeNumber + 1;
    QList<KoShape *> shapes = d->parent->shapes();
    while(i < shapes.count())
        cleanupShape(shapes[i++]);
}

void Layout::cleanupShape(KoShape *daShape) {
    KoTextShapeData *textData = dynamic_cast<KoTextShapeData*> (daShape->userData());
    if(textData) {
        textData->setPosition(-1);
        textData->wipe();
    }
    daShape->repaint();
}

double Layout::listIndent() {
    if(d->blockData == 0)
        return 0;
    if(d->isRtl)
        return 0;
    return d->blockData->counterWidth();
}

void Layout::resetPrivate() {
    d->y = 0;
    d->data = 0;
    shape =0;
    layout = 0;
    d->newShape = true;
    d->blockData = 0;
    d->newParag = true;
    d->block = d->parent->document()->begin();

    shapeNumber = 0;
    int lastPos = -1;
    QList<KoShape *> shapes = d->parent->shapes();
    foreach(KoShape *shape, shapes) {
        KoTextShapeData *data = dynamic_cast<KoTextShapeData*> (shape->userData());
        Q_ASSERT(data);
        if(data->isDirty()) {
            // this shape needs to be recalculated.
            data->setPosition(lastPos+1);
            d->block = d->parent->document()->findBlock( lastPos+1 );
            d->y = data->documentOffset();
            d->format = d->block.blockFormat();

            if(shapeNumber == 0) {
                // no matter what the previous data says, just start from zero.
                d->y = 0;
                data->setDocumentOffset(0);
                Q_ASSERT(lastPos == -1);
                break;
            }
            if(d->block.layout() && d->block.layout()->lineCount() > 0) {
                // block has been layouted. So use its offset.
                d->y = d->block.layout()->lineAt(0).position().y();
                if(d->y < data->documentOffset() - 0.126) { // 0.126 to account of rounding in Qt-scribe
                    Q_ASSERT(shapeNumber > 0);
                    // since we only recalc whole parags; we need to go back a little.
                    shapeNumber--;
                    shape = shapes[shapeNumber];
                    data = dynamic_cast<KoTextShapeData*> (shape->userData());
                    d->newShape = false;
                }
                if(d->y > data->documentOffset() + shape->size().height()) {
                    // hang on; this line is explicitly placed outside the shape. Shape is empty!
                    d->y = data->documentOffset();
                    break;
                }
                // in case this parag has a border we have to subtract that as well
                d->blockData = dynamic_cast<KoTextBlockData*> (d->block.userData());
                if(d->blockData && d->blockData->border()) {
                    double top = d->blockData->border()->inset(KoTextBlockBorderData::Top);
                    // but only when this border actually makes us have an indent.
                    if(qAbs(d->blockData->border()->rect().top() + top - d->y) < 1E-10)
                        d->y -= top;
                }
                // subtract the top margins as well.
                d->y -= topMargin();
            }
            break;
        }
        lastPos = data->endPosition();
        shapeNumber++;
    }
    Q_ASSERT(shapeNumber >= 0);
    if(shapes.count() == 0)
        return;
    shape = shapes[shapeNumber];
    d->data = dynamic_cast<KoTextShapeData*> (shape->userData());
    d->shapeBorder = shape->borderInsets();
    if(d->y == 0)
        d->y = d->shapeBorder.top;

   if(! nextParag())
       shapeNumber++;
}

void Layout::updateBorders() {
    d->borderInsets.top = d->format.doubleProperty(KoParagraphStyle::TopPadding);
    d->borderInsets.left = d->format.doubleProperty(KoParagraphStyle::LeftPadding);
    d->borderInsets.bottom = d->format.doubleProperty(KoParagraphStyle::BottomPadding);
    d->borderInsets.right = d->format.doubleProperty(KoParagraphStyle::RightPadding);

    KoTextBlockBorderData border(QRectF(this->x() - listIndent(), d->y + d->borderInsets.top + topMargin(), width(), 1.));
    border.setEdge(border.Left, d->format, KoParagraphStyle::LeftBorderStyle,
        KoParagraphStyle::LeftBorderWidth, KoParagraphStyle::LeftBorderColor,
        KoParagraphStyle::LeftBorderSpacing, KoParagraphStyle::LeftInnerBorderWidth);
    border.setEdge(border.Right, d->format, KoParagraphStyle::RightBorderStyle,
        KoParagraphStyle::RightBorderWidth, KoParagraphStyle::RightBorderColor,
        KoParagraphStyle::RightBorderSpacing, KoParagraphStyle::RightInnerBorderWidth);
    border.setEdge(border.Top, d->format, KoParagraphStyle::TopBorderStyle,
        KoParagraphStyle::TopBorderWidth, KoParagraphStyle::TopBorderColor,
        KoParagraphStyle::TopBorderSpacing, KoParagraphStyle::TopInnerBorderWidth);
    border.setEdge(border.Bottom, d->format, KoParagraphStyle::BottomBorderStyle,
        KoParagraphStyle::BottomBorderWidth, KoParagraphStyle::BottomBorderColor,
        KoParagraphStyle::BottomBorderSpacing, KoParagraphStyle::BottomInnerBorderWidth);

    // check if prev parag had a border.
    QTextBlock prev = d->block.previous();
    KoTextBlockBorderData *prevBorder = 0;
    if(prev.isValid()) {
        KoTextBlockData *bd = dynamic_cast<KoTextBlockData*> (prev.userData());
        if(bd)
            prevBorder = bd->border();
    }
    if(border.hasBorders()) {
        if(d->blockData == 0) {
            d->blockData = new KoTextBlockData();
            d->block.setUserData(d->blockData);
        }

        // then check if we can merge with the previous parags border.
        if(prevBorder && prevBorder->equals(border))
            d->blockData->setBorder(prevBorder);
        else {
            // can't merge; then these are our new borders.
            KoTextBlockBorderData *newBorder = new KoTextBlockBorderData(border);
            d->blockData->setBorder(newBorder);
            if(prevBorder && !d->newShape)
                d->y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        }
        d->blockData->border()->applyInsets(d->borderInsets, d->y + d->borderInsets.top, false);
    }
    else { // this parag has no border.
        if(prevBorder && !d->newShape)
            d->y += prevBorder->inset(KoTextBlockBorderData::Bottom);
        if(d->blockData)
            d->blockData->setBorder(0); // remove an old one, if there was one.
    }
}

double Layout::topMargin() {
    bool allowMargin = true; // wheather to allow margins at top of shape
    if(d->newShape) {
        allowMargin = false; // false by default, but check 2 exceptions.
        if(d->format.boolProperty(KoParagraphStyle::BreakBefore))
            allowMargin = true;
        else if( d->styleManager && d->format.topMargin() > 0) {
            // also allow it when the paragraph has the margin, but the style has a different one.
            KoParagraphStyle *ps = d->styleManager->paragraphStyle(
                    d->format.intProperty(KoParagraphStyle::StyleId));
            if(ps == 0 || ps->topMargin() != d->format.topMargin())
                allowMargin = true;
        }
    }
    if(allowMargin)
        return d->format.topMargin();
    return 0.0;
}

void Layout::draw(QPainter *painter) {
painter->setPen(Qt::black); // TODO use theme color, or a kword wide hardcoded default.
    const QRegion clipRegion = painter->clipRegion();
    // da real work
    QTextBlock block = d->parent->document()->begin();
    bool started=false;
    while(block.isValid()) {
        QTextLayout *layout = block.layout();

        // the following line is simpler, but due to a Qt bug doesn't work. Try to see if enabling this for Qt4.3
        // will not paint all paragraphs.
        //if(!painter->hasClipping() || ! clipRegion.intersect(QRegion(layout->boundingRect().toRect())).isEmpty()) {
        if(layout->lineCount() >= 1) {
            QTextLine first = layout->lineAt(0);
            QTextLine last = layout->lineAt(layout->lineCount()-1);
            QRectF parag(qMin(first.x(), last.x()), first.y(), qMax(first.width(), last.width()), last.y() + last.height());
            KoTextBlockData *blockData = dynamic_cast<KoTextBlockData*> (block.userData());
            if(blockData) {
                KoTextBlockBorderData *border = blockData->border();
                if(blockData->hasCounterData()) {
                    if(block.layout()->textOption().textDirection() == Qt::RightToLeft)
                        parag.setRight(parag.right() + blockData->counterWidth() + blockData->counterSpacing());
                    else
                        parag.setLeft(blockData->counterPosition().x());
                }
                if(border) {
                    KoInsets insets;
                    border->applyInsets(insets, parag.top(), true);
                    parag.adjust(-insets.left, -insets.top, insets.right, insets.bottom);
                }
            }
            if(!painter->hasClipping() || ! clipRegion.intersect(QRegion(parag.toRect())).isEmpty()) {
                started=true;
                painter->save();
                decorateParagraph(painter, block);
                painter->restore();
                layout->draw(painter, QPointF(0,0));
            }
            else if(started) // when out of the cliprect, then we are done drawing.
                return;
        }
        block = block.next();
    }
}

void Layout::decorateParagraph(QPainter *painter, const QTextBlock &block) {
    KoTextBlockData *data = dynamic_cast<KoTextBlockData*> (block.userData());
    if(data == 0)
        return;

    QTextList *list = block.textList();
    if(list && data->hasCounterData()) {
        QTextListFormat listFormat = list->format();
        QTextCharFormat cf;
        bool filled=false;
        if(d->styleManager) {
            const int id = listFormat.intProperty(KoListStyle::CharacterStyleId);
            KoCharacterStyle *cs = d->styleManager->characterStyle(id);
            if(cs) {
                cs->applyStyle(cf);
                filled = true;
            }
        }
        if(! filled) {
            // use first char of block.
            QTextCursor cursor(block); // I know this is longwinded, but just using the blocks
            // charformat does not work, apparantly
            cf = cursor.charFormat();
        }
        if(! data->counterText().isEmpty()) {
            QFont font(cf.font(), d->parent->paintDevice());
            QTextLayout layout(data->counterText(), font, d->parent->paintDevice());
            layout.setCacheEnabled(true);
            QList<QTextLayout::FormatRange> layouts;
            QTextLayout::FormatRange format;
            format.start=0;
            format.length=data->counterText().length();
            format.format = cf;
            layouts.append(format);
            layout.setAdditionalFormats(layouts);

            Qt::Alignment align = static_cast<Qt::Alignment> (listFormat.intProperty(KoListStyle::Alignment));
            if(align == 0)
                align = Qt::AlignLeft;
            else if(align != Qt::AlignAuto)
                align |= Qt::AlignAbsolute;
            QTextOption option( align );
            option.setTextDirection(block.layout()->textOption().textDirection());
            if(option.textDirection() == Qt::RightToLeft || data->counterText().isRightToLeft())
                option.setAlignment(Qt::AlignRight);
            layout.setTextOption(option);
            layout.beginLayout();
            QTextLine line = layout.createLine();
            line.setLineWidth(data->counterWidth() - data->counterSpacing());
            layout.endLayout();
            layout.draw(painter, data->counterPosition());
        }

        KoListStyle::Style listStyle = static_cast<KoListStyle::Style> ( listFormat.style() );
        if(listStyle == KoListStyle::SquareItem || listStyle == KoListStyle::DiscItem ||
                listStyle == KoListStyle::CircleItem || listStyle == KoListStyle::BoxItem) {
            QFontMetricsF fm(cf.font(), d->parent->paintDevice());
#if 0
// helper lines to show the anatomy of this font.
painter->setPen(Qt::green);
painter->drawLine(QLineF(-1, data->counterPosition().y(), 200, data->counterPosition().y()));
painter->setPen(Qt::yellow);
painter->drawLine(QLineF(-1, data->counterPosition().y() + fm.ascent() - fm.xHeight(), 200, data->counterPosition().y() + fm.ascent() - fm.xHeight()));
painter->setPen(Qt::blue);
painter->drawLine(QLineF(-1, data->counterPosition().y() + fm.ascent(), 200, data->counterPosition().y() + fm.ascent()));
painter->setPen(Qt::gray);
painter->drawLine(QLineF(-1, data->counterPosition().y() + fm.height(), 200, data->counterPosition().y() + fm.height()));
#endif

            double width = fm.xHeight();
            double y = data->counterPosition().y() + fm.ascent() - fm.xHeight(); // at top of text.
            int percent = listFormat.intProperty(KoListStyle::BulletSize);
            if(percent > 0)
                width *= percent / 100.0;
            y -= width / 10.; // move it up just slightly
            double x = qMax(1., data->counterPosition().x() + fm.width(listFormat.stringProperty( KoListStyle::ListItemPrefix )));
            switch( listStyle ) {
                case KoListStyle::SquareItem:
                    painter->fillRect(QRectF(x, y, width, width), QBrush(Qt::black));
                    break;
                case KoListStyle::DiscItem:
                    painter->setBrush(QBrush(Qt::black));
                    // fall through!
                case KoListStyle::CircleItem:
                    painter->drawEllipse(QRectF(x, y, width, width));
                    break;
                case KoListStyle::BoxItem:
                    painter->drawRect(QRectF(x, y, width, width));
                    break;
                default:; // others we ignore.
            }
        }
    }

    KoTextBlockBorderData *border = dynamic_cast<KoTextBlockBorderData*> (data->border());
    // TODO make sure we only paint a border-set one time.
    if(border) {
        painter->save();
        border->paint(*painter);
        painter->restore();
    }
}


