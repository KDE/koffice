/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include <KoTextTool.h>
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>

#include <kdebug.h>
#include <QKeyEvent>
#include <QAction>
#include <QTextBlock>
#include <QTextLayout>
#include <QAbstractTextDocumentLayout>

KoTextTool::KoTextTool(KoCanvasBase *canvas)
: KoTool(canvas)
, m_textShape(0)
, m_textShapeData(0)
{
}

KoTextTool::~KoTextTool() {
}

void KoTextTool::textBold() {
    QTextCharFormat cf = m_caret.charFormat();
    cf.setFontWeight( (cf.fontWeight() <= QFont::Normal) ? QFont::Bold : QFont::Normal );
    m_caret.mergeCharFormat(cf);
}

void KoTextTool::textItalic() {
    QTextCharFormat cf = m_caret.charFormat();
    cf.setFontItalic(! cf.fontItalic());
    m_caret.mergeCharFormat(cf);
}

void KoTextTool::textUnderline() {
    QTextCharFormat cf = m_caret.charFormat();
    cf.setFontUnderline(! cf.fontUnderline());
    m_caret.mergeCharFormat(cf);
}

void KoTextTool::textStrikeOut() {
    QTextCharFormat cf = m_caret.charFormat();
    cf.setFontStrikeOut(! cf.fontStrikeOut());
    m_caret.mergeCharFormat(cf);
}

void KoTextTool::paint( QPainter &painter, KoViewConverter &converter) {
    if(painter.hasClipping()) {
        QRect shape = converter.documentToView(m_textShape->boundingRect()).toRect();
        if(painter.clipRegion().intersect( QRegion(shape) ).isEmpty())
            return;
    }

    painter.setMatrix( painter.matrix() * m_textShape->transformationMatrix(&converter) );
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
    //const QTextDocument *document = m_caret.block().document();

/*
    QAbstractTextDocumentLayout::PaintContext pc;
    pc.cursorPosition = m_caret.position();
    QAbstractTextDocumentLayout::Selection selection;
    selection.cursor = m_caret;
    selection.format.setTextOutline(QPen(Qt::red));
    pc.selections.append(selection);
    document->documentLayout()->draw( &painter, pc);
*/

    QTextBlock block = m_caret.block();
    if(! block.layout())
        return;

#if 0
Hmm, not useful right now due to the implementation of QAbstractTextDocumentLayout
    if(m_caret.selectionStart() != m_caret.selectionEnd()) { // paint selection
        //kDebug(32500) << "Selection: " << m_caret.selectionStart() << "-" << m_caret.selectionEnd() << "\n";
        bool first = true;
        QList<QTextLine> lines;
        QTextBlock block = document->findBlock(m_caret.selectionStart());
        do { // for all textBlocks
            QTextLayout *layout = block.layout();
            if(!block.isValid() || block.position() > m_caret.selectionEnd())
                break;
            if(layout == 0)
                continue;
            layout->setCacheEnabled(true);
kDebug() << " block '" << block.text() << "'" << endl;
            for(int i=0; i < layout->lineCount(); i++) {
                QTextLine line = layout->lineAt(i);
kDebug() << " line: " << line.textStart() << "-" << (line.textStart() + line.textLength()) << endl;
                if(first && line.textStart() + line.textLength() < m_caret.selectionStart())
                    continue;
                lines.append(line);
kDebug() << "    appending" << endl;
                if(line.textStart() + line.textLength() > m_caret.selectionEnd())
                    break;
            }
            first = false;
            block = block.next();
        } while(block.position() + block.length() < m_caret.selectionEnd());

        kDebug(32500) << "found " << lines.count() << " lines that contain the selection" << endl;
        foreach(QTextLine line, lines) {
            if(! line.isValid())
                continue;
            if(painter.clipRegion().intersect(QRegion(line.rect().toRect())).isEmpty())
                continue;
            painter.fillRect(line.rect(), QBrush(Qt::yellow));
        }
    }
#endif

    // paint caret.
    QPen pen(Qt::black);
    if(! m_textShape->hasTransparency()) {
        QColor bg = m_textShape->background().color();
        QColor invert = QColor(255 - bg.red(), 255 - bg.green(), 255 - bg.blue());
        pen.setColor(invert);
    }
    painter.setPen(pen);
    painter.translate(0, -m_textShapeData->documentOffset());
    block.layout()->drawCursor(&painter, QPointF(0,0), m_caret.position() - block.position());
}

