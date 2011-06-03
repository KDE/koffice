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

#include "KWFrameDialog.h"

#include "KWShapeConfigFactory.h"
#include "KWFrameConnectSelector.h"
#include "KWFrameRunaroundProperties.h"
#include "KWGeneralFrameProperties.h"
#include "KWFrameGeometry.h"
#include "KWDocument.h"
#include "frames/KWFrame.h"

#include <QUndoCommand>

KWFrameDialog::KWFrameDialog(const QList<KWFrame*> &frames, KWDocument *document, QWidget *parent)
        : KPageDialog(parent),
        m_frameConnectSelector(0),
        m_frameGeometry(0),
        m_document(document)
{
    setWindowTitle(i18n("Shape Properties"));
    m_state = new FrameConfigSharedState(document);
    setFaceType(Tabbed);
    m_generalFrameProperties = new KWGeneralFrameProperties(m_state);
    addPage(m_generalFrameProperties, i18n("Options"));
    m_frameRunaroundProperties = new KWFrameRunaroundProperties(m_state);
    addPage(m_frameRunaroundProperties, i18n("Text Run Around"));

    if (frames.count() == 1) {
        m_frameConnectSelector = new KWFrameConnectSelector(m_state);
        KWFrame *frame = frames.first();
        m_state->setKeepAspectRatio(frame->shape()->keepAspectRatio());
        if (m_frameConnectSelector->open(frame))
            addPage(m_frameConnectSelector, i18n("Connect Text Frames"));
        else {
            delete m_frameConnectSelector;
            m_frameConnectSelector = 0;
        }
        m_frameGeometry = new KWFrameGeometry(m_state);
        m_frameGeometry->open(frame);
        addPage(m_frameGeometry, i18n("Geometry"));
    }

    m_generalFrameProperties->open(frames);
    m_frameRunaroundProperties->open(frames);

    connect(this, SIGNAL(okClicked()), this, SLOT(okClicked()));
    connect(this, SIGNAL(cancelClicked()), this, SLOT(cancelClicked()));
}

KWFrameDialog::~KWFrameDialog()
{
}

void KWFrameDialog::okClicked()
{
    QUndoCommand *cmd = new QUndoCommand(i18n("Frame changes"));
    if (m_frameConnectSelector) {
        m_frameConnectSelector->save();
        //m_frameConnectSelector->createCommand(cmd); TODO
    }
    m_generalFrameProperties->save();
    //m_generalFrameProperties->createCommand(cmd); TODO
    m_frameRunaroundProperties->save();
    //m_frameRunaroundProperties->createCommand(cmd); TODO
    if (m_frameGeometry) {
        m_frameGeometry->save();
        m_frameGeometry->createCommand(cmd);
    }
    m_document->addCommand(cmd);
}

void KWFrameDialog::cancelClicked()
{
    if (m_frameGeometry)
        m_frameGeometry->cancel();
}

