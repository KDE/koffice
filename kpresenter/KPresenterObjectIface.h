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

#ifndef KPRESENTER_OBJ_IFACE_H
#define KPRESENTER_OBJ_IFACE_H

#include <dcopobject.h>
#include <dcopref.h>

#include <qstring.h>
#include <qcolor.h>

class KPObject;

class KPresenterObjectIface : public DCOPObject
{
    K_DCOP
public:
    KPresenterObjectIface( KPObject *obj_ );

k_dcop:
    int getType() const;

    bool isSelected() const;
    float angle() const;
    int shadowDistance() const;
    int shadowDirection() const;
    QColor shadowColor() const;
    int effect() const;
    int effect2() const;
    int presNum() const;
    int subPresSteps() const;
    bool disappear() const;
    int disappearNum() const;
    int effect3() const;

    void setEffect(const QString & effect);
    void setEffect3(const QString & effect);

    void setSelected( bool _selected );
    void rotate( float _angle );
    void setShadowDistance( int _distance );
    void setSticky( bool b );
    bool isSticky() const;

    void shadowColor( const QColor & _color );

    void setAppearTimer( int _appearTimer );
    void setDisappearTimer( int _disappearTimer );

    void setAppearSoundEffect( bool b );
    void setDisappearSoundEffect( bool b );
    void setAppearSoundEffectFileName( const QString & _a_fileName );
    void setDisappearSoundEffectFileName( const QString &_d_fileName );

    void setPresNum( int _presNum );

    void setDisappear( bool b );

    int appearTimer() const;
    int disappearTimer() const;
    bool appearSoundEffect() const;
    bool disappearSoundEffect() const;
    QString appearSoundEffectFileName() const;
    QString disappearSoundEffectFileName() const;
    QString typeString() const;

    void setProtected( bool b );
    bool isProtected() const;

    void setKeepRatio( bool b );
    bool isKeepRatio() const;

private:
    KPObject *obj;

};

#endif