void KoTextTool::mousePressEvent( KoPointerEvent *event ) {
    if(! m_textShape->boundingRect().contains(event->point)) {
        QRectF area(event->point, QSizeF(1,1));
        foreach(KoShape *shape, m_canvas->shapeManager()->shapesAt(area, true)) {
            KoTextShape *textShape = dynamic_cast<KoTextShape*> (shape);
            if(textShape) {
                m_textShape = textShape;
                KoTextShapeData *d = static_cast<KoTextShapeData*> (textShape->userData());
                if(d->document() == m_textShapeData->document())
                    break; // stop looking.
            }
        }
        m_textShapeData = static_cast<KoTextShapeData*> (m_textShape->userData());
        // in case its a different doc...
        m_caret = QTextCursor(m_textShapeData->document());
    }

    int position = pointToPosition(event->point);
    repaint();
    m_caret.setPosition(position);
    repaint();
}

int KoTextTool::pointToPosition(const QPointF & point) const {
    QPointF p = m_textShape->convertScreenPos(point);
    int caretPos = m_caret.block().document()->documentLayout()->hitTest(p, Qt::FuzzyHit);
    caretPos = qMax(caretPos, m_textShapeData->position());
    if(m_textShapeData->endPosition() == -1)
        kWarning() << "Clicking in not fully laid-out textframe\n";
    caretPos = qMin(caretPos, m_textShapeData->endPosition());
    return caretPos;
}

void KoTextTool::mouseDoubleClickEvent( KoPointerEvent *event ) {
    // TODO select whole word, or when clicking in between two words select 2 words.
}

void KoTextTool::mouseMoveEvent( KoPointerEvent *event ) {
    useCursor(Qt::IBeamCursor);
    if(event->buttons() == Qt::NoButton)
        return;
    int position = pointToPosition(event->point);
    if(position >= 0) {
        repaint();
        m_caret.setPosition(position, QTextCursor::KeepAnchor);
        repaint();
    }
}

void KoTextTool::mouseReleaseEvent( KoPointerEvent *event ) {
    // TODO
}

void KoTextTool::keyPressEvent(QKeyEvent *event) {
    QTextCursor::MoveOperation moveOperation = QTextCursor::NoMove;
    switch(event->key()) { // map input to moveOperation
/* TODO
 * Look at KoTextView::handleKeyPressEvent
 * Use    if ( KShortcut(  e->key() ) == KStdAccel::deleteWordBack() )  and friends
 */
        case Qt::Key_Backspace:
            useCursor(Qt::BlankCursor);
            m_caret.deletePreviousChar();
            break;
        case Qt::Key_Tab:
            kDebug(32500) << "Tab key pressed";
            break;
        case Qt::Key_Delete:
            useCursor(Qt::BlankCursor);
            m_caret.deleteChar();
            break;
        case Qt::Key_Left:
            if(event->modifiers() & Qt::ControlModifier)
                moveOperation = QTextCursor::WordLeft;
            else
                moveOperation = QTextCursor::Left;
            break;
        case Qt::Key_Up:
            moveOperation = QTextCursor::Up;
            break;
        case Qt::Key_Right:
            if(event->modifiers() & Qt::ControlModifier)
                moveOperation = QTextCursor::NextWord;
            else
                moveOperation = QTextCursor::Right;
            break;
        case Qt::Key_Down:
            moveOperation = QTextCursor::Down;
            break;
        case Qt::Key_PageUp:
            //moveOperation = QTextCursor::Left;
            break;
        case Qt::Key_PageDown:
            //moveOperation = QTextCursor::Left;
            break;
        case Qt::Key_End:
            moveOperation = QTextCursor::EndOfLine;
            break;
        case Qt::Key_Home:
            moveOperation = QTextCursor::StartOfLine;
            break;
        default:
            if((event->modifiers() & (Qt::ControlModifier | Qt::AltModifier)) || event->text().length() == 0) {
                event->ignore();
                return;
            }
            useCursor(Qt::BlankCursor);
            m_caret.insertText(event->text());
    }
    if(moveOperation != QTextCursor::NoMove) {
        repaint();
        m_caret.movePosition(moveOperation,
            (event->modifiers() & Qt::ShiftModifier)?QTextCursor::KeepAnchor:QTextCursor::MoveAnchor);
        repaint();
    }
}

