/*
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#ifndef ACTIONJOB_H
#define ACTIONJOB_H
#include <threadweaver/Job.h>

#include <QWaitCondition>
#include <QMutex>
#include <QVariant>
#include <QPointer>

class QEvent;

class KoAction;

/// private class that schedules the action worker stuff.
class ActionJob : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    enum Enable {
        EnableOn,
        EnableOff,
        EnableNoChange
    };
    ActionJob(KoAction *parent, Enable enable, const QVariant &params);

    const KoAction *action() const {
        return m_action;
    }
    bool started() const {
        return m_started;
    }

    void run();

private:
    bool event(QEvent *e);

private:
    QPointer<KoAction> m_action;
    Enable m_enable;
    bool m_started;
    QVariant m_params;
    QWaitCondition m_waiter;
    QMutex m_mutex;
};

#endif
