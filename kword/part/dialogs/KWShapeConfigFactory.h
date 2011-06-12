/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef KWSHAPECONFIGFACTORY_H
#define KWSHAPECONFIGFACTORY_H

#include <QObject>

class KoShape;
class KWFrame;
class KWDocument;

/// \internal
class FrameConfigSharedState : public QObject
{
    Q_OBJECT
public:
    explicit FrameConfigSharedState(KWDocument *document);
    ~FrameConfigSharedState();

    void addUser();
    void removeUser();

    KWFrame *frame() const {
        return m_frame;
    }
    void setFrame(KWFrame *frame);
    KWFrame *createFrame(KoShape *shape);
    void markFrameUsed() {
        m_deleteFrame = false;
    }

    KWDocument *document() const {
        return m_document;
    }

    bool keepAspectRatio() const {
        return m_protectAspectRatio;
    }
    void setKeepAspectRatio(bool on);

signals:
    void keepAspectRatioChanged(bool keep);

private:
    int m_refcount;
    bool m_deleteFrame;
    bool m_protectAspectRatio; // states if the user has this boolean set right now.
    KWFrame *m_frame;
    KWDocument *m_document;
};

#endif
