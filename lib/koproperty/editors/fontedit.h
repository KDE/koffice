/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004 Alexander Dymo <cloudtemple@mskat.net>
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KPROPERTY_FONTEDIT_H
#define KPROPERTY_FONTEDIT_H

#include "../widget.h"
//Added by qt3to4:
#include <QEvent>

class FontEditRequester;

namespace KoProperty {

class KOPROPERTY_EXPORT FontEdit : public Widget
{
	Q_OBJECT

	public:
		FontEdit(Property *property, QWidget *parent=0, const char *name=0);
		virtual ~FontEdit();

		virtual QVariant value() const;
		virtual void setValue(const QVariant &value, bool emitChange=true);

		virtual void drawViewer(QPainter *p, const QColorGroup &cg, const QRect &r, const QVariant &value);

	protected:
		virtual void setReadOnlyInternal(bool readOnly);
		virtual bool eventFilter(QObject* watched, QEvent* e);

	protected slots:
		void  slotValueChanged(const QFont &font);

	private:
		FontEditRequester *m_edit;
};

}

#endif
