/*
    Copyright (C) 2000, S.R.Haque <shaheedhaque@hotmail.com>.
    This file is part of the KDE project

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

DESCRIPTION
*/

#ifndef MSODIMPORT_H
#define MSODIMPORT_H

#include <koFilter.h>
#include <msod.h>

class MSODImport :
    public KoEmbeddingFilter, protected Msod
{
    Q_OBJECT

public:
    MSODImport(
        KoFilter *parent,
        const char *name,
        const QStringList&);
    virtual ~MSODImport();

    virtual KoFilter::ConversionStatus convert( const QCString& from, const QCString& to );

protected:

    virtual void gotEllipse(
        const DrawContext &dc,
        QString type,
        QPoint topLeft,
        QSize halfAxes,
        unsigned startAngle,
        unsigned stopAngle);
    virtual void gotPicture(
        unsigned id,
        QString extension,
        unsigned length,
        const char *data);
    virtual void gotPolygon(
        const DrawContext &dc,
        const QPointArray &points);
    virtual void gotPolyline(
        const DrawContext &dc,
        const QPointArray &points);
    virtual void gotRectangle(
        const DrawContext &dc,
        const QPointArray &points);

private:
    virtual void savePartContents( QIODevice* file );

    // Debug support.
    static const int s_area = 30505;

    void pointArray(
        const QPointArray &points);
    QString m_text;

    // Embedded objects.
    const char* m_embeddeeData;
    int m_embeddeeLength;
};

#endif
