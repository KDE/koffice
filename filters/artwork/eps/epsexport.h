/* This file is part of the KDE project
   Copyright (C) 2002, The Artwork Developers

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

#ifndef EPSEXPORT_H
#define EPSEXPORT_H


#include <KoFilter.h>

#include "vvisitor.h"
#include <QTextStream>
#include <QVariantList>

class QTextStream;
class VColor;
class VPath;
class ArtworkDocument;
class VFill;
class VGroup;
class VLayer;
class VSubpath;
class VStroke;
class VText;


class EpsExport : public KoFilter, private VVisitor
{
    Q_OBJECT

public:
    EpsExport(QObject* parent, const QVariantList&);
    virtual ~EpsExport() {}

    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);

private:
    virtual void visitVPath(VPath& composite);
    virtual void visitVDocument(ArtworkDocument& document);
    virtual void visitVSubpath(VSubpath& path);
    virtual void visitVText(VText& text);

    void getStroke(const VStroke& stroke);
    void getFill(const VFill& fill);
    void getColor(const VColor& color);

    QTextStream* m_stream;

    uint m_psLevel;
};

#endif

