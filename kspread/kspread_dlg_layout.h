/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __kspread_dlg_layout_h__
#define __kspread_dlg_layout_h__


#include <qtabdialog.h>
#include <qapplication.h>
#include <qbuttongroup.h>
#include <qgroupbox.h>
#include <knuminput.h>
#include <kcompletion.h>
#include "kspread_view.h"
#include "kspread_cell.h"
#include "kspread_doc.h"
#include <qpushbutton.h>
#include <qcheckbox.h>
class QPixmap;
class QRadioButton;
class QPushButton;
class QDate;
class QTime;
class QLabel;
class QWidget;
class CellFormatDlg;
class KSpreadSheet;
class KLineEdit;
class QFrame;
class QListBox;
class QCheckBox;
class KColorButton;
class QComboBox;

enum BorderType
{
  BorderType_Top = 0,
  BorderType_Bottom,
  BorderType_Left,
  BorderType_Right,
  BorderType_Vertical,
  BorderType_Horizontal,
  BorderType_FallingDiagonal,
  BorderType_RisingDiagonal,
  BorderType_END
};

enum BorderShortcutType
{
  BorderShortcutType_Remove = 0,
  BorderShortcutType_All,
  BorderShortcutType_Outline,
  BorderShortcutType_END
};
class KSpreadPatternSelect : public QFrame
{
    Q_OBJECT
public:
    KSpreadPatternSelect( QWidget *parent, const char *_name );

    void setPenStyle( PenStyle _pat ) { penStyle = _pat; repaint(); }
    PenStyle getPenStyle() { return penStyle; }
    void setColor( const QColor &_col ) { penColor = _col; repaint(); }
    const QColor& getColor() { return penColor; }
    void setPenWidth( int _w ) { penWidth = _w; repaint(); }
    int getPenWidth() { return penWidth; }

    void setPattern( const QColor &_color, int _width, PenStyle _style );
    void setUndefined();
    void setDefined() { undefined = FALSE; repaint(); }

    bool isDefined() { return !undefined; }

signals:
    void clicked( KSpreadPatternSelect *_this );

public slots:
    void slotUnselect();
    void slotSelect();

protected:
    virtual void paintEvent( QPaintEvent *_ev );
    virtual void mousePressEvent( QMouseEvent *_ev );

    PenStyle penStyle;
    QColor penColor;
    int penWidth;

    bool selected;
    bool undefined;
};

/**
 */
class CellFormatPageFont : public QWidget
{
    Q_OBJECT
public:
    CellFormatPageFont( QWidget* parent, CellFormatDlg *_dlg );

    void apply( KSpreadCell *_cell );
    void apply( ColumnFormat *_col );
    void apply( RowFormat *_row );
    void applyFormat( KSpreadFormat *_obj );

signals:
    /**
     * Connect to this to monitor the font as it as selected if you are
     * not running modal.
     */
    void fontSelected( const QFont &font );

private slots:

    void      family_chosen_slot(const QString & );
    void      size_chosen_slot(const QString & );
    void      weight_chosen_slot(const QString &);
    void      style_chosen_slot(const QString &);
    void      underline_chosen_slot();
    void      strike_chosen_slot();
    void      display_example(const QFont &font);
    void      slotSetTextColor( const QColor &_color );
    void      slotSearchFont(const QString &);

private:

    void setCombos();

    QGroupBox	 *box1;
    QGroupBox	 *box2;

    QLabel	 *family_label;
    QLabel	 *size_label;
    QLabel       *weight_label;
    QLabel       *style_label;

    QLabel	 *actual_family_label;
    QLabel	 *actual_size_label;
    QLabel       *actual_weight_label;
    QLabel       *actual_style_label;

    QLabel	 *actual_family_label_data;
    QLabel	 *actual_size_label_data;
    QLabel       *actual_weight_label_data;
    QLabel       *actual_style_label_data;

    //QComboBox    *family_combo;
    QListBox     *family_combo;
    QComboBox    *size_combo;
    QComboBox    *weight_combo;
    QComboBox    *style_combo;

    QLabel       *example_label;
    QFont         selFont;
    QCheckBox* strike;
    QCheckBox* underline;
    CellFormatDlg *dlg;
    QColor textColor;
    bool bTextColorUndefined;
    KColorButton *textColorButton;
    KLineEdit *searchFont;
    KCompletion listFont;
    bool fontChanged;
};

