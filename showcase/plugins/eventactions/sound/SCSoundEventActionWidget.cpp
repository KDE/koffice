/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or ( at your option) any later version.
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

#include "SCSoundEventActionWidget.h"

#include <QVBoxLayout>
#include <KFileDialog>

#include <KDE/KLocale>
#include <KDE/KComboBox>

#include <KEventActionAddCommand.h>
#include <KEventActionRemoveCommand.h>
#include <SCEventActionData.h>
#include <SCSoundCollection.h>
#include <SCSoundData.h>
#include "SCSoundEventAction.h"

SCSoundEventActionWidget::SCSoundEventActionWidget(QWidget * parent)
: SCEventActionWidget(parent)
, m_shape(0)
, m_eventAction(0)
, m_soundCollection(0)
, m_soundCombo(new KComboBox())
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_soundCombo);

    connect(m_soundCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(soundComboChanged()));

    setEnabled(false);
    updateCombo("");
}

SCSoundEventActionWidget::~SCSoundEventActionWidget()
{
}

void SCSoundEventActionWidget::setData(SCEventActionData * eventActionData)
{
    m_shape = eventActionData->shape();
    m_eventAction = eventActionData->eventAction();
    // TODO get the sound out ot the action
    QString title;
    SCSoundEventAction * eventAction = dynamic_cast<SCSoundEventAction *>(m_eventAction);
    if (eventAction) {
        title = eventAction->soundData()->title();
    }
    m_soundCollection = eventActionData->soundCollection();
    setEnabled(m_shape && m_soundCollection);
    updateCombo(title);
}

void SCSoundEventActionWidget::soundComboChanged()
{
    if (! m_shape) {
        return;
    }

    SCSoundData * soundData = 0;
    if (m_soundCombo->currentIndex() > 1) { // a previous sound was chosen
        // copy it rather then just point to it - so the refcount is updated
        soundData = new SCSoundData(*m_soundCollection->findSound(m_soundCombo->currentText()));
    }
    else if (m_soundCombo->currentIndex() == 1) { // "Import..." was chosen
        KUrl url = KFileDialog::getOpenUrl();
        if (!url.isEmpty()) {
            soundData = new SCSoundData(m_soundCollection, url.toLocalFile());
            // TODO shouldn't that come from the sound collection
            // what if the user opens a already opened sound again?
            QFile *file = new QFile(url.toLocalFile());
            file->open(QIODevice::ReadOnly);
            soundData->loadFromFile(file); //also closes the file and deletes the class
        }
    }

    // TODO better name e.g. on new or remove sound
    QUndoCommand * cmd = new QUndoCommand(i18n("Change sound action"));
    if (m_eventAction) {
        new KEventActionRemoveCommand(m_shape, m_eventAction, cmd);
        m_eventAction = 0;
    }

    if (soundData) {
        SCSoundEventAction * eventAction = new SCSoundEventAction();
        eventAction->setSoundData(soundData);
        m_eventAction = eventAction;
        new KEventActionAddCommand(m_shape, eventAction, cmd);
    }

    emit(addCommand(cmd));

    updateCombo(soundData ? soundData->title() : "");
}

void SCSoundEventActionWidget::updateCombo(const QString & title)
{
    m_soundCombo->blockSignals(true);

    m_soundCombo->clear();
    m_soundCombo->addItem(i18n("No sound"));
    m_soundCombo->addItem(i18n("Import..."));
    if (m_soundCollection) {
        m_soundCombo->addItems(m_soundCollection->titles());
    }
    if (title.isEmpty()) {
        m_soundCombo->setCurrentIndex(0);
    }
    else {
        m_soundCombo->setCurrentIndex(m_soundCombo->findText(title));
    }

    m_soundCombo->blockSignals(false);
}

#include "SCSoundEventActionWidget.moc"
