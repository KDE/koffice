/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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

#ifndef KOTEXTBLOCKDATA_H
#define KOTEXTBLOCKDATA_H

#include <QTextBlockUserData>

#include <kotext_export.h>

class KoTextBlockBorderData;

/**
 * This class is used to store properties for KoText layouting inside Qt QTextBlock
 * instances.
 */
class KOTEXT_EXPORT KoTextBlockData : public QTextBlockUserData {
public:
    KoTextBlockData();
    ~KoTextBlockData();

    /// return if this block has up-to-date counter data
    bool hasCounterData() const;
    /// return the width (in pt) of the counter.
    double counterWidth() const { return m_counterWidth; }
    /// set the width of the counter in pt.
    void setCounterWidth(double width) { m_counterWidth = width; }
    /// return the spacing (in pt) between the counter and the text
    double counterSpacing() const { return m_counterSpacing; }
    /// set the spacing (in pt) between the counter and the text
    void setCounterSpacing(double spacing) { m_counterSpacing = spacing; }
    /// set the exact text that will be painted as the counter
    void setCounterText(const QString &text) { m_counterText = text; }
    /// return the exact text that will be painted as the counter
    const QString &counterText() const { return m_counterText; }

    /**
     * set the text that is used for the counter at this level.
     * If this represents a parag with counter 3.1 then the text is the '1'
     * since the rest is not dependent on this parag, but only its location in the text
     */
    void setPartialCounterText(const QString &text) { m_partialCounterText = text; }
    /// return the partial text for this paragraphs counter
    const QString &partialCounterText() const { return m_partialCounterText; }

    void setCounterPosition(QPointF position) { m_counterPos = position; }
    const QPointF &counterPosition() const { return m_counterPos; }

    void setBorder(KoTextBlockBorderData *border);
    KoTextBlockBorderData *border() const { return m_border; }

private:
    double m_counterWidth;
    double m_counterSpacing;
    QString m_counterText;
    QString m_partialCounterText;
    QPointF m_counterPos;
    KoTextBlockBorderData *m_border;
};

#endif
