/* -*- C++ -*-

  $Id$

  This file is part of Kontour.
  Copyright (C) 1998-1999 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)
  Copyright (C) 2000-2001 Igor Janssen (rm@linux.ru.net)

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

#include "GPage.h"

#include <float.h>

#include <qdom.h>

#include <kdebug.h>

#include "kontour_global.h"
#include "GObject.h"
#include "GLayer.h"
#include "units.h"

/*#if defined(__FreeBSD__) || defined(__NetBSD__)
#include <math.h>
#else
#include <values.h>
#endif*/

GPage::GPage(GDocument *aGDoc)
{
  mGDoc = aGDoc;
//  connect(this, SIGNAL(wasModified(bool)),doc, SLOT(setModified(bool)));
//  connect(this, SIGNAL(changed(const KoRect&)),doc, SLOT(emitChanged(const KoRect&)));
//  connect(this, SIGNAL(changed()),doc, SLOT(emitChanged()));
//  connect(this, SIGNAL(handleChanged()),doc, SLOT(emitHandleChanged()));
  mPageLayout.format = PG_DIN_A4;
  mPageLayout.orientation = PG_PORTRAIT;
  mPageLayout.mmWidth = PG_A4_WIDTH;
  mPageLayout.mmHeight = PG_A4_HEIGHT;
  mPageLayout.mmLeft = 0;
  mPageLayout.mmRight = 0;
  mPageLayout.mmTop = 0;
  mPageLayout.mmBottom = 0;
  mPageLayout.unit = PG_MM;

  mBGColor = white;
  
  mCurLayerNum = 0;

  // in pt !!
  mPaperWidth = static_cast<int>(cvtMmToPt(mPageLayout.mmWidth));
  mPaperHeight = static_cast<int>(cvtMmToPt(mPageLayout.mmHeight));

  selection.clear();
  layers.setAutoDelete(true);
  layers.clear();

  active_layer = addLayer();
  active_layer->name("adfkkd");
  active_layer->setVisible(true);
  active_layer->setPrintable(true);
  active_layer->setEditable(true);

  selBoxIsValid = false;
//  autoUpdate = true;
//  emit changed ();
}

GPage::~GPage()
{
  layers.clear();
  selection.clear();
}

void GPage::name(QString aName)
{
  mName = aName;
}

void GPage::bgColor(QColor color)
{
  mBGColor = color;
}

void GPage::setPaperSize(int width, int height)
{
  mPaperWidth = width;
  mPaperHeight = height;
}

void GPage::pageLayout(const KoPageLayout &layout)
{
  mPageLayout = layout;
  switch(layout.unit)
  {
  case PG_MM:
    mPaperWidth = static_cast<int>(cvtMmToPt(mPageLayout.mmWidth));
    mPaperHeight = static_cast<int>(cvtMmToPt(mPageLayout.mmHeight));
    break;
  case PG_PT:
    mPaperWidth = static_cast<int>(mPageLayout.ptWidth);
    mPaperHeight = static_cast<int>(mPageLayout.ptHeight);
    break;
  case PG_INCH:
    mPaperWidth = static_cast<int>(cvtInchToPt(mPageLayout.inchWidth));
    mPaperHeight = static_cast<int>(cvtInchToPt(mPageLayout.inchHeight));
    break;
  }
//  setModified();
//  emit sizeChanged ();
}

QDomElement GPage::saveToXml(QDomDocument &document)
{
  // Many formats missing. Switching to KoPageFormat::formatString/formatFromString
  // would be the best thing to do, but this would require some conversion code... (DF)
/*  static const char* formats[] = {
    "a3", "a4", "a5", "us_letter", "us_legal", "screen", "custom"};
  static const char* orientations[] = {"portrait", "landscape"};

  QDomElement page = document.createElement("page");

  page.setAttribute("id", name());
  page.setAttribute ("bgcolor", mBGColor.name());  //TODO

  QDomElement layout = document.createElement("layout");
  layout.setAttribute ("format", formats[mPageLayout.format]);
  layout.setAttribute ("orientation", orientations[mPageLayout.orientation]);
  layout.setAttribute ("width", mPageLayout.mmWidth);
  layout.setAttribute ("height", mPageLayout.mmHeight);
  layout.setAttribute ("lmargin", mPageLayout.mmLeft);
  layout.setAttribute ("tmargin", mPageLayout.mmTop);
  layout.setAttribute ("rmargin", mPageLayout.mmRight);
  layout.setAttribute ("bmargin", mPageLayout.mmBottom);
  page.appendChild(layout);

  for (QPtrListIterator<GLayer> li(layers); li.current(); ++li)
  {
    GLayer *l=(*li);

    QDomElement layer;
    layer=document.createElement("layer");
    const QPtrList<GObject>& contents = l->objects ();
    for (QPtrListIterator<GObject> oi(contents);oi.current(); ++oi)
      layer.appendChild((*oi)->writeToXml (document));
    page.appendChild(layer);
  }
  setModified (false);*/
  QDomElement page = document.createElement("page");
  return page;
}

