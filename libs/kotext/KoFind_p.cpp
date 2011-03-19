/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoFind_p.h"

#include <KoResourceManager.h>

#include <KWindowSystem>
#include <KFindDialog>
#include <KReplaceDialog>
#include <KFind>
#include <KLocale>
#include <KAction>

#include <QTextDocument>
#include <QTextCursor>
#include <QTimer>
#include <KDebug>

#include "KoFind.h"
#include "KoText.h"

class InUse
{
public:
    InUse(bool & variable)
            : m_variable(variable) {
        m_variable = true;
    }
    ~InUse() {
        m_variable = false;
    }

private:
    bool & m_variable;
};

KoFindPrivate::KoFindPrivate(KoFind *find, KoResourceManager *crp, QWidget *w)
        : findNext(0),
        findPrev(0),
        q(find),
        m_provider(crp),
        m_findStrategy(w),
        m_replaceStrategy(w),
        m_strategy(&m_findStrategy),
        m_document(0),
        m_restarted(false),
        m_start(false),
        m_inFind(false),
        m_findDirection(0),
        m_findForward(crp),
        m_findBackward(crp)
{
    QObject::connect(m_findStrategy.dialog(), SIGNAL(okClicked()), q, SLOT(startFind()));
    QObject::connect(m_replaceStrategy.dialog(), SIGNAL(okClicked()), q, SLOT(startReplace()));
}

void KoFindPrivate::resourceChanged(int key, const QVariant &variant)
{
    if (key == KoText::CurrentTextDocument) {
        m_document = static_cast<QTextDocument*>(variant.value<void*>());
        if (!m_inFind) {
            m_start = true;
        }
    } else if (key == KoText::CurrentTextPosition || key == KoText::CurrentTextAnchor) {
        if (!m_inFind) {
            const int selectionStart = m_provider->intResource(KoText::CurrentTextPosition);
            const int selectionEnd = m_provider->intResource(KoText::CurrentTextAnchor);
            m_findStrategy.dialog()->setHasSelection(selectionEnd != selectionStart);
            m_replaceStrategy.dialog()->setHasSelection(selectionEnd != selectionStart);

            m_start = true;
            m_provider->clearResource(KoText::SelectedTextPosition);
            m_provider->clearResource(KoText::SelectedTextAnchor);
        }
    }
}

void KoFindPrivate::findActivated()
{
    m_start = true;

    m_findStrategy.dialog()->setFindHistory(m_strategy->dialog()->findHistory());

    m_strategy = &m_findStrategy;

    m_strategy->dialog()->show();
    KWindowSystem::activateWindow(m_strategy->dialog()->winId());

    findNext->setEnabled(true);
    findPrev->setEnabled(true);
}

void KoFindPrivate::findNextActivated()
{
    Q_ASSERT(m_strategy);
    m_findStrategy.dialog()->setOptions((m_strategy->dialog()->options() | KFind::FindBackwards) ^ KFind::FindBackwards);
    m_strategy = &m_findStrategy;
    parseSettingsAndFind();
}

void KoFindPrivate::findPreviousActivated()
{
    Q_ASSERT(m_strategy);
    m_findStrategy.dialog()->setOptions(m_strategy->dialog()->options() | KFind::FindBackwards);
    m_strategy = &m_findStrategy;
    parseSettingsAndFind();
}

void KoFindPrivate::replaceActivated()
{
    m_start = true;

    m_replaceStrategy.dialog()->setFindHistory(m_strategy->dialog()->findHistory());

    m_strategy = &m_replaceStrategy;

    m_strategy->dialog()->show();
    KWindowSystem::activateWindow(m_strategy->dialog()->winId());
}

void KoFindPrivate::startFind()
{
    parseSettingsAndFind();

    QTimer::singleShot(0, m_findStrategy.dialog(), SLOT(show())); // show the findDialog again.
}

void KoFindPrivate::startReplace()
{
    m_replaceStrategy.dialog()->hide(); // We don't want the replace dialog to keep popping up
    parseSettingsAndFind();
}

