/* -*- C++ -*-

  $Id$

  This file is part of Kontour.
  Copyright (C) 1998-99 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)
  Copyright (C) 2001 Igor Janssen (rm@linux.ru.net)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "GDocument.h"

#include <qdom.h>

#include <klocale.h>

#include "kontour_global.h"
#include "kontour_doc.h"
#include "GPage.h"

const double MIN_GRID_DIST = 6.0;

GDocument::GDocument(KontourDocument *aDoc)
{
  mDoc = aDoc;

  mXRes = 72.0;
  mYRes = 72.0;

  mZoomFactor = 1.0;

  mXGridZ = 20.0;
  mYGridZ = 20.0;
  mXGrid = 20.0;
  mYGrid = 20.0;
  mShowGrid = false;
  mSnapToGrid = false;
  mGridColor = lightGray;

  mShowHelplines = true;
  mSnapToHelplines = false;

  pages.setAutoDelete(true);
  pages.clear();

  mActivePage = addPage();
  mActivePage->name(i18n("Page %1").arg(1));
  mCurPageNum = 2;
  
  changeCanvas();
}

GDocument::~GDocument()
{
  pages.clear();
}

void GDocument::zoomFactor(double factor)
{
  if(factor == mZoomFactor)
    return;
  
  /* Change grid distance. */
  mXGridZ = mXGrid * factor;
  while(mXGridZ < MIN_GRID_DIST)
    mXGridZ *= 2.0;
  mYGridZ = mYGrid * factor;
  while(mYGridZ < MIN_GRID_DIST)
    mYGridZ *= 2.0;
  
  double scale = factor / mZoomFactor;
  mZoomFactor = factor;
  changeCanvas();
  emit zoomFactorChanged(scale);
}

void GDocument::showGrid(bool flag)
{
  if(mShowGrid != flag)
  {
    mShowGrid = flag;
    emit gridChanged();   //TODO emit?
  }
  setModified();
}

void GDocument::snapToGrid(bool flag)
{
  mSnapToGrid = flag;
  setModified();
}

void GDocument::gridColor(QColor color)
{
  mGridColor = color;   //TODO emit?
  setModified();
}

double GDocument::xGrid() const
{
  return 1.0;
}

double GDocument::yGrid() const
{
  return 1.0;
}

void GDocument::setGridDistance(double hdist, double vdist)
{
  // TODO ZOOM!!!
  mXGridZ = hdist;
  mYGridZ = vdist;
}

void GDocument::showHelplines(bool flag)
{
  mShowHelplines = flag;
  setModified();
}
  
void GDocument::snapToHelplines(bool flag)
{
  mSnapToHelplines = flag;
  setModified();
}

void GDocument::horizHelplines(const QValueList<double> &lines)
{
  mHorizHelplines = lines;
}

void GDocument::vertHelplines(const QValueList<double> &lines)
{
  mVertHelplines = lines;
}

int GDocument::indexOfHorizHelpline(double pos)
{
  int ret = 0;
  for(QValueList<double>::Iterator i = mHorizHelplines.begin(); i != mHorizHelplines.end(); ++i, ++ret)
    if(pos - Kontour::nearDistance < *i && pos + Kontour::nearDistance > *i)
      return ret;
  return -1;
}

int GDocument::indexOfVertHelpline(double pos)
{
  int ret = 0;
  for(QValueList<double>::Iterator i = mVertHelplines.begin(); i != mVertHelplines.end(); ++i, ++ret)
    if(pos - Kontour::nearDistance < *i && pos + Kontour::nearDistance > *i)
      return ret;
  return -1;
}

void GDocument::updateHorizHelpline(int idx, double pos)
{
  mHorizHelplines[idx] = pos;
}

void GDocument::updateVertHelpline(int idx, double pos)
{
  mVertHelplines[idx] = pos;
}
  
void GDocument::addHorizHelpline(double pos)
{
  mHorizHelplines.append(pos);
}

void GDocument::addVertHelpline(double pos)
{
  mVertHelplines.append(pos);
}

