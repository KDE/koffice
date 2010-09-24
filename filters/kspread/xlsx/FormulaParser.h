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
#ifndef FORMULAPARSER_H
#define FORMULAPARSER_H

#include <QtCore/QString>

class Cell;

namespace MSOOXML
{

/**
 * Convert the MSOOXML \p formula into a ODF formula and return that ODF formula.
 */
QString convertFormula(const QString& formula);

/**
 * Generate and return the ODF formula for \p thisCell based on the formula in the
 * defined \p referencedCell . This is used for formula groups.
 */
QString convertFormulaReference(Cell* referencedCell, Cell* thisCell);

};

#endif // FORMULAPARSER_H
