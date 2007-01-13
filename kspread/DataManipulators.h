/* This file is part of the KDE project
   Copyright (C) 2006 Tomas Mecir <mecirt@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#ifndef KSPREAD_MANIPULATOR_DATA
#define KSPREAD_MANIPULATOR_DATA

#include "Manipulator.h"
#include "Global.h"
#include "RectStorage.h"
#include "Style.h"
#include "Value.h"
#include <koffice_export.h>

#include <QList>
#include <QStringList>

namespace KSpread {


/**
 * AbstractDataManipulator - provides storage of old cell data (for undo)
 * and has an abstract method for the actual setting of new values
 */

struct ADMStorage {
  Value val;
  QString text;
  Format::Type format;
};

class AbstractDataManipulator : public Manipulator {
  public:
    AbstractDataManipulator ();
    virtual ~AbstractDataManipulator ();
    virtual bool process (Element* element);
  protected:
    /** Return new value. row/col are relative to sheet, not element.
    If the function sets *parse to true, the value will be treated as an
    user-entered string and parsed by Cell. */
    virtual Value newValue (Element *element, int col, int row,
      bool *parse, Format::Type *fmtType) = 0;

    /** do we want to change this cell ? */
    virtual bool wantChange (Element *element, int col, int row) {
      Q_UNUSED(element)
      Q_UNUSED(col)
      Q_UNUSED(row)
      return true;
    };
    
    /** return value stored at given coordinates */
    Value stored (int col, int row, bool *parse);

    /** preProcessing will store the old cell's data */
    virtual bool preProcessing ();
    QMap<int, QMap<int, ADMStorage> > oldData;
};

/**
AbstractDFManipulator: AbstractDataManipulator with the option of copying
entire cell formats.
*/
class AbstractDFManipulator : public AbstractDataManipulator {
  public:
    AbstractDFManipulator ();
    virtual ~AbstractDFManipulator ();
    virtual bool process (Element* element);

    /** returns whether this manipulator changes formats */
    bool changeFormat () { return m_changeformat; };
    /** set whether this manipulator changes formats */
    void setChangeFormat (bool chf) { m_changeformat = chf; };
  protected:
    /** this method should return new format for a given cell */
    virtual Style newFormat (Element *element, int col, int row) = 0;

    /** preProcessing will store the old cell's data */
    virtual bool preProcessing ();

    QMap<int, QMap<int, Style> > formats;
    bool m_changeformat;
};


/**
 * DataManipulator - allows setting values on one range.
 * If multiple ranges are selected, they all get set to the same values
 */

class KSPREAD_EXPORT DataManipulator : public AbstractDataManipulator {
  public:
    DataManipulator ();
    virtual ~DataManipulator ();
    void setParsing (bool val) { m_parsing = val; };
    /** set the values for the range. Can be either a single value, or
    a value array */
    void setValue (Value val) { data = val; };
    /** If set, all cells shall be switched to this format. If parsing is
    true, the resulting value may end up being different. */
    void setFormat (Format::Type fmtType) { m_format = fmtType; };
  protected:
    virtual Value newValue (Element *element, int col, int row, bool *, Format::Type *);

    Value data;
    Format::Type m_format;
    bool m_parsing : 1;
};

class KSPREAD_EXPORT SeriesManipulator : public AbstractDataManipulator {
  public:
    enum Series { Column, Row, Linear, Geometric };
    
    SeriesManipulator ();
    virtual ~SeriesManipulator ();
    
    /** Setup the series. This sets the necessary parameters, and also the
    correct range. */
    void setupSeries (const QPoint &_marker, double start, double end,
        double step, Series mode, Series type);
  protected:
    virtual Value newValue (Element *element, int col, int row, bool *,
        Format::Type *);
    
    Series m_type;
    Value m_start, m_step, m_prev;
    int m_last;
};

class KSPREAD_EXPORT ArrayFormulaManipulator : public AbstractDataManipulator {
  public:
    ArrayFormulaManipulator ();
    virtual ~ArrayFormulaManipulator ();
    void setText (const QString text) { m_text = text; };
  protected:
    virtual Value newValue (Element *element, int col, int row, bool *,
        Format::Type *);
    QString cellRef, m_text;
};

/** the FillManipulator is used in the Fill operation */
class KSPREAD_EXPORT FillManipulator : public AbstractDFManipulator {
  public:
    FillManipulator ();
    virtual ~FillManipulator ();
    
    enum Direction { Up = 0, Down, Left, Right };

    void setDirection (Direction d) { m_dir = d; };
  protected:
    virtual Value newValue (Element *element, int col, int row,
        bool *parse, Format::Type *fmtType);
    virtual Style newFormat (Element *element, int col, int row);
    Direction m_dir;
};

/** CaseManipulator converts data to uppercase/lowercase/... */
class KSPREAD_EXPORT CaseManipulator: public AbstractDataManipulator {
  public:
    CaseManipulator ();
    virtual ~CaseManipulator ();
    
    enum CaseMode {
      Upper = 0,
      Lower,
      FirstUpper
    };
    void changeMode (CaseMode mode) { m_mode = mode; };
    void changeLowerCase ();
    void changeFirstUpper ();
  protected:
    virtual Value newValue (Element *element, int col, int row,
        bool *parse, Format::Type *fmtType);

    /** do we want to change this cell ? */
    virtual bool wantChange (Element *element, int col, int row);
    
    CaseMode m_mode;
};



class ShiftManipulator : public Manipulator
{
public:
    enum Direction { ShiftRight, ShiftBottom };
    ShiftManipulator();
    void setDirection( Direction direction ) { m_direction = direction; }

protected:
    bool process(Element*);
    bool postProcessing();
    QString name() const;

private:
    Direction m_direction;
    QList< QPair<QRectF,SharedSubStyle> > m_undoStyles;
    QList< QPair<QRectF,QString> >        m_undoComment;
    QList< QPair<QRectF,Conditions> >     m_undoConditions;
    QList< QPair<QRectF,Validity> >       m_undoValidity;
    QHash<QPoint,QString>                 m_undoCells;
};

} // namespace KSpread

#endif  // KSPREAD_MANIPULATOR_DATA
