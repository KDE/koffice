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

#include "RdfCalendarEvent.h"
#include "KoDocumentRdf.h"
#include "KoDocumentRdf_p.h"

#include <QUuid>
#include <QTemporaryFile>
#include <kdebug.h>
#include <kfiledialog.h>
#include <KMessageBox>

// calendars
#ifdef KDEPIMLIBS_FOUND
#include <kcal/calendarresources.h>
#include <kcal/calendarlocal.h>
#include <kcal/vcalformat.h>
#include <ksystemtimezone.h>
#include <KConfigGroup>
#endif

#include <ui_RdfCalendarEventEditWidget.h>

using namespace Soprano;

class RdfCalendarEvent::Private
{
public:
    Soprano::Node m_linkSubject;
    QString m_location;
    QString m_summary;
    QString m_uid;
    KDateTime m_dtstart;
    KDateTime m_dtend;
    KDateTime::Spec m_startTimespec;
    KDateTime::Spec m_endTimespec;

    Ui::RdfCalendarEventEditWidget editWidget;
};

RdfCalendarEvent::RdfCalendarEvent(QObject *parent, KoDocumentRdf *m_rdf)
        : RdfSemanticItem(parent, m_rdf)
        , d(new Private())
{
    d->m_startTimespec = KSystemTimeZones::local();
    d->m_endTimespec = KSystemTimeZones::local();
}

static KDateTime VEventDateTimeToKDateTime(const QString &s, KDateTime::Spec &tz)
{
    kDebug(30015) << "top... tz.offset:" << tz.timeZone().currentOffset();

    if (s.endsWith("Z")) {
        tz = KSystemTimeZones::zone("UTC");
        kDebug(30015) << "tz.offset:" << tz.timeZone().currentOffset();
        kDebug(30015) << "new date string:" << s;
    }

    KDateTime ret = KDateTime::fromString(s, "yyyyMMddTHHmmss");
    if (!ret.isValid()) {
        // "2003-01-08T13:00:00"
        kDebug(30015) << "parsing dateThh:mm format...from input:" << s;
        ret = KDateTime::fromString(s, KDateTime::ISODate);
    }

    //
    // Parsed as UTC, must now adjust for given timezone
    //
    if (ret.isValid() && tz.timeZone().currentOffset()) {
        ret.setTimeSpec(tz);
    }

    //
    // convert to local tz for ease of editing.
    //
    ret = ret.toLocalZone();
    tz = KSystemTimeZones::local();
    kDebug(30015) << "date string:" << s << "\n"
        << " is valid:" << ret.isValid() << "\n"
        << " parsed:" << ret << "\n"
        << " time.tz.offset:" << ret.timeZone().currentOffset()
        << " tz.offset:" << tz.timeZone().currentOffset();
    return ret;
}

static KTimeZone toKTimeZone(Soprano::Node n)
{
    QString dt = n.dataType().toString();
    dt.replace(QRegExp("#tz$"), "");
    int idx = dt.lastIndexOf('/');
    if (idx > 0) {
        idx = dt.lastIndexOf('/', idx - 1);
    }
    if (idx > 0) {
        dt = dt.mid(idx + 1);
    }
    KTimeZone kt = KSystemTimeZones::zone(dt);
    kDebug(30015) << "input:" << n.dataType().toString()
        << " output tz.valid:" << kt.isValid()
        << " timezone:" << dt;
    if (!kt.isValid()) {
        // UTC "Zulu" Time
        if (dt == "2001/XMLSchema#dateTime" && n.toString().endsWith("Z")) {
            kDebug(30015) << "input:" << n.dataType().toString()
            << " is UTC...";
            kt = KSystemTimeZones::zone("UTC");
        }
    }

    if (!kt.isValid()) {
        kt = KSystemTimeZones::zone("UTC");
    }
    return kt;
}

