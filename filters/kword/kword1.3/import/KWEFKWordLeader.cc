//

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/*
   Part of this code comes from the old file:
    /home/kde/koffice/filters/kword/ascii/asciiexport.cc

   The old file was copyrighted by
    Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
    Copyright (c) 2000 ID-PRO Deutschland GmbH. All rights reserved.
                       Contact: Wolf-Michael Bolle <Wolf-Michael.Bolle@GMX.de>

   The old file was licensed under the terms of the GNU Library General Public
   License version 2.
*/

#include <qbuffer.h>
#include <qimage.h>
#include <qfile.h>
#include <qdom.h>

#include <kdebug.h>

#include <koGlobal.h>
#include <koStore.h>
#include <koPicture.h>

#include "KWEFStructures.h"
#include "TagProcessing.h"
#include "ProcessDocument.h"
#include "KWEFKWordLeader.h"
#include "KWEFBaseWorker.h"


static FrameAnchor *findAnchor ( const KoPictureKey& key,
                                 QValueList<ParaData>& paraList )
{
    kdDebug(30508) << "findAnchor " << key.toString() << endl;
    QValueList<ParaData>::Iterator paraIt;

    for ( paraIt = paraList.begin (); paraIt != paraList.end (); paraIt++ )
    {
        ValueListFormatData::Iterator formattingIt;

        for ( formattingIt = (*paraIt).formattingList.begin ();
              formattingIt != (*paraIt).formattingList.end ();
              formattingIt++ )
        {
            if ( (*formattingIt).id              == 6    &&
                 (*formattingIt).frameAnchor.key == key )
            {
                kdDebug(30508) << "Found anchor " << (*formattingIt).frameAnchor.key.toString() << endl;
                return &(*formattingIt).frameAnchor;
            }
        }
    }

    kdWarning(30508) << "findAnchor returning NULL!" << endl;
    return NULL;
}

static ParaData createTableMgr( const QString& grpMgr )
{
    ParaData  pData;
    LayoutData lData;
    FormatData fData;
    fData.id = 6;
    fData.frameAnchor.key  = KoPictureKey( grpMgr );
    fData.frameAnchor.type = 6;
    lData.formatData = fData;
    pData.layout = lData;
    pData.formattingList << fData;
    return pData;
}

static void ProcessParagraphTag ( QDomNode         myNode,
                                  void            *tagData,
                                  KWEFKWordLeader *leader )
{
#if 0
    kdDebug (30508) << "ProcessParagraphTag () - Begin" << endl;
#endif

    QValueList<ParaData> *paraList = (QValueList<ParaData> *) tagData;

    AllowNoAttributes (myNode);

    ParaData paraData;

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "TEXT",    ProcessTextTag,    (void *) &paraData.text           )
                      << TagProcessing ( "FORMATS", ProcessFormatsTag, (void *) &paraData.formattingList )
                      << TagProcessing ( "LAYOUT",  ProcessLayoutTag,  (void *) &paraData.layout         );
    ProcessSubtags (myNode, tagProcessingList, leader);

    CreateMissingFormatData (paraData.text, paraData.formattingList);

    // TODO/FIXME: why !paraData.text.isEmpty()
    if ( paraData.formattingList.isEmpty () && !paraData.text.isEmpty () )
    {
        if ( paraData.layout.formatData.id == 1 )
        {
            paraData.formattingList << paraData.layout.formatData;
        }
        else
        {
            kdWarning (30508) << "No useful FORMAT tag found for text in PARAGRAPH" << endl;
        }
    }


    *paraList << paraData;

#if 0
    kdDebug (30508) << "ProcessParagraphTag () - End " << paraText << endl;
#endif
}


static void ProcessImageKeyTag ( QDomNode         myNode,
                                 void            *tagData,
                                 KWEFKWordLeader *)
{
    KoPictureKey *key = (KoPictureKey*) tagData;

    // Let KoPicture do the loading
    key->loadAttributes(myNode.toElement());
}


