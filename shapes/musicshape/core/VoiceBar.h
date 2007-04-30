/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#ifndef MUSIC_CORE_VOICEBAR_H
#define MUSIC_CORE_VOICEBAR_H

namespace MusicCore {

class Voice;
class Bar;
class MusicElement;

/**
 *
 * Once a MusicElement is added/inserted to a VoiceBar, the VoiceBar gets ownership of the element and will delete it
 * when it is removed, or if the VoiceBar is deleted. So you should only add dynamicaly allocated elemenst to a voice
 * bar.
 */
class VoiceBar
{
public:
    Voice* voice();
    Bar* bar();
    int elementCount() const;
    MusicElement* element(int index);
    void addElement(MusicElement* element);
    void insertElement(MusicElement* element, int before);
    void insertElement(MusicElement* element, MusicElement* before);
    void removeElement(int index);
    void removeElement(MusicElement* element);
private:
    VoiceBar(Voice* voice, Bar* bar);
    ~VoiceBar();
    friend class Voice;
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_VOICEBAR_H