RdfCalendarEvent::RdfCalendarEvent(QObject *parent, KoDocumentRdf *m_rdf, Soprano::QueryResultIterator &it)
        : RdfSemanticItem(parent, m_rdf, it)
        , d(new Private())
{
    d->m_location = optionalBindingAsString(it, "location");
    d->m_summary = optionalBindingAsString(it, "summary");
    d->m_uid = optionalBindingAsString(it, "uid");
    d->m_linkSubject = it.binding("ev");
    // floating time is the default
    d->m_startTimespec = KSystemTimeZones::local();
    d->m_endTimespec = KSystemTimeZones::local();
    // check for timezones in the type of each date-time binding.
    d->m_startTimespec = toKTimeZone(it.binding("dtstart"));
    d->m_endTimespec = toKTimeZone(it.binding("dtend"));
    d->m_dtstart = VEventDateTimeToKDateTime(it.binding("dtstart").toString(),
                   d->m_startTimespec);
    d->m_dtend = VEventDateTimeToKDateTime(it.binding("dtend").toString(),
                                           d->m_endTimespec);
    kDebug(30015) << "RdfCalendarEvent() start:" << d->m_dtstart
        << " end:" << d->m_dtend;
    kDebug(30015) << "RdfCalendarEvent() long:" << optionalBindingAsString(it, "long")
        << " lat:" << optionalBindingAsString(it, "lat");
    kDebug(30015) << "RdfCalendarEvent() context-direct:" << it.binding("graph").toString();
    kDebug(30015) << "RdfCalendarEvent() context():" << context().toString();
    kDebug(30015) << "d->m_startTimespec.offset:" << d->m_startTimespec.timeZone().currentOffset();
    kDebug(30015) << "dtstart:" << d->m_dtstart.date();
    kDebug(30015) << "dtstart:" << d->m_dtstart.time();
    kDebug(30015) << "dtend:" << d->m_dtend.date();
    kDebug(30015) << "dtend:" << d->m_dtend.time();
}

RdfCalendarEvent::~RdfCalendarEvent()
{
}

QWidget* RdfCalendarEvent::createEditor(QWidget *parent)
{
    QWidget *ret = new QWidget(parent);

    kDebug(30015) << "createEditor()";
    kDebug(30015) << "linkingSubject:" << linkingSubject().toString();
    kDebug(30015) << "context:" << context().toString();
    d->editWidget.setupUi(ret);
    d->editWidget.summary->setText(d->m_summary);
    d->editWidget.location->setText(d->m_location);
    enum {
        // TODO CamelCase
        COL_AREA = 0,
        COL_REGION,
        COL_COMMENT,
        COL_COUNT
    };
    d->editWidget.tz->sortItems(COL_REGION, Qt::AscendingOrder);
    d->editWidget.tz->setColumnHidden(COL_COMMENT, true);
    d->editWidget.tz->header()->resizeSections(QHeaderView::ResizeToContents);
    d->editWidget.tz->headerItem()->setText(COL_AREA,   i18n("Area"));
    d->editWidget.tz->headerItem()->setText(COL_REGION, i18n("Region"));
    d->editWidget.startDate->setDate(d->m_dtstart.date());
    d->editWidget.endDate->setDate(d->m_dtend.date());
    d->editWidget.startTime->setTime(d->m_dtstart.time());
    d->editWidget.endTime->setTime(d->m_dtend.time());

    kDebug(30015) << "summary:" << d->m_summary;
    kDebug(30015) << "location:" << d->m_location;
    kDebug(30015) << "dtstart:" << d->m_dtstart.date();
    kDebug(30015) << "dtstart:" << d->m_dtstart.time();
    kDebug(30015) << "dtend:" << d->m_dtend.date();
    kDebug(30015) << "dtend:" << d->m_dtend.time();
    return ret;
}

void RdfCalendarEvent::updateFromEditorData()
{
    QString predBase = "http://www.w3.org/2002/12/cal/icaltzd#";
    if (!d->m_linkSubject.isValid()) {
        d->m_linkSubject = createNewUUIDNode();
    }
    if (d->m_uid.size() <= 0) {
        d->m_uid = QUuid::createUuid().toString();
    }
    kDebug(30015) << "RdfCalendarEvent::updateFromEditorData()";
    kDebug(30015) << "context:" << context().toString();
    kDebug(30015) << "Old summary:" << d->m_summary;
    kDebug(30015) << "New summary:" << d->editWidget.summary->text();
    setRdfType(predBase + "Vevent");
    updateTriple(d->m_summary,   d->editWidget.summary->text(),   predBase + "summary");
    updateTriple(d->m_location,  d->editWidget.location->text(),  predBase + "location");
    updateTriple(d->m_uid,       d->m_uid,                        predBase + "uid");
    KDateTime::Spec startTimespec = KSystemTimeZones::local();
    QStringList selection = d->editWidget.tz->selection();
    if (selection.size() > 0) {
        QString tzString = selection[0];
        kDebug(30015) << "explicit time zone selected... tzString:" << tzString;
        KTimeZone ktz = KSystemTimeZones::zone(tzString);
        startTimespec = KDateTime::Spec(ktz);
        kDebug(30015) << "explicit time zone selected...startTimespec:" << ktz.name();
    }
    KDateTime::Spec endTimespec = startTimespec;
    d->m_startTimespec = startTimespec;
    d->m_endTimespec   = endTimespec;
    KDateTime dtstart(d->editWidget.startDate->date(), d->editWidget.startTime->time(), d->m_startTimespec);
    KDateTime dtend(d->editWidget.endDate->date(),   d->editWidget.endTime->time(),   d->m_endTimespec);
    kDebug(30015) << "d->m_startTimespec.offset:" << d->m_startTimespec.timeZone().currentOffset();
    kDebug(30015) << "date:" << d->editWidget.startDate->date();
    kDebug(30015) << "time:" << d->editWidget.startTime->time();
    kDebug(30015) << "dtstart:" << dtstart;
    kDebug(30015) << "qdtstart:" << dtstart.dateTime();
    LiteralValue lv(dtstart.dateTime());
    Node n = Node::createLiteralNode(lv);
    kDebug(30015) << "soprano::node:" << n.toString();
    updateTriple(d->m_dtstart,   dtstart,  predBase + "dtstart");
    updateTriple(d->m_dtend,     dtend,    predBase + "dtend");
    if (m_rdf) {
        m_rdf->emitSemanticObjectUpdated(this);
    }
}

