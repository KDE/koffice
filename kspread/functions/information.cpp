/* This file is part of the KDE project
   Copyright (C) 1998-2002 The KSpread Team <koffice-devel@kde.org>
   Copyright (C) 2005 Tomas Mecir <mecirt@gmail.com>

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

// built-in information functions

#include "InformationModule.h"

#include <kofficeversion.h>
#include <sys/utsname.h>

#include <QDir>
#include <kdebug.h>
#include <KLocale>

#include "KCCalculationSettings.h"
#include "Function.h"
#include "FunctionModuleRegistry.h"
#include "ValueCalc.h"
#include "ValueConverter.h"
#include "KCSheet.h"
#include "KCRegion.h"
#include "KCCell.h"
#include "KCFormula.h"

#include <KoDocument.h>

using namespace KSpread;

// prototypes (sorted alphabetically)
KCValue func_errortype(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_filename(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_formula(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_info(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_isblank(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_isdate(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_iserr(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_iserror(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_iseven(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_islogical(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_isna(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_isnottext(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_isnum(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_isodd(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_isref(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_istext(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_istime(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_n(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_na(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_type(valVector args, ValueCalc *calc, FuncExtra *);
KCValue func_version(valVector args, ValueCalc *calc, FuncExtra *);


KSPREAD_EXPORT_FUNCTION_MODULE("information", InformationModule)


InformationModule::InformationModule(QObject* parent, const QVariantList&)
        : FunctionModule(parent)
{
    Function *f;

    f = new Function("ERRORTYPE", func_errortype);
    add(f);
    f = new Function("FILENAME", func_filename);
    f->setParamCount(0);
    add(f);
    f = new Function("FORMULA", func_formula);
    f->setParamCount(1);
    f->setNeedsExtra(true);
    add(f);
    f = new Function("INFO", func_info);
    add(f);
    f = new Function("ISBLANK", func_isblank);
    add(f);
    f = new Function("ISDATE", func_isdate);
    add(f);
    f = new Function("ISERR", func_iserr);
    add(f);
    f = new Function("ISERROR", func_iserror);
    add(f);
    f = new Function("ISEVEN", func_iseven);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETISEVEN");
    add(f);
    f = new Function("ISLOGICAL", func_islogical);
    add(f);
    f = new Function("ISNA", func_isna);
    add(f);
    f = new Function("ISNONTEXT", func_isnottext);
    add(f);
    f = new Function("ISNOTTEXT", func_isnottext);
    add(f);
    f = new Function("ISNUM", func_isnum);
    add(f);
    f = new Function("ISNUMBER", func_isnum);
    add(f);
    f = new Function("ISODD", func_isodd);
    f->setAlternateName("COM.SUN.STAR.SHEET.ADDIN.ANALYSIS.GETISODD");
    add(f);
    f = new Function("ISREF", func_isref);
    f->setNeedsExtra(true);
    f->setAcceptArray();
    add(f);
    f = new Function("ISTEXT", func_istext);
    add(f);
    f = new Function("ISTIME", func_istime);
    add(f);
    f = new Function("N", func_n);
    add(f);
    f = new Function("NA", func_na);
    f->setParamCount(0);
    add(f);
    f = new Function("TYPE", func_type);
    f->setAcceptArray();
    add(f);
}

QString InformationModule::descriptionFileName() const
{
    return QString("information.xml");
}


// Function: ERROR.TYPE
KCValue func_errortype(valVector args, ValueCalc *, FuncExtra *)
{
    if (! args[0].isError()) {
        // its an error if the argument isn't an error...
        return KCValue::errorVALUE();
    }

    if (args[0] == KCValue::errorNULL()) {
        return KCValue(1);
    } else if (args[0] == KCValue::errorDIV0()) {
        return KCValue(2);
    } else if (args[0] == KCValue::errorVALUE()) {
        return KCValue(3);
    } else if (args[0] == KCValue::errorREF()) {
        return KCValue(4);
    } else if (args[0] == KCValue::errorNAME()) {
        return KCValue(5);
    } else if (args[0] == KCValue::errorNUM()) {
        return KCValue(6);
    } else if (args[0] == KCValue::errorNA()) {
        return KCValue(7);
    } else if (args[0] == KCValue::errorCIRCLE()) {
        // non-standard error type
        return KCValue(101);
    } else if (args[0] == KCValue::errorDEPEND()) {
        // non-standard error type
        return KCValue(102);
    } else if (args[0] == KCValue::errorPARSE()) {
        // non-standard error type
        return KCValue(103);
    } else {
        // something I didn't think of...
        kDebug() << "Unexpected error type";
        return KCValue(0);
    }
}
// Function: INFO
KCValue func_info(valVector args, ValueCalc *calc, FuncExtra *)
{
    QString type = calc->conv()->asString(args[0]).asString().toLower();

    if (type == "directory")
        return KCValue(QDir::currentPath());

    if (type == "release")
        return KCValue(QString(KOFFICE_VERSION_STRING));

    if (type == "numfile")
        return KCValue(KoDocument::documentList() ? KoDocument::documentList()->count() : 0);

    if (type == "recalc") {
        QString result;
        if (!calc->settings()->isAutoCalculationEnabled())
            result = i18n("Manual");
        else
            result = i18n("Automatic");
        return KCValue(result);
    }

    if (type == "memavail")
        // not supported
        return KCValue::errorVALUE();
    if (type == "memused")
        // not supported
        return KCValue::errorVALUE();
    if (type == "origin")
        // not supported
        return KCValue::errorVALUE();

    if (type == "system") {
        struct utsname name;
        if (uname(&name) >= 0)
            return KCValue(QString(name.sysname));
    }

    if (type == "totmem")
        // not supported
        return KCValue::errorVALUE();

    if (type == "osversion") {
        struct utsname name;
        if (uname(&name) >= 0) {
            QString os = QString("%1 %2 (%3)").arg(name.sysname).
                         arg(name.release).arg(name.machine);
            return KCValue(os);
        }
    }

    return KCValue::errorVALUE();
}

// Function: ISBLANK
KCValue func_isblank(valVector args, ValueCalc *, FuncExtra *)
{
    return KCValue(args[0].isEmpty());
}

// Function: ISLOGICAL
KCValue func_islogical(valVector args, ValueCalc *, FuncExtra *)
{
    return KCValue(args[0].isBoolean());
}

// Function: ISTEXT
KCValue func_istext(valVector args, ValueCalc *, FuncExtra *)
{
    return KCValue(args[0].isString());
}

// Function: ISREF
KCValue func_isref(valVector args, ValueCalc * /*calc*/, FuncExtra *e)
{
    if (args[0].isError()) return args[0];  // errors pass through
    // no reference ?
    if ((e == 0) || (e->ranges[0].col1 == -1) || (e->ranges[0].row1 == -1))
        return KCValue(false);
    // if we are here, it is a reference (cell/range)
    return KCValue(true);
}

