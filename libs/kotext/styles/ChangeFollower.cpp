/* This file is part of the KDE project
 * Copyright (C) 2006, 2009-2011 Thomas Zander <zander@kde.org>
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

#include "ChangeFollower_p.h"
#include "KCharacterStyle.h"
#include "KParagraphStyle.h"

#include <QVector>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <kdebug.h>

// #define DEBUG_CHANGES

ChangeFollower::ChangeFollower(QTextDocument *parent, KStyleManager *manager)
        : QObject(parent),
        m_document(parent),
        m_styleManager(manager)
{
}

ChangeFollower::~ChangeFollower()
{
    KStyleManager *sm = m_styleManager.data();
    if (sm)
        sm->remove(this);
}

void ChangeFollower::processUpdates(const QMap<int, QMap<int, QVariant> > &changedStyles)
{
#ifdef DEBUG_CHANGES
    kDebug(32500) << "styles changed;" << changedStyles.keys();
#endif
    KStyleManager *sm = m_styleManager.data();
    if (!sm) {
        // since the stylemanager would be the one calling this method, I doubt this
        // will ever happen.  But better safe than sorry..
        deleteLater();
        return;
    }

    QTextCursor cursor(m_document);
    cursor.beginEditBlock();
    QTextBlock block = cursor.block();
    while (block.isValid()) {
        QTextBlockFormat bf = block.blockFormat();
        int id = bf.intProperty(KParagraphStyle::StyleId);
        KParagraphStyle *paragStyle = 0;
        if (id > 0 && changedStyles.contains(id)) {
            paragStyle = sm->paragraphStyle(id);
            Q_ASSERT(paragStyle);
            cursor.setPosition(block.position());

            Q_ASSERT(changedStyles.contains(id)); // otherwise the [] next will be BAD
            const QMap<int, QVariant> &origValues = changedStyles[id];
            QMap<int, QVariant>::const_iterator it = origValues.begin();
            while (it != origValues.end()) {
                QVariant prop = bf.property(it.key());
                if (prop == it.value()) {
                    bf.clearProperty(it.key());
                }
                ++it;
            }
            cursor.setBlockFormat(bf); // first we remove all traces, below we'll reapply.
        }
        QTextCharFormat cf = block.charFormat();
        id = cf.intProperty(KCharacterStyle::StyleId);
        if (paragStyle && id > 0 && changedStyles.contains(id)) {
            KCharacterStyle *style = sm->characterStyle(id);
            Q_ASSERT(style);

            Q_ASSERT(changedStyles.contains(id)); // otherwise the [] next will be BAD
            const QMap<int, QVariant> &origValues = changedStyles[id];
            QMap<int, QVariant>::const_iterator it = origValues.begin();
            while (it != origValues.end()) {
                QVariant prop = cf.property(it.key());
                if (prop == it.value()) {
#ifdef DEBUG_CHANGES
                    QString out("Char-style[%1]; removing property 0x%2");
                    out = out.arg(style->name()).arg(it.key(), 3, 16);
                    kDebug(32500) << out;
#endif
                    cf.clearProperty(it.key());
                }
                ++it;
            }

            cursor.setBlockCharFormat(cf); // first we remove all traces, below we'll reapply.
        }

        QTextBlock::iterator iter = block.begin();
        while (! iter.atEnd()) {
            QTextFragment fragment = iter.fragment();
            cf = fragment.charFormat();
            id = cf.intProperty(KCharacterStyle::StyleId);
            if (id > 0 && changedStyles.contains(id)) {
#ifdef DEBUG_CHANGES
                kDebug(32500) << "Updating segment" << fragment.position() << fragment.text();
#endif
                KCharacterStyle *style = sm->characterStyle(id);
                Q_ASSERT(style);
                Q_ASSERT(changedStyles.contains(id)); // otherwise the [] next will be BAD
                const QMap<int, QVariant> &origValues = changedStyles[id];
                QMap<int, QVariant>::const_iterator it = origValues.begin();
                while (it != origValues.end()) {
                    QVariant prop = cf.property(it.key());
if (it.key() == 0x821) {
    qDebug() << "old style; " << it.value() << " new; " << prop.isNull() << prop.type() << prop.value<QBrush>();
}
                    if (prop == it.value()) {
#ifdef DEBUG_CHANGES
                        QString out("Char-segment[%1]; removing property 0x%2");
                        out = out.arg(style->name()).arg(it.key(), 3, 16);
                        kDebug(32500) << out;
#endif
                        cf.clearProperty(it.key());
                    }
                    ++it;
                }

                // create selection
                cursor.setPosition(fragment.position());
                cursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
                if (paragStyle && style != paragStyle->characterStyle())
                    style->applyStyle(cf);
                cursor.setCharFormat(cf);
            }
            iter++;
        }

        if (paragStyle) {
             // this properly applies both the block and the char style, with inheritence in mind.
            paragStyle->applyStyle(block, false);
        }

        block = block.next();
    }
    cursor.endEditBlock();
}

#include <ChangeFollower_p.moc>

