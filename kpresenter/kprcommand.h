/* This file is part of the KDE project
   Copyright (C) 2001 Laurent Montel <lmontel@mandrakesoft.com>

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

#ifndef kprcommand_h
#define kprcommand_h

#include <kcommand.h>
#include <qptrlist.h>
#include <qpoint.h>
#include <qcolor.h>
#include <qsize.h>
#include <global.h>
#include <kpimage.h>
#include <kpclipartcollection.h>
#include <global.h>
#include <qvaluelist.h>
#include <qpen.h>
#include <qbrush.h>
#include <qmap.h>
#include <koPageLayoutDia.h>

class KPresenterDoc;

class KPresenterDoc;
class KPTextObject;
class KPObject;
class KPPixmapObject;
class KPGroupObject;
class KPresenterView;
class KPClipartObject;

/******************************************************************/
/* Class: TextCmd                                               */
/******************************************************************/

class TextCmd : public KCommand
{
public:
    TextCmd(QString name, KPresenterDoc *doc, KPTextObject *tObj);
    ~TextCmd() {}

    virtual void execute();
    virtual void unexecute();

private:

    KPresenterDoc *document;
    KPTextObject *textObject;
};

/******************************************************************/
/* Class: ShadowCmd                                               */
/******************************************************************/

class ShadowCmd : public KCommand
{
public:
    struct ShadowValues
    {
        int shadowDistance;
        ShadowDirection shadowDirection;
        QColor shadowColor;
    };

    ShadowCmd( QString _name, QPtrList<ShadowValues> &_oldShadow, ShadowValues _newShadow,
               QPtrList<KPObject> &_objects, KPresenterDoc *_doc );
    ~ShadowCmd();

    virtual void execute();
    virtual void unexecute();

protected:
    KPresenterDoc *doc;
    QPtrList<ShadowValues> oldShadow;
    QPtrList<KPObject> objects;
    ShadowValues newShadow;

};

/******************************************************************/
/* Class: SetOptionsCmd                                           */
/******************************************************************/

class SetOptionsCmd : public KCommand
{
public:
    SetOptionsCmd( QString _name, QPtrList<QPoint> &_diffs, QPtrList<KPObject> &_objects,
                   int _rastX, int _rastY, int _orastX, int _orastY,
                   QColor _txtBackCol, QColor _otxtBackCol, KPresenterDoc *_doc );
    ~SetOptionsCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QPtrList<QPoint> diffs;
    QPtrList<KPObject> objects;
    int rastX, rastY;
    int orastX, orastY;
    KPresenterDoc *doc;
    QColor txtBackCol;
    QColor otxtBackCol;

};

/******************************************************************/
/* Class: SetBackCmd						  */
/******************************************************************/

class SetBackCmd : public KCommand
{
public:
    SetBackCmd( QString _name, QColor _backColor1, QColor _backColor2, BCType _bcType,
		bool _backUnbalanced, int _backXFactor, int _backYFactor,
		const KPImageKey & _backPix, const KPClipartKey & _backClip,
                BackView _backView, BackType _backType,
		QColor _oldBackColor1, QColor _oldBackColor2, BCType _oldBcType,
		bool _oldBackUnbalanced, int _oldBackXFactor, int _oldBackYFactor,
		const KPImageKey & _oldBackPix, const KPClipartKey & _oldBackClip,
                BackView _oldBackView, BackType _oldBackType,
		bool _takeGlobal, int _currPgNum, KPresenterDoc *_doc );

    virtual void execute();
    virtual void unexecute();

protected:

    QColor backColor1, backColor2;
    bool unbalanced;
    int xfactor, yfactor;
    KPImageKey backPix;
    KPClipartKey backClip;
    BCType bcType;
    BackView backView;
    BackType backType;
    QColor oldBackColor1, oldBackColor2;
    bool oldUnbalanced;
    int oldXFactor, oldYFactor;
    KPImageKey oldBackPix;
    KPClipartKey oldBackClip;
    BCType oldBcType;
    BackView oldBackView;
    BackType oldBackType;
    bool takeGlobal;
    int currPgNum;
    KPresenterDoc *doc;

};

/******************************************************************/
/* Class: RotateCmd                                               */
/******************************************************************/

class RotateCmd : public KCommand
{
public:
    struct RotateValues
    {
        float angle;
    };

    RotateCmd( QString _name, QPtrList<RotateValues> &_oldRotate, float _newAngle,
               QPtrList<KPObject> &_objects, KPresenterDoc *_doc );
    ~RotateCmd();

