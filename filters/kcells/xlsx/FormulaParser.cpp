/*
 * This file is part of Office 2007 Filters for KOffice
 *
 * Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */
#include "FormulaParser.h"
#include "XlsxXmlWorksheetReader_p.h"

#include <kcells/Util.h>

QString MSOOXML::convertFormula(const QString& formula)
{
    if (formula.isEmpty())
        return QString();
    enum { Start, InArguments, InParenthesizedArgument, InString, InSheetOrAreaName } state;
    state = Start;
    QString result = '=' + formula;
    for(int i = 1; i < result.length(); ++i) {
        QChar ch = result[i];
        switch (state) {
        case Start:
            if(ch == '(')
                state = InArguments;
            break;
        case InArguments:
            if (ch == '"')
                state = InString;
            else if (ch.unicode() == '\'')
                state = InSheetOrAreaName;
            else if (ch == ',')
                result[i] = ';'; // replace argument delimiter
            else if (ch == '(' && !result[i-1].isLetterOrNumber())
                state = InParenthesizedArgument;
            break;
        case InParenthesizedArgument:
            if (ch == ',')
                result[i] = '~'; // union operator
            else if (ch == ')')
                state = InArguments;
            break;
        case InString:
            if (ch == '"')
                state = InArguments;
            break;
        case InSheetOrAreaName:
            if (ch == '\'')
                state = InArguments;
            break;
        };
    };
    return result;
}

QString MSOOXML::convertFormulaReference(KCCell* referencedCell, KCCell* thisCell)
{
    return KCells::adjustFormulaReference(referencedCell->formula, referencedCell->row, referencedCell->column, thisCell->row, thisCell->column);
}
