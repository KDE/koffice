/* This file is part of the KOffice project
   Copyright (C) 2009 Benjamin Cail <cricketc@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   version 2 of the License, or (at your option) version 3 or,
   at the discretion of KDE e.V (which shall act as a proxy as in
   section 14 of the GPLv3), any later version..

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef PARAGRAPH_H
#define PARAGRAPH_H

#include <QBuffer>

#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoXmlWriter.h>

//TODO remove unneeded includes
#include <wv2/styles.h>
#include <wv2/paragraphproperties.h>
#include <wv2/functor.h>
#include <wv2/functordata.h>
#include <wv2/ustring.h>
#include <wv2/parser.h>
#include <wv2/fields.h>

class Paragraph
{
    public:
        Paragraph( KoGenStyles* mainStyles, bool inStylesDotXml = false, bool isHeading = false, int outlineLevel = 0 );
        ~Paragraph();
        void writeToFile( KoXmlWriter* writer );
        void addRunOfText( QString text,  wvWare::SharedPtr<const wvWare::Word97::CHP> chp, QString fontName );
        void openInnerParagraph();
        void closeInnerParagraph();
        void setParagraphProperties( wvWare::SharedPtr<const wvWare::ParagraphProperties> properties );
        //set the general named style that applies to this paragraph
        void setParentStyle( const wvWare::Style* parentStyle, QString parentStyleName );
        KoGenStyle* getParagraphStyle();

        //static functions for parsing wvWare properties into KoGenStyles
        static void parseParagraphProperties( const wvWare::ParagraphProperties& properties,
                KoGenStyle* style );
        static void parseCharacterProperties( const wvWare::Word97::CHP* chp, KoGenStyle* style, const wvWare::Style* parentStyle );
    private:
        wvWare::SharedPtr<const wvWare::ParagraphProperties> m_paragraphProperties;
        wvWare::SharedPtr<const wvWare::ParagraphProperties> m_paragraphProperties2;
        KoGenStyle* m_paragraphStyle; //pointer to KOffice structure for paragraph formatting
        KoGenStyle* m_paragraphStyle2; //place to store original style when we have an inner paragraph
        KoGenStyles* m_mainStyles; //pointer to style collection for this document
        const wvWare::Style* m_parentStyle; //parent style for the paragraph
        const wvWare::Style* m_parentStyle2; //store parent style when in inner paragraph
        std::vector<QString> m_textStrings; //store list of text strings within a paragraph
        std::vector<QString> m_textStrings2; //store original list when in inner paragraph
        std::vector<KoGenStyle*> m_textStyles; //store list of styles for text within a paragraph
        std::vector<KoGenStyle*> m_textStyles2; //store original list when in inner paragraph
        bool m_inStylesDotXml; //let us know if we're in content.xml or styles.xml
        bool m_isHeading; //information for writing a heading instead of a paragraph
                          // (odt looks formats them similarly)
        int m_outlineLevel;
}; //end class Paragraph

#endif //PARAGRAPH_H