class CellFormatPageMisc : public QWidget
{
    Q_OBJECT
public:
    CellFormatPageMisc( QWidget *parent, CellFormatDlg *_dlg );

    void apply( KSpreadCell *_cell );
    void applyColumn();
    void applyRow();
    void applyFormat( KSpreadCell *_obj );
    KSpreadCell::Style getStyle();
    bool getDontPrintTextValue(){return dontPrintText->isChecked();}

public slots:
    void slotStyle( int );
protected:
    QComboBox* styleButton;
    int idStyleNormal;
    int idStyleUndef;
    int idStyleButton;
    int idStyleSelect;

    QLineEdit* actionText;
    QCheckBox *dontPrintText;
    CellFormatDlg *dlg;
};

/**
 * KSpreadFormat of numbers.
 * This widget is part of the format dialog.
 * It allows the user to cinfigure the way numbers are formatted.
 */
class CellFormatPageFloat : public QWidget
{
    Q_OBJECT
public:
    CellFormatPageFloat( QWidget *parent, CellFormatDlg *_dlg );

    void apply( KSpreadCell *_cell );
    void apply( ColumnFormat *_col );
    void apply( RowFormat *_row );
    void applyFormat( KSpreadFormat *_obj );

public slots:
    void slotChangeState();
    void makeformat();
    void makeDateFormat();
    void makeTimeFormat();
    QString makeFractionFormat();
    void init();
    void slotChangeValue(int);
    void formatChanged(int);
    void currencyChanged(const QString &);
protected:
    QLineEdit* postfix;
    KIntNumInput* precision;
    QLineEdit* prefix;
    QComboBox *format;
    QComboBox *currency;
    QLabel    *currencyLabel;
    QRadioButton *number;
    QRadioButton *percent;
    QRadioButton *date;
    QRadioButton *money;
    QRadioButton *scientific;
    QRadioButton *fraction;
    QRadioButton *time;
    QRadioButton *textFormat;
    QListBox *listFormat;
    QLabel *exampleLabel;
    CellFormatDlg *dlg;
    KSpreadCell::FormatType cellFormatType;
    //test if value changed
    bool m_bFormatTypeChanged;
    bool m_bFormatColorChanged;
};



class CellFormatPagePosition : public QWidget
{
    Q_OBJECT
public:
    CellFormatPagePosition( QWidget *parent, CellFormatDlg *_dlg );

    void apply( KSpreadCell *_cell );
    void apply( ColumnFormat *_col );
    void apply( RowFormat *_row );
    void applyFormat( KSpreadFormat *_obj );

    double getSizeHeight();
    double getSizeWidth();
    bool getMergedCellState();

public slots:
    void slotChangeHeightState();
    void slotChangeWidthState();
    void slotChangeAngle(int);
    void slotStateChanged(int);
    void slotChangeVerticalState();
    void slotChangeMultiState();

protected:
    QRadioButton *bottom;
    QRadioButton *top;
    QRadioButton *middle;
    QRadioButton *left;
    QRadioButton *right;
    QRadioButton *center;
    QRadioButton *standard;
    QCheckBox *multi;
    QCheckBox *vertical;
    KDoubleNumInput *width;
    KDoubleNumInput *height;
    CellFormatDlg *dlg;
    QCheckBox *defaultWidth;
    QCheckBox *defaultHeight;
    QCheckBox *mergeCell;
    KIntNumInput *angleRotation;
    KDoubleNumInput *indent;
    bool m_bOptionText;
};



class KSpreadBorder : public QFrame
{
    Q_OBJECT
public:
    KSpreadBorder( QWidget *parent,const char *_name,bool _oneCol,bool _oneRow  );
signals:
    void redraw();
    void choosearea(QMouseEvent * _ev);
protected:
    virtual void paintEvent( QPaintEvent *_ev );
    virtual void mousePressEvent( QMouseEvent* _ev );
    bool oneCol;
    bool oneRow;
};

