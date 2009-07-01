/* This file is part of the KDE project
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net> 
  
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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef KOFORMULATOOL_H
#define KOFORMULATOOL_H

#include <KoTool.h>

class KoFormulaShape;
class BasicElement;
class FormulaCursor;

/**
 * @short The flake tool for a formula
 * @author Martin Pfeiffer <hubipete@gmx.net>
 */
class KoFormulaTool : public KoTool {
    Q_OBJECT
public:
    /// The standard constructor
    explicit KoFormulaTool( KoCanvasBase *canvas );

    /// The standard destructor
    ~KoFormulaTool();

    /// reimplemented
    void paint( QPainter &painter, const KoViewConverter &converter );

    /// reimplemented
    void mousePressEvent( KoPointerEvent *event ) ;
    
    /// reimplemented
    void mouseDoubleClickEvent( KoPointerEvent *event );
    
    /// reimplemented
    void mouseMoveEvent( KoPointerEvent *event );
    
    /// reimplemented
    void mouseReleaseEvent( KoPointerEvent *event );
    
    void keyPressEvent( QKeyEvent *event );
    
    void keyReleaseEvent( QKeyEvent *event );

    void remove( bool backSpace );

    /// @return The currently manipulated KoFormulaShape
    KoFormulaShape* shape();
    
    /// Reset the cursor
    void resetFormulaCursor();

public slots:
    /// Called when this tool instance is activated and fills m_formulaShape
    void activate( bool temporary=false );

    /// Called when this tool instance is deactivated
    void deactivate();

    /// Insert the element tied to the given @p action
    void insert( QAction* action );
 
protected:
    /// Create default option widget
    QWidget* createOptionWidget();
	
private:
    /// Repaint the cursor and selection
    void repaintCursor();

    /// Creates all the actions provided by the tool
    void setupActions();

    /// The FormulaShape the tool is manipulating
    KoFormulaShape* m_formulaShape;

    /// The FormulaCursor the tool uses to move around in the formula
    FormulaCursor* m_formulaCursor;
};

#endif
