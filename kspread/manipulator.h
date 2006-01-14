/* This file is part of the KDE project
   Copyright (C) 2005 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
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


#ifndef KSPREAD_MANIPULATOR
#define KSPREAD_MANIPULATOR

#include <qrect.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <kcommand.h>
#include <klocale.h>

#include <koffice_export.h>

#include "kspread_format.h"
#include "kspread_undo.h"
#include "region.h"

namespace KSpread
{
class Cell;
class ColumnFormat;
class RowFormat;
class Sheet;


// struct layoutCell {
//   int row;
//   int col;
//   Format *l;
// };
// 
// struct layoutColumn {
//   int col;
//   ColumnFormat *l;
// };
// 
// struct layoutRow {
//   int row;
//   RowFormat *l;
// };



/**
 * Manipulator
 */
class Manipulator : public Region, public KCommand
{
public:
  Manipulator();
  virtual ~Manipulator();

  Sheet* sheet() const { return m_sheet; }
  void setSheet(Sheet* sheet) { m_sheet = sheet; }

  bool creation() { return m_creation; }
  void setCreation(bool creation) { m_creation = creation; }

  /** Is this a formatting manipulator ? If so, execute will call
  process(Format*) for each complete row/column, instead of going
  cell-by-cell. True by default. */
  bool format() { return m_format; };
  void setFormat (bool f) { m_format = f; };

  virtual void execute();
  virtual void unexecute();

  virtual void setArgument(const QString& /*arg*/, const QString& /*val*/) {};

  virtual void setReverse(bool reverse) { m_reverse = reverse; }
  void setRegisterUndo(bool registerUndo) { m_register = registerUndo; }

  virtual void setName (const QString &n) { m_name = n; }
  virtual QString name() const { return m_name; };

protected:
  virtual bool process(Element*);
  virtual bool process(Cell*) { return true; }
  virtual bool process(Format*) { return true; }

  virtual bool preProcessing() { return true; }
  virtual bool postProcessing() { return true; }


  Sheet* m_sheet;
  QString m_name;
  bool   m_creation : 1;
  bool   m_reverse  : 1;
  bool   m_firstrun : 1;
  bool   m_format   : 1;
  bool   m_register : 1;
private:
};



/**
 * FormatManipulator
 */
class FormatManipulator : public Manipulator
{
public:
  FormatManipulator();
  virtual ~FormatManipulator();

  void setProperty(Format::Properties property) { m_properties |= property; }
  bool isEmpty() { return m_properties == 0; }

  // SetSelectionFontWorker
  // SetSelectionSizeWorker
  void setFontFamily(const QString& font) { m_properties |= Format::PFont; m_font = font; }
  void setFontSize(int size) { m_properties |= Format::PFont; m_size = size; }
  void setFontBold(uint bold) { m_properties |= Format::PFont; m_bold = bold; }
  void setFontItalic(uint italic) { m_properties |= Format::PFont; m_italic = italic; }
  void setFontStrike(uint strike) { m_properties |= Format::PFont; m_strike = strike; }
  void setFontUnderline(uint underline) { m_properties |= Format::PFont; m_underline = underline; }
  // SetSelectionAngleWorker
  void setAngle(int angle) { m_properties |= Format::PAngle; m_angle = angle; }
  // SetSelectionTextColorWorker
  void setTextColor(const QColor& textColor) { m_properties |= Format::PTextPen; m_textColor = textColor; }
  // SetSelectionBgColorWorker
  void setBackgroundColor(const QColor& bgColor) { m_properties |= Format::PBackgroundColor; m_backgroundColor = bgColor; }
  // SetSelectionBorderAllWorker
  void setTopBorderPen(const QPen& pen) { m_properties |= Format::PTopBorder; m_topBorderPen = pen; }
  void setBottomBorderPen(const QPen& pen) { m_properties |= Format::PBottomBorder; m_bottomBorderPen = pen; }
  void setLeftBorderPen(const QPen& pen) { m_properties |= Format::PLeftBorder; m_leftBorderPen = pen; }
  void setRightBorderPen(const QPen& pen) { m_properties |= Format::PRightBorder; m_rightBorderPen = pen; }
  void setHorizontalPen(const QPen& pen) { m_properties |= Format::PTopBorder | Format::PBottomBorder; m_horizontalPen = pen; }
  void setVerticalPen(const QPen& pen) { m_properties |= Format::PLeftBorder | Format::PRightBorder; m_verticalPen = pen; }
  void setFallDiagonalPen(const QPen& pen) { m_properties |= Format::PFallDiagonal; m_fallDiagonalPen = pen; }
  void setGoUpDiagonalPen(const QPen& pen) { m_properties |= Format::PGoUpDiagonal; m_goUpDiagonalPen = pen; }
  // SetSelectionAlignWorker
  void setHorizontalAlignment(Format::Align align) { m_properties |= Format::PAlign; m_horAlign = align; }
  // SetSelectionAlignWorker
  void setVerticalAlignment(Format::AlignY align) { m_properties |= Format::PAlignY; m_verAlign = align; }