class KSpreadBorderButton : public QPushButton
{
    Q_OBJECT
public:
    KSpreadBorderButton( QWidget *parent, const char *_name );
    void setPenStyle( PenStyle _pat ) { penStyle = _pat;}
    PenStyle getPenStyle() { return penStyle; }
    void setColor( const QColor &_col ) { penColor = _col; }
    const QColor& getColor() { return penColor; }
    void setPenWidth( int _w ) { penWidth = _w; }
    int getPenWidth() { return penWidth; }
    bool isChanged() { return changed; }
    void setChanged(bool _changed ) { changed=_changed;}
    void setUndefined();
    void unselect();
 signals:
    void clicked(KSpreadBorderButton *);
 protected:
    virtual void mousePressEvent( QMouseEvent *_ev );
    PenStyle penStyle;
    QColor penColor;
    int penWidth;
    bool changed;

};

class CellFormatPageBorder : public QWidget
{
  Q_OBJECT
public:
    CellFormatPageBorder( QWidget *parent, CellFormatDlg *_dlg );

    void applyOutline();
    void invertState(KSpreadBorderButton *_button);
    QPixmap paintFormatPixmap(PenStyle _style);
    void applyOutline(KSpreadFormat * format);

public slots:
    void changeState(KSpreadBorderButton *_this);
    void preselect( KSpreadBorderButton *_this);
    void draw();
    void slotSetColorButton( const QColor &_color );
    void slotUnselect2( KSpreadPatternSelect *_select );
    void loadIcon( QString pix,KSpreadBorderButton *_button);
    void slotPressEvent(QMouseEvent *_ev);
    void slotChangeStyle(int );
    void slotChangeStyle(const QString & );
    void cutomize_chosen_slot();

protected:

  KSpreadSheet* table;
  KSpreadBorderButton* borderButtons[BorderType_END];
  KSpreadBorderButton* shortcutButtons[BorderShortcutType_END];
#define NUM_BORDER_PATTERNS 10

  /* the patterns to choose from */
  KSpreadPatternSelect* pattern[NUM_BORDER_PATTERNS];

  /* the pattern box that is the 'preview' of what is selected above. */
  KSpreadPatternSelect* preview;
  QComboBox* size;
  QComboBox* style;
  KColorButton* color;
  QCheckBox* customize;
  QColor currentColor;
  KSpreadBorder *area;
  CellFormatDlg *dlg;
private:

  /*some helper functions to space some tasks apart */
  void InitializeGrids();
  void InitializeBorderButtons();
  void InitializePatterns();
  void SetConnections();
  void applyTopOutline();
  void applyBottomOutline();
  void applyLeftOutline();
  void applyRightOutline();
  void applyVerticalOutline();
  void applyHorizontalOutline();
  void applyDiagonalOutline();
};

class KSpreadBrushSelect : public QFrame
{
    Q_OBJECT
public:
    KSpreadBrushSelect( QWidget *parent, const char *_name );

    void setBrushStyle( BrushStyle _pat ) { brushStyle = _pat; repaint(); }
    BrushStyle getBrushStyle() { return brushStyle; }
    QColor getBrushColor() { return brushColor; }
    void setBrushColor(QColor _c) { brushColor=_c;}
    void setPattern( const QColor &_color, BrushStyle _style );

signals:
    void clicked( KSpreadBrushSelect *_this );

public slots:
    void slotUnselect();
    void slotSelect();

protected:
    virtual void paintEvent( QPaintEvent *_ev );
    virtual void mousePressEvent( QMouseEvent *_ev );

    BrushStyle brushStyle;
    QColor brushColor;
    bool selected;
};


class CellFormatPagePattern : public QWidget
{
    Q_OBJECT
public:
    CellFormatPagePattern( QWidget *parent, CellFormatDlg *_dlg );

    void apply( KSpreadCell *_cell );
    void apply( ColumnFormat *_col );
    void apply( RowFormat *_row );
    void applyFormat( KSpreadFormat *_obj );

    void init();
public slots:
    void slotUnselect2( KSpreadBrushSelect *_select );
    void slotSetColorButton( const QColor &_color );
    void slotSetBackgroundColor( const QColor &_color );
    void slotNotAnyColor( );
protected:
    KSpreadBrushSelect *selectedBrush;
    KSpreadBrushSelect *brush1;
    KSpreadBrushSelect *brush2;
    KSpreadBrushSelect *brush3;
    KSpreadBrushSelect *brush4;
    KSpreadBrushSelect *brush5;
    KSpreadBrushSelect *brush6;
    KSpreadBrushSelect *brush7;
    KSpreadBrushSelect *brush8;
    KSpreadBrushSelect *brush9;
    KSpreadBrushSelect *brush10;
    KSpreadBrushSelect *brush11;
    KSpreadBrushSelect *brush12;
    KSpreadBrushSelect *brush13;
    KSpreadBrushSelect *brush14;
    KSpreadBrushSelect *brush15;
    KSpreadBrushSelect *current;
    KColorButton* color;
    QPushButton* notAnyColor;
    QColor currentColor;

