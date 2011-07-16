/* This file is part of the KDE project
 *
 * Copyright (C) 2010 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SCFADEOVERCOLORSTRATEGY_H
#define SCFADEOVERCOLORSTRATEGY_H

#include "pageeffects/SCPageEffectStrategy.h"

#include <QColor>

class SCFadeOverColorStrategy : public SCPageEffectStrategy
{
public:
    SCFadeOverColorStrategy();
    virtual ~SCFadeOverColorStrategy();

    virtual void setup(const SCPageEffect::Data &data, QTimeLine &timeLine);

    virtual void paintStep(QPainter &p, int currPos, const SCPageEffect::Data &data);

    virtual void next(const SCPageEffect::Data &data);

    virtual void finish(const SCPageEffect::Data &data);

    // reimplemented
    virtual void saveOdfSmilAttributes(KXmlWriter & xmlWriter) const;

    // reimplemented
    virtual void saveOdfSmilAttributes(KOdfGenericStyle & style) const;

    // reimplemented
    virtual void loadOdfSmilAttributes(const KXmlElement & element);

private:
    QColor m_fadeColor;
};

#endif /* SCFADEOVERCOLORSTRATEGY_H */
