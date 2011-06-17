/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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
#include "KVariable.h"
#include "KInlineObject_p.h"
#include "KTextDocumentLayout.h"
#include "KTextShapeData.h"

#include <KShape.h>

#include <QTextCursor>
#include <QPainter>
#include <QFontMetricsF>
#include <QTextDocument>
#include <QTextInlineObject>

class KVariablePrivate : public KInlineObjectPrivate
{
public:
    KVariablePrivate(KVariable *qq)
        : KInlineObjectPrivate(qq),
        modified(true)
    {
    }

    QDebug printDebug(QDebug dbg) const
    {
        dbg.nospace() << "KVariable value=" << value;
        return dbg.space();
    }

    QString value;
    bool modified;

    Q_DECLARE_PUBLIC(KVariable)
};

KVariable::KVariable(bool propertyChangeListener)
        : KInlineObject(*(new KVariablePrivate(this)), propertyChangeListener)
{
}

KVariable::~KVariable()
{
}

void KVariable::setValue(const QString &value)
{
    Q_D(KVariable);
    if (d->value == value)
        return;
    d->value = value;
    d->modified = true;
    if (d->document) {
        KTextDocumentLayout *lay = qobject_cast<KTextDocumentLayout*>(d->document->documentLayout());
        if (lay)
            lay->documentChanged(d->positionInDocument, 0, 0);
    }
}

void KVariable::updatePosition(QTextInlineObject object, const QTextCharFormat &format)
{
    Q_UNUSED(object);
    Q_UNUSED(format);
    positionChanged(); // force recalc. Even if the pos may not have actually changed.
}

void KVariable::resize(QTextInlineObject object, const QTextCharFormat &format, QPaintDevice *pd)
{
    Q_D(KVariable);
    if (d->modified == false)
        return;
    Q_ASSERT(format.isCharFormat());
    QFontMetricsF fm(format.font(), pd);
    object.setWidth(fm.width(d->value));
    object.setAscent(fm.ascent());
    object.setDescent(fm.descent());
    d->modified = true;
}

void KVariable::paint(QPainter &painter, QPaintDevice *pd, const QRectF &rect, QTextInlineObject object, const QTextCharFormat &format)
{
    Q_D(KVariable);
    Q_UNUSED(object);

    // TODO set all the font properties from the format (color etc)
    QFont font(format.font(), pd);
    QTextLayout layout(d->value, font, pd);
    layout.setCacheEnabled(true);
    QList<QTextLayout::FormatRange> layouts;
    QTextLayout::FormatRange range;
    range.start = 0;
    range.length = d->value.length();
    range.format = format;
    layouts.append(range);
    layout.setAdditionalFormats(layouts);

    QTextOption option(Qt::AlignLeft | Qt::AlignAbsolute);
    option.setTextDirection(object.textDirection());
    layout.setTextOption(option);
    layout.beginLayout();
    layout.createLine();
    layout.endLayout();
    layout.draw(&painter, rect.topLeft());
}

QString KVariable::value() const
{
    Q_D(const KVariable);
    return d->value;
}