bool GPage::readFromXml(const QDomElement &page)
{
  return true;
}

void GPage::activeLayer(GLayer *aLayer)
{
  QPtrListIterator<GLayer> i(layers);
  for (; i.current(); ++i)
  {
    if ((*i) == aLayer)
    {
      active_layer = aLayer;
      unselectAllObjects();
      break;
    }
  }
}

GLayer* GPage::activeLayer()
{
  return active_layer;
}

GLayer *GPage::addLayer()
{
  GLayer* layer = new GLayer (this);
//  connect (layer, SIGNAL(propertyChanged ()), this, SLOT(layerChanged ()));
  layers.append(layer);
  unselectAllObjects();
  return layer;
}

void GPage::deleteLayer(GLayer *layer)
{
  if (layers.count() == 1)
    // we need at least one layer
    return;

  bool update = (active_layer == layer);

  int pos=layers.findRef(layer);
  if(pos!=-1) {
      // remove the layer from the array
      GLayer *l=layers.take(pos);
      // and delete the layer
      disconnect (l, SIGNAL(propertyChanged ()),
                  this, SLOT(layerChanged ()));
      delete l;

      if (update) {
          active_layer = layers.current();
          if(!active_layer)   // This one is needed for Qt 3.0 :)
              active_layer=layers.last();
          unselectAllObjects();
      }
  }
//  emit selectionChanged();
//  emit changed();
}

void GPage::raiseLayer(GLayer *layer)
{
  if (!layer)
    return;

  if (layer == layers.last())
    // layer is already on top
    return;

  int pos=layers.findRef(layer);
  if(pos!=-1)
  {
    GLayer *l=layers.take(pos);
    layers.insert(pos+1, l);
    unselectAllObjects();
  }
  emit changed ();
}

void GPage::lowerLayer(GLayer *layer)
{
  if (!layer)
    return;

  if (layer == layers.first())
    // layer is already at bottom
    return;

  int pos=layers.findRef(layer);
  if(pos!=-1)
  {
    GLayer *l=layers.take(pos);
    layers.insert(pos-1, l);
    unselectAllObjects();
  }
  emit changed ();
}

unsigned int GPage::objectCount() const
{
  unsigned int num = 0;
  for(QPtrListIterator<GLayer> i(layers); i.current(); ++i)
    num += (*i)->objectCount();
  return num;
}

void GPage::insertObject(GObject *obj)
{
//  obj->ref ();
  active_layer->insertObject(obj);
//  connect(obj, SIGNAL(changed()), this, SLOT(objectChanged ()));
//  connect (obj, SIGNAL(changed(const KoRect&)), this, SLOT(objectChanged(const KoRect&)));
//  setModified();
  if(autoUpdate)
    emit changed();
}

void GPage::deleteObject(GObject *obj)
{
  bool selected = false;

  GLayer *layer = obj->layer();
//  assert(layer);
  if(layer->isEditable())
  {
    selected = obj->isSelected();
    if(selected)
      selection.removeRef(obj);
    //setModified();
    disconnect(obj, SIGNAL(changed()), this, SLOT(objectChanged ()));
    disconnect(obj, SIGNAL(changed(const KoRect&)), this, SLOT(objectChanged(const KoRect&)));
    layer->deleteObject(obj);
    if(selected)
    {
      selBoxIsValid = false;
//      updateHandle();
      if(autoUpdate)
        emit selectionChanged();
    }
    if(autoUpdate)
      emit changed();
  }
}

unsigned int GPage::findIndexOfObject(GObject *obj)
{
//  assert(obj->layer() != 0L);
  return obj->layer()->findIndexOfObject(obj);
}

void GPage::insertObjectAtIndex(GObject *obj, unsigned int idx)
{
//  obj->ref();
  GLayer *layer = obj->layer();
  if(layer == 0L)
    layer = active_layer;
  layer->insertObjectAtIndex (obj, idx);
  connect (obj, SIGNAL(changed()), this, SLOT(objectChanged ()));
  connect (obj, SIGNAL(changed(const KoRect&)),
           this, SLOT(objectChanged (const KoRect&)));
//  setModified ();
  if (autoUpdate)
  {
    emit changed ();
    emit selectionChanged ();
  }
}