// Function: ISNOTTEXT
KCValue func_isnottext(valVector args, ValueCalc *, FuncExtra *)
{
    return KCValue(args[0].isString() ? false : true);
}

// Function: ISNUM
KCValue func_isnum(valVector args, ValueCalc *, FuncExtra *)
{
    return KCValue(args[0].isNumber());
}

// Function: ISTIME
KCValue func_istime(valVector args, ValueCalc *, FuncExtra *)
{
    return KCValue((args[0].format() == KCValue::fmt_Time)
                 || (args[0].format() == KCValue::fmt_DateTime));
}

// Function: ISDATE
KCValue func_isdate(valVector args, ValueCalc *, FuncExtra *)
{
    return KCValue((args[0].format() == KCValue::fmt_Date)
                 || (args[0].format() == KCValue::fmt_DateTime));
}

// Function: ISODD
KCValue func_isodd(valVector args, ValueCalc *calc, FuncExtra *)
{
    return KCValue(calc->isEven(args[0]) ? false : true);
}

// Function: ISEVEN
KCValue func_iseven(valVector args, ValueCalc *calc, FuncExtra *)
{
    if (args[0].isError())
        return args[0];
    return KCValue(calc->isEven(args[0]));
}

// Function: ISERR
KCValue func_iserr(valVector args, ValueCalc *, FuncExtra *)
{
    return KCValue((args[0].isError() &&
                  (args[0].errorMessage() != KCValue::errorNA().errorMessage())));
}

// Function: ISERROR
KCValue func_iserror(valVector args, ValueCalc *, FuncExtra *)
{
    return KCValue(args[0].isError());
}

// Function: ISNA
KCValue func_isna(valVector args, ValueCalc *, FuncExtra *)
{
    return KCValue((args[0].isError() &&
                  (args[0].errorMessage() == KCValue::errorNA().errorMessage())));
}

// Function: TYPE
KCValue func_type(valVector args, ValueCalc *, FuncExtra *)
{
    // Returns 1 for numbers, 2 for text, 4 for boolean, 16 for error,
    // 64 for arrays
    if (args[0].isArray())
        return KCValue(64);
    if (args[0].isNumber())
        return KCValue(1);
    if (args[0].isString())
        return KCValue(2);
    if (args[0].isBoolean())
        return KCValue(4);
    if (args[0].isError())
        return KCValue(16);

    // something else ?
    return KCValue(0);
}

KCValue func_filename(valVector, ValueCalc *calc, FuncExtra *)
{
    return KCValue(calc->settings()->fileName());
}

KCValue func_formula(valVector, ValueCalc *, FuncExtra *e)
{
    if(e->ranges[0].col1 < 1 || e->ranges[0].row1 < 1)
        return KCValue::errorVALUE();
    const KCCell c(e->sheet, e->ranges[0].col1, e->ranges[0].row1);
    if (c.isNull())
        return KCValue::errorVALUE();
    if (!c.isFormula())
        return KCValue::errorNA();
    return KCValue(c.formula().expression());
}

// Function: N
KCValue func_n(valVector args, ValueCalc *calc, FuncExtra *)
{
    return calc->conv()->asFloat(args[0]);
}

// Function: NA
KCValue func_na(valVector, ValueCalc *, FuncExtra *)
{
    return KCValue::errorNA();
}

#include "InformationModule.moc"
