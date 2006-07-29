/* This file is part of the KDE project
  
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>

   Large parts are taken from kdebase/konqueror/konq_actions.*  
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

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

#ifndef _MAILMERGE_ACTIONS_
#define _MAILMERGE_ACTIONS_

#include <kaction.h>
#include <qtoolbutton.h>

class KWMailMergeLabelAction : public KAction
{
  Q_OBJECT
public:
  KWMailMergeLabelAction( const QString &text, int accel,
                    QObject* receiver, const char* slot, QObject *parent = 0, const char *name = 0 );

  virtual int plug( QWidget *widget, int index = -1 );
  virtual void unplug( QWidget *widget );
  QToolButton * label() { return m_label; }
private:
  QToolButton * m_label;
};

#endif
