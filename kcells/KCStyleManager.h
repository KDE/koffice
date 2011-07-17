/* This file is part of the KDE project
   Copyright (C) 2003 Norbert Andres, nandres@web.de

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCELLS_STYLE_MANAGER
#define KCELLS_STYLE_MANAGER

#include "kcells_export.h"
#include <KXmlReader.h>

#include <KCStyle.h>

class QDomElement;
class QDomDocument;
class QStringList;

class KOdfGenericStyles;
class KOdfStylesReader;

class KCConditions;
class KCMap;
class StyleDialog;
class KCValueParser;

/**
 * \class KCStyleManager
 * \brief Manages cell styles
 * \ingroup KCStyle
 * The KCStyleManager takes care of named styles. It also provides some static
 * methods for the preloading of OpenDocument autostyles.
 */
class KCELLS_EXPORT KCStyleManager
{
    friend class StyleManagerDialog;

public:
    KCStyleManager();
    ~KCStyleManager();

    QDomElement save(QDomDocument & doc);
    bool loadXML(KXmlElement const & styles);

    void saveOdf(KOdfGenericStyles &mainStyles);
    void loadOdfStyleTemplate(KOdfStylesReader& stylesReader, KCMap* map = 0);

    KCCustomStyle * defaultStyle() const {
        return m_defaultStyle;
    }

    /**
     * Searches for a style named \p name in the map of styles.
     * On OpenDocument loading, it searches the name in the map sorted
     * by the OpenDocument internal name .
     * \return the custom style named \p name
     */
    KCCustomStyle * style(QString const & name) const;

    void resetDefaultStyle();

    bool checkCircle(QString const & name, QString const & parent);
    bool validateStyleName(QString const & name, KCCustomStyle * style);
    void changeName(QString const & oldName, QString const & newName);

    void insertStyle(KCCustomStyle *style);

    void takeStyle(KCCustomStyle * style);
    void createBuiltinStyles();

    QStringList styleNames() const;
    int count() const {
        return m_styles.count();
    }

    /**
     * Loads OpenDocument auto styles.
     * The auto styles are preloaded, because an auto style could be shared
     * among cells. So, preloading prevents a multiple loading of the same
     * auto style.
     * This method is called before the cell loading process.
     * @param stylesReader repository of styles
     * @return a hash of styles with the OpenDocument internal name as key
     */
    Styles loadOdfAutoStyles(KOdfStylesReader& stylesReader,
                             QHash<QString, KCConditions>& conditionalStyles,
                             const KCValueParser *parser);

    /**
     * Releases unused auto styles.
     * If there are auto styles, which are not used by any cell (uncommon case)
     * this method makes sure, that these get deleted.
     * This method is called after the cell loading porcess.
     * @param autoStyles a hash of styles with the OpenDocument internal name as
     *                   key
     * @see loadOdfAutoStyles
     */
    void releaseUnusedAutoStyles(Styles autoStyles);

    /// OpenDocument name to internal name (on loading) or vice versa (on saving)
    QString openDocumentName(const QString&) const;

private:
    void dump() const;

    KCCustomStyle * m_defaultStyle;
    CustomStyles  m_styles; // builtin and custom made styles

    // OpenDocument name to internal name (on loading) or vice versa (on saving)
    // NOTE: Temporary! Only valid while loading or saving OpenDocument files.
    QHash<QString, QString>  m_oasisStyles;
};

#endif // KCELLS_STYLE_MANAGER
