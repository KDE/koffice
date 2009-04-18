/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ArtisticTextTool.h"
#include "AttachTextToPathCommand.h"
#include "DetachTextFromPathCommand.h"

#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoPathShape.h>
#include <KoShapeController.h>
#include <KoShapeContainer.h>
#include <KoLineBorder.h>

#include <KLocale>
#include <KIcon>
#include <QAction>
#include <QGridLayout>
#include <QToolButton>
#include <QCheckBox>
#include <QPainter>
#include <QPainterPath>

#include <float.h>

ArtisticTextTool::ArtisticTextTool(KoCanvasBase *canvas)
    : KoTool(canvas), m_currentShape(0), m_path(0), m_tmpPath(0), m_textCursor( -1 ), m_showCursor( true )
{
    m_attachPath  = new QAction(KIcon("artistictext-attach-path"), i18n("Attach Path"), this);
    m_attachPath->setEnabled( false );
    connect( m_attachPath, SIGNAL(triggered()), this, SLOT(attachPath()) );

    m_detachPath  = new QAction(KIcon("artistictext-detach-path"), i18n("Detach Path"), this);
    m_detachPath->setEnabled( false );
    connect( m_detachPath, SIGNAL(triggered()), this, SLOT(detachPath()) );

    m_convertText  = new QAction(KIcon("pathshape"), i18n("Convert to Path"), this);
    m_convertText->setEnabled( false );
    connect( m_convertText, SIGNAL(triggered()), this, SLOT(convertText()) );

    KoShapeManager *manager = m_canvas->shapeManager();
    connect( manager, SIGNAL(selectionContentChanged()), this, SLOT(textChanged()));
}

ArtisticTextTool::~ArtisticTextTool()
{
}

QTransform ArtisticTextTool::cursorTransform() const
{
    QTransform transform( m_currentShape->absoluteTransformation(0) );
    QPointF pos;
    m_currentShape->getCharPositionAt( m_textCursor, pos );
    transform.translate( pos.x() - 1, pos.y() );
    qreal angle;
    m_currentShape->getCharAngleAt( m_textCursor, angle );
    transform.rotate( 360. - angle );
    if ( m_currentShape->isOnPath() ) {
        QFont f = m_currentShape->font();
        QFontMetrics metrics(f);
        transform.translate( 0, metrics.descent() );
    }

    return transform;
}

void ArtisticTextTool::paint( QPainter &painter, const KoViewConverter &converter)
{
    if ( ! m_currentShape || !m_showCursor )
        return;

    painter.save();
    m_currentShape->applyConversion( painter, converter );
    painter.setBrush( Qt::black );
    painter.setWorldTransform( cursorTransform(), true );
    painter.setClipping( false );
    painter.drawPath( m_textCursorShape );
    painter.restore();
}

void ArtisticTextTool::mousePressEvent( KoPointerEvent *event )
{
    ArtisticTextShape *hit = 0;
    QRectF roi( event->point, QSizeF(1,1) );
    QList<KoShape*> shapes = m_canvas->shapeManager()->shapesAt( roi );
    KoSelection *selection = m_canvas->shapeManager()->selection();
    foreach( KoShape *shape, shapes ) 
    {
        hit = dynamic_cast<ArtisticTextShape*>( shape );
        if( hit ) {
            if ( hit != m_currentShape ) {
                selection->deselectAll();
                enableTextCursor( false );
                m_currentShape = hit;
                enableTextCursor( true );
                selection->select( m_currentShape );
            }
            break;
        }
    }
    if ( hit ) {
        QPointF pos = event->point;
        pos -= m_currentShape->absolutePosition( KoFlake::TopLeftCorner );
        const int len = m_currentShape->text().length();
        int hit = len;
        qreal mindist = DBL_MAX;
        for ( int i = 0; i < len;++i ) {
            QPointF center;
            m_currentShape->getCharPositionAt( i, center );
            center = pos - center;
            if ( (fabs(center.x()) + fabs(center.y())) < mindist ) {
                hit = i;
                mindist = fabs(center.x()) + fabs(center.y());
            }
        }
        setTextCursorInternal( hit );
        m_currentText = m_currentShape->text();
    }
    event->ignore();
}

