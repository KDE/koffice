
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

#ifndef __STRUCTURES_PRIVATE_H__
#define __STRUCTURES_PRIVATE_H__

#include "structures_generated.h"
#include "list.h"

namespace MSWrite
{
	class Header : public HeaderGenerated
	{
	private:
		DWord m_numCharBytes;
		Word m_pageCharInfo;

		bool readFromDevice (void);	friend class InternalParser;
		bool writeToDevice (void);	friend class InternalGenerator;

	public:
		Header ();
		virtual ~Header ();

		Header &operator= (const Header &rhs);

		Word getFormat (void) const	{	return getMagic ();	}
		void setFormat (const Word f)	{	setMagic (f);	}

		DWord getNumCharBytes (void) const	{	return m_numCharBytes;	}
		void setNumCharBytes (const DWord w)	{	m_numCharBytes = w;	}

		Word getNumPageCharInfo (void) const	{	return m_pageParaInfo - m_pageCharInfo;	}
		Word getNumPageParaInfo (void) const	{	return m_pageFootnoteTable - m_pageParaInfo;	}
		Word getNumPageFootnoteTable (void) const	{	return m_pageSectionProperty - m_pageFootnoteTable;	}
		Word getNumPageSectionProperty (void) const	{	return m_pageSectionTable - m_pageSectionProperty;	}
		Word getNumPageSectionTable (void) const	{	return m_pagePageTable - m_pageSectionTable;	}
		Word getNumPagePageTable (void) const	{	return m_pageFontTable - m_pagePageTable;	}
		Word getNumPageFontTable (void) const	{	return m_numPages - m_pageFontTable;	}

		Word getPageCharInfo (void) const	{	return m_pageCharInfo;	}
	};

	class NeedsHeader
	{
	protected:
		Header *m_header;

	public:
		NeedsHeader (Header *header = NULL)
		{
			setHeader (header);
		}

		virtual ~NeedsHeader ()
		{
		}

		NeedsHeader &operator= (const NeedsHeader &rhs)
		{
			if (this == &rhs)
				return *this;

			m_header = rhs.m_header;

			return *this;
		}

		void setHeader (Header *header)	{	m_header = header;	}
	};


	class SectionDescriptor : public SectionDescriptorGenerated
	{
	public:
		SectionDescriptor ();
		virtual ~SectionDescriptor ();

		SectionDescriptor &operator= (const SectionDescriptor &rhs);
	};


	class SectionTable : public SectionTableGenerated, public NeedsHeader
	{
	public:
		SectionTable ();
		virtual ~SectionTable ();

		SectionTable &operator= (const SectionTable &rhs);

	private:
		bool readFromDevice (void);	friend class InternalParser;
		bool writeToDevice (const bool needed);	friend class InternalGenerator;
	};


	class Font;	// from structures.h
	class FontTable : public FontTableGenerated, public NeedsHeader
	{
	private:
		List <Font> m_fontList;

		bool readFromDevice (void);	friend class InternalParser;
		bool writeToDevice (void);	friend class InternalGenerator;

	public:
		FontTable ();
		virtual ~FontTable ();

		FontTable &operator= (const FontTable &rhs);

		Font *getFont (const DWord fontCode) const;
		DWord findFont (const Font *want) const;
		DWord addFont (const Font *input);
	};


	class PagePointer : public PagePointerGenerated
	{
	private:
		friend class PageTable;
			bool readFromDevice (void);
			bool writeToDevice (void);

	public:
		PagePointer ();
		virtual ~PagePointer ();

		PagePointer &operator= (const PagePointer &rhs);
	};


	class PageTable : public PageTableGenerated, public NeedsHeader
	{
	private:
		List <PagePointer> m_pagePointerList;
		Word m_pageNumberStart;	// for consistency checking

		friend class InternalParser;
		friend class InternalGenerator;
			bool readFromDevice (void);
			bool writeToDevice (void);

			// call before readFromDevice ()
			void setPageNumberStart (const Word s)	{	m_pageNumberStart = s;	}

		List <PagePointer>::Iterator m_pageTableIterator;

	public:
		PageTable ();
		virtual ~PageTable ();

		PageTable &operator= (const PageTable &rhs);

		PagePointer *begin (void)
		{
			m_pageTableIterator = m_pagePointerList.begin ();
			return &(*m_pageTableIterator);
		}

		PagePointer *next (void)
		{
			m_pageTableIterator = m_pageTableIterator.next ();
			return &(*m_pageTableIterator);
		}

		bool end (void) /*const*/
		{
			return m_pageTableIterator == m_pagePointerList.end ();
		}

		bool add (const PagePointer *newone)
		{
			return m_pagePointerList.addToBack (*newone);
		}
	};


	class FormatPointer : public FormatPointerGenerated
	{
	private:
		DWord m_afterEndCharByte;