static void ProcessFrameTag ( QDomNode myNode, void *tagData,
    KWEFKWordLeader *leader )
{
    FrameAnchor* frameAnchor= (FrameAnchor*) tagData;

    int lRed=0, lBlue=0, lGreen=0;
    int rRed=0, rBlue=0, rGreen=0;
    int tRed=0, tBlue=0, tGreen=0;
    int bRed=0, bBlue=0, bGreen=0;
    int bkRed=255, bkBlue=255, bkGreen=255;

    frameAnchor->frame.lWidth=0.0;
    frameAnchor->frame.rWidth=0.0;
    frameAnchor->frame.tWidth=0.0;
    frameAnchor->frame.bWidth=0.0;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList
        << AttrProcessing ( "right",        "double",  &frameAnchor->frame.right  )
        << AttrProcessing ( "left",         "double",  &frameAnchor->frame.left   )
        << AttrProcessing ( "top",          "double",  &frameAnchor->frame.top    )
        << AttrProcessing ( "bottom",       "double",  &frameAnchor->frame.bottom )

        << AttrProcessing ( "min-height",   "double",  &frameAnchor->frame.minHeight )

        << AttrProcessing ( "runaround",      "int",  &frameAnchor->frame.runaround         )
        << AttrProcessing ( "runaroundSide",  "QString",  &frameAnchor->frame.runaroundSide )
        << AttrProcessing ( "runaroundGap",   "double",  &frameAnchor->frame.runaroundGap   )

        << AttrProcessing ( "autoCreateNewFrame",  "int",  &frameAnchor->frame.autoCreateNewFrame )
        << AttrProcessing ( "newFrameBehavior",    "int",  &frameAnchor->frame.newFrameBehavior   )

        << AttrProcessing ( "copy",       "int",  &frameAnchor->frame.copy      )
        << AttrProcessing ( "sheetSide",  "int",  &frameAnchor->frame.sheetSide )

        << AttrProcessing ( "lWidth",   "double",  &frameAnchor->frame.lWidth )
        << AttrProcessing ( "rWidth",   "double",  &frameAnchor->frame.rWidth )
        << AttrProcessing ( "tWidth",   "double",  &frameAnchor->frame.tWidth )
        << AttrProcessing ( "bWidth",   "double",  &frameAnchor->frame.bWidth )

        << AttrProcessing ( "lRed",     "int",  &lRed   )
        << AttrProcessing ( "lGreen",   "int",  &lGreen )
        << AttrProcessing ( "lBlue",    "int",  &lBlue  )

        << AttrProcessing ( "rRed",     "int",  &rRed   )
        << AttrProcessing ( "rGreen",   "int",  &rGreen )
        << AttrProcessing ( "rBlue",    "int",  &rBlue  )

        << AttrProcessing ( "tRed",     "int",  &tRed   )
        << AttrProcessing ( "tGreen",   "int",  &tGreen )
        << AttrProcessing ( "tBlue",    "int",  &tBlue  )

        << AttrProcessing ( "bRed",     "int",  &bRed   )
        << AttrProcessing ( "bGreen",   "int",  &bGreen )
        << AttrProcessing ( "bBlue",    "int",  &bBlue  )

        << AttrProcessing ( "lStyle",    "int",  &frameAnchor->frame.lStyle )
        << AttrProcessing ( "rStyle",    "int",  &frameAnchor->frame.rStyle )
        << AttrProcessing ( "tStyle",    "int",  &frameAnchor->frame.tStyle )
        << AttrProcessing ( "bStyle",    "int",  &frameAnchor->frame.bStyle )

        << AttrProcessing ( "bkRed",     "int",  &bkRed   )
        << AttrProcessing ( "bkGreen",   "int",  &bkGreen )
        << AttrProcessing ( "bkBlue",    "int",  &bkBlue  )

        << AttrProcessing ( "bkStyle",    "int",  &frameAnchor->frame.bkStyle )

        << AttrProcessing ( "bleftpt",     "double",  &frameAnchor->frame.bleftpt   )
        << AttrProcessing ( "brightpt",    "double",  &frameAnchor->frame.brightpt  )
        << AttrProcessing ( "btoppt",      "double",  &frameAnchor->frame.btoppt    )
        << AttrProcessing ( "bbottompt",   "double",  &frameAnchor->frame.bbottompt )
        ;
    ProcessAttributes (myNode, attrProcessingList);

    frameAnchor->frame.lColor  = QColor( lRed, lGreen, lBlue );
    frameAnchor->frame.rColor  = QColor( rRed, rGreen, rBlue );
    frameAnchor->frame.tColor  = QColor( tRed, tGreen, tBlue );
    frameAnchor->frame.bColor  = QColor( bRed, bGreen, bBlue );
    frameAnchor->frame.bkColor = QColor( bkRed, bkGreen, bkBlue );

    AllowNoSubtags (myNode, leader);
}


static void ProcessImageTag ( QDomNode         myNode,
                              void            *tagData,
                              KWEFKWordLeader *leader )
{ // <PICTURE>
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "keepAspectRatio", NULL, NULL );
    ProcessAttributes (myNode, attrProcessingList);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "KEY", ProcessImageKeyTag, tagData );
    ProcessSubtags (myNode, tagProcessingList, leader);
}


