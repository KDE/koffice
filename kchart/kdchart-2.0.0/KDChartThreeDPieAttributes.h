/****************************************************************************
 ** Copyright (C) 2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be distributed and/or modified under the terms of the
 ** GNU General Public License version 2 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.
 **
 ** Licensees holding valid commercial KD Chart licenses may use this file in
 ** accordance with the KD Chart Commercial License Agreement provided with
 ** the Software.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** See http://www.kdab.net/kdchart for
 **   information about KDChart Commercial License Agreements.
 **
 ** Contact info@kdab.net if any conditions of this
 ** licensing are not clear to you.
 **
 **********************************************************************/

#ifndef KDCHARTTHREEDPIEATTRIBUTES_H
#define KDCHARTTHREEDPIEATTRIBUTES_H

#include <QMetaType>
#include "KDChartAbstractThreeDAttributes.h"
#include "KDChartGlobal.h"

namespace KDChart {

  class KDCHART_EXPORT ThreeDPieAttributes : public AbstractThreeDAttributes
  {
  public:
    ThreeDPieAttributes();
    ThreeDPieAttributes( const ThreeDPieAttributes& );
    ThreeDPieAttributes &operator= ( const ThreeDPieAttributes& );

    ~ThreeDPieAttributes();

    /* threeD Pies specific */
    void setUseShadowColors( bool useShadowColors );
    bool useShadowColors() const;

    bool operator==( const ThreeDPieAttributes& ) const;
    inline bool operator!=( const ThreeDPieAttributes& other ) const { return !operator==(other); }


    KDCHART_DECLARE_SWAP_DERIVED(ThreeDPieAttributes)

private:
    KDCHART_DECLARE_PRIVATE_DERIVED(ThreeDPieAttributes)

  }; // End of class ThreeDPieAttributes

}

#if !defined(QT_NO_DEBUG_STREAM)
KDCHART_EXPORT QDebug operator<<(QDebug, const KDChart::ThreeDPieAttributes& );
#endif /* QT_NO_DEBUG_STREAM */

Q_DECLARE_METATYPE( KDChart::ThreeDPieAttributes )
Q_DECLARE_TYPEINFO( KDChart::ThreeDPieAttributes, Q_MOVABLE_TYPE );
KDCHART_DECLARE_SWAP_SPECIALISATION_DERIVED( KDChart::ThreeDPieAttributes )

#endif // KDCHARTTHREEDPIEATTRIBUTES_H
