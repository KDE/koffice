/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <koGenStyles.h>
#include <koxmlwriter.h>
#include "../../store/tests/xmlwritertest.h"
#include <kdebug.h>

int main( int, char** ) {

    KoGenStyles coll;

    enum { STYLE_USER, STYLE_AUTO, STYLE_OTHER };

    KoGenStyle first( STYLE_AUTO );
    first.addAttribute( "style:family", "paragraph" );
    first.addAttribute( "style:master-page-name", "Standard" );
    first.addProperty( "style:page-number", "0" );

    QString firstName = coll.lookup( first );
    kdDebug() << "The first style got assigned the name " << firstName << endl;
    assert( firstName == "A1" ); // it's fine if it's something else, but the koxmlwriter tests require a known name
    assert( first.type() == STYLE_AUTO );

    KoGenStyle second( STYLE_AUTO );
    second.addAttribute( "style:family", "paragraph" );
    second.addAttribute( "style:master-page-name", "Standard" );
    second.addProperty( "style:page-number", "0" );

    QString secondName = coll.lookup( second );
    kdDebug() << "The second style got assigned the name " << secondName << endl;

    assert( firstName == secondName ); // check that sharing works
    // Interesting
    //assert( first == second ); // check that operator== works :)

    KoGenStyle third( STYLE_AUTO, secondName ); // inherited style
    // We *have* to set the family even in derived styles.
    // But that means we can't implement "diff with parent" in koGenStyles...
    // Hmm, well we'll see. Either it will have an exception, or this needs
    // to be done at the level above.
    third.addAttribute( "style:family", "paragraph" );
    third.addProperty( "style:margin-left", "1.249cm" );
    assert( third.parentName() == secondName );

    QString thirdName = coll.lookup( third, "P" );
    kdDebug() << "The third style got assigned the name " << thirdName << endl;
    assert( thirdName == "P1" );

    KoGenStyle user( STYLE_USER ); // differs from third since it doesn't inherit second, and has a different type
    user.addProperty( "style:margin-left", "1.249cm" );

    QString userStyleName = coll.lookup( user, "User", false );
    kdDebug() << "The user style got assigned the name " << userStyleName << endl;
    assert( userStyleName == "User" );

    assert( coll.styles().count() == 3 );
    assert( coll.styles( STYLE_AUTO ).count() == 2 );
    assert( coll.styles( STYLE_USER ).count() == 1 );

    TEST_BEGIN( 0, 0 );
    first.writeStyle( &writer, "style:style", firstName );
    TEST_END( "XML for first/second style", "<!DOCTYPE r>\n<r>\n <style:style style:name=\"A1\" style:family=\"paragraph\" style:master-page-name=\"Standard\">\n  <style:properties style:page-number=\"0\"/>\n </style:style>\n</r>\n" );

    TEST_BEGIN( 0, 0 );
    third.writeStyle( &writer, "style:style", thirdName );
    TEST_END( "XML for third style", "<!DOCTYPE r>\n<r>\n <style:style style:name=\"P1\" style:parent-style-name=\"A1\" style:family=\"paragraph\">\n  <style:properties style:margin-left=\"1.249cm\"/>\n </style:style>\n</r>\n" );


    fprintf( stderr, "OK\n" );

    return 0;
}
