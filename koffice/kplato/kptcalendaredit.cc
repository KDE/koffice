/* This file is part of the KDE project
   Copyright (C) 2004 Dag Andersen <danders@get2net.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation;
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kptcalendaredit.h"
#include "kptproject.h"
#include "kptcalendar.h"
#include "kptcalendarpanel.h"
#include "kptmap.h"
#include "intervalitem.h"

#include <qbuttongroup.h>
#include <qheader.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qlineedit.h>
#include <qdatetimeedit.h>
#include <qdatetime.h>
#include <qtabwidget.h>
#include <qtextbrowser.h>

#include <klocale.h>

#include <kabc/addressee.h>
#include <kabc/addresseedialog.h>

#include <qmap.h>

#include <kdebug.h>

namespace KPlato
{

CalendarEdit::CalendarEdit (QWidget *parent, const char */*name*/)
    : CalendarEditBase(parent),
      m_calendar(0)
 {

    clear();
    intervalList->header()->setStretchEnabled(true, 0);
    intervalList->setShowSortIndicator(true);
    intervalList->setSorting(0);

    connect (calendarPanel, SIGNAL(dateChanged(QDate)), SLOT(slotDateSelected(QDate)));
    connect (calendarPanel, SIGNAL(weekdaySelected(int)), SLOT(slotWeekdaySelected(int)));
    connect(calendarPanel, SIGNAL(selectionCleared()), SLOT(slotSelectionCleared()));

    connect (state, SIGNAL(activated(int)), SLOT(slotStateActivated(int)));
    connect (bClear, SIGNAL(clicked()), SLOT(slotClearClicked()));
    connect (bAddInterval, SIGNAL(clicked()), SLOT(slotAddIntervalClicked()));

    connect (bApply, SIGNAL(clicked()), SLOT(slotApplyClicked()));
}

void CalendarEdit::slotStateActivated(int id) {
    //kdDebug()<<k_funcinfo<<"id="<<id<<endl;
    if (id == 0) { // undefined
        startTime->setEnabled(false);
        endTime->setEnabled(false);
        bClear->setEnabled(false);
        bAddInterval->setEnabled(false);
        intervalList->setEnabled(false);
        bApply->setEnabled(true);
    } else if (id == 1) { // non working
        startTime->setEnabled(false);
        endTime->setEnabled(false);
        bClear->setEnabled(false);
        bAddInterval->setEnabled(false);
        intervalList->setEnabled(false);
        bApply->setEnabled(true);
    } else if (id == 2) { //working
        startTime->setEnabled(true);
        endTime->setEnabled(true);
        bClear->setEnabled(true);
        bAddInterval->setEnabled(true);
        intervalList->setEnabled(true);
        bApply->setEnabled(intervalList->firstChild());
    }
}

void CalendarEdit::slotClearClicked() {
    //kdDebug()<<k_funcinfo<<endl;
    intervalList->clear();
    bApply->setEnabled(false);
}
void CalendarEdit::slotAddIntervalClicked() {
    //kdDebug()<<k_funcinfo<<endl;
    intervalList->insertItem(new IntervalItem(intervalList, startTime->time(), endTime->time()));
    bApply->setEnabled(true);
}

//NOTE: enum Map::State must match combobox state!
void CalendarEdit::slotApplyClicked() {
    //kdDebug()<<k_funcinfo<<"("<<m_calendar<<")"<<endl;
    DateMap dates = calendarPanel->selectedDates();
    for(DateMap::iterator it = dates.begin(); it != dates.end(); ++it) {
        QDate date = QDate::fromString(it.key(), Qt::ISODate);
        //kdDebug()<<k_funcinfo<<"Date: "<<date<<endl;
        CalendarDay *calDay = m_calendar->findDay(date);
        if (!calDay) {
            calDay = new CalendarDay(date);
            m_calendar->addDay(calDay);
        }
        calDay->setState(state->currentItem()); //NOTE!!
        calDay->clearIntervals();
        if (calDay->state() == Map::Working) {
            for (QListViewItem *item = intervalList->firstChild(); item; item = item->nextSibling()) {
                //kdDebug()<<k_funcinfo<<"Adding interval: "<<static_cast<IntervalItem *>(item)->interval().first.toString()<<"-"<<static_cast<IntervalItem *>(item)->interval().second.toString()<<endl;
                calDay->addInterval(static_cast<IntervalItem *>(item)->interval());
            }
        }
    }

    IntMap weekdays = calendarPanel->selectedWeekdays();
    for(IntMap::iterator it = weekdays.begin(); it != weekdays.end(); ++it) {
        //kdDebug()<<k_funcinfo<<"weekday="<<it.key()<<endl;
        CalendarDay *weekday = m_calendar->weekday(it.key()-1);
        weekday->setState(state->currentItem());//NOTE!!
        weekday->clearIntervals();
        if (weekday->state() == Map::Working) {
            for (QListViewItem *item = intervalList->firstChild(); item; item = item->nextSibling()) {
                //kdDebug()<<k_funcinfo<<"Adding interval: "<<static_cast<IntervalItem *>(item)->interval().first.toString()<<"-"<<static_cast<IntervalItem *>(item)->interval().second.toString()<<endl;
                weekday->addInterval(static_cast<IntervalItem *>(item)->interval());
            }
        }
    }

    calendarPanel->markSelected(state->currentItem()); //NOTE!!
    emit applyClicked();
    slotCheckAllFieldsFilled();
}

