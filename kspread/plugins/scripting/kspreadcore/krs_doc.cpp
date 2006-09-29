/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2006 Isaac Clerencia <isaac@warp.es>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "krs_doc.h"
#include "krs_sheet.h"

#include <kspread_map.h>
#include <kspread_sheet.h>

namespace Kross { namespace KSpreadCore {

Doc::Doc(KSpread::Doc* doc)
	: Kross::Api::Class<Doc>("KSpreadDocument"), m_doc(doc)
{
	this->addFunction0< Sheet >("currentSheet", this, &Doc::currentSheet);
	this->addFunction1< Sheet, Kross::Api::Variant >("sheetByName", this, &Doc::sheetByName);
	this->addFunction0< Kross::Api::Variant >("sheetNames", this, &Doc::sheetNames);

	this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >("addSheet", this, &Doc::addSheet);
	this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >("removeSheet", this, &Doc::removeSheet);

	this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >("loadNativeXML", this, &Doc::loadNativeXML);
	this->addFunction0< Kross::Api::Variant >("saveNativeXML", this, &Doc::saveNativeXML);

	this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >("openUrl", this, &Doc::openUrl);
	this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >("saveUrl", this, &Doc::saveUrl);
	this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >("import", this, &Doc::import);
	this->addFunction1< Kross::Api::Variant, Kross::Api::Variant >("exp0rt", this, &Doc::exp0rt);

}

Doc::~Doc() {

}

const QString Doc::getClassName() const {
	return "Kross::KSpreadCore::Doc";
}

Sheet* Doc::currentSheet()
{
	return new Sheet(m_doc->displaySheet(), m_doc);
}

Sheet* Doc::sheetByName(const QString& name)
{
	QPtrListIterator<KSpread::Sheet> it (m_doc->map()->sheetList());
	for( ; it.current(); ++it )
		if(it.current()->sheetName() == name)
			return new Sheet(it.current(), m_doc);
	return 0;
}

QStringList Doc::sheetNames()
{
	QStringList names;
	QPtrListIterator<KSpread::Sheet> it (m_doc->map()->sheetList());
	for( ; it.current(); ++it )
		names.append( it.current()->sheetName() );
	return names;
}

bool Doc::addSheet(const QString& sheetname)
{
	KSpread::Sheet* sheet = m_doc->map()->createSheet();
	if(sheet) {
		if(! sheet->setSheetName(sheetname)) {
			delete sheet;
			return false;
		}
		m_doc->map()->addSheet(sheet);
		return true;
	}
	return false;
}

bool Doc::removeSheet(const QString& sheetname)
{
	KSpread::Sheet* sheet = m_doc->map()->findSheet(sheetname);
	if(sheet) {
		m_doc->map()->takeSheet(sheet);
		return true;
	}
	return false;
}

bool Doc::loadNativeXML(const QString& xml) {
	QDomDocument doc;
	if(! doc.setContent(xml, true))
		return false;
	return m_doc->loadXML(0, doc);
}

QString Doc::saveNativeXML() {
	return m_doc->saveXML().toString(2);
}

bool Doc::openUrl(const QString& url)
{
	return m_doc->openURL(url);
}

bool Doc::saveUrl(const QString& url)
{
	return m_doc->saveAs(url);
}

bool Doc::import(const QString& url)
{
	return m_doc->import(url);
}

bool Doc::exp0rt(const QString& url)
{
	return m_doc->exp0rt(url);
}

}}

