/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SCPLACEHOLDERTOOL_H
#define SCPLACEHOLDERTOOL_H

#include <KToolBase.h>

class SCPlaceholderTool : public KToolBase
{
    Q_OBJECT
public:
    SCPlaceholderTool(KCanvasBase *canvas);
    virtual ~SCPlaceholderTool();

    virtual void paint(QPainter &painter, const KViewConverter &converter);
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
};

#endif /* SCPLACEHOLDERTOOL_H */
