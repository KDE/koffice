/* This file is part of the KDE project
   Copyright (C)  2006 Peter Simonsson <peter.simonsson@gmail.com>

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

#include "KivioAbstractPage.h"

#include <KoShape.h>

#include "KivioDocument.h"

KivioAbstractPage::KivioAbstractPage(KivioDocument* doc, const QString& title)
{
  m_document = doc;
  setTitle(title);
}

KivioAbstractPage::~KivioAbstractPage()
{
  qDeleteAll(m_shapeList);
  m_shapeList.clear();
}

void KivioAbstractPage::setTitle(const QString& newTitle)
{
  if(newTitle.isEmpty()) return;

  m_title = newTitle;
}

QString KivioAbstractPage::title() const
{
  return m_title;
}

void KivioAbstractPage::addShape(KoShape* shape)
{
  if(shape == 0) {
    return;
  }

  m_shapeList.append(shape);
  m_document->addShapeToViews(this, shape);
}

void KivioAbstractPage::removeShape(KoShape* shape)
{
  if(shape == 0) {
    return;
  }

  m_document->removeShapeFromViews(this, shape);
  m_shapeList.removeAll(shape);
}

QList<KoShape*> KivioAbstractPage::shapeList() const
{
  return m_shapeList;
}
