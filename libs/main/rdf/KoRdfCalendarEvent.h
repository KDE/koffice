/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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

#ifndef __rdf_KoRdfCalendarEvent_h__
#define __rdf_KoRdfCalendarEvent_h__

#include "KoRdfSemanticItem.h"
#include <QSharedPointer>

namespace KCal
{
    class Event;
}

class KoRdfCalendarEventPrivate;

/**
 * @short Calendar event information from Rdf (ical/vevent).
 * @author Ben Martin <ben.martin@kogmbh.com>
 *
 * see the following places for more information
 * http://www.w3.org/TR/rdfcal/
 * http://www.w3.org/2002/12/cal/
 * http://www.w3.org/2002/12/cal/test/
 *
 */
class KoRdfCalendarEvent : public KoRdfSemanticItem
{
    Q_OBJECT

public:
    KoRdfCalendarEvent(QObject *parent, KoDocumentRdf *m_rdf = 0);
    KoRdfCalendarEvent(QObject *parent, KoDocumentRdf *m_rdf, Soprano::QueryResultIterator &it);
    virtual ~KoRdfCalendarEvent();

    // inherited and reimplemented...

    virtual void exportToFile(const QString& fileName = QString()) const;
    virtual void importFromData(const QByteArray& ba, KoDocumentRdf *m_rdf = 0, KoCanvasBase *host = 0);
    virtual QWidget* createEditor(QWidget *parent);
    virtual void updateFromEditorData();
    virtual KoRdfSemanticTreeWidgetItem* createQTreeWidgetItem(QTreeWidgetItem *parent = 0);
    virtual Soprano::Node linkingSubject() const;
    virtual void setupStylesheetReplacementMapping(QMap<QString, QString> &m);
    virtual void exportToMime(QMimeData *md) const;
    virtual QList<KoSemanticStylesheet*> stylesheets() const;
    virtual QString className() const;

    /**
     * Save ourself to the users KDE events calendar.
     */
    virtual void saveToKCal();

    // accessor methods...

    virtual QString name() const;
    QString location() const;
    QString summary() const;
    QString uid() const;
    KDateTime start() const;
    KDateTime end() const;

private:
    KCal::Event *toKEvent() const;
    void fromKEvent(KCal::Event *e);
private:
    Q_DECLARE_PRIVATE(KoRdfCalendarEvent);
};
#endif
