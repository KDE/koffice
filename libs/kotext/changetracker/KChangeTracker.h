/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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
#ifndef KCHANGETRACKER_H
#define KCHANGETRACKER_H

//KOffice includes
#include "kotext_export.h"

#include <KOdfGenericChange.h>
#include <KOdfGenericChanges.h>

//Qt includes
#include <QObject>
#include <QMetaType>

class QTextCursor;
class QTextFormat;
class QString;
class QTextDocumentFragment;
class QTextList;
class KDeleteChangeMarker;
class KXmlElement;
class KChangeTrackerElement;
class KFormatChangeInformation;
class KDeletedRowColumnDataStore;

class KOTEXT_EXPORT KChangeTracker : public QObject
{
    Q_OBJECT
public:

    typedef enum
    {
        ODF_1_2 = 0,
        DELTAXML
    }ChangeSaveFormat;

    KChangeTracker(QObject *parent = 0);

    ~KChangeTracker();

    void setRecordChanges(bool enabled);
    bool recordChanges() const;

    void setDisplayChanges(bool enabled);
    bool displayChanges() const;

    /// returns the changeId of the changeElement registered for the given change. This may be an already existing changeId, if the change could be merged.
    int changeId(const QString &title, KOdfGenericChange::Type type, const QTextCursor &selection, const QTextFormat &newFormat, int prevCharChangeId, int nextCharChangeId);

    int formatChangeId(const QString &title, const QTextFormat &format, const QTextFormat &prevFormat, int existingChangeId);
    int insertChangeId(const QString &title, int existingChangeId);
    int deleteChangeId(const QString &title, const QTextDocumentFragment &selection, int existingChangeId);

    void setFormatChangeInformation(int formatChangeId, KFormatChangeInformation *formatInformation);
    KFormatChangeInformation *formatChangeInformation(int formatChangeId);

    KChangeTrackerElement* elementById(int id);
    bool removeById(int id, bool freeMemory = true);

    //Returns all the deleted changes
    int deletedChanges(QVector<KChangeTrackerElement *> &deleteVector) const;

    int allChangeIds(QVector<int> &changesVector) const;

    bool containsInlineChanges(const QTextFormat &format);
    int mergeableId(KOdfGenericChange::Type type, const QString &title, int existingId) const;

    QColor insertionBgColor() const;
    QColor deletionBgColor() const;
    QColor formatChangeBgColor() const;

    void setInsertionBgColor(const QColor &color);
    void setDeletionBgColor(const QColor &color);
    void setFormatChangeBgColor(const QColor &color);

    /// Splits a changeElement. This creates a duplicate changeElement with a different changeId. This is used because we do not support overlapping change regions. The function returns the new changeId
    int split(int changeId);

    bool isParent(int testedParentId, int testedChildId) const;
    void setParent(int child, int parent);
    int parent(int changeId) const;

    int createDuplicateChangeId(int existingChangeId);
    bool isDuplicateChangeId(int duplicateChangeId);
    int originalChangeId(int duplicateChangeId);

    void acceptRejectChange(int changeId, bool set);

    /// Load/save methods
    bool saveInlineChange(int changeId, KOdfGenericChange &change);

    void loadOdfChanges(const KXmlElement &element);
    int loadedChangeId(QString odfId);

    static QTextDocumentFragment generateDeleteFragment(QTextCursor &cursor, KDeleteChangeMarker *marker);
    static void insertDeleteFragment(QTextCursor &cursor, KDeleteChangeMarker *marker);
    static int fragmentLength(QTextDocumentFragment fragment);

    QString authorName();
    void setAuthorName(const QString &authorName);

    ChangeSaveFormat saveFormat();
    void setSaveFormat(ChangeSaveFormat saveFormat);

    KDeletedRowColumnDataStore *deletedRowColumnData();
private:
    class Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KChangeTracker*)

#endif
