/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2006 Thomas Zander <zander@kde.org>
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
#include "KoToolDocker.h"

#include <klocale.h>
#include <kdebug.h>
#include <QGridLayout>
#include <QWidget>

class KoToolDocker::Private {
public:
    Private(KoToolDocker *dock) : currentWidget(0), q(dock) {}
    QWidget *currentWidget;
    QWidget *housekeeperWidget;
    QGridLayout *housekeeperLayout;
    QSpacerItem *bottomRightSpacer;
    KoToolDocker *q;

    void optionWidgetDestroyed(QObject* child)
    {
        if (child == currentWidget)
            currentWidget = 0;
    }

    void locationChanged(Qt::DockWidgetArea area)
    {
        switch(area) {
            case Qt::TopDockWidgetArea:
            case Qt::BottomDockWidgetArea:
                bottomRightSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
                break;
            case Qt::LeftDockWidgetArea:
            case Qt::RightDockWidgetArea:
                bottomRightSpacer->changeSize(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
                break;
            default:
                break;
        }
        housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        housekeeperLayout->invalidate();
    }
};

KoToolDocker::KoToolDocker(QWidget *parent)
    : QDockWidget("Tool Options initial name - never seen", parent),
    d( new Private(this) )
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea )), this, SLOT(locationChanged(Qt::DockWidgetArea)));
    d->currentWidget = 0;

    d->housekeeperWidget = new QWidget();
    d->housekeeperLayout = new QGridLayout();
    d->housekeeperWidget->setLayout(d->housekeeperLayout);
    d->housekeeperLayout->setHorizontalSpacing(0);
    d->housekeeperLayout->setVerticalSpacing(0);
    d->housekeeperLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    d->bottomRightSpacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->housekeeperLayout->addItem(d->bottomRightSpacer, 1, 1);

    setWidget(d->housekeeperWidget);
}

KoToolDocker::~KoToolDocker() {
    delete d->housekeeperWidget;
    delete d;
}

bool KoToolDocker::hasOptionWidget()
{
    return  d->currentWidget != 0;
}

void KoToolDocker::newOptionWidget(QWidget *optionWidget) {
    if(d->currentWidget == optionWidget)
        return;

    d->currentWidget = optionWidget;

    connect(d->currentWidget, SIGNAL(destroyed(QObject*)), this, SLOT(optionWidgetDestroyed(QObject*)));

    d->housekeeperLayout->addWidget(optionWidget, 0, 0);

    update(); // force qt to update the layout even when we are floating
}

#include "KoToolDocker.moc"
