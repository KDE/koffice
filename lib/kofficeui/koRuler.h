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

// Description: Ruler (header)

/******************************************************************/

#ifndef koRuler_h
#define koRuler_h

#include <koPageLayoutDia.h>

#include <qframe.h>
#include <qpixmap.h>

#include <koGlobal.h>
#include <koTabChooser.h>

class QPainter;

enum KoTabulators { T_LEFT = 0, T_CENTER = 1, T_RIGHT = 2, T_DEC_PNT = 3, T_INVALID = -1 };
enum KoTabulatorFilling { TF_BLANK = 0, TF_DOTS = 1, TF_LINE = 2, TF_DASH = 3, TF_DASH_DOT = 4, TF_DASH_DOT_DOT = 5};

/**
 * Struct: KoTabulator
 * Defines the position of a tabulation (in pt), and its type
 */
struct KoTabulator {
    /**
     * Position of the tab in pt
     */
    double ptPos;
    /**
     * Type of tab (left/center/right/decimalpoint)
     */
    KoTabulators type;
    /**
     * Type of tab filling.
     */
    KoTabulatorFilling filling;
    /**
     * Width of the tab filling line.
     */
    double ptWidth;

    bool operator==( const KoTabulator & t ) const {
        return QABS( ptPos - t.ptPos ) < 1E-4 && type == t.type &&
               filling == t.filling && QABS( ptWidth - t.ptWidth ) < 1E-4;
    }
    bool operator!=( const KoTabulator & t ) const {
        return !operator==(t);
    }
    // Operators used for sorting
    bool operator < ( const KoTabulator & t ) const {
        return ptPos < t.ptPos;
    }
    bool operator <= ( const KoTabulator & t ) const {
        return ptPos <= t.ptPos;
    }
    bool operator > ( const KoTabulator & t ) const {
        return ptPos > t.ptPos;
    }
};

typedef QValueList<KoTabulator> KoTabulatorList;

class KoRulerPrivate;

/**
 * KoRuler is the horizontal or vertical ruler, to be used around
 * the drawing area of most KOffice programs.
 *
 * It shows the graduated ruler with numbering, in any of the 3 base units (mm/pt/inch),
 * and supports zooming, tabulators, paragraph indents, showing the mouse position, etc.
 *
 * It also offers a popupmenu upon right-clicking, for changing the unit,
 * the page layout, or removing a tab.
 */
class KoRuler : public QFrame
{
    Q_OBJECT
    friend class KoRulerPrivate; // for the Action enum
public:
    static const int F_TABS;
    static const int F_INDENTS;
    static const int F_HELPLINES;
    static const int F_NORESIZE;

    /**
     * Create a ruler
     * TODO document params
     */
    KoRuler( QWidget *_parent,  QWidget *_canvas, Orientation _orientation,
             const KoPageLayout& _layout, int _flags, KoUnit::Unit _unit,
             KoTabChooser *_tabChooser = 0L );
    ~KoRuler();

    /**
     * Set the unit to be used. Currently supported are "mm", "pt" and "inch".
     * You should use the KoUnit::Unit variant instead.
     */
    void setUnit( const QString& unit );
    /**
     * Set the unit to be used.
     */
    void setUnit( KoUnit::Unit unit );

    void setZoom( const double& zoom=1.0 );
    const double& zoom() const { return m_zoom; }

    /**
     * Set the page layout, @see KoPageLayout.
     * This defines the size of the page and the margins,
     * from which the size of the ruler is deducted.
     */
    void setPageLayout( KoPageLayout _layout )
    { layout = _layout; update(); }

    /**
     * Call showMousePos(true) if the ruler should indicate the position
     * of the mouse. This is usually only the case for drawing applications,
     * so it is not the case by default.
     */
    void showMousePos( bool _showMPos );
    /**
     * Set the position of the mouse, to update the indication in the ruler.
     * This is only effective if showMousePos(true) was called previously.
     * The position to give is not zoomed, it's in real pixel coordinates!
     */
    void setMousePos( int mx, int my );

    /**
     * Set a global offset to the X and Y coordinates.
     * Usually the main drawing area is a QScrollView, and this is called
     * with contentsX() and contentsY(), each time those values change.
     */
    void setOffset( int _diffx, int _diffy );

    /**
     * Set the [paragraph] left indent to the specified position (in the current unit)
     */
    void setLeftIndent( double _left )
    { i_left = makeIntern( _left ); update(); }

    /**
     * Set the [paragraph] first-line left indent to the specified position (in the current unit)
     * This indent is cumulated with the left or right margin, depending on the [paragraph] direction.
     */
    void setFirstIndent( double _first )
    { i_first = makeIntern( _first ); update(); }

    /**
     * Set the [paragraph] right indent to the specified position (in the current unit)
     */
    void setRightIndent( double _right );

    /**
     * Set the [paragraph] direction. By default (rtl=false), the left indent is on the
     * left, and the right indent is on the right ;)
     * If rtl=true, it's the opposite.
     */
    void setDirection( bool rtl );

