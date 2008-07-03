/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@web.de>
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

#ifndef RULER_H
#define RULER_H

#include <KoUnit.h>

#include <QColor>
#include <QLineF>
#include <QMatrix>
#include <QObject>
#include <QPointF>

class QPainter;

/** A Ruler is a user interface element which can be used to change the Spacing and Dimensions in a KOffice document,
 * for example the spacing in a text paragraph, by simply dragging the ruler lacross the screen */
class Ruler : public QObject
{
    Q_OBJECT
public:
    Ruler(QObject *parent);

    ~Ruler() {}

    qreal width() const { return m_width; }
    void setBaseline(const QLineF &baseline);
    void setBaseline(const QPointF &from, const QPointF &to);

    KoUnit unit() const { return m_unit; }
    void setUnit(KoUnit unit);

    qreal value() const;
    QString valueString() const;
    void setValue(qreal value);

    qreal oldValue() const { return m_oldValue; }
    void reset();

    // distance in points between steps when moving the ruler, set this to 0.0 to disable stepping
    qreal stepValue() const { return m_stepValue; }
    void setStepValue(qreal stepValue) { m_stepValue = stepValue; }

    qreal minimumValue() const;
    void setMinimumValue(qreal value);

    qreal maximumValue() const;
    void setMaximumValue(qreal value);

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }
    void hide() { setVisible(false); }
    void show() { setVisible(true); }

    // these options specify how the ruler will be drawn
    enum Options{
        noOptions = 0,
        drawSides = 1<<0
    };

    int options() const { return m_options; }
    void setOptions(int options) { m_options = m_options | options; }

    void moveRuler(const QPointF &point, bool smooth);
    void moveRuler(qreal value, bool smooth);

    bool isActive() const { return m_active; }
    void setActive(bool active);

    bool isHighlighted() const { return m_highlighted; }
    void setHighlighted(bool highlighted);

    QLineF labelConnector() const;

    QColor activeColor() const { return QColor(100, 148, 255); }
    QColor highlightColor() const { return QColor(78, 117, 201); }
    QColor normalColor() const { return QColor(100, 100, 100); }

    void paint(QPainter &painter) const;

    bool hitTest(const QPointF &point) const;

signals:
    // emitted when value has been changed via the user interface
    // (in contrast to via the setValue() method)
    void valueChanged(qreal value);

    // emitted when the ruler needs to be repainted
    void needsRepaint();

protected:
    void setOldValue(qreal value) { m_oldValue = value; }
    void paintArrow(QPainter &painter, const QPointF &tip, const qreal angle, qreal value) const;

    // baseline properties
    QMatrix m_matrix;
    qreal m_width;

    // all values in points
    qreal m_value; // value is distance between baseline and ruler line
    qreal m_oldValue;
    qreal m_stepValue;
    qreal m_minValue;
    qreal m_maxValue;
    KoUnit m_unit;

private:
    // some convenience methods which describe how an arrow should be rendered
    static qreal arrowSize() { return 10.0; }
    static qreal arrowDiagonal() { return arrowSize() / sqrt(2.0) / 2.0; }
    static qreal arrowMinimumValue() { return arrowDiagonal() *2.0 + 2.0; }

    bool m_visible;
    bool m_active;
    bool m_highlighted;
    int m_options;
};

#endif