RdfSemanticTreeWidgetItem *RdfCalendarEvent::createQTreeWidgetItem(QTreeWidgetItem* parent)
{
    RdfCalendarEventTreeWidgetItem *item  =
        new RdfCalendarEventTreeWidgetItem(parent, this);
    return item;
}

Soprano::Node RdfCalendarEvent::linkingSubject() const
{
    kDebug(30015) << "linkingSubject() subj:" << d->m_linkSubject;
    return d->m_linkSubject;
}

void RdfCalendarEvent::setupStylesheetReplacementMapping(QMap<QString, QString> &m)
{
    m["%UID%"] = d->m_uid;
    m["%START%"] = KGlobal::locale()->formatDateTime(d->m_dtstart.dateTime());
    m["%END%"] = KGlobal::locale()->formatDateTime(d->m_dtend.dateTime());
    m["%SUMMARY%"] = d->m_summary;
    m["%LOCATION%"] = d->m_location;
    QMap< QString, KDateTime > times;
    times["START"] = d->m_dtstart;
    times["END"] = d->m_dtend;
    for (QMap< QString, KDateTime >::iterator ti = times.begin(); ti != times.end(); ++ti) {
        QString key = QString("%") + ti.key();
        KDateTime dt = ti.value();
        QDate qd = dt.date();
        m[key + "-YEAR%"] = qd.year();
        m[key + "-MONTH-NUM%"] = qd.month();
        m[key + "-MONTH-NAME%"] = QDate::shortMonthName(qd.month());
        m[key + "-DAY-NUM%"] = qd.day();
        m[key + "-DAY-NAME%"] = QDate::shortDayName(qd.day());
    }
}

void RdfCalendarEvent::exportToMime(QMimeData *md)
{
    QTemporaryFile file;
    if (file.open()) {
        QString fileName = file.fileName();
        kDebug(30015) << "adding text/calendar data, temporary filename:" << file.fileName();
        QString mimeType = "text/calendar";
        exportToFile(file.fileName());
        QByteArray ba = fileToByteArray(fileName);
        md->setData(mimeType, ba);
        kDebug(30015) << "ba.sz:" << ba.size();
    }

    // plain text export for drag to other editors and console
    kDebug(30015) << "adding text/plain data";
    QString data;
    QTextStream oss(&data);
    oss << name() << ", ";
    if (location().size())
        oss << location() << ", ";
    oss << start().toString() << flush;
    md->setText(data);
}

QList<SemanticStylesheet*> &RdfCalendarEvent::stylesheets()
{
    // TODO we probably want a namespace for these (like KoXmlNS).
    static QList<SemanticStylesheet*> stylesheets;
    if (stylesheets.empty()) {
        stylesheets.append(
            new SemanticStylesheet("92f5d6c5-2c3a-4988-9646-2f29f3731f89",
                                   "name", "%NAME%"));
        stylesheets.append(
            new SemanticStylesheet("b4817ce4-d2c3-4ed3-bc5a-601010b33363",
                                   "summary", "%SUMMARY%"));
        stylesheets.append(
            new SemanticStylesheet("853242eb-031c-4a36-abb2-7ef1881c777e",
                                   "summary, location", "%SUMMARY%, %LOCATION%"));
        stylesheets.append(
            new SemanticStylesheet("2d6b87a8-23be-4b61-a881-876177812ad4",
                                   "summary, location, start date/time", "%SUMMARY%, %LOCATION%, %START%"));
        stylesheets.append(
            new SemanticStylesheet("115e3ceb-6bc8-445c-a932-baee09686895",
                                   "summary, start date/time", "%SUMMARY%, %START%"));
    }
    return stylesheets;
}

QList<SemanticStylesheet*> &RdfCalendarEvent::userStylesheets()
{
    static QList<SemanticStylesheet*> ret;
    return ret;
}

QString RdfCalendarEvent::name() const
{
    return d->m_summary;
}

