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
#include <koPictureCollection.h>
#include <qvaluelist.h>
#include <qpen.h>
#include <qbrush.h>
#include <koPageLayoutDia.h>
#include <koparaglayout.h>
#include <kocommand.h>
#include <koPoint.h>
#include <koSize.h>
#include <qvariant.h>

class KPresenterDoc;
class KPTextObject;
class KPObject;
class KPPixmapObject;
class KPGroupObject;
class KPresenterView;
class KoParagLayout;
class KPrPage;
class KoCustomVariable;
class KoLinkVariable;
class KPPolylineObject;
class KPrFieldVariable;
class KPrTimeVariable;
class KPrDateVariable;
class KPrPgNumVariable;

/******************************************************************/
/* Class: ShadowCmd                                               */
/******************************************************************/

class ShadowCmd : public KNamedCommand
{
public:
    struct ShadowValues
    {
        int shadowDistance;
        ShadowDirection shadowDirection;
        QColor shadowColor;
    };

    ShadowCmd( const QString &_name, QPtrList<ShadowValues> &_oldShadow, ShadowValues _newShadow,
               QPtrList<KPObject> &_objects, KPresenterDoc *_doc );
    ~ShadowCmd();

    virtual void execute();
    virtual void unexecute();

protected:
    KPresenterDoc *doc;
    QPtrList<ShadowValues> oldShadow;
    QPtrList<KPObject> objects;
    ShadowValues newShadow;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: SetOptionsCmd                                           */
/******************************************************************/

class SetOptionsCmd : public KNamedCommand
{
public:
    SetOptionsCmd( const QString &_name, QValueList<KoPoint> &_diffs, QPtrList<KPObject> &_objects,
                   double _rastX, double _rastY, double _orastX, double _orastY,
                   const QColor &_txtBackCol, const QColor &_otxtBackCol, KPresenterDoc *_doc );
    ~SetOptionsCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QValueList<KoPoint> diffs;
    QPtrList<KPObject> objects;
    double gridX;
    double gridY;
    double oldGridX;
    double oldGridY;
    KPresenterDoc *doc;
    QColor txtBackCol;
    QColor otxtBackCol;
};

/******************************************************************/
/* Class: SetBackCmd						  */
/******************************************************************/

class SetBackCmd : public KNamedCommand
{
public:
    SetBackCmd( const QString &_name, const QColor &_backColor1, const QColor &_backColor2, BCType _bcType,
		bool _backUnbalanced, int _backXFactor, int _backYFactor,
		const KoPictureKey & _backPix,
                BackView _backView, BackType _backType,
		const QColor &_oldBackColor1, const QColor &_oldBackColor2, BCType _oldBcType,
		bool _oldBackUnbalanced, int _oldBackXFactor, int _oldBackYFactor,
		const KoPictureKey & _oldBackPix,
                BackView _oldBackView, BackType _oldBackType,
		bool _takeGlobal, KPresenterDoc *_doc, KPrPage *_page );

    virtual void execute();
    virtual void unexecute();

protected:

    QColor backColor1, backColor2;
    bool unbalanced;
    int xfactor, yfactor;
    KoPictureKey backPix;
    BCType bcType;
    BackView backView;
    BackType backType;
    QColor oldBackColor1, oldBackColor2;
    bool oldUnbalanced;
    int oldXFactor, oldYFactor;
    KoPictureKey oldBackPix;
    BCType oldBcType;
    BackView oldBackView;
    BackType oldBackType;
    bool takeGlobal;
    KPresenterDoc *doc;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: RotateCmd                                               */
/******************************************************************/

class RotateCmd : public KNamedCommand
{
public:
    struct RotateValues
    {
        float angle;
    };

    RotateCmd( const QString &_name, QPtrList<RotateValues> &_oldRotate, float _newAngle,
               QPtrList<KPObject> &_objects, KPresenterDoc *_doc, bool _addAngle = false );
    ~RotateCmd();

