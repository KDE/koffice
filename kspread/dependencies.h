/* This file is part of the KDE project
   Copyright 2004 Tomas Mecir <mecirt@gmail.com>

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

#ifndef KSPREAD_DEPENDENCIES
#define KSPREAD_DEPENDENCIES

#include <qvaluelist.h>

#include "kspread_util.h"

class KSpreadSheet;

// KSpread namespace
namespace KSpread {


struct DependencyList;

/** Range dependency - stores information about one dependency of one cell on
one range of cells. */

struct RangeDependency {
  int cellrow, cellcolumn;
  KSpreadRange range;
};


/**
This class manages dependencies.
TODO: describe how it works and why there are two types of dependencies
*/

class DependencyManager {
 public:
  /** constructor */
   DependencyManager (KSpreadSheet *s);
  /** destructor */
  ~DependencyManager ();
 
  /** clear all data */
  void reset ();
  
  /** handle the fact that cell's contents have changed */
  void cellChanged (const KSpreadPoint &cell);
  /** handle the fact that a range has been changed */
  void rangeChanged (const KSpreadRange &range);
  /** handle the fact that a range list has been changed */
  void rangeListChanged (const RangeList &rangeList);

  /** get dependencies of a cell */
  RangeList getDependencies (const KSpreadPoint &cell);
  /** get cells depending on this cell, either through normal or range dependency */
  QValueList<KSpreadPoint> getDependants (const KSpreadPoint &cell);
protected:
  
  /** local d-pointer */
  DependencyList *deps;
  friend class DependencyList;
};

//end of namespace
}

#endif // KSPREAD_DEPENDENCIES