static void ProcessFramesetTag ( QDomNode        myNode,
                                void            *tagData,
                                KWEFKWordLeader *leader )
{
#if 0
    kdDebug (30508) << "ProcessFramesetTag () - Begin" << endl;
#endif

    QValueList<ParaData> *paraList = (QValueList<ParaData> *) tagData;

    int     frameType = -1;
    int     frameInfo = -1;
    int     col       = -1;
    int     row       = -1;
    int     cols      = -1;
    int     rows      = -1;
    QString name;
    QString grpMgr;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "name",        "QString", (void *) &name      )
                       << AttrProcessing ( "frameType",   "int",     (void *) &frameType )
                       << AttrProcessing ( "frameInfo",   "int",     (void *) &frameInfo )
                       << AttrProcessing ( "removable",   "",        NULL                )
                       << AttrProcessing ( "visible",     "",        NULL                )
                       << AttrProcessing ( "grpMgr",      "QString", (void *) &grpMgr    )
                       << AttrProcessing ( "row",         "int",     (void *) &row       )
                       << AttrProcessing ( "col",         "int",     (void *) &col       )
                       << AttrProcessing ( "rows",        "int",     (void *) &rows      )
                       << AttrProcessing ( "cols",        "int",     (void *) &cols      )
                       << AttrProcessing ( "protectSize", "",        NULL                )
                        ;
    ProcessAttributes (myNode, attrProcessingList);

    switch ( frameType )
    {
    case 1:
            if ( grpMgr.isEmpty () )
            {
                // As we do not support anything else than normal text, process only normal text.
                // TODO: Treat the other types of frames (frameType)
                if (frameInfo==0)
                {
                    // Normal Text
                    QValueList<TagProcessing> tagProcessingList;
                    tagProcessingList.append(TagProcessing ( "FRAME",     NULL,                NULL              ));
                    tagProcessingList.append(TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) paraList ));
                    ProcessSubtags (myNode, tagProcessingList, leader);
                }
                else if (frameInfo==1)
                {
                    // header for first page
                    HeaderData header;
                    header.page = HeaderData::PAGE_FIRST;
                    QValueList<TagProcessing> tagProcessingList;
                    tagProcessingList.append(TagProcessing ( "FRAME",     NULL,                NULL              ));
                    tagProcessingList.append(TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) &header.para ));

                    ProcessSubtags (myNode, tagProcessingList, leader);
                    leader->doHeader( header );
                }
                else if (frameInfo==2)
                {
                    // header for even page
                    HeaderData header;
                    header.page = HeaderData::PAGE_EVEN;
                    QValueList<TagProcessing> tagProcessingList;
                    tagProcessingList.append(TagProcessing ( "FRAME",     NULL,                NULL              ));
                    tagProcessingList.append(TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) &header.para ));
                    ProcessSubtags (myNode, tagProcessingList, leader);
                    leader->doHeader( header );
                }
                else if (frameInfo==3)
                {
                    // header for odd page (or all page, if hType=0)
                    HeaderData header;
                    header.page = (leader->headerType() != 0 ) ? HeaderData::PAGE_ODD : HeaderData::PAGE_ALL;
                    QValueList<TagProcessing> tagProcessingList;
                    tagProcessingList.append(TagProcessing ( "FRAME",     NULL,                NULL              ));
                    tagProcessingList.append(TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) &header.para ));
                    ProcessSubtags (myNode, tagProcessingList, leader);
                    leader->doHeader( header );
                }
                else if (frameInfo==4)
                {
                    // footer for first page
                    FooterData footer;
                    footer.page = FooterData::PAGE_FIRST;
                    QValueList<TagProcessing> tagProcessingList;
                    tagProcessingList.append(TagProcessing ( "FRAME",     NULL,                NULL              ));
                    tagProcessingList.append(TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) &footer.para ));
                    ProcessSubtags (myNode, tagProcessingList, leader);
                    leader->doFooter( footer );
                }
                else if (frameInfo==5)
                {
                    // footer for even page
                    FooterData footer;
                    footer.page = FooterData::PAGE_EVEN;
                    QValueList<TagProcessing> tagProcessingList;
                    tagProcessingList.append(TagProcessing ( "FRAME",     NULL,                NULL              ));
                    tagProcessingList.append(TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) &footer.para ));
                    ProcessSubtags (myNode, tagProcessingList, leader);
                    leader->doFooter( footer );
                }
                else if (frameInfo==6)
                {
                    // footer for odd page (or all page, if fType=0)
                    FooterData footer;
                    footer.page = (leader->footerType() != 0) ? FooterData::PAGE_ODD : FooterData::PAGE_ALL;
                    QValueList<TagProcessing> tagProcessingList;
                    tagProcessingList.append(TagProcessing ( "FRAME",     NULL,                NULL              ));
                    tagProcessingList.append(TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) &footer.para ));
                    ProcessSubtags (myNode, tagProcessingList, leader);
                    leader->doFooter( footer );
                }
            }
            else
            {
                if ( col != -1 && row != -1 )
                {
                    if ( cols == 1 && rows == 1 )
                    {
#if 0
                        kdDebug (30508) << "DEBUG - FRAMESET: table " << name << " col, row = "
                                        << col << ", " << row << ", Mgr = "<< grpMgr << endl;
#endif
                        FrameAnchor *frameAnchor = findAnchor (grpMgr, *paraList);
                        if ( !frameAnchor ) {
                            *paraList << createTableMgr( grpMgr );
                            frameAnchor = &paraList->last().formattingList.first().frameAnchor;
                        }

                        frameAnchor->type = 6;

                        QValueList<ParaData> cellParaList;
                        QValueList<TagProcessing> tagProcessingList;
                        tagProcessingList << TagProcessing ( "FRAME",     ProcessFrameTag,     frameAnchor   )
                                          << TagProcessing ( "PARAGRAPH", ProcessParagraphTag, &cellParaList );
                        ProcessSubtags (myNode, tagProcessingList, leader);

                        frameAnchor->table.addCell (col, row, cellParaList, frameAnchor->frame);
                    }
                    else
                    {
                        kdWarning (30508) << "Unexpected value for one of, or all FRAMESET attribute cols, rows: "
                                        << cols << ", " << rows << "!" << endl;
                        AllowNoSubtags (myNode, leader);
                    }
                }
                else
                {
                    kdWarning (30508) << "Unset value for one of, or all FRAMESET attributes col, row: "
                                    << col << ", " << row << "!" << endl;
                    AllowNoSubtags (myNode, leader);
                }
            }
            break;

    case 2: // PICTURE
    case 5: // CLIPART: deprecated (up to KOffice 1.2 Beta 2)
        {
#if 0
            kdDebug (30508) << "DEBUG: FRAMESET name of picture is " << name << endl;
#endif

            FrameAnchor *frameAnchor = findAnchor (name, *paraList);

            if ( frameAnchor )
            {
                frameAnchor->type = frameType;

                QValueList<TagProcessing> tagProcessingList;
                tagProcessingList
                    << TagProcessing ( "FRAME",   ProcessFrameTag, frameAnchor )
                    << TagProcessing ( "PICTURE", ProcessImageTag, &frameAnchor->picture.key )
                    << TagProcessing ( "IMAGE",   ProcessImageTag, &frameAnchor->picture.key )
                    << TagProcessing ( "CLIPART", ProcessImageTag, &frameAnchor->picture.key )
                    ;
                ProcessSubtags (myNode, tagProcessingList, leader);

#if 0
                kdDebug (30508) << "DEBUG: FRAMESET PICTURE KEY filename of picture is " << frameAnchor->picture.key << endl;
#endif

                frameAnchor->key = frameAnchor->picture.key;
            }
            else
            {
                kdWarning (30508) << "ProcessFramesetTag: Couldn't find anchor " << name << endl;
            }

            break;
        }

    default:
            kdWarning (30508) << "Unexpected frametype " << frameType << " (in ProcessFramesetTag)" << endl;
    }

