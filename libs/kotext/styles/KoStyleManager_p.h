/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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
#ifndef KOSTYLEMANAGERPRIVATE_P_H
#define KOSTYLEMANAGERPRIVATE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KoText API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QHash>
#include <QList>
#include <QMap>

class KOTEXT_TEST_EXPORT KoStyleManagerPrivate
{
public:
    KoStyleManagerPrivate();
    ~KoStyleManagerPrivate();
    void refreshUnsetStoreFor(int key);
    void requestFireUpdate(KoStyleManager *q);
    void updateAlteredStyles(); // slot for the QTimer::singleshot

    /// recursively add all char styles under \a ps
    void requestUpdateForChildren(KoParagraphStyle *ps);


    static int s_stylesNumber; // For giving out unique numbers to the styles for referencing

    QHash<int, KoCharacterStyle*> charStyles;
    QHash<int, KoParagraphStyle*> paragStyles;
    QHash<int, KoListStyle*> listStyles;
    QHash<int, KoListStyle *> automaticListStyles;
    QHash<int, KoTableStyle *> tableStyles;
    QHash<int, KoTableColumnStyle *> tableColumnStyles;
    QHash<int, KoTableRowStyle *> tableRowStyles;
    QHash<int, KoTableCellStyle *> tableCellStyles;
    QHash<int, KoSectionStyle *> sectionStyles;
    QList<ChangeFollower*> documentUpdaterProxies;

    /// the unsetStore has a hash from the styleId to a set of all the property ids of a style
    QHash<int, QList<int> > unsetStore;

    bool updateTriggered;
    QMap<int, QList<int> > updateQueue;

    KoParagraphStyle *defaultParagraphStyle;
    KoListStyle *defaultListStyle;
    KoListStyle *outlineStyle;
};

#endif
