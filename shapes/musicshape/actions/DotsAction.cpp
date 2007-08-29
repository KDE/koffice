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
#include "DotsAction.h"

#include "../SimpleEntryTool.h"
#include "../MusicShape.h"
#include "../Renderer.h"

#include "../core/Chord.h"
#include "../core/Note.h"
#include "../core/VoiceBar.h"
#include "../core/Clef.h"
#include "../core/Voice.h"
#include "../core/Sheet.h"
#include "../core/Bar.h"
#include "../core/Part.h"
#include "../core/Staff.h"

#include "../commands/AddDotCommand.h"

#include <kicon.h>
#include <kdebug.h>
#include <klocale.h>

#include <math.h>

using namespace MusicCore;

DotsAction::DotsAction(SimpleEntryTool* tool)
    : AbstractMusicAction(KIcon("music-dottednote"), i18n("Dots"), tool)
{
}

inline static double sqr(double a) { return a*a; }

void DotsAction::mousePress(Staff* staff, int barIdx, const QPointF& pos)
{
    Part* part = staff->part();
    Sheet* sheet = part->sheet();
    Bar* bar = sheet->bar(barIdx);
    
    // loop over all chords
    double closestDist = 1e9;
    Chord* chord = 0;
    
    // outer loop, loop over all voices
    for (int v = 0; v < part->voiceCount(); v++) {
        Voice* voice = part->voice(v);
        VoiceBar* vb = voice->bar(bar);
        
        // next loop over all chords
        for (int e = 0; e < vb->elementCount(); e++) {
            Chord* c = dynamic_cast<Chord*>(vb->element(e));
            if (!c) continue;
            
            double centerX = c->x() + (c->width() / 2);
            double centerY = c->y() + (c->height() / 2);
            double dist = sqrt(sqr(centerX - pos.x()) + sqr(centerY - pos.y()));
            if (dist < closestDist) {
                closestDist = dist;
                chord = c;
            }
        }
    }
    
    if (!chord) return;
    if (closestDist > 10) return; // bah, magic numbers are ugly....
    
    m_tool->addCommand(new AddDotCommand(m_tool->shape(), chord));
}