		friend class FormatInfoPage;
			bool readFromDevice (void);
			bool writeToDevice (void);

			//
			// for exporting
			//
			const void *m_formatProperty;

			const void *getFormatProperty (void) const	{	return m_formatProperty;	}
			void setFormatProperty (const void *val)	{	m_formatProperty = val;	}

	public:
		FormatPointer ();
		virtual ~FormatPointer ();

		FormatPointer &operator= (const FormatPointer &rhs);

		DWord getAfterEndCharByte (void) const	{	return m_afterEndCharByte;	}
		void setAfterEndCharByte (const DWord val)	{	m_afterEndCharByte = val;	}
	};


	enum FormatInfoPageTypes
	{
		CharType,
		ParaType
	};

	class FormatCharProperty;	// from structures.h
	class FormatParaProperty;	// from structures.h
	class FormatInfoPage : public FormatInfoPageGenerated, public NeedsHeader
	{
	private:
		DWord m_firstCharByte;	// FormatInfoPageGenerated::m_firstCharBytePlus128 - 128
		enum FormatInfoPageTypes m_type;	// CharInfo or ParaInfo?


		//
		// uncompressed data
		//

		// list of FormatPointers (what byte it covers upto and which FormatProperty)
		FormatPointer *m_formatPointer;

		// FormatProperty (which one?  It depends on m_type)
		FormatCharProperty *m_formatCharProperty;
			FontTable *m_fontTable;	// for CharProperty
		FormatParaProperty *m_formatParaProperty;
			Word m_leftMargin, m_rightMargin;	// for ParaProperty


		//
		// Iterator helpers
		//
		int m_formatPointerUpto;	// [import] which formatPointer we are up to
		DWord m_nextCharByte;	// [import] for consistency checking
		int m_lastPropertyOffset;	// [import] lazy optimization (don't reload Property if FormatPointer points to last Property)
		int m_formatPointerPos, m_formatPropertyPos;	// [export]
		int m_numProperty;	// [export]


		friend class FormatInfo;	// libmswrite.h
			bool readFromDevice (void);
			bool writeToDevice (void);
			bool writeToArray (void);

		// TODO operator=
		FormatInfoPage &operator= (const FormatInfoPage &rhs);

	public:
		FormatInfoPage ();
		virtual ~FormatInfoPage ();

		void setMargins (const Word leftMargin, const Word rightMargin)
		{
			m_leftMargin = leftMargin, m_rightMargin = rightMargin;
		}
		void setFontTable (FontTable *fontTable)
		{
			m_fontTable = fontTable;
		}

		void setType (const enum FormatInfoPageTypes t)	{	m_type = t;	}

		DWord getFirstCharByte (void) const	{	return m_firstCharByte;	}
		void setFirstCharByte (const DWord val)	{	m_firstCharByte = val;	}

		DWord getNextCharByte (void) const	{	return m_nextCharByte;	}

		// my version of iterators is a lot simpler now, isn't it? :)
		void *begin (void);
		void *next (void);
		bool end (void) const;

		bool add (const void *property);
	};


	class BMP_BitmapFileHeader : public BMP_BitmapFileHeaderGenerated
	{
	public:
		BMP_BitmapFileHeader ();
		virtual ~BMP_BitmapFileHeader ();

		BMP_BitmapFileHeader &operator= (const BMP_BitmapFileHeader &rhs);

		bool readFromDevice (void);
		bool writeToDevice (void);
	};


	class BMP_BitmapInfoHeader : public BMP_BitmapInfoHeaderGenerated
	{
	public:
		BMP_BitmapInfoHeader ();
		virtual ~BMP_BitmapInfoHeader ();

		BMP_BitmapInfoHeader &operator= (const BMP_BitmapInfoHeader &rhs);

		bool readFromDevice (void);
		bool writeToDevice (void);
	};


	class BMP_BitmapColorIndex : public BMP_BitmapColorIndexGenerated
	{
	public:
		BMP_BitmapColorIndex ();
		virtual ~BMP_BitmapColorIndex ();

		BMP_BitmapColorIndex &operator= (const BMP_BitmapColorIndex &rhs);
	};


	class BitmapHeader : public BitmapHeaderGenerated
	{
	//private:
	//	friend class ImageGenerated;
	public:
		BitmapHeader ();
		virtual ~BitmapHeader ();

		BitmapHeader &operator= (const BitmapHeader &rhs);

		bool readFromDevice (void);
		bool writeToDevice (void);
	};


	class WMFHeader : public WMFHeaderGenerated
	{
	//private:
	//	friend class Image;
	public:
		WMFHeader ();
		virtual ~WMFHeader ();

		WMFHeader &operator= (const WMFHeader &rhs);

		bool readFromDevice (void);
		bool writeToDevice (void);
	};

}	// namespace MSWrite	{

#endif	//	__STRUCTURES_PRIVATE_H__

// end of structures_private.h