#if 0
    kdDebug (30508) << "ProcessFramesetTag () - End" << endl;
#endif
}


static void ProcessFramesetsTag ( QDomNode        myNode,
                                  void            *tagData,
                                  KWEFKWordLeader *leader )
{
    AllowNoAttributes (myNode);
    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "FRAMESET", ProcessFramesetTag, tagData );
    ProcessSubtags (myNode, tagProcessingList, leader);
}


static void ProcessStyleTag (QDomNode myNode, void *, KWEFKWordLeader *leader )
{
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList
        << AttrProcessing ( "outline",   "", NULL );
    ProcessAttributes (myNode, attrProcessingList);

    LayoutData layout;

    ProcessLayoutTag (myNode, &layout, leader);

    leader->doFullDefineStyle (layout);
}


static void ProcessStylesPluralTag (QDomNode myNode, void *, KWEFKWordLeader *leader )
{
    AllowNoAttributes (myNode);

    leader->doOpenStyles ();

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "STYLE", ProcessStyleTag, leader );
    ProcessSubtags (myNode, tagProcessingList, leader);

    leader->doCloseStyles ();
}


static void ProcessPaperBordersTag (QDomNode myNode, void*, KWEFKWordLeader* leader)
{

    double left   = 0.0;
    double right  = 0.0;
    double top    = 0.0;
    double bottom = 0.0;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList
        << AttrProcessing ( "left",   "double", &left )
        << AttrProcessing ( "right",  "double", &right )
        << AttrProcessing ( "top",    "double", &top )
        << AttrProcessing ( "bottom", "double", &bottom )
        ;
    ProcessAttributes (myNode, attrProcessingList);

    leader->doFullPaperBorders(top, left, bottom, right);

    AllowNoSubtags (myNode, leader);
}