    /**
     * Set the list of tabulators to show in the ruler.
     */
    void setTabList( const KoTabulatorList & tabList );

    /**
     * Set the start and the end of the current 'frame', i.e. the part
     * of the page in which we are currently working. See KWord frames
     * for an example where this is used. The tab positions and paragraph
     * indents then become relative to the beginning of the frame, and the
     * ruler goes from frameStart to frameEnd instead of using the page margins.
     * @p _frameStart et @p _frameEnd are in pixel coordinates.
     */
    void setFrameStartEnd( int _frameStart, int _frameEnd );

    /**
     * KoRuler is in "read write" mode by default.
     * Use setReadWrite(false) to use it in read-only mode.
     */
    void setReadWrite( bool _readWrite );

    /**
     * Change the flag (i.e. activate or deactivate certain features of KoRuler)
     */
    void changeFlags(int _flags);

    /**
     * set the size of the grid used for tabs positioning, size in pt.
     * default value is 0. 0 means no grid.
     */
    void setGridSize(double newGridSize) { gridSize=newGridSize; }

    /**
     * @return the current flags
     */
    int flags() const;

    /**
     * Enable or disable the "Page Layout" menu item.
     */
    void setPageLayoutMenuItemEnabled(bool b);

signals:
    void newPageLayout( KoPageLayout );
    void newLeftIndent( double );
    void newFirstIndent( double );
    void newRightIndent( double );
    /** Old signal, kept for compatibility. Use doubleClicked instead. */
    void openPageLayoutDia();
    /** This signal is emitted when double-clicking the ruler */
    void doubleClicked();
    /** This signal is emitted when double-clicking a tab */
    void doubleClicked( double ptPos );

    void tabListChanged( const KoTabulatorList & );
    void unitChanged( QString ); // TODO KoUnit::Unit instead ?

    void addHelpline(const QPoint &, bool );
    void moveHelpLines( const QPoint &, bool );

protected:
    enum Action {A_NONE, A_BR_LEFT, A_BR_RIGHT, A_BR_TOP, A_BR_BOTTOM,
                 A_LEFT_INDENT, A_FIRST_INDENT, A_TAB, A_RIGHT_INDENT,
                 A_HELPLINES };

    void drawContents( QPainter *_painter )
    { orientation == Qt::Horizontal ? drawHorizontal( _painter ) : drawVertical( _painter ); }

    void drawHorizontal( QPainter *_painter );
    void drawVertical( QPainter *_painter );
    void drawTabs( QPainter &_painter );

    void mousePressEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseDoubleClickEvent( QMouseEvent* );
    void resizeEvent( QResizeEvent *e );
    void handleDoubleClick();

    double makeIntern( double _v );
    double zoomIt(const double &value) const;
    int zoomIt(const int &value) const;
    unsigned int zoomIt(const unsigned int &value) const;
    double unZoomIt(const double &value) const;
    int unZoomIt(const int &value) const;
    unsigned int unZoomIt(const unsigned int &value) const;
    void setupMenu();
    void uncheckMenu();
    void searchTab(int mx);
    void drawLine(int oldX, int newX);

private:
    double applyRtlAndZoom( double value ) const;
    double unZoomItRtl( int pixValue ) const;

    KoRulerPrivate *d;

    Qt::Orientation orientation;
    int diffx, diffy;
    double i_left, i_first;
    KoPageLayout layout;
    QPixmap buffer;
    double m_zoom, m_1_zoom;
    KoUnit::Unit m_unit;
    bool hasToDelete;
    bool showMPos;
    int mposX, mposY;
    int frameStart;
    bool m_bFrameStartSet;

    bool m_bReadWrite;

    double gridSize;

protected slots:
    void rbPT() { setUnit( KoUnit::U_PT ); emit unitChanged( "pt" ); }
    void rbMM() { setUnit( KoUnit::U_MM ); emit unitChanged( "mm" ); }
    void rbINCH() { setUnit( KoUnit::U_INCH ); emit unitChanged( "inch" ); }
    void pageLayoutDia() { emit doubleClicked()/*openPageLayoutDia()*/; }
    void rbRemoveTab();

};

inline double KoRuler::zoomIt(const double &value) const {
    if (m_zoom==1.0)
        return value;
    return m_zoom*value;
}

inline int KoRuler::zoomIt(const int &value) const {
    if (m_zoom==1.0)
        return value;
    return qRound(m_zoom*value);
}

inline unsigned int KoRuler::zoomIt(const unsigned int &value) const {
    if (m_zoom==1.0)
        return value;
    return static_cast<unsigned int>(qRound(m_zoom*value));
}

inline double KoRuler::unZoomIt(const double &value) const {
    if(m_zoom==1.0)
        return value;
    return value*m_1_zoom;
}

inline int KoRuler::unZoomIt(const int &value) const {
    if(m_zoom==1.0)
        return value;
    return qRound(value*m_1_zoom);
}

inline unsigned int KoRuler::unZoomIt(const unsigned int &value) const {
    if(m_zoom==1.0)
        return value;
    return static_cast<unsigned int>(qRound(value*m_1_zoom));
}

#endif