void ArtisticTextTool::mouseMoveEvent( KoPointerEvent *event )
{
    m_tmpPath = 0;
    ArtisticTextShape *textShape = 0;

    QRectF roi( event->point, QSizeF(1,1) );
    QList<KoShape*> shapes = m_canvas->shapeManager()->shapesAt( roi );
    foreach( KoShape * shape, shapes )
    {
        ArtisticTextShape * text = dynamic_cast<ArtisticTextShape*>( shape );
        if ( text )
        {
            textShape = text;
            break;
        }
        KoPathShape * path = dynamic_cast<KoPathShape*>( shape );
        if ( path )
        {
            m_tmpPath = path;
            break;
        }
    }
    if( m_tmpPath )
        useCursor( QCursor( Qt::PointingHandCursor ) );
    else if ( textShape )
        useCursor( QCursor( Qt::IBeamCursor ) );
    else
        useCursor( QCursor( Qt::ArrowCursor ) );
}

void ArtisticTextTool::mouseReleaseEvent( KoPointerEvent *event )
{
    Q_UNUSED(event);
    m_path = m_tmpPath;
    updateActions();
}

void ArtisticTextTool::keyPressEvent(QKeyEvent *event)
{
    event->accept();
    if ( m_currentShape && textCursor() > -1 ) {
        switch(event->key())
        {
        case Qt::Key_Backspace:
            removeFromTextCursor( m_textCursor, 1 );
            break;
        case Qt::Key_Right:
            setTextCursor( textCursor() + 1 );
            break;
        case Qt::Key_Left:
            setTextCursor( textCursor() - 1 );
            break;
        case Qt::Key_Home:
            setTextCursor( 0 );
            break;
        case Qt::Key_End:
            setTextCursor( m_currentShape->text().length() );
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit done();
            break;
        default:
            addToTextCursor( event->text() );
        }
    } else {
        event->ignore();
    }
}

void ArtisticTextTool::activate( bool )
{
    KoSelection *selection = m_canvas->shapeManager()->selection();
    foreach( KoShape *shape, selection->selectedShapes() ) 
    {
        m_currentShape = dynamic_cast<ArtisticTextShape*>( shape );
        if(m_currentShape)
            break;
    }
    if( m_currentShape == 0 ) 
    {
        // none found
        emit done();
        return;
    }

    enableTextCursor( true );

    createTextCursorShape();

    updateActions();

    emit statusTextChanged( i18n("Press return to finish editing.") );
}

void ArtisticTextTool::blinkCursor()
{
    updateTextCursorArea();
    m_showCursor = !m_showCursor;
    updateTextCursorArea();
}

void ArtisticTextTool::deactivate()
{
    if ( m_currentShape ) {
        if( m_currentShape->text().isEmpty() ) {
            m_canvas->addCommand( m_canvas->shapeController()->removeShape( m_currentShape ) );
        }
        enableTextCursor( false );
        m_currentShape = 0;
    }
    m_path = 0;
}

void ArtisticTextTool::updateActions()
{
    m_attachPath->setEnabled( m_path != 0 );
    if( m_currentShape )
    {
        m_detachPath->setEnabled( m_currentShape->isOnPath() );
        m_convertText->setEnabled( true );
    } else {
        m_detachPath->setEnabled( false );
        m_convertText->setEnabled( false );
    }
}

void ArtisticTextTool::attachPath()
{
    if( m_path && m_currentShape ) {
        m_blinkingCursor.stop();
        m_showCursor = false;
        updateTextCursorArea();
        m_canvas->addCommand( new AttachTextToPathCommand( m_currentShape, m_path ) );
        m_blinkingCursor.start( 500 );
        updateActions();
    }
}

void ArtisticTextTool::detachPath()
{
    if( m_currentShape && m_currentShape->isOnPath() )
    {
        m_canvas->addCommand( new DetachTextFromPathCommand( m_currentShape ) );
        updateActions();
    }
}

