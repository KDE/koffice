/****************************************************************************
** $Id$
**
** Internal header file.
**
** Created :
**
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QCOMPLEXTEXT_H
#define QCOMPLEXTEXT_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Remote Control. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include <qstring.h>
#include <qpointarray.h>
#include <qfont.h>
#include <qpainter.h>
#include <qptrlist.h>
#include <qshared.h>
#endif // QT_H

namespace Qt3 {
#ifndef QT_NO_COMPLEXTEXT

// bidi helper classes. Internal to Qt
struct Q_EXPORT QBidiStatus {
    QBidiStatus() {
	eor = QChar::DirON;
	lastStrong = QChar::DirON;
	last = QChar:: DirON;
    }
    QChar::Direction eor;
    QChar::Direction lastStrong;
    QChar::Direction last;
};

struct Q_EXPORT QBidiContext : public QShared {
    // ### ref and deref parent?
    QBidiContext( uchar level, QChar::Direction embedding, QBidiContext *parent = 0, bool override = FALSE );
    ~QBidiContext();

    unsigned char level;
    bool override : 1;
    QChar::Direction dir : 5;

    QBidiContext *parent;
};

struct Q_EXPORT QBidiControl {
    QBidiControl() { context = 0; }
    QBidiControl( QBidiContext *c, QBidiStatus s)
    { context = c; if( context ) context->ref(); status = s; }
    ~QBidiControl() { if ( context && context->deref() ) delete context; }
    void setContext( QBidiContext *c ) { if ( context == c ) return; if ( context && context->deref() ) delete context; context = c; context->ref(); }
    QBidiContext *context;
    QBidiStatus status;
};

struct Q_EXPORT QTextRun {
    QTextRun(int _start, int _stop, QBidiContext *context, QChar::Direction dir);

    int start;
    int stop;
    // explicit + implicit levels here
    uchar level;
};

class Q_EXPORT QComplexText {
public:
    enum Shape {
	XIsolated,
	XFinal,
	XInitial,
	XMedial
    };
    static Shape glyphVariant( const QString &str, int pos);
    static Shape glyphVariantLogical( const QString &str, int pos);

    static QString shapedString( const QString &str, int from = 0, int len = -1, QPainter::TextDirection dir = QPainter::Auto);
    static QChar shapedCharacter(const QString &str, int pos);

#if 0
    // positions non spacing marks relative to the base character at position pos.
    static QPointArray positionMarks( QFontPrivate *f, const QString &str, int pos, QRect *boundingRect = 0 );
#endif

    static QPtrList<QTextRun> *bidiReorderLine( QBidiControl *control, const QString &str, int start, int len );
    static QString bidiReorderString( const QString &str, QChar::Direction basicDir = QChar::DirON );
};

}; // namespace
#endif //QT_NO_COMPLEXTEXT

#endif
