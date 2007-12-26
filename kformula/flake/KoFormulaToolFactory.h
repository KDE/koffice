/* This file is part of the KDE project
 * Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>
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

#ifndef KOFORMULATOOLFACTORY_H
#define KOFORMULATOOLFACTORY_H

#include <KoToolFactory.h>

/**
 * @short The factory for KoFormulaTool
 *
 * This reimplements the KoToolFactory class from the flake library in order
 * to provide a factory for the KoTool based class KoFormulaTool. This is the
 * KoTool that is used to edit a KoFormulaShape.
 * This class is part of the FormulaShape plugin and follows the factory design
 * pattern.
 */
class KoFormulaToolFactory : public KoToolFactory {
    Q_OBJECT
public:
    /// The constructor - reimplemented from KoToolFactory
    explicit KoFormulaToolFactory( QObject* parent );

    /// The destructor - reimplemented from KoToolFactory
    ~KoFormulaToolFactory();

    /// @return an instance of KoFormulaTool
    KoTool* createTool( KoCanvasBase* canvas );
};

#endif
