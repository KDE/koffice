/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <m.Kruisselbrink@student.tue.nl>
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
#include "MusicShape.h"
#include <limits.h>
#include <QPainter>
#include <kdebug.h>
#include <KoViewConverter.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>

#include "core/Sheet.h"
#include "core/Part.h"
#include "core/Voice.h"
#include "core/Staff.h"
#include "core/VoiceBar.h"
#include "core/Chord.h"
#include "core/Note.h"
#include "core/Clef.h"
#include "core/Bar.h"
#include "core/KeySignature.h"
#include "core/TimeSignature.h"
#include "core/MusicXmlWriter.h"
#include "core/MusicXmlReader.h"
#include "core/StaffSystem.h"

#include "MusicStyle.h"
#include "Engraver.h"
#include "Renderer.h"

using namespace MusicCore;

static MusicShape* firstShape = 0;

MusicShape::MusicShape()
    : m_firstSystem(0),
    m_style(new MusicStyle),
    m_engraver(new Engraver()),
    m_renderer(new MusicRenderer(m_style)),
    m_successor(0),
    m_predecessor(0)
{
    kDebug() << "firstShape:" << firstShape << "this:" << this;
    if (firstShape) {
        firstShape->m_successor = this;
        m_predecessor = firstShape;
        m_sheet = firstShape->m_sheet;
        m_firstSystem = firstShape->m_lastSystem+1;
        m_engraver->engraveSheet(m_sheet, m_firstSystem, QSizeF(1e9, 1e9), true, &m_lastSystem);
        firstShape = this;
    } else {
        firstShape = this;
        m_sheet = new Sheet();
        Bar* bar = m_sheet->addBar();

        Part* part = m_sheet->addPart("Part 1");
        Staff* staff = part->addStaff();
        part->addVoice();
        bar->addStaffElement(new Clef(staff, 0, Clef::Trebble, 2, 0));
        bar->addStaffElement(new TimeSignature(staff, 0, 4, 4));

        m_engraver->engraveSheet(m_sheet, 0, QSizeF(1e9, 1e9), true, &m_lastSystem);
    }
}

MusicShape::~MusicShape()
{
    kDebug() << "destroying" << this;
    if (!m_predecessor && !m_successor) {
        delete m_sheet;
    }
    delete m_style;
    delete m_engraver;
    delete m_renderer;
    if (this == firstShape) firstShape = this->m_predecessor;
}

void MusicShape::setSize( const QSizeF &newSize )
{
    KoShape::setSize(newSize);

    engrave(false);
}

void MusicShape::paint( QPainter& painter, const KoViewConverter& converter )
{
    applyConversion( painter, converter );

    painter.setClipping(true);
    painter.setClipRect(QRectF(0, 0, size().width(), size().height()));

    m_renderer->renderSheet( painter, m_sheet, m_firstSystem, m_lastSystem );
}

void MusicShape::saveOdf( KoShapeSavingContext & context ) const
{
    KoXmlWriter& writer = context.xmlWriter();
    writer.startElement("music:shape");
    writer.addAttribute("xmlns:music", "http://www.koffice.org/music");
    saveOdfAttributes(context, OdfMandatories | OdfSize | OdfPosition | OdfTransformation);

    MusicXmlWriter().writeSheet( writer, m_sheet, false );

    writer.endElement();
    saveOdfConnections(context);
}

bool MusicShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context ) {
    loadOdfAttributes( element, context, OdfMandatories | OdfSize | OdfPosition | OdfTransformation );

    KoXmlElement score = KoXml::namedItemNS(element, "http://www.koffice.org/music", "score-partwise");
    if (score.isNull()) {
        kWarning() << "no music:score-partwise element as first child";
        return false;
    }
    Sheet* sheet = MusicXmlReader().loadSheet(score);
    if (sheet) {
        if (!m_predecessor && !m_successor) {
            delete m_sheet;
        }
        m_sheet = sheet;
        m_engraver->engraveSheet(m_sheet, m_firstSystem, size(), true, &m_lastSystem);
        return true;
    }
    return false;
}

Sheet* MusicShape::sheet()
{
    return m_sheet;
}

void MusicShape::setSheet(Sheet* sheet, int firstSystem)
{
    if (!m_predecessor && !m_successor) {
        delete m_sheet;
    }
    m_sheet = sheet;
    m_firstSystem = firstSystem;
    m_engraver->engraveSheet(m_sheet, m_firstSystem, size(), true, &m_lastSystem);
}

int MusicShape::firstSystem() const
{
    return m_firstSystem;
}

void MusicShape::setFirstSystem(int system)
{
    m_firstSystem = system;
    engrave();
    update();
}

int MusicShape::lastSystem() const
{
    return m_lastSystem;
}

int MusicShape::firstBar() const
{
    return m_sheet->staffSystem(m_firstSystem)->firstBar();    
}

int MusicShape::lastBar() const
{
    int lastBar = INT_MAX;
    if (m_lastSystem < m_sheet->staffSystemCount()-1) {
        lastBar = m_sheet->staffSystem(m_lastSystem+1)->firstBar()-1;
    }
    return lastBar;
}

MusicRenderer* MusicShape::renderer()
{
    return m_renderer;
}

void MusicShape::engrave(bool engraveBars)
{
    m_engraver->engraveSheet(m_sheet, m_firstSystem, size(), engraveBars, &m_lastSystem);
    if (m_successor) {
        m_successor->setFirstSystem(m_lastSystem+1);
    }
}

MusicStyle* MusicShape::style()
{
    return m_style;
}