static void ProcessPaperTag (QDomNode myNode, void *, KWEFKWordLeader *leader)
{

    int format      = -1;
    int orientation = -1;
    double width    = -1.0;
    double height   = -1.0;
    int hType       = -1;
    int fType       = -1;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "format",              "int",    (void *) &format      )
                       << AttrProcessing ( "width",               "double", (void *) &width       )
                       << AttrProcessing ( "height",              "double", (void *) &height      )
                       << AttrProcessing ( "orientation",         "int",    (void *) &orientation )
                       << AttrProcessing ( "columns",             "",       NULL                  )
                       << AttrProcessing ( "columnspacing",       "",       NULL                  )
                       << AttrProcessing ( "pages",               "",       NULL                  )
                       << AttrProcessing ( "hType",               "int",    (void*) &hType        )
                       << AttrProcessing ( "fType",               "int",    (void*) &fType        )
                       << AttrProcessing ( "spHeadBody",          "",       NULL                  )
                       << AttrProcessing ( "spFootBody",          "",       NULL                  )
                       << AttrProcessing ( "spFootNoteBody",      "",       NULL                  )
                       << AttrProcessing ( "slFootNotePosition",  "",       NULL                  )
                       << AttrProcessing ( "slFootNoteLength",    "",       NULL                  )
                       << AttrProcessing ( "slFootNoteWidth",     "",       NULL                  )
                       << AttrProcessing ( "slFootNoteType",      "",       NULL                  );
    ProcessAttributes (myNode, attrProcessingList);

    leader->setHeaderType( hType );
    leader->setFooterType( fType );

    leader->doPageInfo ( hType, fType );
    leader->doFullPaperFormat (format, width, height, orientation);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList
        << TagProcessing ( "PAPERBORDERS", ProcessPaperBordersTag, NULL )
        ;

    ProcessSubtags (myNode, tagProcessingList, leader);
}

static void ProcessVariableSettingsTag (QDomNode myNode, void *, KWEFKWordLeader *leader)
{
    VariableSettingsData vs;
    QString print, creation, modification; // Dates

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "startingPageNumber",
                                           "int",
                                           (void *) &vs.startingPageNumber )
                       << AttrProcessing ( "displaylink",
                                           "bool",
                                           (void *) &vs.displaylink )
                       << AttrProcessing ( "underlinelink",
                                           "bool",
                                           (void *) &vs.underlinelink )
                       << AttrProcessing ( "displaycomment",
                                           "bool",
                                           (void *) &vs.displaycomment )
                       << AttrProcessing ( "displayfieldcode",
                                           "bool",
                                           (void *) &vs.displayfieldcode )
                       << AttrProcessing ( "lastPrintingDate",
                                           "QString",
                                           &print )
                       << AttrProcessing ( "creationDate",
                                           "QString",
                                           &creation )
                       << AttrProcessing ( "modificationDate",
                                           "QString",
                                           &modification );
    ProcessAttributes (myNode, attrProcessingList);

    if (!creation.isEmpty())
        vs.creationTime=QDateTime::fromString(creation, Qt::ISODate);
    if (!modification.isEmpty())
        vs.modificationTime=QDateTime::fromString(modification, Qt::ISODate);
    if (!print.isEmpty())
        vs.printTime=QDateTime::fromString(print, Qt::ISODate);

    leader->doVariableSettings (vs);
}

static void ProcessSpellCheckIgnoreWordTag (QDomNode myNode, void *, KWEFKWordLeader *leader )
{
    QString ignoreword;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList
        << AttrProcessing ( "word", "QString", &ignoreword )
        ;
    ProcessAttributes (myNode, attrProcessingList);

    leader->doFullSpellCheckIgnoreWord (ignoreword);

    AllowNoSubtags (myNode, leader);
}


static void ProcessSpellCheckIgnoreListTag (QDomNode myNode, void *, KWEFKWordLeader *leader )
{
    AllowNoAttributes (myNode);

    leader->doOpenSpellCheckIgnoreList ();

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "SPELLCHECKIGNOREWORD", ProcessSpellCheckIgnoreWordTag, leader );
    ProcessSubtags (myNode, tagProcessingList, leader);

    leader->doCloseSpellCheckIgnoreList ();
}


static void ProcessPixmapsKeyTag ( QDomNode         myNode,
                                   void            *tagData,
                                   KWEFKWordLeader *leader )
{
    QValueList<ParaData> *paraList = (QValueList<ParaData> *) tagData;

    KoPictureKey key;

    // Let KoPicture do most of the loading
    key.loadAttributes(myNode.toElement());
    QString name(myNode.toElement().attribute("name"));

    // TODO/FIXME: an image could be re-used a few times!
    FrameAnchor *frameAnchor = findAnchor (key, *paraList);

    if ( frameAnchor )
    {
#if 0
        kdDebug (30508) << "DEBUG: ProcessPixmapsKeyTag (): koStore for picture "
                        << fileName << " is " << name << endl;
#endif

        frameAnchor->picture.koStoreName = name;
    }
    else
    {
        kdWarning (30508) << "Could find anchor for picture " << key.toString() << endl;
    }


    AllowNoSubtags (myNode, leader);
}


static void ProcessPixmapsTag ( QDomNode         myNode,
                                void            *tagData,
                                KWEFKWordLeader *leader )
{
    AllowNoAttributes (myNode);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "KEY", ProcessPixmapsKeyTag, tagData );
    ProcessSubtags (myNode, tagProcessingList, leader);
}


