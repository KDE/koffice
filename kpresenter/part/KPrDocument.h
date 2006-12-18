/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KPRDOCUMENT_H
#define KPRDOCUMENT_H

#include <QObject>

#include <KoDocument.h>
#include <KoShape.h>
class KCommand;
class KCommandHistory;

class KPrPage;

class KPrDocument : public KoDocument
{
    Q_OBJECT
public:
    KPrDocument( QWidget* parentWidget, QObject* parent, bool singleViewMode = false );
    ~KPrDocument();

    void paintContent( QPainter &painter, const QRect &rect, bool transparent = false,
                       double zoomX = 1.0, double zoomY = 1.0 );

    bool loadXML( QIODevice *, const KoXmlDocument & doc );
    bool loadOasis( const KoXmlDocument & doc, KoOasisStyles& oasisStyles,
                    const KoXmlDocument & settings, KoStore* store );

    bool saveOasis( KoStore* store, KoXmlWriter* manifestWriter );

    void addCommand( KCommand* command, bool execute );
    
    KPrPage* pageByIndex(int index);
    void addShapeToViews(KPrPage *page, KoShape *shape);
    void removeShapeFromViews(KPrPage* page, KoShape* shape);

protected:
    KoView * createViewInstance( QWidget *parent );

private:
    KCommandHistory * m_commandHistory;
    QList<KPrPage*> m_pageList;
};

#endif /* KPRDOCUMENT_H */