void GPage::moveObjectToIndex(GObject *obj, unsigned int idx)
{
  GLayer *layer = obj->layer();
  if (layer == 0L)
    layer = active_layer;
  layer->moveObjectToIndex (obj, idx);

  //setModified ();
  if (autoUpdate)
  {
    emit changed ();
    emit selectionChanged ();
  }
}
  
void GPage::selectObject(GObject *obj)
{
  if(selection.containsRef(obj)==0)
  {
    // object isn't yet in selection list
    obj->select (true);
    selection.append(obj);
    selBoxIsValid = false;
//    updateHandle ();
    if (autoUpdate)
    {
      emit changed ();
      emit selectionChanged ();
    }
  }
}

void GPage::unselectObject(GObject *obj)
{
  int i = selection.findRef(obj);
  if(i != -1)
  {
    // remove object from the selection list
    obj->select (false);
    selection.remove(i);
    selBoxIsValid = false;
//    updateHandle ();
    if (autoUpdate)
    {
      emit changed ();
      emit selectionChanged ();
    }
  }
}

void GPage::selectAllObjects()
{
  selection.clear();
  for(QPtrListIterator<GLayer> i(layers); i.current(); ++i)
  {
    if((*i)->isEditable ())
    {
      const QPtrList<GObject> &contents = (*i)->objects();
      QPtrListIterator<GObject> oi(contents);
      for(; oi.current(); ++oi)
      {
        GObject *obj = *oi;
        obj->select(true);
        selection.append(obj);
      }
    }
  }
  selBoxIsValid = false;
//  updateHandle();
//  if(autoUpdate)
//  {
//    emit changed();
//    emit selectionChanged();
//  }
}
  
void GPage::unselectAllObjects()
{
  if(selection.isEmpty())
    return;

  for(GObject *o = selection.first(); o != 0L; o = selection.next())
    o->select(false);
  selection.clear();
  selBoxIsValid = false;
//  if (autoUpdate)
//  {
//    emit changed();
//    emit selectionChanged();
//  }
}
  
void GPage::selectNextObject()
{
  GObject *newSel = 0L;
//TODO if not editable layer?
  if(selectionIsEmpty())
    newSel = const_cast<QPtrList<GObject>&>(active_layer->objects()).first();
  else
  {
    GObject *oldSel = selection.first();
    unsigned int idx = findIndexOfObject (oldSel);
    if (++idx >= active_layer->objects ().count())
      idx = 0;
    newSel = active_layer->objectAtIndex (idx);
  }
//  setAutoUpdate(false);
//  unselectAllObjects();

//  setAutoUpdate(true);
//  if(newSel)
//  {
//    handle().show(true);
//    selectObject(newSel);
//   }
}

void GPage::selectPrevObject()
{
}
  
void GPage::deleteSelectedObjects()
{
  if (! selectionIsEmpty ())
  {
    for(QPtrListIterator<GObject> i(selection); i.current(); ++i)
    {
      GObject* obj = *i;
      disconnect (obj, SIGNAL(changed()), this, SLOT(objectChanged ()));
      disconnect (obj, SIGNAL(changed(const KoRect&)),
                  this, SLOT(objectChanged (const KoRect&)));
      obj->layer ()->deleteObject (obj);
    }
    selection.clear ();
    //setModified ();
    selBoxIsValid = false;
    if (autoUpdate)
    {
      emit changed ();
      emit selectionChanged ();
    }
  }
}
  
KoRect GPage::boundingBoxForSelection()
{
  if (! selBoxIsValid)
  {
    if (! selectionIsEmpty ())
    {
      QPtrListIterator<GObject> i(selection);
      mSelBox = (*i)->boundingBox ();
      ++i;
      for (; i.current(); ++i)
        mSelBox = mSelBox.unite ((*i)->boundingBox ());
    }
    else
    {
      mSelBox = KoRect ();
    }
    selBoxIsValid = true;
  }
  return mSelBox;
}

KoRect GPage::boundingBoxForAllObjects()
{
  KoRect box;

  bool init = false;

  for (QPtrListIterator<GLayer> li(layers); li.current(); ++li)
  {
    if((*li)->isEditable())
    {
      const QPtrList<GObject> &contents = (*li)->objects ();
      QPtrListIterator<GObject> oi(contents);
      if (! init)
      {
        box = (*oi)->boundingBox ();
        ++oi;
        init = true;
      }
      for (; oi.current(); ++oi)
        box = box.unite ((*oi)->boundingBox ());
    }
  }
  return box;
}

