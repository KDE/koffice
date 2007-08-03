/*
   This file is part of the KDE project
   Copyright (C) 2001, 2002, 2004 Nicolas GOUTTE <goutte@kde.org>
   Copyright (c) 2001 IABG mbH. All rights reserved.
                      Contact: Wolf-Michael Bolle <Bolle@IABG.de>

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
 * Boston, MA 02110-1301, USA.
*/

/*
   This file is based on the old file:
    /home/kde/koffice/filters/kword/ascii/asciiexport.cc

   The old file was copyrighted by
    Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
    Copyright (c) 2000 ID-PRO Deutschland GmbH. All rights reserved.
                       Contact: Wolf-Michael Bolle <Bolle@GMX.de>

   The old file was licensed under the terms of the GNU Library General Public
   License version 2.
*/

#include <kdebug.h>
#include <qdom.h>
//Added by qt3to4:
#include <Q3ValueList>

#include "TagProcessing.h"

#define DEBUG_KWORD_TAGS
// #define DEBUG_KWORD_IGNORED_TAGS


void ProcessSubtags ( const QDomNode             &parentNode,
                      Q3ValueList<TagProcessing>  &tagProcessingList,
                      KWEFKWordLeader            *leader)
{
    //kDebug(30508) <<"Starting ProcessSubtags for node:" << parentNode.nodeName();

    QDomNode childNode;

    for ( childNode = parentNode.firstChild (); !childNode.isNull (); childNode = childNode.nextSibling () )
    {
        if ( childNode.isElement () )
        {
            bool found = false;

            Q3ValueList<TagProcessing>::Iterator  tagProcessingIt;

            for ( tagProcessingIt = tagProcessingList.begin ();
                  tagProcessingIt != tagProcessingList.end ();
                  tagProcessingIt++ )
            {
                if ( childNode.nodeName () == (*tagProcessingIt).name )
                {
                    found = true;

                    if ( (*tagProcessingIt).processor != NULL )
                    {
                        ((*tagProcessingIt).processor) ( childNode, (*tagProcessingIt).data, leader );
                    }
#ifdef DEBUG_KWORD_IGNORED_TAGS
                    else
                    {
                        kDebug(30508) <<"Ignoring" << childNode.nodeName ()
                            << " tag in " << parentNode.nodeName () << endl;
                    }
#endif
                    break;
                }
            }

            if ( !found )
            {
                kDebug(30508) <<"Unexpected tag" << childNode.nodeName ()
                    << " in " << parentNode.nodeName () << "!" << endl;
            }
        }
    }
    //kDebug(30508) <<"Ending ProcessSubtags for node:" << parentNode.nodeName();
}

void AllowNoSubtags ( const QDomNode& myNode, KWEFKWordLeader *leader )
{
#ifdef DEBUG_KWORD_TAGS
    QString outputText;
    Q3ValueList<TagProcessing> tagProcessingList;
    ProcessSubtags (myNode, tagProcessingList, leader);
#else
    @_UNUSED( leader ):
#endif
}

AttrProcessing::AttrProcessing ( const QString& n, const QString& t, void *d )
    : name (n), data (d)
{
    if ( t == "int" )
        type = AttrInt;
    else if ( t == "QString" )
        type = AttrQString;
    else if ( t == "double" )
        type = AttrDouble;
    else if ( t == "bool" )
        type = AttrBool;
    else if ( t.isEmpty() )
        type = AttrNull;
    else
    {
        kWarning(30508) << "Unknown type: " << t << " for element " << n << " assuming NULL";
        type = AttrNull;
    }
}


void ProcessAttributes ( const QDomNode              &myNode,
                         Q3ValueList<AttrProcessing>  &attrProcessingList )
{
    //kDebug(30508) <<"Starting ProcessAttributes for node:" << myNode.nodeName();

    QDomNamedNodeMap myAttribs ( myNode.attributes () );
    //kDebug(30508) <<"Attributes =" << myAttribs.length ();
    for ( uint i = 0; i <  myAttribs.length (); i++ )
    {
        QDomAttr myAttrib ( myAttribs.item (i).toAttr () );

        if ( !myAttrib.isNull () )
        {
            bool found = false;

            Q3ValueList<AttrProcessing>::Iterator attrProcessingIt;

            for ( attrProcessingIt = attrProcessingList.begin ();
                  attrProcessingIt != attrProcessingList.end ();
                  attrProcessingIt++ )
            {
              //kDebug(30508) <<"NAME:" << myAttrib.name () <<" ==" << (*attrProcessingIt).name;
                if ( myAttrib.name () == (*attrProcessingIt).name )
                {
                    found = true;

                    if ( (*attrProcessingIt).data != NULL )
                    {
                        switch ( (*attrProcessingIt).type )
                        {
                        case AttrProcessing::AttrQString:
                            {
                                *((QString *) (*attrProcessingIt).data) = myAttrib.value ();
                                break;
                            }
                        case AttrProcessing::AttrInt:
                            {
                                *((int *) (*attrProcessingIt).data) = myAttrib.value ().toInt ();
                                break;
                            }
                        case AttrProcessing::AttrDouble:
                            {
                                *((double *) (*attrProcessingIt).data) = myAttrib.value ().toDouble ();
                                break;
                            }
                        case AttrProcessing::AttrBool:
                            {
                                const QString strAttr ( myAttrib.value().simplified() );
                                bool flag;
                                if ((strAttr=="yes")||(strAttr=="1")||(strAttr=="true"))
                                {
                                    flag=true;
                                }
                                else if ((strAttr=="no")||(strAttr=="0")||(strAttr=="false"))
                                {
                                    flag=false;
                                }
                                else
                                {
                                    flag=false;
                                    kWarning(30508) << "Unknown value for a boolean: " << strAttr
                                        << " in tag " << myNode.nodeName () << ", attribute "
                                        << myAttrib.name() << endl;
                                }
                                *((bool *) (*attrProcessingIt).data) = flag;
                                break;
                            }
                        case AttrProcessing::AttrNull:
                            break;
                        default:
                            {
                                kDebug(30508) <<"Unexpected data type" << int( (*attrProcessingIt).type )
                                    << " in " << myNode.nodeName ()
                                    << " attribute " << (*attrProcessingIt).name
                                    << endl;
                                break;
                            }
                        }
                    }
#ifdef DEBUG_KWORD_IGNORED_TAGS
                    else
                    {
                        kDebug(30508) <<"Ignoring" << myNode.nodeName()
                            << " attribute " << (*attrProcessingIt).name
                            << endl;
                    }
#endif
                    break;
                }
            }

            if ( !found )
            {
                kWarning(30508) << "Unexpected attribute " << myAttrib.name ()
                    << " in " << myNode.nodeName () << "!" << endl;
            }
        }
    }
    //kDebug(30508) <<"Ending ProcessAttributes for node:" << myNode.nodeName();
}

void AllowNoAttributes ( const QDomNode & myNode )
{
#ifdef DEBUG_KWORD_TAGS
    Q3ValueList<AttrProcessing> attrProcessingList;
    ProcessAttributes (myNode, attrProcessingList);
#else
    Q_UNUSED( myNode );
#endif
}