static void FreeCellParaLists ( QValueList<ParaData> &paraList )
{
    QValueList<ParaData>::Iterator paraIt;

    for ( paraIt = paraList.begin (); paraIt != paraList.end (); paraIt++ )
    {
        ValueListFormatData::Iterator formattingIt;

        for ( formattingIt = (*paraIt).formattingList.begin ();
              formattingIt != (*paraIt).formattingList.end ();
              formattingIt++ )
        {
            if ( (*formattingIt).id == 6 && (*formattingIt).frameAnchor.type == 6 )
            {
                QValueList<TableCell>::Iterator cellIt;

                for ( cellIt = (*formattingIt).frameAnchor.table.cellList.begin ();
                      cellIt != (*formattingIt).frameAnchor.table.cellList.end ();
                      cellIt++ )
                {
                    FreeCellParaLists ( *(*cellIt).paraList );   // recursion is great
                    delete (*cellIt).paraList;
                }
            }
        }
    }
}

// like ProcessFramesetTag, but only handle footnotes
static void ProcessFootnoteFramesetTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    QString frameName;
    int frameType = -1, frameInfo = -1;
    bool visible = false;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList
        << AttrProcessing ( "name",        "QString", (void *) &frameName      )
        << AttrProcessing ( "frameType",   "int",     (void *) &frameType )
        << AttrProcessing ( "frameInfo",   "int",     (void *) &frameInfo )
        << AttrProcessing ( "removable",   "",        NULL                )
        << AttrProcessing ( "visible",     "bool",    (void *) &visible )
        << AttrProcessing ( "grpMgr",      "QString", NULL    )
        << AttrProcessing ( "row",         "int",     NULL    )
        << AttrProcessing ( "col",         "int",     NULL    )
        << AttrProcessing ( "rows",        "int",     NULL    )
        << AttrProcessing ( "cols",        "int",     NULL    )
        << AttrProcessing ( "protectSize",  "",        NULL    )
        ;
    ProcessAttributes (myNode, attrProcessingList);

    // for footnote frame, frameType is 1 and frameInfo is 7
    if( ( frameType == 1 ) && ( frameInfo == 7 ) )
    {
        FootnoteData footnote;
        footnote.frameName = frameName;
        QValueList<TagProcessing> tagProcessingList;
        tagProcessingList.append(TagProcessing ( "FRAME",     NULL,                NULL              ));
        tagProcessingList.append(TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) &footnote.para ));
        ProcessSubtags (myNode, tagProcessingList, leader);
        leader->footnoteList.append( footnote );
    }
}

// like ProcessFramesetsTag, but only handle footnotes
static void ProcessFootnoteFramesetsTag ( QDomNode myNode, void *tagData, KWEFKWordLeader *leader )
{
    AllowNoAttributes (myNode);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "FRAMESET", ProcessFootnoteFramesetTag, tagData );
    ProcessSubtags (myNode, tagProcessingList, leader);
}

/*static*/ void ProcessDocTag ( QDomNode         myNode,
    void* /*tagData*/, KWEFKWordLeader* leader )
{
    //kdDebug (30508) << "Entering ProcessDocTag" << endl;

    QValueList<AttrProcessing> attrProcessingList;

    attrProcessingList << AttrProcessing ( "xmlns",         "QString", 0  )
                       << AttrProcessing ( "editor",        "QString", 0  )
                       << AttrProcessing ( "mime",          "QString", 0  )
                       << AttrProcessing ( "syntaxVersion", "int",     0  );

    ProcessAttributes( myNode, attrProcessingList );
    // TODO: verify syntax version and perhaps mime

    leader->doOpenHead();

    // At first, process <SPELLCHECKIGNORELIST>, even if mostly it will not be needed
    QDomNode nodeIgnoreList=myNode.namedItem("SPELLCHECKIGNORELIST");
    if ( nodeIgnoreList.isNull () )
        kdDebug (30508) << "No <SPELLCHECKIGNORELIST>" << endl; // Most files will not have it!
    else
        ProcessSpellCheckIgnoreListTag (nodeIgnoreList, NULL, leader);

    // Process <PAPER> now, even if mostly the output will need to be delayed.
    QDomNode nodePaper=myNode.namedItem("PAPER");
    if ( nodePaper.isNull () )
        kdWarning (30508) << "No <PAPER>" << endl;
    else
        ProcessPaperTag (nodePaper, NULL, leader);

    // Process <VARIABLESETTINGS>
    QDomNode nodeVariableSettings=myNode.namedItem("VARIABLESETTINGS");
    if ( nodeVariableSettings.isNull () )
	kdWarning (30508) << "No <VARIABLESETTINGS>" << endl;
    else
	ProcessVariableSettingsTag (nodeVariableSettings, NULL, leader);

    // Then we process the styles
    QDomNode nodeStyles=myNode.namedItem("STYLES");
    if ( nodeStyles.isNull () )
        kdWarning (30508) << "No <STYLES>" << endl;
    else
        ProcessStylesPluralTag (nodeStyles, NULL, leader);

    // Process framesets, but only to find and extract footnotes
    QValueList<FootnoteData> footnotes;
    QDomNode nodeFramesets=myNode.namedItem("FRAMESETS");
    if ( !nodeFramesets.isNull() )
        ProcessFootnoteFramesetsTag(nodeFramesets, &footnotes, leader );

    // Process all framesets and pictures
    QValueList<TagProcessing> tagProcessingList;
    QValueList<ParaData> paraList;

    tagProcessingList
        << TagProcessing ( "PAPER",       NULL,                   NULL      ) // Already done
        << TagProcessing ( "ATTRIBUTES",  NULL,                   NULL      )
        << TagProcessing ( "FRAMESETS",   ProcessFramesetsTag,    &paraList )
        << TagProcessing ( "STYLES",      NULL,                   NULL      ) // Already done
        << TagProcessing ( "PICTURES",    ProcessPixmapsTag,      &paraList )
        << TagProcessing ( "PIXMAPS",     ProcessPixmapsTag,      &paraList )
        << TagProcessing ( "CLIPARTS",    ProcessPixmapsTag,      &paraList )
        << TagProcessing ( "EMBEDDED",    NULL,                   NULL      )
        ;

    // TODO: why are the followings used by KWord 1.2 but are not in its DTD?
    tagProcessingList << TagProcessing ( "SERIALL",     NULL,                   NULL               );
    tagProcessingList << TagProcessing ( "FOOTNOTEMGR", NULL,                   NULL               );

    ProcessSubtags (myNode, tagProcessingList, leader);

    leader->doCloseHead();
    leader->doOpenBody();

    leader->doFullDocument (paraList);

    FreeCellParaLists (paraList);

    leader->doCloseBody();

    //kdDebug (30508) << "Exiting ProcessDocTag" << endl;
}

