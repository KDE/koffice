/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __kspread_depend_h__
#define __kspread_depend_h__

class KSpreadTable;

class KSpreadDependency
{
 public:
  KSpreadDependency(int col, int row, KSpreadTable* table);
  KSpreadDependency(int left, int top, int right, int bottom, KSpreadTable* table);

  int Left();
  int Right();
  int Top();
  int Bottom();
  
  KSpreadTable* Table();

 private:
  int m_left;
  int m_right;
  int m_top;
  int m_bottom;
  KSpreadTable *m_table;


};

#endif /* defined __kspread_depend_h__ */
