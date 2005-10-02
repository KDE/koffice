/* This file is part of the KDE project
   Copyright (C) 2003 Ulrich Kuettler <ulrich.kuettler@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KWFORMULAFRAME_H
#define KWFORMULAFRAME_H

#include "kwframe.h"

namespace KFormula {
    class FormulaCursor;
    class Container;
    class View;
}

/******************************************************************/
/* Class: KWFormulaFrameSet                                       */
/******************************************************************/

// needed for signals & slots ;(
using KFormula::Container;
using KFormula::FormulaCursor;
using KFormula::View;

class KWFormulaFrameSetEdit;

class KWFormulaFrameSet : public KWFrameSet
{
    Q_OBJECT
public:
    KWFormulaFrameSet( KWDocument *_doc, const QString & name );
    virtual ~KWFormulaFrameSet();

    virtual KWordFrameSetIface* dcopObject();

    /** The type of frameset. Use this to differentiate between different instantiations of
     *  the framesets. Each implementation will return a different frameType.
     */
    virtual FrameSetType type() const { return FT_FORMULA; }

    virtual void addFrame( KWFrame *_frame, bool recalc = true );

    /**
     * Delete a frame from the set of frames this frameSet has.
     * @param num The frameNumber to be removed.
     * @param remove passing true means that there can not be an undo of the action.
     * @param recalc do an updateFrames()
     */
    virtual void delFrame( unsigned int _num, bool remove = true, bool recalc = true );

    virtual void moveFrame( KWFrame* frame );

    virtual KWFrameSetEdit* createFrameSetEdit(KWCanvas*);

    virtual MouseMeaning getMouseMeaningInsideFrame( const KoPoint& );

    /**
     * Paint this frameset
     */
    virtual void drawFrameContents(KWFrame *, QPainter*, const QRect&,
                                   const QColorGroup&, bool onlyChanged, bool resetChanged,
                                   KWFrameSetEdit *edit, KWViewMode *viewMode);

    virtual QDomElement save( QDomElement &parentElem, bool saveFrames = true );
    virtual void load( QDomElement &attributes, bool loadFrames = true );
    virtual void saveOasis(KoXmlWriter&, KoSavingContext&, bool saveFrames ) const;
    void paste( QDomNode& formulaElem );

    KFormula::Container* getFormula() const { return formula; }

    void setChanged() { m_changed = true; }

    virtual void moveFloatingFrame( int frameNum, const KoPoint &position );
    virtual int floatingFrameBaseline( int /*frameNum*/ );

    virtual void setAnchorFormat( KoTextFormat* format, int /*frameNum*/ );

    void showPopup( KWFrame *, KWView *view, const QPoint &point );

    // TODO support for protecting the formula's contents
    virtual void setProtectContent ( bool ) {}
    virtual bool protectContent() const { return false; }

protected slots:

    void slotFormulaChanged( double width, double height );
    void slotErrorMessage( const QString& msg );

private:

    static QPixmap* doubleBufferPixmap( const QSize& s );
    static QPixmap* m_bufPixmap;

    friend class KWFormulaFrameSetEdit;

    KFormula::Container* formula;
    bool m_changed;

    KWFormulaFrameSetEdit* m_edit;
};


class KWFormulaFrameSetEdit : public QObject, public KWFrameSetEdit
{
    Q_OBJECT
public:
    KWFormulaFrameSetEdit(KWFormulaFrameSet* fs, KWCanvas* canvas);
    virtual ~KWFormulaFrameSetEdit();

    KWFormulaFrameSet* formulaFrameSet() const
    {
        return static_cast<KWFormulaFrameSet*>(frameSet());
    }

    const KFormula::View* getFormulaView() const;
    KFormula::View* getFormulaView();

    virtual DCOPObject* dcopObject();

    // Events forwarded by the canvas (when being in "edit" mode)
    virtual void keyPressEvent(QKeyEvent*);
    virtual void mousePressEvent(QMouseEvent*, const QPoint & n, const KoPoint & d );
    virtual void mouseMoveEvent(QMouseEvent*, const QPoint & n, const KoPoint & d); // only called if button is pressed
    virtual void mouseReleaseEvent(QMouseEvent*, const QPoint & n, const KoPoint & d);
    //virtual void mouseDoubleClickEvent( QMouseEvent *, const QPoint & n, const KoPoint & d ) {}
    //virtual void dragEnterEvent( QDragEnterEvent * ) {}
    //virtual void dragMoveEvent( QDragMoveEvent *, const QPoint &, const KoPoint & ) {}
    //virtual void dragLeaveEvent( QDragLeaveEvent * ) {}
    //virtual void dropEvent( QDropEvent *, const QPoint &, const KoPoint &, KWView* ) {}
    virtual void focusInEvent();
    virtual void focusOutEvent();
    virtual void copy();
    virtual void cut();
    virtual void paste();
    virtual void pasteData( QMimeSource* data, int provides );
    virtual void selectAll();

    /** Moves the cursor to the first position */
    void moveHome();
    /** Moves the cursor to the last position */
    void moveEnd();

    void removeFormula();

protected slots:

    /**
     * Make sure the cursor can be seen at its new position.
     */
    void cursorChanged( bool visible, bool selecting );

    void slotLeaveFormula( Container*, FormulaCursor*, int );

private:
    KFormula::View* formulaView;
    DCOPObject *dcop;
};

#endif
