/* This file is part of the KDE project
   Copyright (C) 2003,2004 Ariya Hidayat <ariya@kde.org>
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


#ifndef KCELLS_FUNCTIONS
#define KCELLS_FUNCTIONS

#include <QList>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>

#include "KCRegion.h"

#include "kcells_export.h"

class QDomElement;

class KCSheet;
class KCValue;
class KCValueCalc;
class KCFunction;

typedef QVector<KCValue> valVector;

struct rangeInfo {
    int col1, col2, row1, row2;
    int columns() {
        return col2 - col1 + 1;
    }
    int rows() {
        return row2 - row1 + 1;
    }
};
struct FuncExtra {
    // here we'll add all the extras a function may need
    KCFunction* function;
    QVector<rangeInfo> ranges;
    QVector<KCRegion> regions;
    KCSheet *sheet;
    int myrow, mycol;
};

typedef KCValue(*FunctionPtr)(valVector, KCValueCalc *, FuncExtra *);

/**
 * \ingroup KCValue
 * A function pointer and context.
 */
class KCELLS_EXPORT KCFunction
{
public:
    KCFunction(const QString& name, FunctionPtr ptr);
    virtual ~KCFunction();
    /**
    setParamCount sets allowed parameter count for a function.
    if max=0, it means max=min. If max=-1, there is no upper limit.
    */
    void setParamCount(int min, int max = 0);
    /** is it okay for the function to receive this many parameters ? */
    bool paramCountOkay(int count);
    /** when set to true, the function can receive arrays. When set to
    false, the auto-array mechamism will be used for arrays (so the
    function will receive simple values, not arrays). */
    void setAcceptArray(bool accept = true);
    bool needsExtra();
    void setNeedsExtra(bool extra);
    QString name() const;
    QString localizedName() const;
    QString helpText() const;
    void setHelpText(const QString& text);
    KCValue exec(valVector args, KCValueCalc *calc, FuncExtra *extra = 0);

    QString alternateName() const;
    void setAlternateName(const QString &name);

private:
    Q_DISABLE_COPY(KCFunction)

    class Private;
    Private * const d;
};

#endif // KCELLS_FUNCTIONS
