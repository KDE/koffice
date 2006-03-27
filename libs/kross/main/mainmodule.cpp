/***************************************************************************
 * mainmodule.cpp
 * This file is part of the KDE project
 * copyright (C)2004-2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "mainmodule.h"

#include <kdebug.h>
//Added by qt3to4:
#include <Q3CString>

using namespace Kross::Api;

namespace Kross { namespace Api {

    /// \internal
    class MainModulePrivate
    {
        public:
           /**
            * The \a Exception this \a MainModule throwed or
            * NULL if we don't had an exception.
            */
            Exception::Ptr exception;
    };

}}

MainModule::MainModule(const QString& name)
    : Module(name)
    , d(new MainModulePrivate())
{
    d->exception = 0;
}

MainModule::~MainModule()
{
    delete d;
}

const QString MainModule::getClassName() const
{
    return "Kross::Api::MainModule";
}

bool MainModule::hadException()
{
    return d->exception != 0;
}

Exception::Ptr MainModule::getException()
{
    return d->exception;
}

void MainModule::setException(Exception::Ptr exception)
{
    d->exception = exception;
}

bool MainModule::hasChild(const QString& name) const
{
    return Object::hasChild(name);
}

EventSignal::Ptr MainModule::addSignal(const QString& name, QObject* sender, Q3CString signal)
{
    EventSignal* event = new EventSignal(name, this, sender, signal);
    if(! addChild(event)) {
        kWarning() << QString("Failed to add signal name='%1' signature='%2'").arg(name).arg(signal) << endl;
        return 0;
    }
    return event;
}

EventSlot::Ptr MainModule::addSlot(const QString& name, QObject* receiver, Q3CString slot)
{
    EventSlot* event = new EventSlot(name, this, receiver, slot);
    if(! addChild(event)) {
        kWarning() << QString("Failed to add slot name='%1' signature='%2'").arg(name).arg(slot) << endl;
        delete event;
        return 0;
    }
    return event;
}

QtObject::Ptr MainModule::addQObject(QObject* object, const QString& name)
{
    QtObject* qtobject = new QtObject(this, object, name);
    if(! addChild(qtobject)) {
        kWarning() << QString("Failed to add QObject name='%1'").arg(object->name()) << endl;
        delete qtobject;
        return 0;
    }
    return qtobject;
}

EventAction::Ptr MainModule::addKAction(KAction* action, const QString& name)
{
    EventAction* event = new EventAction(name, this, action);
    if(! addChild(event)) {
        kWarning() << QString("Failed to add KAction name='%1'").arg(action->name()) << endl;
        delete event;
        return 0;
    }
    return event;
}

