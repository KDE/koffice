/* This file is part of the KDE project

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#ifndef HANDLER_H
#define HANDLER_H

#include "worker.h"

class Handler
{
public:
	Handler();
	~Handler();

	const QDomDocument *const part();
	bool invokeHandler(Q_UINT16 opcode, Q_UINT32 bytes, QDataStream &operands);
		
private:
	Worker *m_worker;
};

#endif