bool KWEFKWordLeader::doAbortFile(void)
{
    // Mostly, aborting is the same than closing the file!
    return doCloseFile();
}

// Short simple definition for methods with void parameter
#define DO_VOID_DEFINITION(string) \
    bool KWEFKWordLeader::string() \
    {\
        return true; \
    }


DO_VOID_DEFINITION (doOpenStyles)
DO_VOID_DEFINITION (doCloseStyles)
DO_VOID_DEFINITION (doOpenHead)
DO_VOID_DEFINITION (doCloseHead)
DO_VOID_DEFINITION (doOpenBody)
DO_VOID_DEFINITION (doCloseBody)
DO_VOID_DEFINITION (doOpenSpellCheckIgnoreList)
DO_VOID_DEFINITION (doCloseSpellCheckIgnoreList)
DO_VOID_DEFINITION (doOpenTextFrameSet)
DO_VOID_DEFINITION (doCloseTextFrameSet)

bool KWEFKWordLeader::doFullDocumentInfo (const KWEFDocumentInfo &docInfo)
{
    return false;
}


bool KWEFKWordLeader::doVariableSettings (const VariableSettingsData &varSettings)
{
    return false;
}

bool KWEFKWordLeader::doPageInfo ( const int headerType, const int footerType )
{
    return false;
}

bool KWEFKWordLeader::doFullPaperFormat ( const int format, const double width, const double height, const int orientation )
{
    return false;
}

bool KWEFKWordLeader::doFullPaperBorders (const double top, const double left, const double bottom, const double right)
{
    return false;
}

bool KWEFKWordLeader::doFullDefineStyle ( LayoutData &layout )
{
    return false;
}

bool KWEFKWordLeader::doFullSpellCheckIgnoreWord (const QString& ignoreword)
{
    return false;
}

bool KWEFKWordLeader::doHeader ( const HeaderData& header )
{
    return false;
}

bool KWEFKWordLeader::doFooter ( const FooterData& footer )
{
    return false;
}

static bool ParseFile ( QIODevice* subFile, QDomDocument& doc)
{
    QString errorMsg;
    int errorLine;
    int errorColumn;

    if ( !doc.setContent (subFile, &errorMsg, &errorLine, &errorColumn) )
    {
        kdError (30508) << "Parsing Error! Aborting! (in ParseFile)" << endl
            << "  Line: " << errorLine << " Column: " << errorColumn << endl
            << "  Message: " << errorMsg << endl;
        // TODO: user message
        return false;
    }
    return true;
}

static bool ProcessStoreFile ( QIODevice* subFile,
    void (*processor) (QDomNode, void *, KWEFKWordLeader *),
    KWEFKWordLeader* leader)
{
    if (!subFile)
    {
        kdWarning(30508) << "Could not get a device for the document!" << endl;
    }
    else if ( subFile->open ( IO_ReadOnly ) )
    {
        kdDebug (30508) << "Processing Document..." << endl;
        QDomDocument doc;
        if (!ParseFile(subFile, doc))
        {
            subFile->close();
            return false;
        }
        // We must close the subFile before processing,
        //  as the processing could open other sub files.
        //  However, it would crash if two sub files are opened together
        subFile->close();

        QDomNode docNode = doc.documentElement();
        processor (docNode, NULL, leader);
        return true;
    }
    else
    {
        // Note: we do not worry too much if we cannot open the document info!
        kdWarning (30508) << "Unable to open document!" << endl;
    }
    return false;
}

