//

/*
   This file is part of the KDE project
   Copyright (C) 2004 Nicolas GOUTTE <goutte@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <kdebug.h>

#include "kword13utils.h"
#include "kword13frameset.h"


KWord13Frameset::KWord13Frameset( int frameType, int frameInfo, const QString& name )
    : m_numFrames(0), m_frameType( frameType ), m_frameInfo( frameInfo ), m_name( name )
{    
}

KWord13Frameset::~KWord13Frameset( void )
{
}

bool KWord13Frameset::addParagraph(const KWord13Paragraph&)
{
    kdDebug(30520) << "Cannot add paragraph! Not a text frameset!" << endl;
    return false;
}

void KWord13Frameset::xmldump( QTextStream& iostream )
{
    iostream << "  <frameset variant=\"None\" type=\"" << m_frameType
         << "\" info=\"" << m_frameInfo
         << "\" name=\"" << EscapeXmlDump( m_name ) <<"\"/>\n";
}


KWordTextFrameset::KWordTextFrameset( int frameType, int frameInfo, const QString& name )
    : KWord13Frameset( frameType, frameInfo, name )
{    
}

KWordTextFrameset::~KWordTextFrameset( void )
{
}

bool KWordTextFrameset::addParagraph(const KWord13Paragraph& para)
{
    m_paragraphGroup << para;
    return true;
}

void KWordTextFrameset::xmldump( QTextStream& iostream )
{
    iostream << "  <frameset variant=\"Text\" type=\"" << m_frameType
         << "\" info=\"" << m_frameInfo
         << "\" name=\"" << EscapeXmlDump( m_name ) <<"\">\n";
    m_paragraphGroup.xmldump( iostream );
    iostream << "  </frameset>\n";
}

KWord13PictureFrameset::KWord13PictureFrameset( int frameType, int frameInfo, const QString& name )
    : KWord13Frameset( frameType, frameInfo, name )
{    
}

KWord13PictureFrameset::~KWord13PictureFrameset( void )
{
}

void KWord13PictureFrameset::xmldump( QTextStream& iostream )
{
    iostream << "  <frameset variant=\"Text\" type=\"" << m_frameType
         << "\" info=\"" << m_frameInfo
         << "\" name=\"" << EscapeXmlDump( m_name ) <<"\">\n";
    iostream << "   <key>" << m_pictureKey << "</key>\n";
    iostream << "  </frameset>\n";
}
