/*
** Header file for inclusion with kword_xml2latex.c
**
** Copyright (C) 2002, 2003 Robert JACOLIN
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
** Free Software Foundation, Inc., 59 Temple Place - Suite 330,
** Boston, MA  02111-1307, USA.
**
*/

#ifndef __KWORD_LATEX_CONFIG_H__
#define __KWORD_LATEX_CONFIG_H__

#include <qtextstream.h>
#include <qstringlist.h>

/***********************************************************************/
/* Class: Config                                                       */
/***********************************************************************/

/**
 * This class hold all parameters and configuration from a file or from
 * the filter configuration dialog box. it's a singleton, so you may use
 * the instance() method to get this instance.
 */
class Config
{
	/* Document tab */
	bool _useLatexStyle;
	bool _isEmbeded;
	QString _class;
	QString _quality;
	
	/* Pictures tab */
	bool _convertPictures;
	QString _picturesDir;

	/* Language tab */
	bool _useUnicode;
	bool _useLatin1;
	QString _encoding;
	QStringList _languagesList;
	QString _defaultLanguage;

	int _tabSize;	/* Size of the para indentation. */
	int _tabulation;	/* Total size  of the indentation. */

	public:

		static const char SPACE_CHAR;
		
		static Config* instance(void);
		
		Config(const Config&);

		/* 
		 * Destructor
		 */
		virtual ~Config();

		/**
		 * Accessors
		 */

		/**
		 * Return the value of a tabulation.
		 */
		bool isKwordStyleUsed() const { return (_useLatexStyle == false); }
		bool isEmbeded() const { return _isEmbeded; }
		QString getClass() const { return _class; }
		QString getQuality() const { return _quality; }

		bool convertPictures() const { return _convertPictures; }
		QString getPicturesDir() const { return _picturesDir; }
		
		bool mustUseUnicode() const { return _useUnicode; }
		bool mustUseLatin1() const { return _useLatin1; }
		QString getEncoding() const { return _encoding; }
		QStringList getLanguageList() const { return _languagesList; }
		QString getDefaultLanguage() const { return _defaultLanguage; }
		
		int getTabSize() const { return _tabSize; }
		int getIndentation() const { return _tabulation; }

		/**
		 * Modifiers
		 */

		/**
		 * Initialise the tab size.
		 * @param size New size. Must be superior or eguals to 0.
		 */
		void setTabSize(int size)
		{
			if(size >= 0)
				_tabSize = size;
		}
		
		void useLatexStyle() { _useLatexStyle = true;  }
		void useKwordStyle() { _useLatexStyle = false; }
		void setEmbeded(bool emb) { _isEmbeded = emb; }
		/** The class can be article, book, letter, report or slides. It's the type of the
		 * latex document. */
		void setClass(QString lclass) { _class = lclass; }
		void setQuality(QString quality) { _quality = quality; }

		void convertPictures(bool state) { _convertPictures = state; }
		void setPicturesDir(QString dir) { _picturesDir = dir; }

		void useUnicodeEnc() { _useUnicode    = true; _useLatin1 = false;  }
		void useLatin1Enc () { _useLatin1     = true; _useUnicode = false; }
		void setEncoding(QString enc) { _encoding = enc; }
		void addLanguage(QString l) { _languagesList.append(l); }
		void setDefaultLanguage(QString l) { _defaultLanguage = l; }
		
		void setIndentation(int indent) { _tabulation = indent; }
		
		/**
		 * Helpfull functions
		 */
		void indent();
		void desindent();

		void writeIndent(QTextStream& out);

	protected:
		/**
		 * Constructors
		 *
		 * Creates a new instance of Config.
		 * Initialise param. at default value.
		 */
		Config(); /* Ensure singleton */

		static Config* _instance; /* Singleton */

	private:

};

#endif /* __KWORD_LATEX_CONFIG_H__ */
