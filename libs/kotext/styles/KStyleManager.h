/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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

#ifndef KSTYLEMANAGER_H
#define KSTYLEMANAGER_H

#include "kotext_export.h"

#include <QObject>
#include <QMetaType>

class QTextDocument;
class KCharacterStyle;
class KParagraphStyle;
class KListStyle;
class KTableStyle;
class KTableColumnStyle;
class KTableRowStyle;
class KTableCellStyle;
class KSectionStyle;
class KXmlWriter;
class ChangeFollower;
class KOdfGenericStyles;
class KTextShapeData;
class KStyleManagerPrivate;

/**
 * Manages all character, paragraph, table and table cell styles for any number
 * of documents.
 */
class KOTEXT_EXPORT KStyleManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a new style manager.
     * @param parent pass a parent to use qobject memory management
     */
    explicit KStyleManager(QObject *parent = 0);

    /**
     * Destructor.
     */
    virtual ~KStyleManager();

    // load is not needed as it is done in KTextSharedLoadingData

    /**
     * Save document styles
     */
    void saveOdf(KOdfGenericStyles &mainStyles);

    /**
     * Add a new style, automatically giving it a new styleId.
     */
    void add(KCharacterStyle *style);
    /**
     * Add a new style, automatically giving it a new styleId.
     */
    void add(KParagraphStyle *style);
    /**
     * Add a new list style, automatically giving it a new styleId.
     */
    void add(KListStyle *style);
    /**
     * Add a new table style, automatically giving it a new styleId.
     */
    void add(KTableStyle *style);
    /**
     * Add a new table column style, automatically giving it a new styleId.
     */
    void add(KTableColumnStyle *style);
    /**
     * Add a new table row style, automatically giving it a new styleId.
     */
    void add(KTableRowStyle *style);
    /**
     * Add a new table cell style, automatically giving it a new styleId.
     */
    void add(KTableCellStyle *style);
    /**
     * Add a new sewction style, automatically giving it a new styleId.
     */
    void add(KSectionStyle *style);

    /**
     * Remove a style.
     */
    void remove(KCharacterStyle *style);
    /**
     * Remove a style.
     */
    void remove(KParagraphStyle *style);
    /**
     * Remove a list style.
     */
    void remove(KListStyle *style);
    /**
     * Remove a table style.
     */
    void remove(KTableStyle *style);
    /**
     * Remove a table column style.
     */
    void remove(KTableColumnStyle *style);
    /**
     * Remove a table row style.
     */
    void remove(KTableRowStyle *style);
    /**
     * Remove a table cell style.
     */
    void remove(KTableCellStyle *style);
    /**
     * Remove a section style.
     */
    void remove(KSectionStyle *style);

    /**
     * Add a document for which the styles will be applied.
     * Whenever a style is changed (signified by a alteredStyle() call) all
     * registered documents will be updated to reflect that change.
     */
    void add(QTextDocument *document);
    /**
     * Remove a previously registered document.
     */
    void remove(QTextDocument *document);

    /**
     * Return a characterStyle by its id.
     * From documents you can retrieve the id out of each QTextCharFormat
     * by requesting the KCharacterStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KCharacterStyle::styleId()
     */
    KCharacterStyle *characterStyle(int id) const;

    /**
     * Return a paragraphStyle by its id.
     * From documents you can retrieve the id out of each QTextBlockFormat
     * by requesting the KParagraphStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KParagraphStyle::styleId()
     */
    KParagraphStyle *paragraphStyle(int id) const;

    /**
     * Return a list style by its id.
     */
    KListStyle *listStyle(int id) const;

    /**
     * Return a tableStyle by its id.
     * From documents you can retrieve the id out of each QTextTableFormat
     * by requesting the KTableStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KTableStyle::styleId()
     */
    KTableStyle *tableStyle(int id) const;

    /**
     * Return a tableColumnStyle by its id.
     * From documents you can retrieve the id out of the KoTableRowandColumnStyleManager
     * @param id the unique Id to search for.
     * @see KTableColumnStyle::styleId()
     */
    KTableColumnStyle *tableColumnStyle(int id) const;

    /**
     * Return a tableRowStyle by its id.
     * From documents you can retrieve the id out of the KoTableRowandColumnStyleManager
     * @param id the unique Id to search for.
     * @see KTableRowStyle::styleId()
     */
    KTableRowStyle *tableRowStyle(int id) const;

    /**
     * Return a tableCellStyle by its id.
     * From documents you can retrieve the id out of each QTextTableCellFormat
     * by requesting the KTableCellStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KTableCellStyle::styleId()
     */
    KTableCellStyle *tableCellStyle(int id) const;

    /**
     * Return a sectionStyle by its id.
     * From documents you can retrieve the id out of each QTextFrameFormat
     * by requesting the KSectionStyle::StyleId property.
     * @param id the unique Id to search for.
     * @see KSectionStyle::styleId()
     */
    KSectionStyle *sectionStyle(int id) const;

    /**
     * Return the first characterStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see characterStyle(id);
     */
    KCharacterStyle *characterStyle(const QString &name) const;

    /**
     * Return the first paragraphStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see paragraphStyle(id);
     */
    KParagraphStyle *paragraphStyle(const QString &name) const;

    /**
     * Returns the first  listStyle ith the param use-visible-name.
     */
    KListStyle *listStyle(const QString &name) const;

    /**
     * Return the first tableStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see tableStyle(id);
     */
    KTableStyle *tableStyle(const QString &name) const;

    /**
     * Return the first tableColumnStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see tableColumnStyle(id);
     */
    KTableColumnStyle *tableColumnStyle(const QString &name) const;

    /**
     * Return the first tableRowStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see tableRowStyle(id);
     */
    KTableRowStyle *tableRowStyle(const QString &name) const;

    /**
     * Return the first tableCellStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see tableCellStyle(id);
     */
    KTableCellStyle *tableCellStyle(const QString &name) const;

    /**
     * Return the first sectionStyle with the param user-visible-name.
     * Since the name does not have to be unique there can be multiple
     * styles registered with that name, only the first is returned
     * @param name the name of the style.
     * @see sectionStyle(id);
     */
    KSectionStyle *sectionStyle(const QString &name) const;

     /**
     * Return the default paragraph style that will always be present in each
     * document. You can alter the style, but you can never delete it.
     * The default is suppost to stay invisible to the user and its called
     * i18n("[No Paragraph Style]") for that reason. Applications should not
     * show this style in their document-level configure dialogs.
     *
     * All paragraph styles will have this style as a parent, making this the ultimate fall-back.
     */
    KParagraphStyle *defaultParagraphStyle() const;

    /**
     * Returns the default list style to be used for lists, headers, paragraphs
     * that do not specify a list-style
     */
    KListStyle *defaultListStyle() const;

    /**
     * Sets the outline style to be used for headers that are not specified as lists
     */
    void setOutlineStyle(KListStyle *listStyle);

    /**
     * Returns the outline style to be used for headers that are not specified as lists
     */
    KListStyle *outlineStyle() const;

    /// return all the characterStyles registered.
    QList<KCharacterStyle*> characterStyles() const;

    /// return all the paragraphStyles registered.
    QList<KParagraphStyle*> paragraphStyles() const;

    /// return all the listStyles registered.
    QList<KListStyle*> listStyles() const;

    /// return all the tableStyles registered.
    QList<KTableStyle*> tableStyles() const;

    /// return all the tableColumnStyles registered.
    QList<KTableColumnStyle*> tableColumnStyles() const;

    /// return all the tableRowStyles registered.
    QList<KTableRowStyle*> tableRowStyles() const;

    /// return all the tableCellStyles registered.
    QList<KTableCellStyle*> tableCellStyles() const;

    /// return all the sectionStyles registered.
    QList<KSectionStyle*> sectionStyles() const;

    /// \internal
    KStyleManagerPrivate *priv();