  void setBackgroundBrush(const QBrush& brush) { m_properties |= Format::PBackgroundBrush; m_backgroundBrush = brush; }
  void setIndent(double indent) { m_properties |= Format::PIndent; m_indent = indent; }
  void setMultiRow(bool multiRow) { m_properties |= Format::PMultiRow; m_multiRow = multiRow; }
  void setVerticalText(bool verticalText) { m_properties |= Format::PVerticalText; m_verticalText = verticalText; }
  void setDontPrintText(bool dontPrintText) { m_properties |= Format::PDontPrintText; m_dontPrintText = dontPrintText; }
  void setNotProtected(bool notProtected) { m_properties |= Format::PNotProtected; m_notProtected = notProtected; }
  void setHideAll(bool hideAll) { m_properties |= Format::PHideAll; m_hideAll = hideAll; }
  void setHideFormula(bool hideFormula) { m_properties |= Format::PHideFormula; m_hideFormula = hideFormula; }
  void setComment(const QString& comment) { m_properties |= Format::PComment; m_comment = comment; }
  void setPrefix(const QString& prefix) { m_properties |= Format::PPrefix; m_prefix = prefix; }
  void setPostfix(const QString& postfix) { m_properties |= Format::PPostfix; m_postfix = postfix; }
  void setPrecision(int precision) { m_properties |= Format::PPrecision; m_precision = precision; }
  void setFloatFormat(Format::FloatFormat floatFormat) { m_properties |= Format::PFloatFormat; m_floatFormat = floatFormat; }
  void setFloatColor(Format::FloatColor floatColor) { m_properties |= Format::PFloatColor; m_floatColor = floatColor; }
  void setFormatType(FormatType formatType) { m_properties |= Format::PFormatType; m_formatType = formatType; }
  void setCurrency(int type, const QString& symbol) { m_currencyType = type; m_currencySymbol = symbol; }

protected:
  virtual QString name() const { return i18n("Format Change"); }

  virtual bool preProcessing();
  virtual bool process(Element*);

  void copyFormat(QValueList<layoutCell> &list,
                  QValueList<layoutColumn> &listCol,
                  QValueList<layoutRow> &listRow);
  bool testCondition(RowFormat*);
  void doWork(Format*, bool isTop, bool isBottom, bool isLeft, bool isRight);
  void prepareCell(Cell*);

private:
  Q_UINT32 m_properties;

  // TODO Stefan: find a more elegant way to store the format
  QValueList<layoutCell> m_lstFormats;
  QValueList<layoutCell> m_lstRedoFormats;
  QValueList<layoutColumn> m_lstColFormats;
  QValueList<layoutColumn> m_lstRedoColFormats;
  QValueList<layoutRow> m_lstRowFormats;
  QValueList<layoutRow> m_lstRedoRowFormats;

  // SetSelectionFontWorker
  // SetSelectionSizeWorker
  QString m_font;
  int m_size;
  signed char m_bold;
  signed char m_italic;
  signed char m_strike;
  signed char m_underline;
  // SetSelectionAngleWorker
  int m_angle;
  int m_precision;
  int m_currencyType;
  double m_indent;
  bool m_multiRow;
  bool m_verticalText;
  bool m_dontPrintText;
  bool m_notProtected;
  bool m_hideAll;
  bool m_hideFormula;

  // SetSelectionTextColorWorker
  QColor m_textColor;
  // SetSelectionBgColorWorker
  QColor m_backgroundColor;
  // SetSelectionBorderAllWorker
  QPen m_topBorderPen;
  QPen m_bottomBorderPen;
  QPen m_leftBorderPen;
  QPen m_rightBorderPen;
  QPen m_horizontalPen;
  QPen m_verticalPen;
  QPen m_fallDiagonalPen;
  QPen m_goUpDiagonalPen;

  QBrush m_backgroundBrush;
  QString m_comment;
  QString m_prefix;
  QString m_postfix;
  QString m_currencySymbol;

  // SetSelectionAlignWorker
  Format::Align m_horAlign;
  // SetSelectionAlignWorker
  Format::AlignY m_verAlign;
  Format::FloatFormat m_floatFormat;
  Format::FloatColor m_floatColor;
  FormatType m_formatType;
};



/**
 * ResizeColumnManipulator
 */
class ResizeColumnManipulator : public Manipulator
{
public:
  ResizeColumnManipulator();
  ~ResizeColumnManipulator();

  void setSize(double size) { m_newSize = size; }
  void setOldSize(double size) { m_oldSize = size; }

protected:
  virtual bool process(Element*);

  virtual QString name() const { return i18n("Resize Column"); }

private:
  double m_newSize;
  double m_oldSize;
};



/**
 * ResizeRowManipulator
 */
class ResizeRowManipulator : public Manipulator
{
public:
  ResizeRowManipulator();
  ~ResizeRowManipulator();

  void setSize(double size) { m_newSize = size; }
  void setOldSize(double size) { m_oldSize = size; }

protected:
  virtual bool process(Element*);

