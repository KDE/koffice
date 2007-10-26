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
#ifndef MUSIC_CORE_NOTE_H
#define MUSIC_CORE_NOTE_H

#include <QtCore/QObject>

namespace MusicCore {

class Staff;

/**
 * This class represents one note in a chord. You should not add the same note instance to more than one chord, nor
 * should you add a note to a chord in a different part than the part in which staff this note belongs to is in.
 */
class Note : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new note instance, displayed on the given staff.
     *
     * @param staff the staff the note should be on
     * @param pitch the pitch of the new note
     * @param accidentals the accidentals of the new note
     */
    Note(Staff* staff, int pitch, int accidentals = 0);

    /**
     * Destructor.
     */
    ~Note();

    /**
     * Returns the staff for this note.
     */
    Staff* staff();
    
    void setStaff(Staff* staff);

    /**
     * Returns the pitch for this note.
     */
    int pitch() const;

    /**
     * Returns the accidentals for this note.
     */
    int accidentals() const;

    void setAccidentals(int accidentals);
    
    bool isStartTie() const;
    void setStartTie(bool startTie);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_NOTE_H