void CalendarEdit::slotCheckAllFieldsFilled() {
    //kdDebug()<<k_funcinfo<<endl;
    if (state->currentItem() == 0 /*undefined*/ ||
        state->currentItem() == 1 /*Non-working*/||
        (state->currentItem() == 2 /*Working*/ && intervalList->firstChild()))
    {
        emit obligatedFieldsFilled(true);
    }
    else if (state->currentItem() == 2 && !intervalList->firstChild())
    {
        emit obligatedFieldsFilled(false);
    }
}

void CalendarEdit::setCalendar(Calendar *cal) {
    m_calendar = cal;
    clear();
    calendarPanel->setCalendar(cal);
}

void CalendarEdit::clear() {
    clearPanel();
    clearEditPart();
}

void CalendarEdit::clearPanel() {
    calendarPanel->clear();
}

void CalendarEdit::clearEditPart() {
    day->setEnabled(true);
    intervalList->clear();
    intervalList->setEnabled(false);
    startTime->setEnabled(false);
    startTime->setTime(QTime(8, 0, 0)); //FIXME
    endTime->setEnabled(false);
    endTime->setTime(QTime(16, 0, 0)); //FIXME

    bAddInterval->setEnabled(false);
    bClear->setEnabled(false);
    bApply->setEnabled(false);
    state->setEnabled(false);
}

void CalendarEdit::slotDateSelected(QDate date) {
    if (m_calendar == 0)
        return;
    //kdDebug()<<k_funcinfo<<"("<<date.toString()<<")"<<endl;
    clearEditPart();
    state->clear();
    state->insertItem(i18n("Undefined"));
    state->insertItem(i18n("Non-working"));
    state->insertItem(i18n("Working"));

    CalendarDay *calDay = m_calendar->findDay(date);
    state->setEnabled(true);
    if (calDay) {
        QPtrListIterator<QPair<QTime, QTime> > it = calDay->workingIntervals();
        for (; it.current(); ++it) {
            IntervalItem *item = new IntervalItem(intervalList, it.current()->first, it.current()->second);
            intervalList->insertItem(item);
        }
        if (calDay->state() == Map::Working) {
            //kdDebug()<<k_funcinfo<<"("<<date.toString()<<") is workday"<<endl;
            state->setCurrentItem(2);
            slotStateActivated(2);
            bApply->setEnabled(calDay->workingIntervals().count() > 0);
        } else if (calDay->state() == Map::NonWorking){
            //kdDebug()<<k_funcinfo<<"("<<date.toString()<<") is holiday"<<endl;
            state->setCurrentItem(1);
            slotStateActivated(1);
            bApply->setEnabled(true);
        } else  {
            //kdDebug()<<k_funcinfo<<"("<<date.toString()<<")=none"<<endl;
            state->setCurrentItem(0);
            slotStateActivated(0);
            bApply->setEnabled(true);
        }
    } else {
        // default
        state->setCurrentItem(0);
        slotStateActivated(0);
        bApply->setEnabled(true);
    }
}

void CalendarEdit::slotWeekdaySelected(int day_/* 1..7 */) {
    if (m_calendar == 0 || day_ < 1 || day_ > 7) {
        kdError()<<k_funcinfo<<"No calendar or weekday ("<<day_<<") not defined!"<<endl;
        return;
    }
    //kdDebug()<<k_funcinfo<<"("<<day_<<")"<<endl;
    clearEditPart();
    CalendarDay *calDay = m_calendar->weekday(day_-1); // 0..6
    if (!calDay) {
        kdError()<<k_funcinfo<<"Weekday ("<<day_<<") not defined!"<<endl;
        return;
    }
    state->clear();
    state->insertItem(i18n("Undefined"));
    state->insertItem(i18n("Non-working"));
    state->insertItem(i18n("Working"));
    QPtrListIterator<QPair<QTime, QTime> > it = calDay->workingIntervals();
    for (; it.current(); ++it) {
        IntervalItem *item = new IntervalItem(intervalList, it.current()->first, it.current()->second);
        intervalList->insertItem(item);
    }
    state->setEnabled(true);
    if (calDay->state() == Map::Working) {
        //kdDebug()<<k_funcinfo<<"("<<day_<<")=workday"<<endl;
        state->setCurrentItem(2);
        slotStateActivated(2);
        bApply->setEnabled(calDay->workingIntervals().count() > 0);
    } else if (calDay->state() == Map::NonWorking) {
        //kdDebug()<<k_funcinfo<<"("<<day_<<")=Holiday"<<endl;
        state->setCurrentItem(1);
        slotStateActivated(1);
        bApply->setEnabled(true);
    } else {
        //kdDebug()<<k_funcinfo<<"("<<day_<<")=none"<<endl;
        state->setCurrentItem(0);
        slotStateActivated(0);
        bApply->setEnabled(true);
    }
}

void CalendarEdit::slotSelectionCleared() {
    clearEditPart();
}

}  //KPlato namespace

#include "kptcalendaredit.moc"