  virtual QString name() const { return i18n("Resize Row"); }

private:
  double m_newSize;
  double m_oldSize;
};



/**
 * BorderManipulator
 */
class BorderManipulator : public FormatManipulator
{
public:
  BorderManipulator() {}
  ~BorderManipulator() {}

protected:
  virtual QString name() const { return i18n("Change Border"); }

private:
};



/**
 * class BackgroundColorManipulator
 */
class BackgroundColorManipulator : public FormatManipulator
{
public:
  BackgroundColorManipulator() {}
  ~BackgroundColorManipulator() {}

protected:
  virtual QString name() const { return i18n("Change Background Color"); }

private:
};



/**
 * class FontColorManipulator
 */
class FontColorManipulator : public FormatManipulator
{
public:
  FontColorManipulator() {}
  ~FontColorManipulator() {}

protected:
  virtual QString name() const { return i18n("Change Text Color"); }

private:
};



/**
 * class FontManipulator
 */
class FontManipulator : public FormatManipulator
{
public:
  FontManipulator() {}
  ~FontManipulator() {}

protected:
  virtual QString name() const { return i18n("Change Font"); }

private:
};



/**
 * class AngleManipulator
 */
class AngleManipulator : public FormatManipulator
{
  public:
    AngleManipulator() {}
    ~AngleManipulator() {}

  protected:
    virtual QString name() const { return i18n("Change Angle"); }

  private:
};



/**
 * class HorAlignManipulator
 */
class HorAlignManipulator : public FormatManipulator
{
  public:
    HorAlignManipulator() {}
    ~HorAlignManipulator() {}

  protected:
    virtual QString name() const { return i18n("Change Horizontal Alignment"); }

  private:
};



/**
 * class VerAlignManipulator
 */
class VerAlignManipulator : public FormatManipulator
{
  public:
    VerAlignManipulator() {}
    ~VerAlignManipulator() {}

  protected:
    virtual QString name() const { return i18n("Change Vertical Alignment"); }

  private:
};




/**
 * MergeManipulator
 */
class MergeManipulator : public Manipulator
{
public:
  MergeManipulator();
  virtual ~MergeManipulator();

  virtual bool preProcessing();

  virtual void setReverse(bool reverse) { m_merge = !reverse; }
  void setHorizontalMerge(bool state) { m_mergeHorizontal = state; }
  void setVerticalMerge(bool state) { m_mergeVertical = state; }

protected:
  virtual bool process(Element*);

  virtual bool postProcessing();

  virtual QString name() const;

  bool m_merge;
private:
  bool m_mergeHorizontal : 1;
  bool m_mergeVertical   : 1;
  Manipulator* m_unmerger; // to restore old merging
};



/**
 * DilationManipulator
 */
class DilationManipulator : public Manipulator
{
public:
  DilationManipulator();
  virtual ~DilationManipulator();

  virtual void execute();
  virtual void unexecute();

protected:
  virtual QString name() const { return i18n("Dilate Region"); }

private:
};



/**
 * AdjustColumnRowManipulator
 */
class AdjustColumnRowManipulator : public Manipulator
{
public:
  AdjustColumnRowManipulator();
  virtual ~AdjustColumnRowManipulator();

  virtual bool process(Element*);
  virtual bool preProcessing();

  void setAdjustColumn(bool state) { m_adjustColumn = state; }
  void setAdjustRow(bool state) { m_adjustRow = state; }

protected:
  virtual QString name() const;

  double adjustColumnHelper( Cell * c, int _col, int _row );
  double adjustRowHelper( Cell * c, int _col, int _row );

private:
  bool m_adjustColumn : 1;
  bool m_adjustRow    : 1;
  QMap<int, double> m_newWidths;
  QMap<int, double> m_oldWidths;
  QMap<int, double> m_newHeights;
  QMap<int, double> m_oldHeights;
};



/**
 * HideShowManipulator
 */
class HideShowManipulator : public Manipulator
{
public:
  HideShowManipulator();
  virtual ~HideShowManipulator();

  virtual bool process(Element*);
  virtual bool preProcessing();
  virtual bool postProcessing();

  void setManipulateColumns(bool state) { m_manipulateColumns = state; }
  void setManipulateRows(bool state) { m_manipulateRows = state; }

protected:
  virtual QString name() const;

private:
  bool m_manipulateColumns : 1;
  bool m_manipulateRows    : 1;
};



/**
 * InsertDeleteManipulator
 */
class InsertDeleteManipulator : public Manipulator
{
public:
  InsertDeleteManipulator();
  ~InsertDeleteManipulator();

protected:

private:
  bool m_manipulateColumns : 1;
  bool m_manipulateRows    : 1;
};





/**
 * ManipulatorManager
 */
class ManipulatorManager
{
  public:
    static ManipulatorManager* self();
    ~ManipulatorManager();
    Manipulator* create(const QString&);

  private:
    ManipulatorManager();
    static ManipulatorManager* m_self;
};

} // namespace KSpread

#endif // KSPREAD_MANIPULATOR
