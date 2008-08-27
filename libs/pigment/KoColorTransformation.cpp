/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoColorTransformation.h"

KoColorTransformation::~KoColorTransformation()
{
}

void KoColorTransformation::setParameters(const QHash<QString, QVariant> & parameters)
{
  for( QHash<QString, QVariant>::const_iterator it = parameters.begin(); it != parameters.end(); ++it )
  {
    setParameter( it.key(), it.value() );
  }

}

void KoColorTransformation::setParameter(const QString& name, const QVariant& parameter)
{
  Q_UNUSED(name);
  Q_UNUSED(parameter);
}