    virtual void execute();
    virtual void unexecute();

protected:
    KPresenterDoc *doc;
    QPtrList<RotateValues> oldRotate;
    QPtrList<KPObject> objects;
    float newAngle;

};

/******************************************************************/
/* Class: ResizeCmd                                               */
/******************************************************************/

class ResizeCmd : public KCommand
{
public:
    ResizeCmd( QString _name, QPoint _m_diff, QSize _r_diff, KPObject *_object, KPresenterDoc *_doc );
    ~ResizeCmd();

    virtual void execute();
    virtual void unexecute();
    virtual void unexecute( bool _repaint );

protected:

    QPoint m_diff;
    QSize r_diff;
    KPObject *object;
    KPresenterDoc *doc;

};

/******************************************************************/
/* Class: ChgClipCmd                                              */
/******************************************************************/

class ChgClipCmd : public KCommand
{
public:
    ChgClipCmd( QString _name, KPClipartObject *_object, KPClipartCollection::Key _oldName,
                KPClipartCollection::Key _newName, KPresenterDoc *_doc );
    ~ChgClipCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KPClipartObject *object;
    KPresenterDoc *doc;
    KPClipartCollection::Key oldKey, newKey;
};

/******************************************************************/
/* Class: ChgPixCmd                                               */
/******************************************************************/

class ChgPixCmd : public KCommand
{
public:
    ChgPixCmd( QString _name, KPPixmapObject *_oldObject, KPPixmapObject *_newObject,
               KPresenterDoc *_doc );
    ~ChgPixCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KPPixmapObject *oldObject, *newObject;
    KPresenterDoc *doc;

};

/******************************************************************/
/* Class: DeleteCmd                                               */
/******************************************************************/

class DeleteCmd : public KCommand
{
public:
    DeleteCmd( QString _name, QPtrList<KPObject> &_objects, KPresenterDoc *_doc );
    ~DeleteCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QPtrList<KPObject> objects;
    KPresenterDoc *doc;

};

/******************************************************************/
/* Class: EffectCmd                                               */
/******************************************************************/

class EffectCmd : public KCommand
{
public:
    struct EffectStruct {
	int presNum, disappearNum;
	Effect effect;
	Effect2 effect2;
	Effect3 effect3;
	bool disappear;
    };

    EffectCmd( QString _name, const QPtrList<KPObject> &_objs,
	       const QValueList<EffectStruct> &_oldEffects, EffectStruct _newEffect );
    ~EffectCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QValueList<EffectStruct> oldEffects;
    EffectStruct newEffect;
    QPtrList<KPObject> objs;

};

/******************************************************************/
/* Class: GroupObjCmd						  */
/******************************************************************/

class GroupObjCmd : public KCommand
{
public:
    GroupObjCmd( const QString &_name,
		 const QPtrList<KPObject> &_objects,
		 KPresenterDoc *_doc );
    ~GroupObjCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QPtrList<KPObject> objects;
    KPresenterDoc *doc;
    KPGroupObject *grpObj;

};

/******************************************************************/
/* Class: UnGroupObjCmd						  */
/******************************************************************/

class UnGroupObjCmd : public KCommand
{
public:
    UnGroupObjCmd( const QString &_name,
		 KPGroupObject *grpObj_,
		 KPresenterDoc *_doc );
    ~UnGroupObjCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QPtrList<KPObject> objects;
    KPresenterDoc *doc;
    KPGroupObject *grpObj;

};


/******************************************************************/
/* Class: InsertCmd                                               */
/******************************************************************/

class InsertCmd : public KCommand
{
public:
    InsertCmd( QString _name, KPObject *_object, KPresenterDoc *_doc );
    ~InsertCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KPObject *object;
    KPresenterDoc *doc;

};

/******************************************************************/
/* Class: LowerRaiseCmd                                           */
/******************************************************************/

class LowerRaiseCmd : public KCommand
{
public:
    LowerRaiseCmd( QString _name, QPtrList<KPObject> *_oldList, QPtrList<KPObject> *_newList, KPresenterDoc *_doc );
    ~LowerRaiseCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QPtrList<KPObject> *oldList, *newList;
    KPresenterDoc *doc;
    bool m_executed;

};

/******************************************************************/
/* Class: MoveByCmd                                               */
/******************************************************************/

class MoveByCmd : public KCommand
{
public:
    MoveByCmd( QString _name, QPoint _diff, QPtrList<KPObject> &_objects, KPresenterDoc *_doc );
    ~MoveByCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QPoint diff;
    QPtrList<KPObject> objects;
    KPresenterDoc *doc;

};

/******************************************************************/
/* Class: MoveByCmd2                                              */
/******************************************************************/