void GPage::drawContents(QPainter &p, bool withBasePoints, bool outline, bool withEditMarks)
{
  for(QPtrListIterator<GLayer> i(layers); i.current(); ++i)
  {
    if((*i)->isVisible())
    {
      const QPtrList<GObject> &contents = (*i)->objects();
      QPtrListIterator<GObject> oi(contents);
      for(; oi.current(); ++oi)
        (*oi)->draw(p, withBasePoints && (*oi)->isSelected(), outline, withEditMarks);
    }
  }
}

void GPage::drawContentsInRegion(QPainter &p, const KoRect &r, bool withBasePoints, bool outline, bool withEditMarks)
{
  GLayer *i = layers.first();
  for(; i != 0; i = layers.next())
  {
    if(i->isVisible())
    {
      const QPtrList<GObject> &contents = i->objects();
      QPtrListIterator<GObject> oi(contents);
      for(; oi.current(); ++oi)
      {
        // draw the object only if its bounding box
        // intersects the active region
        //      const KoRect& bbox = (*oi)->boundingBox ();
        //      if (r.intersects (bbox))
        if((*oi)->intersects(r))
          (*oi)->draw(p, withBasePoints && (*oi)->isSelected(), outline, withEditMarks);
      }
    }
  }
}

void GPage::invalidateClipRegions ()
{
  for (QPtrListIterator<GLayer> li(layers); li.current(); ++li)
  {
    if ((*li)->isVisible ())
    {
      const QPtrList<GObject>& contents = (*li)->objects ();
      QPtrListIterator<GObject> oi(contents);
      for (; oi.current(); ++oi)
        (*oi)->invalidateClipRegion ();
    }
  }
}

/**
 * Looks for an object of type <tt>otype</tt> which endpoints are distant
 * not more than <tt>max_dist</tt> from the point <tt>x, y</tt>.
 * The method returns <tt>true</tt> if an object was found as well as
 * the object in <tt>obj</tt> and the index of the nearest point in
 * <tt>pidx</tt>.
 */
bool GPage::findNearestObject (const QString &otype, int x, int y,
                                   double max_dist, GObject*& obj,
                                   int& pidx, bool all)
{
  double d, distance = DBL_MAX;
  obj = 0L;
  KoPoint p (x, y);

  QPtrListIterator<GLayer> li(layers);
  for (li.toLast(); li.current(); --li)
  {
    if ((*li)->isEditable ())
    {
      const QPtrList<GObject>& contents = (*li)->objects ();
      QPtrListIterator<GObject> oi(contents);
      for ( ; oi.current(); ++oi)
      {
        if (otype == 0L || (*oi)->isA(otype.latin1()))
	{
          if ((*oi)->findNearestPoint (p, max_dist, d, pidx, all) && d < distance)
	  {
            obj = *oi;
            distance = d;
          }
        }
      }
    }
  }
  if (obj == 0L)
    pidx = -1;
  return obj != 0L;
}

GObject* GPage::findContainingObject (int x, int y)
{
  GObject* result = 0L;
  // We are looking for the most relevant object, that means the object
  // in front of all others. So, we have to start at the upper layer
  QPtrListIterator<GLayer> i(layers);
  for (i.toLast(); i.current(); --i)
  {
    if ((*i)->isEditable ())
    {
      result = (*i)->findContainingObject (x, y);
      if (result)
        break;
    }
  }
  return result;
}

bool GPage::findContainingObjects(int x, int y, QPtrList<GObject> &olist)
{
  KoPoint coord(x, y);
  for (QPtrListIterator<GLayer> li(layers); li.current(); ++li)
  {
    if ((*li)->isEditable ())
    {
      const QPtrList<GObject>& contents = (*li)->objects ();
      QPtrListIterator<GObject> oi(contents);
      for ( ; oi.current(); ++oi)
        if ((*oi)->contains (coord))
          olist.append (*oi);
    }
  }
  return olist.count () > 0;
}

bool GPage::findObjectsContainedIn(const KoRect &r, QPtrList<GObject> &olist)
{
  for(QPtrListIterator<GLayer> li(layers); li.current(); ++li)
  {
    if ((*li)->isEditable ())
    {
      const QPtrList<GObject> &contents = (*li)->objects();
      QPtrListIterator<GObject> oi(contents);
      for ( ; oi.current(); ++oi)
        if (r.contains ((*oi)->boundingBox ()))
          olist.append (*oi);
    }
  }
  return olist.count () > 0;
}

