/*
 * This file is part of Office 2007 Filters for KOffice
 *
 * Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef MSOOXMLDIAGRAMREADER_H
#define MSOOXMLDIAGRAMREADER_H

#include <MsooXmlReader.h>
#include <MsooXmlCommonReader.h>

#include "msooxml_export.h"

namespace MSOOXML
{

namespace Diagram
{
    class Context;
    class DataModel;
    class LayoutNode;
}

class MSOOXML_EXPORT MsooXmlDiagramReaderContext : public MSOOXML::MsooXmlReaderContext
{
public:
    KOdfGenericStyles* m_styles;
    Diagram::Context* m_context;
    explicit MsooXmlDiagramReaderContext(KOdfGenericStyles* styles);
    virtual ~MsooXmlDiagramReaderContext();
    void saveIndex(KXmlWriter* xmlWriter, const QRect &rect);
};

class MSOOXML_EXPORT MsooXmlDiagramReader : public MSOOXML::MsooXmlCommonReader
{
public:
    MsooXmlDiagramReader(KoOdfWriters *writers);
    virtual ~MsooXmlDiagramReader();
    virtual KoFilter::ConversionStatus read(MSOOXML::MsooXmlReaderContext* context = 0);
protected:
    //KoFilter::ConversionStatus read_layoutNode();
    //KoFilter::ConversionStatus read_choose();
    //KoFilter::ConversionStatus read_if();
    //KoFilter::ConversionStatus read_else();
    //KoFilter::ConversionStatus read_forEach();
private:
    MsooXmlDiagramReaderContext *m_context;

    enum Type {
        InvalidType,
        DataModelType,
        LayoutDefType,
        StyleDefType,
        ColorsDefType
    } m_type;
};

}

#endif
