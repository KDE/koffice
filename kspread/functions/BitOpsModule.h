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

#ifndef KSPREAD_BITOPS_MODULE
#define KSPREAD_BITOPS_MODULE

#include "FunctionModule.h"

#include <QVariantList>

namespace KSpread
{

class BitOpsModule : public FunctionModule
{
    Q_OBJECT
public:
    explicit BitOpsModule(QObject* parent, const QVariantList& args = QVariantList());

    QString descriptionFileName() const;

    void registerFunctions();
    void removeFunctions();
};

} // namespace KSpread

#endif // KSPREAD_BITOPS_MODULE