/*******************[OLD]*********************
bool GPage::readFromXml (const QDomElement &page)
{
  setAutoUpdate (false);

  setName(page.attribute("id"));
  mBGColor.setNamedColor(page.attribute("bgcolor", "#FFFFFF"));
  QDomElement layout=page.namedItem("layout").toElement();
  QString tmp=layout.attribute("format");
  if (tmp == "a3")
    mPageLayout.format = PG_DIN_A3;
  else
    if (tmp == "a4")
      mPageLayout.format = PG_DIN_A4;
    else
      if (tmp == "a5")
        mPageLayout.format = PG_DIN_A5;
      else
      if (tmp == "usletter")
        mPageLayout.format = PG_US_LETTER;
      else
        if (tmp == "uslegal")
          mPageLayout.format = PG_US_LEGAL;
        else
	  if (tmp == "custom")
            mPageLayout.format = PG_CUSTOM;
          else
            mPageLayout.format = PG_DIN_A4;

  tmp=layout.attribute("orientation");
  if (tmp == "portrait")
    mPageLayout.orientation = PG_PORTRAIT;
  else
    if (tmp == "landscape")
      mPageLayout.orientation = PG_LANDSCAPE;
    else
      mPageLayout.orientation = PG_PORTRAIT;

  mPageLayout.mmWidth=layout.attribute("width").toFloat();
  mPageLayout.mmHeight=layout.attribute("height").toFloat();
  mPageLayout.mmLeft=layout.attribute("lmargin").toFloat();
  mPageLayout.mmRight=layout.attribute("rmargin").toFloat();
  mPageLayout.mmBottom=layout.attribute("bmargin").toFloat();
  mPageLayout.mmTop=layout.attribute("tmargin").toFloat();

// update page layout
  setPageLayout (mPageLayout);

  QPtrList<GObject> dummy;
  bool result = parseBody (page, dummy, false);

  setModified (false);
  return result;
}

bool GPage::insertFromXml (const QDomDocument &document, QPtrList<GObject>& newObjs)
{
  if ( document.doctype().name() != "killustrator" )
    return false;
  QDomElement doc = document.documentElement();

  if ( doc.attribute( "mime" ) != KILLUSTRATOR_MIMETYPE )
    return false;
  return parseBody (doc, newObjs, true);
}

bool GPage::parseBody (const QDomElement &element, QPtrList<GObject>& newObjs, bool markNew)
{
  GObject* obj = 0L;
  QDict<GObject> refDict;

  QDomNode n = element.firstChild();
  while(!n.isNull())
  {
    QDomElement layerelem=n.toElement();
    if (layerelem.tagName() == "layer")
    {
      QString id=layerelem.attribute("id");
      if(!id.isEmpty())
      {
        active_layer = addLayer ();
        active_layer->setName (id);
        int flags = layerelem.attribute("flags").toInt();
        active_layer->setVisible (flags & LAYER_VISIBLE);
        active_layer->setPrintable (flags & LAYER_EDITABLE);
        active_layer->setEditable (flags & LAYER_PRINTABLE);
      }

      QDomNode cn=layerelem.firstChild();
      while(!cn.isNull())
      {
        QDomElement child=cn.toElement();
        obj=KIllustrator::objectFactory(child, document()->document());
        if(!obj)
//                {
//                   GObject *proto = GObject::lookupPrototype (child.tagName());
//                   if (proto != 0L)
//                   {
//                      obj = proto->create (this, child);
//                   }
//                   else
                      kdDebug(38000) << "invalid object type: " << child.tagName() << endl;
//                }
        if (child.tagName() == "group")
          ((GGroup*)obj)->setLayer (active_layer);
        if(obj->hasId())
          refDict.insert(obj->getId(), obj);
        insertObject(obj);
        cn=cn.nextSibling();
        newObjs.append(obj);
      }
    }
    n=n.nextSibling();
  }

// update object connections
  for (QPtrListIterator<GLayer> i(layers); i.current(); ++i)
  {
    const QPtrList<GObject>& contents = (*i)->objects ();
    for (QPtrListIterator<GObject> oi(contents); oi.current(); ++oi)
    {
    // this should be more general !!
      if ((*oi)->hasRefId () && (*oi)->isA ("GText"))
      {
        GObject *o = refDict[(*oi)->getRefId ()];
        if(o)
	{
          GText *tobj = (GText *) *oi;
          tobj->setPathObject (o);
        }
      }
    }
  }
  setAutoUpdate (true);
  return true;
}
*/

#include "GPage.moc"