    QColor bgColor;
    bool bBgColorUndefined;
    KColorButton *bgColorButton;
    bool b_notAnyColor;
    CellFormatDlg *dlg;
};


/**
 */
class CellFormatDlg : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a format dlg for the rectangular area in '_table'.
     */
    CellFormatDlg( KSpreadView *_view, KSpreadSheet *_table, int _left, int _top, int _right, int _bottom );

    ~CellFormatDlg();

    void init();
    void initParameters(KSpreadFormat *_obj,int column,int row);
    void checkBorderRight(KSpreadFormat *obj,int x,int y);
    void checkBorderLeft(KSpreadFormat *obj,int x,int y);
    void checkBorderTop(KSpreadFormat *obj,int x,int y);
    void checkBorderBottom(KSpreadFormat *obj,int x,int y);
    void checkBorderVertical(KSpreadFormat *obj,int x,int y);
    void checkBorderHorizontal(KSpreadFormat *obj,int x,int y);
    /**
     * Run the dialogs event loop and return when closed.
     */
    int exec();

    KSpreadSheet* getTable() const {	return table; }

    bool isSingleCell() { return ( left == right && top == bottom ); }

    KLocale* locale()const {return m_pView->doc()->locale();}

    struct CellBorderFormat
    {
      int width;
      bool bStyle;
      QColor color;
      bool bColor;
      PenStyle style;
    };

    // The format of the selected area
    CellBorderFormat borders[BorderType_END];

    BrushStyle brushStyle;
    QColor brushColor;

    bool oneCol;
    bool oneRow;

    QString prefix;
    QString postfix;
    int precision;
    KSpreadCell::FloatFormat floatFormat;
    bool bFloatFormat;
    KSpreadCell::FloatColor floatColor;
    KSpreadCell::Currency   cCurrency;
    bool bFloatColor;
    bool bCurrency;
    QColor textColor;
    bool bTextColor;
    bool bTextFontBold;
    bool textFontBold;
    bool bTextFontItalic;
    bool textFontItalic;
    bool bTextFontSize;
    int textFontSize;
    bool bTextFontFamily;
    QString textFontFamily;
    bool bStrike;
    bool strike;
    bool bUnderline;
    bool underline;
    QFont textFont;
    QColor bgColor;
    bool bBgColor;
    KSpreadCell::Style eStyle;
    QString actionText;
    KSpreadCell::Align alignX;
    KSpreadCell::AlignY alignY;

    bool bMultiRow;
    bool bVerticalText;
    bool bDontprintText;

    double heigthSize;
    double widthSize;

    double indent;

    QPixmap* formatOnlyNegSignedPixmap;
    QPixmap* formatRedOnlyNegSignedPixmap;
    QPixmap* formatRedNeverSignedPixmap;
    QPixmap* formatAlwaysSignedPixmap;
    QPixmap* formatRedAlwaysSignedPixmap;

    int textRotation;
    bool bTextRotation;

    KSpreadCell::FormatType formatType;
    bool bFormatType;

    bool m_bValue;
    bool m_bDate;
    bool m_bTime;

    QTime m_time;
    QDate m_date;
    QString cellText;
    double m_value;

    bool isMerged;
    bool oneCell;

    bool isRowSelected;
    bool isColumnSelected;

    // The rectangular area for which this dlg has been opened.
    int left;
    int right;
    int top;
    int bottom;


public slots:
    void slotApply();
    void slotDefault();

protected:

    /**
     * Used to draw the @ref #formatPixmap and friends.
     */
    QPixmap* paintFormatPixmap( const char *_string1, const QColor & _color1,
				const char *_string2, const QColor & _color2 );

    CellFormatPageFloat *floatPage;
    CellFormatPageBorder *borderPage;
    CellFormatPageMisc *miscPage;
    CellFormatPageFont *fontPage;
    CellFormatPagePosition *positionPage;
    CellFormatPagePattern *patternPage;
    QTabDialog *tab;

    /**
     * The table that opened this dlg.
     */
    KSpreadSheet *table;

    KSpreadView *m_pView;


};

#endif
