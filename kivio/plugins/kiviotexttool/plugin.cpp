/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "plugin.h"
#include "tool_text.h"

#include "kivio_view.h"

#include <kinstance.h>
#include <kiconloader.h>

/***************************************************
 * Factory
 ***************************************************/
extern "C"
{
  void* init_libkiviotexttool()
  {
    return new TextToolFactory;
  }
};

KInstance* TextToolFactory::s_global = 0;

TextToolFactory::TextToolFactory( QObject* parent, const char* name )
: KLibFactory( parent, name )
{
  s_global = new KInstance("kivio");
}

TextToolFactory::~TextToolFactory()
{
  delete s_global;
}

QObject* TextToolFactory::createObject( QObject* parent, const char* name, const char*, const QStringList& )
{
  if ( !parent->inherits("KivioView") )
    return 0;

  QObject *obj = new TextTool( (KivioView*)parent );
  return obj;
}

KInstance* TextToolFactory::global()
{
  return s_global;
}
#include "plugin.moc"
