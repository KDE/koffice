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
#ifndef KOFIND_P_H
#define KOFIND_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KOdfText API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "KFindStrategy_p.h"
#include "KReplaceStrategy_p.h"
#include "FindDirection_p.h"

#include <QTextCursor>

class KoFind;
class KResourceManager;
class QAction;
class QTextDocument;
class QVariant;
class QWidget;

class KoFindPrivate
{
public:
    KoFindPrivate(KoFind *find, KResourceManager *crp, QWidget *w);

    void resourceChanged(int key, const QVariant &variant);

    void findActivated();

    void findNextActivated();

    void findPreviousActivated();

    void replaceActivated();

    // executed when the user presses the 'find' button.
    void startFind();

    // executed when the user presses the 'replace' button.
    void startReplace();

    QAction *findNext;
    QAction *findPrev;

    void findDocumentSetNext(QTextDocument * document);
    void findDocumentSetPrevious(QTextDocument * document);

    KoFind *q;

private:
    void parseSettingsAndFind();

    KResourceManager *m_provider;
    KFindStrategy m_findStrategy; /// strategy used for find
    KReplaceStrategy m_replaceStrategy; /// strategy used for replace
    KFindStrategyBase *m_strategy; /// the current strategy used

    QTextDocument *m_document;
    QTextDocument *m_startDocument;
    QTextCursor m_lastKnownPosition;
    bool m_restarted;
    bool m_start; /// if true find/replace is restarted
    bool m_inFind; /// if true find/replace is running (not showing the dialog)
    QTextCursor m_startPosition;
    QTextCursor m_endPosition;
    FindDirection *m_findDirection; /// the current direction used
    FindForward m_findForward;
    FindBackward m_findBackward;
};

#endif /* KOFIND_P_H */
