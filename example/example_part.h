/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>

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
*/

#ifndef EXAMPLE_PART_H
#define EXAMPLE_PART_H

#include <koDocument.h>

class ExamplePart : public KoDocument
{
    Q_OBJECT
public:
    ExamplePart( QWidget *parentWidget = 0, const char *widgetName = 0, QObject* parent = 0, const char* name = 0, bool singleViewMode = false );

    virtual KoMainWindow* createShell();

    virtual void paintContent( QPainter& painter, const QRect& rect, bool transparent = FALSE );

    virtual bool initDoc();

    virtual bool loadXML( QIODevice *, const QDomDocument & );
    virtual QDomDocument saveXML();

    virtual QCString mimeType() const;

protected:
    virtual KoView* createViewInstance( QWidget* parent, const char* name );
};

#endif