QIODevice* KWEFKWordLeader::getSubFileDevice(const QString& fileName) const
{
    KoStoreDevice* subFile;

    subFile=m_chain->storageFile(fileName,KoStore::Read);

    if (!subFile)
    {
        kdError(30508) << "Could not get a device for sub-file: " << fileName << endl;
        return NULL;
    }
    return subFile;
}


bool KWEFKWordLeader::loadSubFile(const QString& fileName, QByteArray& array)
{
    KoStoreDevice* subFile;

    subFile=m_chain->storageFile(fileName,KoStore::Read);

    if (!subFile)
    {
        kdError(30508) << "Could not get a device for sub-file: " << fileName << endl;
        return false;
    }
    else if ( subFile->open ( IO_ReadOnly ) )
    {
        array = subFile->readAll();
        subFile->close ();
    }
    else
    {
        kdError(30508) << "Unable to open " << fileName << " sub-file" << endl;
        return false;
    }

    return true;
}

KoFilter::ConversionStatus KWEFKWordLeader::convert( KoFilterChain* chain,
    const QCString& from, const QCString& to)
{
    if ( from != "application/x-kword" )
    {
        return KoFilter::NotImplemented;
    }

    if (!chain)
    {
        kdError(30508) << "'Chain' is NULL! Internal error of the filter system?" << endl;
        return KoFilter::StupidError;
    }

    m_chain=chain;

    if ( !doOpenFile (chain->outputFile(),to) )
    {
        kdError (30508) << "Worker could not open export file! Aborting!" << endl;
        return KoFilter::StupidError;
    }

    if ( !doOpenDocument () )
    {
        kdError (30508) << "Worker could not open document! Aborting!" << endl;
        doAbortFile ();
        return KoFilter::StupidError;
    }

    KoStoreDevice* subFile;

    subFile=chain->storageFile("documentinfo.xml",KoStore::Read);
    kdDebug (30508) << "Processing documentinfo.xml..." << endl;
    // Do not care if we cannot open the document info.
    ProcessStoreFile (subFile, ProcessDocumentInfoTag, this);

    subFile=chain->storageFile("root",KoStore::Read);
    kdDebug (30508) << "Processing root..." << endl;
    if (!ProcessStoreFile (subFile, ProcessDocTag, this))
    {
        doAbortFile ();
        return KoFilter::StupidError;
    }

    doCloseDocument ();

    doCloseFile ();

    return KoFilter::OK;
}

bool KWEFKWordLeader::doFullDocument (const QValueList<ParaData>& paraList)
{
    if (!doOpenTextFrameSet())
        return false;
    if (!doFullAllParagraphs(paraList))
        return false;
    if (!doCloseTextFrameSet())
        return false;

    return true;
}

bool KWEFKWordLeader::doFullAllParagraphs (const QValueList<ParaData>& paraList)
{
    QValueList<ParaData>::ConstIterator it;
    for (it=paraList.begin();it!=paraList.end();it++)
    {
        if (!doFullParagraph((*it).text,(*it).layout,(*it).formattingList))
            return false;
    }
    return true;
}

QImage KWEFKWordLeader::loadAndConvertToImage(const QString& strName, const QString& inExtension) const
{
    QIODevice* io=getSubFileDevice(strName);
    if (!io)
    {
        // NO message error, as there must be already one
        return QImage();
    }

    kdDebug(30508) << "Picture " << strName << " has size: " << io->size() << endl;

    KoPicture picture;
    if (!picture.load(io, inExtension)) // we do not care about KoPictureKey
    {
        kdWarning(30508) << "Could not read picture: " << strName << " (KWEFBaseWorker::loadAndConvertToImage)" << endl;
        return QImage();
    }

    return picture.generateImage(picture.getOriginalSize()); // ### TODO: KoPicture::getOriginalSize is bad for cliparts
}

bool KWEFKWordLeader::loadAndConvertToImage(const QString& strName, const QString& inExtension, const QString& outExtension, QByteArray& image) const
{
    QImage qimage(loadAndConvertToImage(strName,inExtension));
    
    if (qimage.isNull())
    {
        kdWarning(30508) << "Could not load image (KWEFBaseWorker::loadAndConvertToImage)" <<endl;
        return false;
    }

    QImageIO imageIO;
    imageIO.setImage(qimage);

    QBuffer buffer(image); // A QBuffer is a QIODevice
    if (!buffer.open(IO_WriteOnly))
    {
        kdWarning(30508) << "Could not open buffer! (KWEFBaseWorker::loadAndConvertToImage)" << endl;
        return false;
    }

    imageIO.setIODevice(&buffer);
    imageIO.setFormat(outExtension.utf8());

    if (!imageIO.write())
    {
        kdWarning(30508) << "Could not write converted image! (KWEFBaseWorker::loadAndConvertToImage)" << endl;
        return false;
    }
    buffer.close();

    return true;
}