QString RdfCalendarEvent::location()
{
    return d->m_location;
}

QString RdfCalendarEvent::summary()
{
    return d->m_summary;
}

QString RdfCalendarEvent::uid()
{
    return d->m_uid;
}

KDateTime RdfCalendarEvent::start()
{
    return d->m_dtstart;
}

KDateTime RdfCalendarEvent::end()
{
    return d->m_dtend;
}

static KCal::CalendarResources *StdCalendar()
{
    static KCal::CalendarResources *ret = 0;
    if (!ret) {
        ret = new KCal::CalendarResources(KSystemTimeZones::local());
        ret->readConfig();
        KCal::CalendarResourceManager* manager = ret->resourceManager();
        KCal::CalendarResourceManager::Iterator it;
        for (it = manager->begin(); it != manager->end(); ++it) {
            (*it)->load();
        }
    }
    return ret;
}

KCal::Event *RdfCalendarEvent::toKEvent()
{
    KCal::Event *event = new KCal::Event();
    event->setDtStart(start());
    event->setDtEnd(end());
    event->setSummary(summary());
    event->setLocation(location());
    event->setUid(uid());
    return event;
}

void RdfCalendarEvent::fromKEvent(KCal::Event *event)
{
    d->m_dtstart = event->dtStart();
    d->m_dtend   = event->dtEnd();
    d->m_summary = event->summary();
    d->m_location = event->location();
    d->m_uid = event->uid();
    Soprano::Node n = Soprano::LiteralValue(d->m_dtstart.dateTime());
    KDateTime::Spec tz = toKTimeZone(n);
    KDateTime roundTrip = VEventDateTimeToKDateTime(n.toString(), tz);

    kDebug(30015) << "summary:" << d->m_summary;
    kDebug(30015) << "location:" << d->m_location;
    kDebug(30015) << "uid:" << d->m_uid;
    kDebug(30015) << "dtstart:" << d->m_dtstart;
    kDebug(30015) << "dtstart.offset:" << d->m_dtstart.timeZone().currentOffset();
    kDebug(30015) << "dtstart.utc:" << d->m_dtstart.toUtc();
    kDebug(30015) << "  local.offset:" << KSystemTimeZones::local().currentOffset();
    kDebug(30015) << "dtstart.roundTrip:" << roundTrip;
    kDebug(30015) << "dtend:" << d->m_dtend;
    kDebug(30015) << "dtstart.rdfnode:" << n;
    kDebug(30015) << "dtstart.roundTrip.offset:" << tz.timeZone().currentOffset();
}

void RdfCalendarEvent::saveToKCal()
{
#ifdef KDEPIMLIBS_FOUND
    KCal::CalendarResources *calendarResource = StdCalendar();
    calendarResource->load();
    KCal::Event *event = toKEvent();
    if (calendarResource->addEvent(event)) {
        kDebug(30015) << "Added calendar entry:" << name();
        calendarResource->save();
    } else {
        KMessageBox::error(0, i18n("Could not add entry\n%1", name()));
    }
#endif
}

void RdfCalendarEvent::exportToFile(const QString &fileNameConst)
{
    QString fileName = fileNameConst;
#ifdef KDEPIMLIBS_FOUND

    if (!fileName.size()) {
        fileName = KFileDialog::getSaveFileName(
                       KUrl("kfiledialog:///ExportDialog"),
                       "*.ics|ICalendar files",
                       0,
                       "Export to selected iCal file");

        if (!fileName.size()) {
            kDebug(30015) << "no filename given, cancel export..";
            return;
        }
    }
    KCal::CalendarLocal cal(KSystemTimeZones::local());
    cal.addEvent(toKEvent());
    if (!cal.save(fileName)) {
        KMessageBox::error(0, i18n("Could not save iCal file\n%1", fileName));
    }
    kDebug(30015) << "wrote to export file:" << fileName;
#endif
}

void RdfCalendarEvent::importFromData(const QByteArray &ba, KoDocumentRdf *_rdf, KoCanvasBase *host)
{
    kDebug(30015) << "data.sz:" << ba.size();
    kDebug(30015) << "_rdf:" << _rdf;
    if (_rdf) {
        m_rdf = _rdf;
    }
    KCal::VCalFormat v;
    KCal::Calendar *cal = new KCal::CalendarLocal(KSystemTimeZones::local());
    bool rc = v.fromRawString(cal, ba);
    kDebug(30015) << "parse rc:" << rc;
    KCal::Event::List events = cal->rawEvents();
    kDebug(30015) << "found event count:" << events.size();
    if (events.size() >= 1) {
        KCal::Event* e = events[0];
        fromKEvent(e);
    }
    delete cal;
    importFromDataComplete(ba, m_rdf, host);
}
