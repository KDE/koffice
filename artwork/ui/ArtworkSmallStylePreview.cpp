/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.t-com.hr)
   Copyright (c) 2005 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (C) 2006 Tim Beaulen <tbscope@gmail.com>
   Copyright (c) 2008 Jan Hambrecht <jaham@gmx.net>

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

#include "ArtworkSmallStylePreview.h"
#include <ArtworkGradientHelper.h>
#include <KGradientBackground.h>
#include <KCanvasBase.h>
#include <KToolManager.h>
#include <KCanvasController.h>
#include <KShapeManager.h>
#include <KShape.h>
#include <KSelection.h>
#include <KLineBorder.h>

#include <KLocale>
#include <KGlobalSettings>

#include <QtGui/QColor>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtGui/QGridLayout>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtCore/QPointF>
#include <QtCore/QRectF>

#define FRAMEWIDTH 75
#define FRAMEHEIGHT 15

class ArtworkFillStyleWidget : public QPushButton {
public:
    ArtworkFillStyleWidget(QWidget * parent)
            : QPushButton(parent), m_fill(0), m_checkerPainter(5) {
        setCursor(Qt::PointingHandCursor);
        setToolTip(i18n("Press to apply fill to selection"));
    }

    virtual ~ArtworkFillStyleWidget() {
        if (m_fill && !m_fill->deref())
            delete m_fill;
    }

    void setFill(KShapeBackgroundBase * fill) {
        if (fill != m_fill) {
            if (m_fill && !m_fill->deref())
                delete m_fill;
            m_fill = fill;
            if (m_fill)
                m_fill->ref();
        }

        update();
    }
protected:
    virtual void paintEvent(QPaintEvent* event) {
        QPainter painter(this);
        painter.setClipRect(event->rect());

        if (m_fill) {
            m_checkerPainter.paint(painter, rect());

            KGradientBackground * gradientFill = dynamic_cast<KGradientBackground*>(m_fill);
            if (gradientFill) {
                const QGradient * gradient = gradientFill->gradient();
                QGradient * defGradient = ArtworkGradientHelper::defaultGradient(gradient->type(), gradient->spread(), gradient->stops());
                QBrush brush(*defGradient);
                delete defGradient;
                painter.setBrush(brush);
                painter.setPen(Qt::NoPen);
                painter.drawRect(rect());
            } else {
                // use the background to draw
                painter.setPen(Qt::NoPen);
                QPainterPath p;
                p.addRect(rect());
                m_fill->paint(painter, p);
            }
        } else {
            painter.setFont(KGlobalSettings::smallestReadableFont());
            painter.setBrush(Qt::black);
            painter.setPen(Qt::black);
            painter.drawText(rect(), Qt::AlignCenter, i18nc("The style has no fill", "None"));
        }

        painter.end();

        //QPushButton::paintEvent( event );
    }

private:
    KShapeBackgroundBase * m_fill; ///< the fill to preview
    KoCheckerBoardPainter m_checkerPainter;
};

class ArtworkStrokeStyleWidget : public QPushButton {
public:
    ArtworkStrokeStyleWidget(QWidget * parent)
            : QPushButton(parent), m_stroke(0), m_checkerPainter(5) {
        setCursor(Qt::PointingHandCursor);
        setToolTip(i18n("Press to apply stroke to selection"));
    }

    virtual ~ArtworkStrokeStyleWidget() {
        if (m_stroke && !m_stroke->deref())
            delete m_stroke;
    }

