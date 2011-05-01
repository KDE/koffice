/* This file is part of the KDE project
   Copyright (C) 1998-2002 The KSpread Team <koffice-devel@kde.org>
   Copyright (C) 2006 Brad Hards <bradh@frogmouth.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// built-in logical functions

#include "BitOpsModule.h"

#include "KCFunction.h"
#include "FunctionModuleRegistry.h"
#include "ValueCalc.h"
#include "ValueConverter.h"

#include <KLocale>

using namespace KSpread;

// prototypes (sorted alphabetically)
KCValue func_bitand(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_bitor(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_bitxor(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_bitlshift(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_bitrshift(valVector args, ValueCalc *calc, FuncExtra *);


KSPREAD_EXPORT_FUNCTION_MODULE("bitops", BitOpsModule)


BitOpsModule::BitOpsModule(QObject* parent, const QVariantList&)
        : KCFunctionModule(parent)
{
    KCFunction *f;

    f = new KCFunction("BITAND", func_bitand);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("BITOR", func_bitor);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("BITXOR", func_bitxor);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("BITLSHIFT", func_bitlshift);
    f->setParamCount(2);
    add(f);
    f = new KCFunction("BITRSHIFT", func_bitrshift);
    f->setParamCount(2);
    add(f);
}

QString BitOpsModule::descriptionFileName() const
{
    return QString("bitops.xml");
}


// KCFunction: BITAND
KCValue func_bitand(valVector args, ValueCalc *, FuncExtra *)
{
    const quint64 x = args[0].asInteger();
    const quint64 y = args[1].asInteger();
    return KCValue(static_cast<qint64>(x & y));
}

// KCFunction: BITOR
KCValue func_bitor(valVector args, ValueCalc *, FuncExtra *)
{
    const quint64 x = args[0].asInteger();
    const quint64 y = args[1].asInteger();
    return KCValue(static_cast<qint64>(x | y));
}

// KCFunction: BITXOR
KCValue func_bitxor(valVector args, ValueCalc *, FuncExtra *)
{
    const quint64 x = args[0].asInteger();
    const quint64 y = args[1].asInteger();
    return KCValue(static_cast<qint64>(x ^ y));
}

// KCFunction: BITLSHIFT
KCValue func_bitlshift(valVector args, ValueCalc *, FuncExtra *)
{
    const quint64 x = args[0].asInteger();
    const int numshift = args[1].asInteger();
    if (numshift == 0)
        return KCValue(static_cast<qint64>(x));
    else if (numshift > 0)
        return KCValue(static_cast<qint64>(x << numshift));
    else // negative left shift, becomes right shift
        return KCValue(static_cast<qint64>(x >>(-1 * numshift)));
}

// KCFunction: BITRSHIFT
KCValue func_bitrshift(valVector args, ValueCalc *, FuncExtra *)
{
    const quint64 x = args[0].asInteger();
    const int numshift = args[1].asInteger();
    if (numshift == 0)
        return KCValue(static_cast<qint64>(x));
    else if (numshift > 0)
        return KCValue(static_cast<qint64>(x >> numshift));
    else // negative right shift, becomes left shift
        return KCValue(static_cast<qint64>(x << (-1 * numshift)));
}

#include "BitOpsModule.moc"
