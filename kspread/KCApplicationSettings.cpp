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

#include "KCApplicationSettings.h"

class KCApplicationSettings::Private
{
public:
    QColor gridColor;
    QColor pageBorderColor;
    KGlobalSettings::Completion completionMode;
    KSpread::MoveTo moveTo;
    KSpread::MethodOfCalc calcMethod;
    double indentValue;
    bool verticalScrollBar      : 1;
    bool horizontalScrollBar    : 1;
    bool columnHeader           : 1;
    bool rowHeader              : 1;
    bool showStatusBar          : 1;
    bool showTabBar             : 1;
    bool captureAllArrowKeys    : 1;
};

KCApplicationSettings::KCApplicationSettings()
        : d(new Private)
{
    d->gridColor = Qt::lightGray;
    d->pageBorderColor = Qt::red;
    d->completionMode = KGlobalSettings::CompletionAuto;
    d->moveTo = KSpread::Bottom;
    d->calcMethod = KSpread::SumOfNumber;
    d->indentValue = 10.0;
    d->verticalScrollBar = true;
    d->horizontalScrollBar = true;
    d->columnHeader = true;
    d->rowHeader = true;
    d->showStatusBar = true;
    d->showTabBar = true;
    d->captureAllArrowKeys = true;
}

KCApplicationSettings::~KCApplicationSettings()
{
    delete d;
}

void KCApplicationSettings::load()
{
}

void KCApplicationSettings::save() const
{
}

void KCApplicationSettings::setShowVerticalScrollBar(bool show)
{
    d->verticalScrollBar = show;
}

bool KCApplicationSettings::showVerticalScrollBar()const
{
    return d->verticalScrollBar;
}

void KCApplicationSettings::setShowHorizontalScrollBar(bool show)
{
    d->horizontalScrollBar = show;
}

bool KCApplicationSettings::showHorizontalScrollBar()const
{
    return d->horizontalScrollBar;
}

KGlobalSettings::Completion KCApplicationSettings::completionMode() const
{
    return d->completionMode;
}

void KCApplicationSettings::setShowColumnHeader(bool show)
{
    d->columnHeader = show;
}

bool KCApplicationSettings::showColumnHeader() const
{
    return d->columnHeader;
}

void KCApplicationSettings::setShowRowHeader(bool show)
{
    d->rowHeader = show;
}

bool KCApplicationSettings::showRowHeader() const
{
    return d->rowHeader;
}

void KCApplicationSettings::setGridColor(const QColor& color)
{
    d->gridColor = color;
}

QColor KCApplicationSettings::gridColor() const
{
    return d->gridColor;
}

void KCApplicationSettings::setCompletionMode(KGlobalSettings::Completion complMode)
{
    d->completionMode = complMode;
}

double KCApplicationSettings::indentValue() const
{
    return d->indentValue;
}

void KCApplicationSettings::setIndentValue(double val)
{
    d->indentValue = val;
}

void KCApplicationSettings::setShowStatusBar(bool statusBar)
{
    d->showStatusBar = statusBar;
}

bool KCApplicationSettings::showStatusBar() const
{
    return d->showStatusBar;
}

void KCApplicationSettings::setShowTabBar(bool tabbar)
{
    d->showTabBar = tabbar;
}

bool KCApplicationSettings::showTabBar()const
{
    return d->showTabBar;
}

KSpread::MoveTo KCApplicationSettings::moveToValue() const
{
    return d->moveTo;
}

void KCApplicationSettings::setMoveToValue(KSpread::MoveTo moveTo)
{
    d->moveTo = moveTo;
}

void KCApplicationSettings::setTypeOfCalc(KSpread::MethodOfCalc calc)
{
    d->calcMethod = calc;
}

KSpread::MethodOfCalc KCApplicationSettings::getTypeOfCalc() const
{
    return d->calcMethod;
}

QColor KCApplicationSettings::pageBorderColor() const
{
    return d->pageBorderColor;
}

void KCApplicationSettings::changePageBorderColor(const QColor& color)
{
    d->pageBorderColor = color;
}

void KCApplicationSettings::setCaptureAllArrowKeys(bool capture)
{
    d->captureAllArrowKeys = capture;
}

bool KCApplicationSettings::captureAllArrowKeys() const
{
    return d->captureAllArrowKeys;
}
