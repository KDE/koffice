/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef confrectdia_h
#define confrectdia_h

#include <qframe.h>
#include <qpen.h>
#include <qbrush.h>

class QPainter;
class QLabel;
class KIntNumInput;
class QGroupBox;
class QPushButton;

/******************************************************************/
/* class RectPreview                                               */
/******************************************************************/

class RectPreview : public QFrame
{
    Q_OBJECT

public:
    RectPreview( QWidget* parent, const char* );
    ~RectPreview() {}

    void setRnds( int _rx, int _ry )
    { xRnd = _rx; yRnd = _ry; repaint( contentsRect(), true ); }
    void setPenBrush( const QPen &_pen, const QBrush &_brush )
    { pen = _pen; brush = _brush; repaint( true ); }

protected:
    void drawContents( QPainter* );

    int xRnd, yRnd;
    QPen pen;
    QBrush brush;

};

/******************************************************************/
/* class ConfRectDia                                              */
/******************************************************************/

class ConfRectDia : public QWidget
{
    Q_OBJECT

public:
    ConfRectDia( QWidget* parent, const char* );
    ~ConfRectDia();

    void setRnds( int _rx, int _ry );
    void setPenBrush( const QPen &_pen, const QBrush &_brush );

    int getRndX() { return xRnd; }
    int getRndY() { return yRnd; }

protected:
    QLabel *lRndX, *lRndY;
    KIntNumInput *eRndX, *eRndY;
    QGroupBox *gSettings;
    RectPreview *rectPreview;
    int xRnd, yRnd;
    int oldXRnd, oldYRnd;

protected slots:
    void rndXChanged( int );
    void rndYChanged( int );
    void Apply() { emit confRectDiaOk(); }
    void slotReset();
signals:
    void confRectDiaOk();

};

#endif
