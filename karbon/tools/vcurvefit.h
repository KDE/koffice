#ifndef __VCURVEFIT_H__
#define __VCURVEFIT_H__

#include <qstring.h>
#include <qptrlist.h>
#include "koPoint.h"

VPath *bezierFit(QPtrList<KoPoint> &points,float error);

#endif