void KoFindPrivate::findDocumentSetNext(QTextDocument *document)
{
    emit q->findDocumentSetNext(document);
}

void KoFindPrivate::findDocumentSetPrevious(QTextDocument *document)
{
    emit q->findDocumentSetPrevious(document);
}

void KoFindPrivate::parseSettingsAndFind()
{
    if (m_document == 0 || m_document->isEmpty())
        return;

    InUse used(m_inFind);

    long options = m_strategy->dialog()->options();

    QTextDocument::FindFlags flags;
    if ((options & KFind::WholeWordsOnly) != 0) {
        flags |= QTextDocument::FindWholeWords;
    }
    if ((options & KFind::CaseSensitive) != 0) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if ((options & KFind::FindBackwards) != 0) {
        flags |= QTextDocument::FindBackward;
        m_findDirection = &m_findBackward;
    } else {
        m_findDirection = &m_findForward;
    }

    const bool selectedText = (options & KFind::SelectedText) != 0;

    if (m_start) {
        m_start = false;
        m_restarted = false;
        m_strategy->reset();
        m_startDocument = m_document;
        m_lastKnownPosition = QTextCursor(m_document);
        if (selectedText) {
            int selectionStart = m_provider->intResource(KoText::CurrentTextPosition);
            int selectionEnd = m_provider->intResource(KoText::CurrentTextAnchor);
            if (selectionEnd < selectionStart) {
                qSwap(selectionStart, selectionEnd);
            }
            // TODO the SelectedTextPosition and SelectedTextAnchor are not highlighted yet
            // it would be cool to have the highlighted ligher when searching in selected text
            m_provider->setResource(KoText::SelectedTextPosition, selectionStart);
            m_provider->setResource(KoText::SelectedTextAnchor, selectionEnd);
            if ((options & KFind::FindBackwards) != 0) {
                m_lastKnownPosition.setPosition(selectionEnd);
                m_endPosition.setPosition(selectionStart);
            } else {
                m_lastKnownPosition.setPosition(selectionStart);
                m_endPosition.setPosition(selectionEnd);
            }
            m_startPosition = m_lastKnownPosition;
        } else {
            if ((options & KFind::FromCursor) != 0) {
                m_lastKnownPosition.setPosition(m_provider->intResource(KoText::CurrentTextPosition));
            } else {
                m_lastKnownPosition.setPosition(0);
            }
            m_endPosition = m_lastKnownPosition;
            m_startPosition = m_lastKnownPosition;
        }
        //kDebug() << "start" << m_lastKnownPosition.position();
    }

    QRegExp regExp;
    QString pattern = m_strategy->dialog()->pattern();
    if (options & KFind::RegularExpression) {
        regExp = QRegExp(pattern);
    }

    QTextCursor cursor;
    if (!regExp.isEmpty() && regExp.isValid()) {
        cursor = m_document->find(regExp, m_lastKnownPosition, flags);
    } else {
        cursor = m_document->find(pattern, m_lastKnownPosition, flags);
    }

    //kDebug() << "r" << m_restarted << "c > e" << ( m_document == m_startDocument && cursor > m_endPosition ) << ( m_startDocument == m_document && m_findDirection->positionReached(  cursor, m_endPosition ) )<< "e" << cursor.atEnd() << "n" << cursor.isNull();
    if ((((m_document == m_startDocument) && m_restarted) || selectedText)
            && (cursor.isNull() || m_findDirection->positionReached(cursor, m_endPosition))) {
        m_restarted = false;
        m_strategy->displayFinalDialog();
        m_lastKnownPosition = m_startPosition;
        return;
    } else if (cursor.isNull()) {
        m_restarted = true;
        m_findDirection->nextDocument(m_document, this);
        m_lastKnownPosition = QTextCursor(m_document);
        m_findDirection->positionCursor(m_lastKnownPosition);
        // restart from the beginning
        parseSettingsAndFind();
        return;
    } else {
        // found something
        bool goOn = m_strategy->foundMatch(cursor, m_findDirection);
        m_lastKnownPosition = cursor;
        if (goOn) {
            parseSettingsAndFind();
        }
    }
}
