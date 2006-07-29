/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_TILE_COMMAND_H_
#define KIS_TILE_COMMAND_H_

#include <qstring.h>

#include <kcommand.h>

#include "kis_types.h"

class QRect;
class KisTransactionPrivate;

class KisTransaction : public KCommand {
public:
    KisTransaction(const QString& name, KisPaintDeviceSP device);
    virtual ~KisTransaction();

public:
    virtual void execute();
    virtual void unexecute();
    virtual void unexecuteNoUpdate();
    virtual QString name() const;
private:
    KisTransactionPrivate * m_private;
};

#endif // KIS_TILE_COMMAND_H_