signals:
    void styleAdded(KParagraphStyle*);
    void styleAdded(KCharacterStyle*);
    void styleAdded(KListStyle*);
    void styleAdded(KTableStyle*);
    void styleAdded(KTableColumnStyle*);
    void styleAdded(KTableRowStyle*);
    void styleAdded(KTableCellStyle*);
    void styleAdded(KSectionStyle*);
    void styleRemoved(KParagraphStyle*);
    void styleRemoved(KCharacterStyle*);
    void styleRemoved(KListStyle*);
    void styleRemoved(KTableStyle*);
    void styleRemoved(KTableColumnStyle*);
    void styleRemoved(KTableRowStyle*);
    void styleRemoved(KTableCellStyle*);
    void styleRemoved(KSectionStyle*);

public slots:
    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KParagraphStyle *style);
    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KCharacterStyle *style);

    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KListStyle *style);

    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KTableStyle *style);

    /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KTableColumnStyle *style);

     /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KTableRowStyle *style);

   /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KTableCellStyle *style);

   /**
     * Slot that should be called whenever a style is changed. This will update
     * all documents with the style.
     * Note that successive calls are aggregated.
     */
    void alteredStyle(const KSectionStyle *style);

private:
    friend class ChangeFollower;
    void remove(ChangeFollower *cf);

    friend class KTextSharedLoadingData;
    void addAutomaticListStyle(KListStyle *listStyle);
    friend class KTextShapeData;
    friend class KTextShapeDataPrivate;
    KListStyle *listStyle(int id, bool *automatic) const;

private:
    KStyleManagerPrivate *d;

    Q_PRIVATE_SLOT(d, void updateAlteredStyles())
};

Q_DECLARE_METATYPE(KStyleManager*)

#endif