    virtual void execute();
    virtual void unexecute();

protected:
    KPresenterDoc *doc;
    QPtrList<RotateValues> oldRotate;
    QPtrList<KPObject> objects;
    float newAngle;
    //necessary for duplicate object, we can duplicated and add angle.
    bool addAngle;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: ResizeCmd                                               */
/******************************************************************/

class ResizeCmd : public KNamedCommand
{
public:
    ResizeCmd( const QString &_name, const KoPoint &_m_diff, const KoSize &_r_diff, KPObject *_object, KPresenterDoc *_doc );
    ~ResizeCmd();

    virtual void execute();
    virtual void unexecute();
    virtual void unexecute( bool _repaint );

protected:

    KoPoint m_diff;
    KoSize r_diff;
    KPObject *object;
    KPresenterDoc *doc;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: ChgPixCmd                                               */
/******************************************************************/

class ChgPixCmd : public KNamedCommand
{
public:
    ChgPixCmd( const QString &_name, KPPixmapObject *_oldObject, KPPixmapObject *_newObject,
               KPresenterDoc *_doc, KPrPage *_page );
    ~ChgPixCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KPPixmapObject *oldObject, *newObject;
    KPresenterDoc *doc;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: DeleteCmd                                               */
/******************************************************************/

class DeleteCmd : public KNamedCommand
{
public:
    DeleteCmd( const QString &_name, QPtrList<KPObject> &_objects, KPresenterDoc *_doc , KPrPage *_page);
    ~DeleteCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QPtrList<KPObject> objects;
    KPresenterDoc *doc;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: EffectCmd                                               */
/******************************************************************/

class EffectCmd : public KNamedCommand
{
public:
    struct EffectStruct {
	int presNum, disappearNum;
	Effect effect;
	Effect2 effect2;
	Effect3 effect3;
	bool disappear;
	int appearTimer, disappearTimer;
        bool appearSoundEffect, disappearSoundEffect;
        QString a_fileName, d_fileName;
    };

