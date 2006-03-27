/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2004  Alexander Dymo <cloudtemple@mskat.net>

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

#ifndef KPROPERTY_URLEDIT_H
#define KPROPERTY_URLEDIT_H

#include "../widget.h"

#ifndef QT_ONLY
class KUrlRequester;
#else
class QPushButton;
class QLineEdit;
#endif

namespace KoProperty {

class KOPROPERTY_EXPORT URLEdit : public Widget
{
	Q_OBJECT

	public:
		URLEdit(Property *property, QWidget *parent=0, const char *name=0);
		virtual ~URLEdit();

		virtual QVariant value() const;
		virtual void setValue(const QVariant &value, bool emitChange=true);

		virtual void setProperty(Property *property);

	protected:
		virtual void setReadOnlyInternal(bool readOnly);

	protected slots:
		void selectFile();
		void slotValueChanged(const QString &url);

	private:
#ifndef QT_ONLY
		KUrlRequester *m_edit;
#else
		QLineEdit *m_edit;
		QPushButton *m_select;
#endif
};

}

#endif
