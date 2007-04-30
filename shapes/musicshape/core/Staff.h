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
#ifndef MUSIC_CORE_STAFF_H
#define MUSIC_CORE_STAFF_H

#include <QtCore/QString>

namespace MusicCore {

class Part;

/**
 * A Staff is purely used for displaying/formatting. The staff class is used to indicate on what staff
 * music elements should be printed, but does only contain information regarding the formatting.
 */
class Staff
{
public:
    /**
     * Returns the part this staff is part of.
     */
    Part* part();

    /**
     * Returns the spacing in points between this staff and the staff above it.
     */
    double spacing() const;

    /**
     * Set the spacing between this staff and the staff above it.
     *
     * @param spacing the new spacing.
     */
    void setSpacing(double spacing);

    /**
     * Returns the vertical position of this staff relative to the top of the staff system.
     */
    double top();

    /**
     * Returns the number of lines in this staff. Typical values are 5 for normal music, or 1 for a percussion part.
     * The default value for this attribute is 5.
     */
    int lineCount() const;

    /**
     * Sets the number of lines of this staff.
     *
     * @param lineCount the new number of lines in this staff.
     */
    void setLineCount(int lineCount);

    /**
     * Returns the distance in points between two lines of this staff.
     */
    double lineSpacing() const;

    /**
     * Sets the distance in points between two lines of this staff.
     *
     * @param lineSpacing the new distance between two lines.
     */
    void setLineSpacing(double lineSpacing);
private:
    Staff(Part* part);
    ~Staff();
    friend class Part;
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_PART_H