    EffectCmd( const QString &_name, const QPtrList<KPObject> &_objs,
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

class GroupObjCmd : public KNamedCommand
{
public:
    GroupObjCmd( const QString &_name,
		 const QPtrList<KPObject> &_objects,
		 KPresenterDoc *_doc, KPrPage *_page );
    ~GroupObjCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QPtrList<KPObject> objects;
    KPresenterDoc *doc;
    KPGroupObject *grpObj;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: UnGroupObjCmd						  */
/******************************************************************/

class UnGroupObjCmd : public KNamedCommand
{
public:
    UnGroupObjCmd( const QString &_name,
		 KPGroupObject *grpObj_,
		 KPresenterDoc *_doc, KPrPage *_page );
    ~UnGroupObjCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QPtrList<KPObject> objects;
    KPresenterDoc *doc;
    KPGroupObject *grpObj;
    KPrPage *m_page;

};


/******************************************************************/
/* Class: InsertCmd                                               */
/******************************************************************/

class InsertCmd : public KNamedCommand
{
public:
    InsertCmd( const QString &_name, KPObject *_object, KPresenterDoc *_doc, KPrPage *_page );
    ~InsertCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KPObject *object;
    KPresenterDoc *doc;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: LowerRaiseCmd                                           */
/******************************************************************/

class LowerRaiseCmd : public KNamedCommand
{
public:
    LowerRaiseCmd( const QString &_name, QPtrList<KPObject> _oldList, QPtrList<KPObject> _newList, KPresenterDoc *_doc, KPrPage *_page );
    ~LowerRaiseCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    QPtrList<KPObject> oldList, newList;
    KPresenterDoc *doc;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: MoveByCmd                                               */
/******************************************************************/

class MoveByCmd : public KNamedCommand
{
public:
    MoveByCmd( const QString &_name, const KoPoint &_diff, QPtrList<KPObject> &_objects, KPresenterDoc *_doc, KPrPage *_page );
    ~MoveByCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KoPoint diff;
    QPtrList<KPObject> objects;
    KPresenterDoc *doc;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: MoveByCmd2                                              */
/******************************************************************/

class MoveByCmd2 : public KNamedCommand
{
public:
    MoveByCmd2( const QString &_name, QPtrList<KoPoint> &_diffs, QPtrList<KPObject> &_objects, KPresenterDoc *_doc, KPrPage *_page );
    ~MoveByCmd2();

    virtual void execute();
    virtual void unexecute();

protected:

    QPtrList<KoPoint> diffs;
    QPtrList<KPObject> objects;
    KPresenterDoc *doc;
    KPrPage *m_page;
};

/******************************************************************/
/* Class: PenCmd						  */
/******************************************************************/

class PenCmd : public KNamedCommand
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

    // the flags indicate what has changed
    enum PenConfigChange {
        LineBegin = 1,
        LineEnd = 2,
        Color = 4,
        Width = 8,
        Style = 16,
        All = LineBegin | LineEnd | Color | Width | Style
    };

    PenCmd(const QString &_name, QPtrList<Pen> &_oldPen, Pen _newPen,
           QPtrList<KPObject> &_objects, KPresenterDoc *_doc, KPrPage *_page, int _flags = All);
    ~PenCmd();
    void applyPen(KPObject *kpobject, Pen *tmpPen);

    virtual void execute();
    virtual void unexecute();

protected:
    KPresenterDoc *doc;
    KPrPage *m_page;
    QPtrList<Pen> oldPen;
    QPtrList<KPObject> objects;
    Pen newPen;
    int flags;
};

/******************************************************************/
/* Class: BrushCmd						  */
/******************************************************************/

class BrushCmd : public KNamedCommand
{
public:
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

    // the flags indicate what has changed
    enum BrushConfigChange {
        BrushColor = 1,
        BrushStyle = 2,
        BrushGradientSelect = 4,
        GradientColor1 = 8,
        GradientColor2 = 16,
        GradientType = 32,
        GradientBalanced = 64,
        All = BrushColor | BrushStyle | BrushGradientSelect | GradientColor1 | GradientColor2 | GradientType | GradientBalanced
    };

    BrushCmd(const QString &_name, QPtrList<Brush> &_oldBrush, Brush _newBrush,
             QPtrList<KPObject> &_objects, KPresenterDoc *_doc, KPrPage *_page, int _flags = All);
    ~BrushCmd();
    void applyBrush(KPObject *kpobject, Brush *tmpBrush);

    virtual void execute();
    virtual void unexecute();

protected:
    KPresenterDoc *doc;
    QPtrList<Brush> oldBrush;
    QPtrList<KPObject> objects;
    Brush newBrush;
    KPrPage *m_page;
    int flags;
};

/******************************************************************/
/* Class: PgConfCmd                                               */
/******************************************************************/

class PgConfCmd : public KNamedCommand
{
public:
    PgConfCmd( const QString &_name, bool _manualSwitch, bool _infiniteLoop,
               bool _showPresentationDuration,
               bool _oldManualSwitch, bool _oldInfiniteLoop,
               bool _oldShowPresentationDuration,
               KPresenterDoc *_doc );

    virtual void execute();
    virtual void unexecute();

protected:
    bool manualSwitch, oldManualSwitch;
    bool infiniteLoop, oldInfiniteLoop;
    bool showPresentationDuration, oldShowPresentationDuration;
    KPresenterDoc *doc;
};

/******************************************************************/
/* Class: TransEffectCmd                                               */
/******************************************************************/

class TransEffectCmd : public KNamedCommand
{
public:
    TransEffectCmd( const QString &_name, PageEffect _pageEffect, PresSpeed _presSpeed, 
               bool _soundEffect, const QString& _soundFileName, 
               bool _autoAdvance, int _slideTime, 
               PageEffect _oldPageEffect, PresSpeed _oldPresSpeed, 
               bool _oldSoundEffect, const QString& _oldSoundFileName, 
               bool _oldAutoAdvance, int _oldSlideTime, 
               KPresenterDoc *_doc, KPrPage *_page );

    virtual void execute();
    virtual void unexecute();

protected:
    PageEffect pageEffect, oldPageEffect;
    PresSpeed presSpeed, oldPresSpeed;
    bool soundEffect, oldSoundEffect;
    QString soundFileName, oldSoundFileName;
    bool autoAdvance, oldAutoAdvance;
    int slideTime, oldSlideTime;

    KPrPage *m_page;
    KPresenterDoc *doc;

};

/******************************************************************/
/* Class: PgLayoutCmd                                             */
/******************************************************************/

class PgLayoutCmd : public KNamedCommand
{
public:
    PgLayoutCmd( const QString &_name, KoPageLayout _layout, KoPageLayout _oldLayout,
                 KoUnit::Unit _oldUnit, KoUnit::Unit _unit,KPresenterDoc *_doc );

    virtual void execute();
    virtual void unexecute();

protected:
    KPresenterDoc *m_doc;
    KoPageLayout layout, oldLayout;
    KoUnit::Unit unit, oldUnit;
};

/******************************************************************/
/* Class: PieValueCmd                                             */
/******************************************************************/

class PieValueCmd : public KNamedCommand
{
public:
    struct PieValues
    {
        PieType pieType;
        int pieAngle, pieLength;
    };

    // the flags indicate what has changed
    enum PieConfigChange {
        Type = 1,
        Angle = 2,
        Length = 4,
        All = Type | Angle | Length
    };

    PieValueCmd( const QString &_name, QPtrList<PieValues> &_oldValues, PieValues _newValues,
                 QPtrList<KPObject> &_objects, KPresenterDoc *_doc, KPrPage *_page, int _flags = All );
    ~PieValueCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KPresenterDoc *doc;
    KPrPage *m_page;
    QPtrList<PieValues> oldValues;
    QPtrList<KPObject> objects;
    PieValues newValues;
    int flags;
};

/******************************************************************/
/* Class: PolygonSettingCmd                                       */
/******************************************************************/

class PolygonSettingCmd : public KNamedCommand
{
public:
    struct PolygonSettings
    {
        bool checkConcavePolygon;
        int cornersValue;
        int sharpnessValue;
    };

    // the flags indicate what has changed
    enum PolygonConfigChange {
        ConcaveConvex = 1,
        Corners = 2,
        Sharpness = 4,
        All = ConcaveConvex | Corners | Sharpness
    };

    PolygonSettingCmd( const QString &_name, QPtrList<PolygonSettings> &_oldSettings,
                       PolygonSettings _newSettings, QPtrList<KPObject> &_objects, KPresenterDoc *_doc,  KPrPage *_page, int _flags = All );
    ~PolygonSettingCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KPresenterDoc *doc;
    KPrPage *m_page;
    QPtrList<PolygonSettings> oldSettings;
    QPtrList<KPObject> objects;
    PolygonSettings newSettings;
    int flags;
};

/******************************************************************/
/* Class: PictureSettingCmd                                       */
/******************************************************************/

class PictureSettingCmd : public KNamedCommand
{
public:
    struct PictureSettings
    {
        PictureMirrorType mirrorType;
        int depth;
        bool swapRGB;
        bool grayscal;
        int bright;
    };

    PictureSettingCmd( const QString &_name, QPtrList<PictureSettings> &_oldSettings,
                       PictureSettings _newSettings, QPtrList<KPObject> &_objects, KPresenterDoc *_doc );
    ~PictureSettingCmd();

    virtual void execute();
    virtual void unexecute();

protected:
    KPresenterDoc *doc;
    QPtrList<PictureSettings> oldSettings;
    QPtrList<KPObject> objects;
    PictureSettings newSettings;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: ImageEffectCmd                                          */
/******************************************************************/

class ImageEffectCmd : public KNamedCommand
{
public:
    struct ImageEffectSettings
    {
        ImageEffect effect;
        QVariant param1;
        QVariant param2;
        QVariant param3;
    };

    ImageEffectCmd(const QString &_name, QPtrList<ImageEffectSettings> &_oldSettings,
                   ImageEffectSettings _newSettings, QPtrList<KPObject> &_objects,
                   KPresenterDoc *_doc );
    ~ImageEffectCmd();

    virtual void execute();
    virtual void unexecute();

protected:
    KPresenterDoc *doc;
    QPtrList<ImageEffectSettings> oldSettings;
    QPtrList<KPObject> objects;
    ImageEffectSettings newSettings;
    KPrPage *m_page;

};

/******************************************************************/
/* Class: RectValueCmd                                            */
/******************************************************************/

class RectValueCmd : public KNamedCommand
{
public:
    struct RectValues
    {
        int xRnd, yRnd;
    };

    // the flags indicate what has changed
    enum RectangleConfigChange {
        XRnd = 1,
        YRnd = 2,
        All = XRnd | YRnd
    };

    RectValueCmd( const QString &_name, QPtrList<RectValues> &_oldValues, RectValues _newValues,
                  QPtrList<KPObject> &_objects, KPresenterDoc *_doc, KPrPage *_page, int _flags = All );
    ~RectValueCmd();

    virtual void execute();
    virtual void unexecute();

protected:

    KPresenterDoc *doc;
    KPrPage *m_page;
    QPtrList<RectValues> oldValues;
    QPtrList<KPObject> objects;
    RectValues newValues;
    int flags;
};

/******************************************************************/
/* Class: DeletePageCmd                                           */
/******************************************************************/

class KPrDeletePageCmd : public KNamedCommand
{
public:
    KPrDeletePageCmd( const QString &_name,int _pos, KPrPage *page, KPresenterDoc *_doc );
    ~KPrDeletePageCmd();

    virtual void execute();
    virtual void unexecute();

protected:
    KPresenterDoc *doc;
    KPrPage *m_page;
    int position;
};

/******************************************************************/
/* Class: KPrInsertPageCmd                                        */
/******************************************************************/

class KPrInsertPageCmd : public KNamedCommand
{
public:
    KPrInsertPageCmd( const QString &_name,int _pos, KPrPage *page, KPresenterDoc *_doc );
    ~KPrInsertPageCmd();

    virtual void execute();
    virtual void unexecute();
protected:
    KPresenterDoc *doc;
    KPrPage *m_page;
    int position;
};

class KPrMovePageCmd : public KNamedCommand
{
public:
    KPrMovePageCmd( const QString &_name,int _oldpos,int newPos, KPrPage *page, KPresenterDoc *_doc );
    ~KPrMovePageCmd();

    virtual void execute();
    virtual void unexecute();
protected:
    KPresenterDoc *doc;
    KPrPage *m_page;
    int oldPosition;
    int newPosition;
};

/******************************************************************/
/* Class: KPrPasteTextCommand                                     */
/******************************************************************/
class KPrPasteTextCommand : public KoTextDocCommand
{
public:
    KPrPasteTextCommand( KoTextDocument *d, int parag, int idx,
                    const QCString & data );
    ~KPrPasteTextCommand() {}
    KoTextCursor *execute( KoTextCursor *c );
    KoTextCursor *unexecute( KoTextCursor *c );
protected:
    int m_parag;
    int m_idx;
    QCString m_data;
    // filled in by execute(), for unexecute()
    int m_lastParag;
    int m_lastIndex;
    KoParagLayout m_oldParagLayout;
};

/**
 * Command to change variable setting
 */
class KPrChangeStartingPageCommand : public KNamedCommand
{
public:
    KPrChangeStartingPageCommand( const QString &name, KPresenterDoc *_doc, int _oldStartingPage, int _newStartingPage);
    ~KPrChangeStartingPageCommand(){}

    void execute();
    void unexecute();
protected:
    KPresenterDoc *m_doc;
    int oldStartingPage;
    int newStartingPage;
};

/**
 * Command to display link setting
 */
class KPrChangeVariableSettingsCommand : public KNamedCommand
{
public:
    enum VariableProperties { VS_DISPLAYLINK, VS_UNDERLINELINK, VS_DISPLAYCOMMENT, VS_DISPLAYFIELDCODE};
    KPrChangeVariableSettingsCommand( const QString &name, KPresenterDoc *_doc, bool _oldValue, bool _newValue, VariableProperties _type);
    ~KPrChangeVariableSettingsCommand(){}

    void execute();
    void unexecute();
protected:
    void changeValue( bool b );
    KPresenterDoc *m_doc;
    VariableProperties type;
    bool m_bOldValue;
    bool m_bNewValue;
};

/**
 * Command to change title page name
 */
class KPrChangeTitlePageNameCommand : public KNamedCommand
{
public:
    KPrChangeTitlePageNameCommand( const QString &name, KPresenterDoc *_doc, const QString &_oldPageName, const QString &_newPageName, KPrPage *_page);
    ~KPrChangeTitlePageNameCommand(){}

    void execute();
    void unexecute();
protected:
    KPresenterDoc *m_doc;
    QString oldPageName;
    QString newPageName;
    KPrPage *m_page;
};

class KPrChangeCustomVariableValue : public KNamedCommand
{
 public:
    KPrChangeCustomVariableValue( const QString &name, KPresenterDoc *_doc,const QString & _oldValue, const QString & _newValue, KoCustomVariable *var);

    void execute();
    void unexecute();
 protected:
    KPresenterDoc *m_doc;
    QString newValue;
    QString oldValue;
    KoCustomVariable *m_var;
};

class KPrChangeFieldVariableSubType : public KNamedCommand
{
 public:
    KPrChangeFieldVariableSubType( const QString &name, KPresenterDoc *_doc, short int _oldValue, short int _newValue, KPrFieldVariable *var);
    ~KPrChangeFieldVariableSubType();
    void execute();
    void unexecute();
 protected:
    KPresenterDoc *m_doc;
    short int newValue;
    short int oldValue;
    KPrFieldVariable *m_var;
};

class KPrChangeTimeVariableSubType : public KNamedCommand
{
 public:
    KPrChangeTimeVariableSubType( const QString &name, KPresenterDoc *_doc, short int _oldValue, short int _newValue, KPrTimeVariable *var);
    ~KPrChangeTimeVariableSubType();
    void execute();
    void unexecute();
 protected:
    KPresenterDoc *m_doc;
    short int newValue;
    short int oldValue;
    KPrTimeVariable *m_var;
};

class KPrChangeTimeVariableFormat : public KNamedCommand
{
 public:
    KPrChangeTimeVariableFormat( const QString &name, KPresenterDoc *_doc, const QString _oldValue, const QString _newValue, KPrTimeVariable *var);
    ~KPrChangeTimeVariableFormat();
    void execute();
    void unexecute();
 protected:
    KPresenterDoc *m_doc;
    QString newValue;
    QString oldValue;
    KPrTimeVariable *m_var;
};

class KPrChangeDateVariableSubType : public KNamedCommand
{
 public:
    KPrChangeDateVariableSubType( const QString &name, KPresenterDoc *_doc, short int _oldValue, short int _newValue, KPrDateVariable *var);
    ~KPrChangeDateVariableSubType();
    void execute();
    void unexecute();
 protected:
    KPresenterDoc *m_doc;
    short int newValue;
    short int oldValue;
    KPrDateVariable *m_var;
};

class KPrChangeDateVariableFormat : public KNamedCommand
{
 public:
    KPrChangeDateVariableFormat( const QString &name, KPresenterDoc *_doc, const QString _oldValue, const QString _newValue, KPrDateVariable *var);
    ~KPrChangeDateVariableFormat();
    void execute();
    void unexecute();
 protected:
    KPresenterDoc *m_doc;
    QString newValue;
    QString oldValue;
    KPrDateVariable *m_var;
};

class KPrChangePgNumVariableValue : public KNamedCommand
{
 public:
    KPrChangePgNumVariableValue( const QString &name, KPresenterDoc *_doc, short int _oldValue, short int _newValue, KPrPgNumVariable *var);
    ~KPrChangePgNumVariableValue();
    void execute();
    void unexecute();
 protected:
    KPresenterDoc *m_doc;
    short int newValue;
    short int oldValue;
    KPrPgNumVariable *m_var;
};


class KPrChangeLinkVariable : public KNamedCommand
{
 public:
    KPrChangeLinkVariable( const QString &name, KPresenterDoc *_doc,const QString & _oldHref, const QString & _newHref, const QString & _oldLink,const QString &_newLink, KoLinkVariable *var);
    ~KPrChangeLinkVariable(){};
    void execute();
    void unexecute();
 protected:
    KPresenterDoc *m_doc;
    QString oldHref;
    QString newHref;
    QString oldLink;
    QString newLink;
    KoLinkVariable *m_var;
};

class KPrStickyObjCommand : public KNamedCommand
{
 public:
    KPrStickyObjCommand( const QString &_name, QPtrList<KPObject> &_objects, bool sticky , KPrPage*_page,KPresenterDoc *_doc );

    ~KPrStickyObjCommand();
    void execute();
    void unexecute();
    void stickObj(KPObject *_obj);
    void unstickObj(KPObject *_obj);
 protected:
    QPtrList<KPObject> objects;
    bool m_bSticky;
    KPresenterDoc *m_doc;
    KPrPage*m_page;
};

class KPrHideShowHeaderFooter : public KNamedCommand
{
 public:
    KPrHideShowHeaderFooter( const QString &name, KPresenterDoc *_doc, bool _newValue,KPTextObject *_textObject);
    ~KPrHideShowHeaderFooter(){};
    void execute();
    void unexecute();
 protected:
    KPresenterDoc *m_doc;
    KPTextObject *m_textObject;
    bool newValue;
};

class KPrFlipObjectCommand : public KNamedCommand
{
public:
    KPrFlipObjectCommand( const QString &name, KPresenterDoc *_doc, bool _horizontal ,KPObject *_obj);
    ~KPrFlipObjectCommand() {};
    void execute();
    void unexecute();
protected:
    void flipObject();
    KPresenterDoc *m_doc;
    KPObject *m_object;
    bool horizontal;
    KPrPage *m_page;
};

class KPrGeometryPropertiesCommand : public KNamedCommand
{
public:
    enum KgpType { ProtectSize, KeepRatio};
    KPrGeometryPropertiesCommand( const QString &_name, QValueList<bool> &_lst, QPtrList<KPObject> &_objects, bool _newValue, KPresenterDoc *_doc, KgpType _type );
    ~KPrGeometryPropertiesCommand();

    virtual void execute();
    virtual void unexecute();

protected:
    QValueList<bool> list;
    QPtrList<KPObject> objects;
    bool newValue;
    KPresenterDoc *doc;
    KgpType m_type;
};

class KPrProtectContentCommand : public KNamedCommand
{
public:
    KPrProtectContentCommand( const QString &_name, bool _protectContent, KPTextObject *_obj, KPresenterDoc *_doc );

    ~KPrProtectContentCommand();
    virtual void execute();
    virtual void unexecute();

protected:
    bool protectContent;
    KPTextObject *objects;
    KPresenterDoc * doc;
};

class KPrCloseObjectCommand : public KNamedCommand
{
public:
    KPrCloseObjectCommand( const QString &_name, KPObject *_obj, KPresenterDoc *_doc );

    ~KPrCloseObjectCommand();
    virtual void execute();
    virtual void unexecute();

protected:
    void closeObject(bool close);

    KPObject *objects;
    KPresenterDoc * doc;
    KPrPage *m_page;
};

struct MarginsStruct {
    MarginsStruct() {}
    MarginsStruct( KPTextObject *obj );
    MarginsStruct( double _left, double top, double right, double bottom );
    double topMargin;
    double bottomMargin;
    double leftMargin;
    double rightMargin;
};

class KPrChangeMarginCommand : public KNamedCommand
{
public:
    KPrChangeMarginCommand( const QString &name, KPTextObject *_obj, MarginsStruct _MarginsBegin, MarginsStruct _MarginsEnd, KPresenterDoc *_doc );
    ~KPrChangeMarginCommand() {}

    virtual void execute();
    virtual void unexecute();
protected:
    KPTextObject *m_obj;
    MarginsStruct m_marginsBegin;
    MarginsStruct m_marginsEnd;
    KPrPage *m_page;
    KPresenterDoc *m_doc;
};


class KPrChangeVerticalAlignmentCommand : public KNamedCommand
{
public:
    KPrChangeVerticalAlignmentCommand( const QString &name, KPTextObject *_obj, VerticalAlignmentType _oldAlign, VerticalAlignmentType _newAlign, KPresenterDoc *_doc);
    ~KPrChangeVerticalAlignmentCommand() {}

    virtual void execute();
    virtual void unexecute();
protected:
    KPTextObject *m_obj;
    VerticalAlignmentType m_oldAlign;
    VerticalAlignmentType m_newAlign;
    KPrPage *m_page;
    KPresenterDoc *m_doc;
};


class KPrChangeTabStopValueCommand : public KNamedCommand
{
public:
    KPrChangeTabStopValueCommand( const QString &name, double _oldValue, double _newValue, KPresenterDoc *_doc);

    ~KPrChangeTabStopValueCommand() {}

    virtual void execute();
    virtual void unexecute();
protected:
    KPresenterDoc *m_doc;
    double m_oldValue;
    double m_newValue;
};

#endif