void KoTextTool::keyReleaseEvent(QKeyEvent *event) {
    event->ignore();
}

void KoTextTool::activate (bool temporary) {
    Q_UNUSED(temporary);
    KoSelection *selection = m_canvas->shapeManager()->selection();
    foreach(KoShape *shape, selection->selectedShapes().toList()) {
        m_textShape = dynamic_cast<KoTextShape*> (shape);
        if(m_textShape)
            break;
    }
    if(m_textShape == 0) { // none found
        emit sigDone();
        return;
    }
    foreach(KoShape *shape, selection->selectedShapes().toList()) {
        // deselect others.
        if(m_textShape == shape) continue;
        selection->deselect(shape);
    }
    m_textShapeData = static_cast<KoTextShapeData*> (m_textShape->userData());
    m_textShapeData->document()->setUndoRedoEnabled(true); // allow undo history
    m_caret = QTextCursor(m_textShapeData->document());
    useCursor(Qt::IBeamCursor, true);
    m_textShape->repaint();
}

void KoTextTool::deactivate() {
    m_textShape = 0;
    if(m_textShapeData)
        m_textShapeData->document()->setUndoRedoEnabled(false); // erase undo history.
    m_textShapeData = 0;
}

void KoTextTool::repaint() {
    QTextBlock block = m_caret.block();
    if(block.isValid()) {
        QTextLine tl = block.layout()->lineForTextPosition(m_caret.position() - block.position());
        QRectF repaintRect;
        if(tl.isValid())
            repaintRect = tl.rect();
        else // layouting info was removed already :(
            repaintRect = block.layout()->boundingRect();
        repaintRect.moveTop(repaintRect.y() - m_textShapeData->documentOffset());
        repaintRect.moveTopLeft(repaintRect.topLeft() + m_textShape->position());
        m_canvas->updateCanvas(repaintRect);
    }
}

void KoTextTool::createOptionWidget() {
    class OptionWidget : public QWidget {
      public:
        OptionWidget() {
        }
    };
    QWidget *widget = new OptionWidget();

    QAction *bold = new QAction(widget);
    bold->setText("Bold");
    bold->setShortcut(QKeySequence ("Ctrl+B"));

    QAction *italic = new QAction(widget);
    italic->setText("Italic");
    italic->setShortcut(QKeySequence ("Ctrl+I"));

    QAction *underline = new QAction(widget);
    underline->setText("Underline");
    underline->setShortcut(QKeySequence ("Ctrl+U"));

    widget->addAction(bold);
    widget->addAction(italic);
    widget->addAction(underline);

    connect(bold, SIGNAL(triggered()), this, SLOT( textBold()) );
    connect(italic, SIGNAL(triggered()), this, SLOT( textItalic()) );
    connect(underline, SIGNAL(triggered()), this, SLOT( textUnderline()) );
    m_optionWidget = widget;
}

#include "KoTextTool.moc"