class MoveByCmd2 : public KCommand
{
public:
    MoveByCmd2( QString _name, QPtrList<QPoint> &_diffs, QPtrList<KPObject> &_objects, KPresenterDoc *_doc );
    ~MoveByCmd2();

    virtual void execute();
    virtual void unexecute();

protected:

    QPtrList<QPoint> diffs;
    QPtrList<KPObject> objects;
    KPresenterDoc *doc;

};

/******************************************************************/
/* Class: PenBrushCmd						  */
/******************************************************************/

class PenBrushCmd : public KCommand
{
public:
    struct Pen {
	QPen pen;
	LineEnd lineBegin, lineEnd;

	Pen &operator=( const Pen &_pen ) {
	    pen	 = _pen.pen;
	    lineBegin = _pen.lineBegin;
	    lineEnd = _pen.lineEnd;
	    return *this;
	}
    };

    struct Brush {
	QBrush brush;
	QColor gColor1;
	QColor gColor2;
	BCType gType;
	FillType fillType;
	bool unbalanced;
	int xfactor, yfactor;

	Brush &operator=( const Brush &_brush ) {
	    brush = _brush.brush;
	    gColor1 = _brush.gColor1;
	    gColor2 = _brush.gColor2;
	    gType = _brush.gType;
	    fillType = _brush.fillType;
	    unbalanced = _brush.unbalanced;
	    xfactor = _brush.xfactor;
	    yfactor = _brush.yfactor;
	    return *this;
	}
    };

    static const int LB_ONLY = 1;
    static const int LE_ONLY = 2;
    static const int PEN_ONLY = 4;
    static const int BRUSH_ONLY = 8;

    PenBrushCmd( QString _name, QPtrList<Pen> &_oldPen, QPtrList<Brush> &_oldBrush,
		 Pen _newPen, Brush _newBrush, QPtrList<KPObject> &_objects, KPresenterDoc *_doc, int _flags = 0 );
    ~PenBrushCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KPresenterDoc *doc;
    QPtrList<Pen> oldPen;
    QPtrList<Brush> oldBrush;
    QPtrList<KPObject> objects;
    Pen newPen;
    Brush newBrush;
    int flags;

};

/******************************************************************/
/* Class: PgConfCmd                                               */
/******************************************************************/

class PgConfCmd : public KCommand
{
public:
    PgConfCmd( QString _name, bool _manualSwitch, bool _infinitLoop,
               PageEffect _pageEffect, PresSpeed _presSpeed,
               bool _oldManualSwitch, bool _oldInfinitLoop,
               PageEffect _oldPageEffect, PresSpeed _oldPresSpeed,
               KPresenterDoc *_doc, int _pgNum );

    virtual void execute();
    virtual void unexecute();

protected:
    bool manualSwitch, oldManualSwitch;
    bool infinitLoop, oldInfinitLoop;
    PageEffect pageEffect, oldPageEffect;
    PresSpeed presSpeed, oldPresSpeed;
    int pgNum;
    KPresenterDoc *doc;

};

/******************************************************************/
/* Class: PgLayoutCmd                                             */
/******************************************************************/

class PgLayoutCmd : public KCommand
{
public:
    PgLayoutCmd( QString _name, KoPageLayout _layout, KoPageLayout _oldLayout,
                 KPresenterView *_view );

    virtual void execute();
    virtual void unexecute();

protected:

    KoPageLayout layout, oldLayout;
    KPresenterView *view;

};

/******************************************************************/
/* Class: PieValueCmd                                             */
/******************************************************************/

class PieValueCmd : public KCommand
{
public:
    struct PieValues
    {
        PieType pieType;
        int pieAngle, pieLength;
    };

    PieValueCmd( QString _name, QPtrList<PieValues> &_oldValues, PieValues _newValues,
                 QPtrList<KPObject> &_objects, KPresenterDoc *_doc );
    ~PieValueCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KPresenterDoc *doc;
    QPtrList<PieValues> oldValues;
    QPtrList<KPObject> objects;
    PieValues newValues;

};

/******************************************************************/
/* Class: RectValueCmd                                            */
/******************************************************************/

class RectValueCmd : public KCommand
{
public:
    struct RectValues
    {
        int xRnd, yRnd;
    };

    RectValueCmd( QString _name, QPtrList<RectValues> &_oldValues, RectValues _newValues,
                  QPtrList<KPObject> &_objects, KPresenterDoc *_doc );
    ~RectValueCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KPresenterDoc *doc;
    QPtrList<RectValues> oldValues;
    QPtrList<KPObject> objects;
    RectValues newValues;

};

#endif