QDomDocument GDocument::saveToXml()
{
  QDomDocument document("kontour");
  document.appendChild(document.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
/*  QDomElement kontour = document.createElement("kontour");
  kontour.setAttribute("editor", "kontour 2.0");
  kontour.setAttribute("mime", );
  kontour.setAttribute("version", "1");
  document.appendChild(kontour);

  QDomElement head = document.createElement("head");
  head.setAttribute("currentpagenum", curPageNum);
  kontour.appendChild(head);

  QDomElement grid = document.createElement("grid");
  grid.setAttribute("dx", gridx);
  grid.setAttribute("dy", gridy);
  grid.setAttribute ("align", gridSnapIsOn ? 1 : 0);
  grid.setAttribute ("show", gridIsOn ? 1 : 0);
  grid.setAttribute ("color", mGridColor.name());
  head.appendChild(grid);

  QDomElement helplines=document.createElement("helplines");
  helplines.setAttribute ("align", helplinesSnapIsOn ? 1 : 0);
  helplines.setAttribute ("show", helplinesAreOn ? 1 : 0);
  QValueList<double>::Iterator hi;
  for(hi = hHelplines.begin(); hi != hHelplines.end(); ++hi)
  {
    QDomElement hl=document.createElement("hl");
    hl.setAttribute ("pos", (*hi));
    helplines.appendChild(hl);
  }
  for(hi = vHelplines.begin(); hi != vHelplines.end(); ++hi)
  {
    QDomElement vl=document.createElement("vl");
    vl.setAttribute ("pos", (*hi));
    helplines.appendChild(vl);
  }
  grid.appendChild(helplines);

  for (QPtrListIterator<GPage> pi(pages); pi.current(); ++pi)
  {
    GPage *p = (*pi);
    QDomElement page;
    page = p->saveToXml(document);
    killustrator.appendChild(page);
  }*/
  setModified(false);  //TODO need?
  return document;
}

bool GDocument::readFromXml(const QDomDocument &document)
{
  return true;
  /*if ( document.doctype().name() != "kontour" )
    return false;
  QDomElement killustrator = document.documentElement();
  if ( killustrator.attribute( "mime" ) != KILLUSTRATOR_MIMETYPE )
    return false;
  if( killustrator.attribute("version")=="3")
  {
    QDomElement head=killustrator.namedItem("head").toElement();
    setAutoUpdate (false);
    curPageNum = head.attribute("currentpagenum").toInt();

    QDomElement grid=head.namedItem("grid").toElement();
    gridx=grid.attribute("dx").todouble();
    gridy=grid.attribute("dy").todouble();
    gridSnapIsOn = (grid.attribute("align").toInt() == 1);
    gridIsOn = (grid.attribute("show").toInt() == 1);
    mGridColor.setNamedColor(grid.attribute("color"));

    QDomElement helplines=grid.namedItem("helplines").toElement();
    helplinesSnapIsOn = (helplines.attribute("align").toInt() == 1);
    helplinesAreOn = (helplines.attribute("show").toInt() == 1);

    QDomElement l=helplines.firstChild().toElement();
    for( ; !l.isNull(); l=helplines.nextSibling().toElement())
    {
      if(l.tagName()=="hl")
        hHelplines.append(l.attribute("pos").todouble());
      else
        if(l.tagName()=="vl")
          vHelplines.append(l.attribute("pos").todouble());
    }

    pages.clear ();
    mActivePage = 0;
    QDomNode n = killustrator.firstChild();
    while(!n.isNull())
    {
      QDomElement pe=n.toElement();
      kdDebug(38000) << "Tag=" << pe.tagName() << endl;
      if (pe.tagName() == "page")
      {
        GPage *page = addPage();
        if ( !mActivePage )
            mActivePage = page;
        page->readFromXml(pe);
      }
      n=n.nextSibling();
    }
    if ( !mActivePage )
        kdWarning(38000) << "No page found !" << endl;

    setModified (false);
    emit gridChanged ();
    return true;
  }
  return false;*/
}

void GDocument::activePage(GPage *page)
{
  // TODO rewrite?
  QPtrListIterator<GPage> i(pages);
  for(; i.current(); ++i)
  {
    if((*i) == page)
    {
      mActivePage = page;
      emit pageChanged();
      break;
    }
  }
}

void GDocument::activePage(int i)
{
  mActivePage = pages.at(i);
  emit pageChanged();
}

GPage *GDocument::addPage()
{
  GPage *page = new GPage(this);
  pages.append(page);
  page->name(i18n("Page %1").arg(mCurPageNum));
  mCurPageNum++;
  emit updateLayerView();
  return page;
}

void GDocument::deletePage(GPage *page)
{
  // TODO test and rewrite (active page....)
  if(pages.count() == 1)
    return;

  int pos = pages.findRef(page);
  if(pos != -1)
  {
    /* remove the page from the array */
    GPage *p = pages.take(pos);
    /* and delete the page */
    delete p;
    emit updateLayerView();
  }
}

GPage *GDocument::pageForIndex(int i)
{
  return pages.at(i);
}

void GDocument::movePage(int from, int to, bool before)
{
  // TODO test and rewrite (active page....)
  if(!before)
    ++to;

  if(to > static_cast<int>(pages.count()))
  {
  }
  else
  {
    GPage *p = pages.take(from);
    if(from < to)
      pages.insert(to-1, p);
    else
      pages.insert(to, p);
  }
}

GPage *GDocument::findPage(QString name)
{
  for(QPtrListIterator<GPage> it(pages); it.current(); ++it)
    if(((GPage *)it)->name() == name)
      return (GPage *)it;
  return 0L;
}

void GDocument::setModified(bool flag)
{
  mDoc->setModified(flag);
}

void GDocument::changeCanvas()
{
  mXCanvas = static_cast<int>(activePage()->paperWidth() * mZoomFactor * mXRes / 72.0);
  mYCanvas = static_cast<int>(activePage()->paperHeight() * mZoomFactor * mYRes / 72.0);
}

#include "GDocument.moc"
