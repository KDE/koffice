/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Michael Koch <koch@kde.org>
 
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

#include "formregion.h"
#include "formarea.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

// only for debug
#include <iostream.h>

FormRegion::FormRegion( const QString& _title, QWidget* _form )
  : QWidget( _form )
{
  QVBoxLayout* layout = new QVBoxLayout( this );

  // TODO: Label oder nicht drueckbaren Button einfuegen fuer Titel

  QLabel* label = new QLabel( _title, this );
  label->resize( label->sizeHint() );
  layout->addWidget( label, 0 );

  m_pArea = new FormArea( this );
  layout->addWidget( m_pArea, 1 );

  resize( 300, 500 );
}

FormRegion::~FormRegion()
{
}

#include "formregion.moc"

