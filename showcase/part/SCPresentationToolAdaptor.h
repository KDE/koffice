/* This file is part of the KDE project
 * Copyright (C) 2010 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#ifndef SCPRESENTATIONTOOLADAPTOR_H
#define SCPRESENTATIONTOOLADAPTOR_H

#include <QtDBus/QtDBus>

class SCViewModePresentation;
class SCPresentationTool;

class SCPresentationToolAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.koffice.presentation.tool")

public:
    explicit SCPresentationToolAdaptor(SCPresentationTool *tool);
    virtual ~SCPresentationToolAdaptor();

public slots: // METHODS

    /**
     * Blank the presentation.
     */
    void blankPresentation();

    /**
     * Highlight the presentation.
     */
    Q_NOREPLY void highlightPresentation(int pointx, int pointy);

    /**
     * Draw on presentation.
     * @color can take the color values as a string. Eg: red, green, black.
     */
    void startDrawPresentation(int pointx, int pointy, int penSize, QString color);

    /**
     * Start drawing on presentation tool.
     * The parameters pointx and pointy specify a point in the path.
     */
    Q_NOREPLY void drawOnPresentation(int pointx,int pointy);

    /**
     * Stop drawing path. The current path is stopped.
     */
    void stopDrawPresentation();

    /**
     * Normal presentation mode.
     */
    void normalPresentation();

private:
    SCPresentationTool *m_tool;
    SCViewModePresentation &m_viewModePresentation;
};

#endif /* SCPRESENTATIONTOOLADAPTOR_H */
