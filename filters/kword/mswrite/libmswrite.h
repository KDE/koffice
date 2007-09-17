
/* This file is part of the LibMSWrite project
   Copyright (c) 2001-2003, 2007 Clarence Dang <clarencedang@users.sf.net>
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   LibMSWrite Project Website:
   http://sourceforge.net/projects/libmswrite/
*/

#ifndef __LIBMSWRITE_H__
#define __LIBMSWRITE_H__

#include "structures.h"
#include "structures_private.h"

namespace MSWrite
{
#if 0
	class DocumentInfo
	{
	private:
		Word m_numObjects;
		bool m_hasHeader;
		bool m_hasFooter;
		bool m_hasHeaderOnFirstPage;
		bool m_hasFooterOnFirstPage;

	public:
		DocumentInfo ()
		{
			m_numObjects = 0;
			m_hasHeader = m_hasFooter = false;
			m_hasHeaderOnFirstPage = m_hasFooterOnFirstPage = false;
		}

		Word getNumObjects (void) const	{	return m_numObjects;	}
		void setNumObjects (const Word numObjects)	{	m_numObjects = numObjects;	}

		bool getHasHeader (void) const	{	return m_hasHeader;	}
		void setHasHeader (const bool hasHeader)	{	m_hasHeader = hasHeader;	}

		bool getHasFooter (void) const	{	return m_hasFooter;	}
		void setHasFooter (const bool hasFooter)	{	m_hasFooter = hasFooter;	}

		bool getHasHeaderOnFirstPage (void) const	{	return m_hasHeaderOnFirstPage;	}
		void setHasHeaderOnFirstPage (const bool hasHeaderOnFirstPage)	{	m_hasHeader = hasHeaderOnFirstPage;	}

		bool getHasFooterOnFirstPage (void) const	{	return m_hasFooterOnFirstPage;	}
		void setHasFooterOnFirstPage (const bool hasFooterOnFirstPage)	{	m_hasFooterOnFirstPage = hasFooterOnFirstPage;	}
	};
#endif


	// [PRIVATE] don't use
	class FormatInfo : public NeedsDevice, public NeedsHeader
	{
	private:
		List <FormatInfoPage> m_formatInfoPageList;
		List <FormatInfoPage>::Iterator m_formatInfoPageIterator;
		DWord m_nextChar;

		enum FormatInfoPageTypes m_type;

		friend class InternalParser;
		friend class InternalGenerator;
			Word m_leftMargin, m_rightMargin;	// for ParaType only
			FontTable *m_fontTable;	// for CharType only
			void setMargins (const Word leftMargin, const Word rightMargin)
			{
				m_leftMargin = leftMargin, m_rightMargin = rightMargin;
			}
			void setFontTable (FontTable *fontTable)	{	m_fontTable = fontTable;	}

		FormatInfo &operator= (const FormatInfo &rhs);	// TODO
	public:
		FormatInfo ();
		virtual ~FormatInfo ();

		void setType (const enum FormatInfoPageTypes t)	{	m_type = t;	}

		bool readFromDevice (void);
		bool writeToDevice (const void *defaultProperty);

		void *begin (void);
		void *next (void);
		bool end (void) /*const*/;

		bool add (const void *property, const bool force = false);
	};


	class Generator;
	class Parser
	{
	protected:
		Generator *m_generator;

	public:
		Parser (Generator *generator = NULL)
		{
			setGenerator (generator);
		}
		
		virtual ~Parser ()
		{
		}

		void setGenerator (Generator *generator)
		{
			m_generator = generator;
		}

		// Note: You aren't allowed to call parse() more than once.
		//       Create another Parser instance if you need to reparse.
		//
		//       This permits subclasses implementations of parse() to defer
		//       uninitialization to the destructor, if desired.
		virtual bool parse (void) = 0;
	};


	class Generator
	{
	public:
		Generator ();
		virtual ~Generator ();

		/**
		 *
		 * Document block (you must implement these signals)
		 *
		 */
		virtual bool writeDocumentBegin (const Word format, const PageLayout *pageLayout) = 0;
		virtual bool writeDocumentEnd (const Word format, const PageLayout *pageLayout) = 0;

		/**
		 *
		 * Various sections in the document
		 *
		 */
		virtual bool writeFooterBegin (void)	{	return true;	}
		virtual bool writeFooterEnd (void)	{	return true;	}

		virtual bool writeHeaderBegin (void)	{	return true;	}
		virtual bool writeHeaderEnd (void)	{	return true;	}

		virtual bool writeBodyBegin (void) = 0;
		virtual bool writeBodyEnd (void)	= 0;

		/**
		 *
		 * Paragraph block
		 *
		 */
		virtual bool writeParaInfoBegin (const FormatParaProperty * /*paraProperty*/,
													const OLE * /*ole*/ = NULL,
													const Image * /*image*/ = NULL)	{	return true;	}
		virtual bool writeParaInfoEnd (const FormatParaProperty * /*paraProperty*/,
													const OLE * /*ole*/ = NULL,
													const Image * /*image*/ = NULL)	{	return true;	}