void ArtisticTextTool::convertText()
{
    if( ! m_currentShape )
        return;

    KoPathShape * path = KoPathShape::fromQPainterPath( m_currentShape->outline() );
    path->setParent( m_currentShape->parent() );
    path->setZIndex( m_currentShape->zIndex() );
    path->setBorder( m_currentShape->border() );
    path->setBackground( m_currentShape->background() );
    path->setTransformation( m_currentShape->transformation() );
    path->setShapeId( KoPathShapeId );

    QUndoCommand * cmd = m_canvas->shapeController()->addShapeDirect( path );
    cmd->setText( i18n("Convert to Path") );
    m_canvas->shapeController()->removeShape( m_currentShape, cmd );
    m_canvas->addCommand( cmd );

    emit done();
}

QWidget *ArtisticTextTool::createOptionWidget()
{
    QWidget * widget = new QWidget();
    QGridLayout * layout = new QGridLayout(widget);

    QToolButton * attachButton = new QToolButton(widget);
    attachButton->setDefaultAction( m_attachPath );
    layout->addWidget( attachButton, 0, 0 );

    QToolButton * detachButton = new QToolButton(widget);
    detachButton->setDefaultAction( m_detachPath );
    layout->addWidget( detachButton, 0, 1 );

    QToolButton * convertButton = new QToolButton(widget);
    convertButton->setDefaultAction( m_convertText );
    layout->addWidget( convertButton, 0, 3 );

    layout->setSpacing(0);
    layout->setMargin(6);
    layout->setRowStretch(3, 1);
    layout->setColumnStretch(2, 1);

    return widget;
}

void ArtisticTextTool::enableTextCursor( bool enable )
{
    if ( enable ) {
        if( m_currentShape )
            setTextCursorInternal( m_currentShape->text().length() );
        connect( &m_blinkingCursor, SIGNAL(timeout()), this, SLOT(blinkCursor()) );
        m_blinkingCursor.start( 500 );
    } else {
        m_blinkingCursor.stop();
        disconnect( &m_blinkingCursor, SIGNAL(timeout()), this, SLOT(blinkCursor()) );
        setTextCursorInternal( -1 );
    }
}

void ArtisticTextTool::setTextCursor( int textCursor )
{
    if ( m_textCursor == textCursor || textCursor < 0 
    || ! m_currentShape || textCursor > m_currentShape->text().length() )
        return;

    setTextCursorInternal( textCursor );
}

void ArtisticTextTool::updateTextCursorArea() const
{
    if( ! m_currentShape || m_textCursor < 0 )
        return;

    QRectF bbox = cursorTransform().mapRect( m_textCursorShape.boundingRect() );
    m_canvas->updateCanvas( bbox );
}

void ArtisticTextTool::setTextCursorInternal( int textCursor )
{
    updateTextCursorArea();
    m_textCursor = textCursor;
    updateTextCursorArea();
}

void ArtisticTextTool::createTextCursorShape()
{
    if ( m_textCursor < 0 || ! m_currentShape ) 
        return;
    m_textCursorShape = QPainterPath();
    QRectF extents;
    m_currentShape->getCharExtentsAt( m_textCursor, extents );
    m_textCursorShape.addRect( 0, 0, 1, -extents.height() );
    m_textCursorShape.closeSubpath();
}

void ArtisticTextTool::removeFromTextCursor( int from, unsigned int nr )
{
    if ( from > 0 && from >= int( nr ) ) {
        m_currentText.remove( from - nr, nr );
        QUndoCommand *cmd = new RemoveTextRange( this, from - nr, nr );
        m_canvas->addCommand( cmd );
    }
}

void ArtisticTextTool::addToTextCursor( const QString &str )
{
    if ( !str.isEmpty() && m_textCursor > -1 ) {
        QString printable;
        for ( int i = 0;i < str.length();i++ ) {
            if ( str[i].isPrint() )
                printable.append( str[i] );
        }
        unsigned int len = printable.length();
        if ( len ) {
            m_currentText.insert( m_textCursor, printable );
            QUndoCommand *cmd = new AddTextRange( this, printable, m_textCursor );
            m_canvas->addCommand( cmd );
        }
    }
}

void ArtisticTextTool::deleteSelection()
{
    if ( m_currentShape ) {
        removeFromTextCursor( m_textCursor + 1, 1 );
    }
}

void ArtisticTextTool::textChanged()
{
    if ( !m_currentShape || m_currentShape->text() == m_currentText )
        return;

    setTextCursorInternal( m_currentShape->text().length() );
}

#include "ArtisticTextTool.moc"
