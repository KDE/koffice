/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

#include "KCFunctionModule.h"

#include "KCFunction.h"

#include <QList>

class KCFunctionModule::Private
{
public:
    QList<QSharedPointer<KCFunction> > functions;
};


KCFunctionModule::KCFunctionModule(QObject* parent)
        : QObject(parent)
        , d(new Private)
{
}

KCFunctionModule::~KCFunctionModule()
{
    delete d;
}

QList<QSharedPointer<KCFunction> > KCFunctionModule::functions() const
{
    return d->functions;
}

bool KCFunctionModule::isRemovable()
{
    QList<KCFunction*> checkedFunctions;
    QWeakPointer<KCFunction> weakPointer;
    while (d->functions.count() != 0) {
        weakPointer = d->functions.last().toWeakRef();
        checkedFunctions.append(d->functions.takeLast().data());
        if (!weakPointer.isNull()) {
            // Put it and the other checked ones back in.
            d->functions.append(weakPointer.toStrongRef());
            // The failing on was used, so we do not put it in twice.
            checkedFunctions.removeLast();
            foreach(KCFunction* function, checkedFunctions) {
                // It is okay to recreate the shared pointers, as they were not used.
                d->functions.append(QSharedPointer<KCFunction>(function));
            }
            return false;
        }
    }
    return true;
}

QString KCFunctionModule::id() const
{
    return descriptionFileName();
}

void KCFunctionModule::add(KCFunction* function)
{
    if (!function) {
        return;
    }
    d->functions.append(QSharedPointer<KCFunction>(function));
}

#include "KCFunctionModule.moc"