		/**
		 *
		 * Character block
		 *
		 */
		virtual bool writeCharInfoBegin (const FormatCharProperty * /*charProperty*/)	{	return true;	}
		virtual bool writeCharInfoEnd (const FormatCharProperty * /*charProperty*/,
													const bool /*endOfParagraph*/ = false)	{	return true;	}


		/**
		 *
		 * Binary output
		 *
		 */
		virtual bool writeBinary (const Byte * /*buffer*/, const DWord /*length*/)	{	return true;	}


		/*
		 *
		 * Text output
		 *
		 */

		/**
		 *
		 * processText
		 * this calls writeText, newLineWrite etc...
		 */
		bool processText (const Byte *string, bool willReachEndOfParagraph);
		virtual bool writeText (const Byte *string) = 0;


		/**
		 *
		 * Miscellaneous special signals called by processText
		 *
		 */
		virtual bool writePageNew (const int /*pageNumberClaimed*/ = 0)
		{
			return true;
		}
		virtual bool writePageBreak (void)
		{
			return writeText ((const Byte *) "\xc");	// i.e. (char)12
		}
		virtual bool writePageNumber (void)
		{
			return writeText ((const Byte *) "\x1");	// i.e. (char)1
		}
		virtual bool writeCarriageReturn (void)
		{
			return writeText ((const Byte *) "\xd");	// i.e. (char)13
		}
		virtual bool writeNewLine (const bool /*endOfParagraph*/ = true)
		{
			return writeText ((const Byte *) "\n");	// i.e. (char)10
		}
		virtual bool writeOptionalHyphen (void)
		{
			return writeText ((const Byte *) "-");
		}

		/**
		 *
		 * implement this signal to show the progress of the conversion
		 * (0 <= value <= 100)
		 *
		 */
		virtual void sigProgress (const int /*value*/)	{}
	};


	class InternalParser : public Parser, public NeedsDevice
	{
	private:
		Header *m_header;
		SectionTable *m_sectionTable;
		PageLayout *m_pageLayout;
		PageTable *m_pageTable;
		FontTable *m_fontTable;
		FormatInfo *m_paragraphInfo, *m_characterInfo;

		Image *m_image;
		OLE *m_ole;

	public:
		InternalParser ();
		virtual ~InternalParser ();

		bool parse (void);
	};


	class InternalGenerator : public Generator, public NeedsDevice
	{
	private:
		Header *m_header;
		SectionTable *m_sectionTable;
		PageLayout *m_pageLayout;
		PageTable *m_pageTable;
		FontTable *m_fontTable;
		FormatInfo *m_paragraphInfo, *m_characterInfo;

		Image *m_image;
		OLE *m_ole;
		
		bool seekNextPage (void);
		
	public:
		InternalGenerator ();
		virtual ~InternalGenerator ();

		bool writeDocumentBegin (const Word format, const PageLayout *pageLayout);
		bool writeDocumentEnd (const Word format, const PageLayout *pageLayout);

		bool writeFooterBegin (void)
		{
			// don't care
			return true;
		}
		bool writeFooterEnd(void)
		{
			// don't care
			return true;
		}

		bool writeHeaderBegin (void)
		{
			// don't care
			return true;
		}
		bool writeHeaderEnd (void)
		{
			// don't care
			return true;
		}

		bool writeBodyBegin (void)
		{
			// don't care
			return true;
		}

		bool writeBodyEnd (void)
		{
			// don't care
			return true;
		}


		/**
		 *
		 * Paragraph block
		 *
		 */
		bool writeParaInfoBegin (const FormatParaProperty *paraProperty,
											const OLE *oleHeader = NULL,
											const Image *imageHeader = NULL);
		bool writeParaInfoEnd (const FormatParaProperty *paraProperty,
										const OLE *oleHeader = NULL,
										const Image *imageHeader = NULL);

		bool writeCharInfoBegin (const FormatCharProperty *charProperty);
		bool writeCharInfoEnd (const FormatCharProperty *charProperty,
										const bool endOfParagraph = false);


		bool writeBinary (const Byte *buffer, const DWord length);


		bool writeText (const Byte *string);


		bool writePageNew (const int pageNumberClaimed = 0);

		bool writePageBreak (void)
		{
			return writeText ((const Byte *) "\xC");	// i.e. (char)12
		}
		bool writePageNumber (void)
		{
			return writeText ((const Byte *) "\x1");	// i.e. (char)1
		}
		bool writeCarriageReturn (void)
		{
			// it's DOS/Windows(r) which has \r\n newlines
			return writeText ((const Byte *) "\xD");	// i.e. (char)13
		}
		bool writeNewLine (const bool)
		{
			return writeText ((const Byte *) "\n");	// i.e. (char)10
		}
		bool writeOptionalHyphen (void)
		{
			return writeText ((const Byte *) "\x1F");	// i.e. (char)31
		}

		void sigProgress (const int)
		{
			// don't care
			return;
		}

	};
}	// namespace MSWrite	{

#endif	// __LIBMSWRITE_H__

// end of libmswrite.h
