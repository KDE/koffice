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
#ifndef MUSIC_CORE_VOICEELEMENT_H
#define MUSIC_CORE_VOICEELEMENT_H

#include <QtCore/QObject>

namespace MusicCore {

class Staff;
class VoiceBar;

/**
 * This is the base class for all musical elements that can be added to a voice.
 */
class VoiceElement : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new VoiceElement.
     */
    explicit VoiceElement(int length = 0);

    /**
     * Destructor.
     */
    virtual ~VoiceElement();

    /**
     * Returns the staff this music element should be displayed on. It can also be NULL, for example if the element
     * should not be visible.
     */
    Staff* staff() const;

    /**
     * Sets the staff this element should be displayed on.
     *
     * @param staff the new staff this element should be displayed on
     */
    void setStaff(Staff* staff);

    VoiceBar* voiceBar() const;
    void setVoiceBar(VoiceBar* voiceBar);

    /**
     * Returns the x position of this musical element. The x position of an element is measured relative to the left
     * barline of the bar the element is in.
     */
    virtual double x() const;

    /**
     * Returns the y position of this musical element. The y position of an element is measure relative to the top
     * of the staff it is in.
     */
    virtual double y() const;

    /**
     * Returns the width of this musical element.
     */
    virtual double width() const;

    /**
     * Returns the height of this musical element.
     */
    virtual double height() const;

    /**
     * Returns the duration of this musical elements in ticks.
     */
    int length() const;
public slots:
    /**
     * Sets the x position of this musical element.
     */
    void setX(double x);

    /**
     * Sets the y position of this musical element.
     */
    void setY(double y);
protected slots:
    /**
     * Changes the duration of this musical element.
     *
     * @param length the new duration of this musical element
     */
    void setLength(int length);

    /**
     * Sets the width of this musical element.
     *
     * @param width the new width of this musical element
     */
    void setWidth(double width);

    /**
     * Sets the height of this musical element.
     *
     * @param height the new height of this musical element
     */
    void setHeight(double height);
signals:
    void xChanged(double x);
    void yChanged(double y);
    void lengthChanged(int length);
    void widthChanged(double width);
    void heightChanged(double height);
private:
    class Private;
    Private * const d;
};

} // namespace MusicCore

#endif // MUSIC_CORE_VOICEELEMENT_H
