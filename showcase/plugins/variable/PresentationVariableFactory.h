/*
* This file is part of the KDE project
*
* Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
*
* Contact: Amit Aggarwal <amitcs06@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
*
*/

#ifndef PRESENTATION_VARIABLE_FACTORY_H
#define PRESENTATION_VARIABLE_FACTORY_H

#include <KoInlineObjectFactoryBase.h>

class PresentationVariableFactory : public KoInlineObjectFactoryBase
{
public:
    PresentationVariableFactory(QObject *parent = 0);

    /// reimplemented
    virtual KInlineObject *createInlineObject(const KProperties *properties = 0) const;
};

#endif

