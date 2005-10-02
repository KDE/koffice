/* This file is part of the KDE project
   Copyright 2004 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_STYLECLUSTER_TESTER
#define KSPREAD_STYLECLUSTER_TESTER

#include <qstring.h>

#include "tester.h"

#include <kspread_value.h>

namespace KSpread
{

class StyleClusterTester: public Tester
{
public:
  StyleClusterTester();
  virtual QString name();
  virtual void run();
private:
  template<typename T>
  void check_ptr( const char *file, int line, const char* msg, const T& result, const T& expected );
  void check_value( const char *file, int line, const char* msg, void * result, void * expected );
  void check_fails_value( const char *file, int line, const char* msg, void * result, void * expected );
};


} // namespace KSpread

#endif // KSPREAD_STYLECLUSTER_TESTER