    void setStroke(KShapeBorderBase * stroke) {
        if (stroke != m_stroke) {
            if (m_stroke && !m_stroke->deref())
                delete m_stroke;
            m_stroke = stroke;
            if (m_stroke)
                m_stroke->ref();
        }
        update();
    }
protected:
    virtual void paintEvent(QPaintEvent* event) {
        QPainter painter(this);
        painter.setClipRect(event->rect());

        if (m_stroke) {
            m_checkerPainter.paint(painter, rect());
            const KLineBorder * line = dynamic_cast<const KLineBorder*>(m_stroke);
            if (line) {
                painter.setPen(Qt::NoPen);
                QBrush brush = line->pen().brush();
                if (brush.gradient()) {
                    QGradient * defGradient = ArtworkGradientHelper::defaultGradient(brush.gradient()->type(), brush.gradient()->spread(), brush.gradient()->stops());
                    QBrush brush(*defGradient);
                    delete defGradient;
                    painter.setBrush(brush);
                    painter.setPen(Qt::NoPen);
                    painter.drawRect(rect());
                } else if (brush.style() == Qt::TexturePattern) {
                    painter.fillRect(rect(), brush);
                } else {
                    painter.fillRect(rect(), QBrush(line->pen().color()));
                }
            } else {
                painter.setFont(KGlobalSettings::smallestReadableFont());
                painter.setBrush(Qt::black);
                painter.setPen(Qt::black);
                painter.drawText(rect(), Qt::AlignCenter, i18nc("The style has a custom stroking", "Custom"));
            }
        } else {
            painter.setFont(KGlobalSettings::smallestReadableFont());
            painter.setBrush(Qt::black);
            painter.setPen(Qt::black);
            painter.drawText(rect(), Qt::AlignCenter, i18nc("The style has no stroking", "None"));
        }

        painter.end();

        //QPushButton::paintEvent( event );
    }

private:
    KShapeBorderBase * m_stroke; ///< the stroke to preview
    KoCheckerBoardPainter m_checkerPainter;
};

ArtworkSmallStylePreview::ArtworkSmallStylePreview(QWidget* parent)
        : QWidget(parent)
{
    setFont(KGlobalSettings::smallestReadableFont());

    /* Create widget layout */
    QGridLayout *layout = new QGridLayout(this);
    QLabel * strokeLabel = new QLabel(i18n("Stroke:"), this);
    strokeLabel->setMinimumHeight(FRAMEHEIGHT);
    m_strokeFrame = new ArtworkStrokeStyleWidget(this);
    m_strokeFrame->setMinimumSize(QSize(FRAMEWIDTH, FRAMEHEIGHT));

    QLabel * fillLabel = new QLabel(i18n("Fill:"), this);
    fillLabel->setMinimumHeight(FRAMEHEIGHT);
    m_fillFrame = new ArtworkFillStyleWidget(this);
    m_fillFrame->setMinimumSize(QSize(FRAMEWIDTH, FRAMEHEIGHT));

    layout->addWidget(strokeLabel, 0, 0);
    layout->addWidget(m_strokeFrame, 0, 1);
    layout->addWidget(fillLabel, 1, 0);
    layout->addWidget(m_fillFrame, 1, 1);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setVerticalSpacing(0);

    setLayout(layout);

    connect(KToolManager::instance(), SIGNAL(changedCanvas(const KCanvasBase *)),
            this, SLOT(canvasChanged(const KCanvasBase *)));
    connect(m_strokeFrame, SIGNAL(clicked()), this, SIGNAL(strokeApplied()));
    connect(m_fillFrame, SIGNAL(clicked()), this, SIGNAL(fillApplied()));
}

ArtworkSmallStylePreview::~ArtworkSmallStylePreview()
{
}

void ArtworkSmallStylePreview::canvasChanged(const KCanvasBase *canvas)
{
    if (canvas) {
        connect(canvas->shapeManager(), SIGNAL(selectionChanged()),
                this, SLOT(selectionChanged()));
        connect(canvas->shapeManager(), SIGNAL(selectionContentChanged()),
                this, SLOT(selectionChanged()));
    }
    selectionChanged();
}

void ArtworkSmallStylePreview::selectionChanged()
{
    KCanvasController * controller = KToolManager::instance()->activeCanvasController();
    if (! controller || ! controller->canvas()) {
        m_fillFrame->setFill(0);
        m_strokeFrame->setStroke(0);
        QWidget::update();
        return;
    }

    KShape * shape = controller->canvas()->shapeManager()->selection()->firstSelectedShape();
    if (shape) {
        m_fillFrame->setFill(shape->background());
        m_strokeFrame->setStroke(shape->border());
    } else {
        m_fillFrame->setFill(0);
        m_strokeFrame->setStroke(0);
    }
    QWidget::update();
}

#include "ArtworkSmallStylePreview.moc"

