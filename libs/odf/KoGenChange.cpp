/* This file is part of the KDE project
   Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QDateTime>

#include "KoGenChange.h"
#include <KoXmlWriter.h>

#include <kdebug.h>

// Returns -1, 0 (equal) or 1
static int compareMap(const QMap<QString, QString>& map1, const QMap<QString, QString>& map2)
{
    QMap<QString, QString>::const_iterator it = map1.begin();
    QMap<QString, QString>::const_iterator oit = map2.begin();
    for (; it != map1.end(); ++it, ++oit) {   // both maps have been checked for size already
        if (it.key() != oit.key())
            return it.key() < oit.key() ? -1 : + 1;
        if (it.value() != oit.value())
            return it.value() < oit.value() ? -1 : + 1;
    }
    return 0; // equal
}


KoGenChange::KoGenChange()
{
}

KoGenChange::~KoGenChange()
{
}

void KoGenChange::writeChangeMetaData(KoXmlWriter* writer) const
{
  QMap<QString, QString>::const_iterator it = m_changeMetaData.begin();
  const QMap<QString, QString>::const_iterator end = m_changeMetaData.end();
  for (; it != end; ++it) {
    
//FIXME: if the propName is passed directly as it.key().toUtf8(), the opening tag is correct but the closing tag becomes undefined
//FIXME: example: <dc-creator>.......</`ok>

    if (it.key() == "dc-creator") {
      writer->startElement("dc-creator");
      writer->addTextNode(it.value());
      writer->endElement();
    }
    if (it.key() == "dc-date") {
      writer->startElement("dc-date");
      writer->addTextNode(it.value());
      writer->endElement();
    }
  }
}

void KoGenChange::writeChange(KoXmlWriter* writer, const QString& name) const
{
  writer->startElement("text:changed-region");
  writer->addAttribute("text:id", name);

  const char* elementName;
  switch (m_type) {
    case KoGenChange::deleteChange:
      elementName = "text:deletion";
      break;
    case KoGenChange::formatChange:
      elementName = "text:format-change";
      break;
    case KoGenChange::insertChange:
      elementName = "text:insertion";
      break;
    default:
      elementName = "text:format-change"; //should not happen, format-change is probably the most harmless of the three.
  }
  writer->startElement(elementName);
  if (!m_changeMetaData.isEmpty()) {
    writer->startElement("office:change-info");
    writeChangeMetaData(writer);
    if (m_literalData.contains("changeMetaData"))
      writer->addCompleteElement(m_literalData.value("changeMetaData").toUtf8());
    writer->endElement(); // office:change-info
  }
  if ((m_type == KoGenChange::deleteChange) && m_literalData.contains("deletedData"))
    writer->addCompleteElement(m_literalData.value("deletedData").toUtf8());
  writer->endElement(); // text:insertion/format/deletion
  writer->endElement(); // text:change
}

bool KoGenChange::operator<(const KoGenChange &other) const
{
//    if (m_changeMetaData.value("dc-date") != other.m_changeMetaData.value("dc-date")) return QDateTime::fromString(m_changeMetaData.value("dc-date"), Qt::ISODate) < QDateTime::fromString(other.m_changeMetaData.value("dc-date"), Qt::ISODate);


    return true;
}

bool KoGenChange::operator==(const KoGenChange &other) const
{
    if (m_type != other.m_type) return false;
    if (m_changeMetaData.count() != other.m_changeMetaData.count()) return false;
    if (m_literalData.count() != other.m_literalData.count()) return false;
    int comp = compareMap(m_changeMetaData, other.m_changeMetaData);
    if (comp != 0) return false;
    comp = compareMap(m_literalData, other.m_literalData);
    if (comp != 0) return false;
    return true;
}
