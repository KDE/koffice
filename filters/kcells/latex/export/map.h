/*
** Header file for inclusion with kword_xml2latex.c
**
** Copyright (C) 2000 Robert JACOLIN
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Library General Public
** License as published by the Free Software Foundation; either
** version 2 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.
**
** To receive a copy of the GNU Library General Public License, write to the
** Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
**
*/

#ifndef KCELLS_LATEX_MAP_H
#define KCELLS_LATEX_MAP_H

#include <QString>
#include <QStack>  /* historic list */
#include <QList>  /* for list of format */
#include <QTextStream>

#include "xmlparser.h"
#include "latex_config.h"
#include "table.h"

/***********************************************************************/
/* Class: KCMap                                                         */
/***********************************************************************/

/**
 * This class hold a real paragraph. It tells about the text in this
 * paragraph, its format, etc. The complete text is a list of KCMap instances.
 * A footnote is a list of paragraph instances (now but not in the "futur").
 */
class KCMap: public XmlParser, Config
{
    QList<Table*> _tables;

public:
    /**
     * Constructors
     *
     * Creates a new instance of KCMap.
     */
    KCMap();

    /*
     * Destructor
     *
     * The destructor must remove the list of little zones.
     */
    virtual ~KCMap();

    /**
     * Accessors
     */

    /**
     * Modifiers
     */

    /**
     * Helpful functions
     */

    /**
     * Get information from a markup tree.
     */
    void analyze(const QDomNode);

    /**
     * Write the paragraph in a file.
     */
    void generate(QTextStream&);


private:

};

#endif /* KCELLS_LATEX_MAP_H */
