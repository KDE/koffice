// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001, 2002 Nicolas GOUTTE <nicog@snafu.de>
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

#include <qfile.h>
#include <qdom.h>

#include <kdebug.h>

#include <koGlobal.h>
#include <koStore.h>

#include "KWEFStructures.h"
#include "TagProcessing.h"
#include "ProcessDocument.h"
#include "KWEFBaseWorker.h"
#include "KWEFKWordLeader.h"


static FrameAnchor *findAnchor ( QString              &name,
                                 QValueList<ParaData> &paraList )
{
    QValueList<ParaData>::Iterator paraIt;

    for ( paraIt = paraList.begin (); paraIt != paraList.end (); paraIt++ )
    {
        ValueListFormatData::Iterator formattingIt;

        for ( formattingIt = (*paraIt).formattingList.begin ();
              formattingIt != (*paraIt).formattingList.end ();
              formattingIt++ )
        {
            if ( (*formattingIt).id               == 6    &&
                 (*formattingIt).frameAnchor.name == name    )
            {
                return &(*formattingIt).frameAnchor;
            }
        }
    }

    return NULL;
}


static void ProcessParagraphTag ( QDomNode         myNode,
                                  void            *tagData,
                                  KWEFKWordLeader *leader )
{
#if 0
    kdError (30508) << "ProcessParagraphTag () - Begin" << endl;
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


    if ( paraData.formattingList.isEmpty () && !paraData.text.isEmpty () )
    {
        if ( paraData.layout.formatData.id == 1 )
        {
            paraData.formattingList << paraData.layout.formatData;
        }
        else
        {
            kdError (30508) << "No useful FORMAT tag found for text in PARAGRAPH" << endl;
        }
    }


    *paraList << paraData;

#if 0
    kdError (30508) << "ProcessParagraphTag () - End " << paraText << endl;
#endif
}


static void ProcessImageKeyTag ( QDomNode         myNode,
                                 void            *tagData,
                                 KWEFKWordLeader *leader )
{
    QString *key = (QString *) tagData;   // the name where the picture came from used as the identifier

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "year",     NULL,      NULL         )
                       << AttrProcessing ( "month",    NULL,      NULL         )
                       << AttrProcessing ( "day",      NULL,      NULL         )
                       << AttrProcessing ( "hour",     NULL,      NULL         )
                       << AttrProcessing ( "minute",   NULL,      NULL         )
                       << AttrProcessing ( "second",   NULL,      NULL         )
                       << AttrProcessing ( "msec",     NULL,      NULL         )
                       << AttrProcessing ( "filename", "QString", (void *) key );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode);
}


static void ProcessImageTag ( QDomNode         myNode,
                              void            *tagData,
                              KWEFKWordLeader *leader )
{
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
    kdError (30508) << "ProcessFramesetTag () - Begin" << endl;
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
    attrProcessingList << AttrProcessing ( "name",      "QString", (void *) &name      )
                       << AttrProcessing ( "frameType", "int",     (void *) &frameType )
                       << AttrProcessing ( "frameInfo", "int",     (void *) &frameInfo )
                       << AttrProcessing ( "removable", "",        NULL                )
                       << AttrProcessing ( "visible",   "",        NULL                )
                       << AttrProcessing ( "grpMgr",    "QString", (void *) &grpMgr    )
                       << AttrProcessing ( "row",       "int",     (void *) &row       )
                       << AttrProcessing ( "col",       "int",     (void *) &col       )
                       << AttrProcessing ( "rows",      "int",     (void *) &rows      )
                       << AttrProcessing ( "cols",      "int",     (void *) &cols      )
                        ;
    ProcessAttributes (myNode, attrProcessingList);

    switch ( frameType )
    {
        case 1:
            if ( grpMgr.isEmpty () )
            {
                // As we do not support anything else than normal text, process only normal text.
                // TODO: Treat the other types of frames (frameType)
                if (!frameInfo)
                {
                    // Normal Text
                    QValueList<TagProcessing> tagProcessingList;
                    tagProcessingList.append(TagProcessing ( "FRAME",     NULL,                NULL              ));
                    tagProcessingList.append(TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) paraList ));
                    ProcessSubtags (myNode, tagProcessingList, leader);
                }
            }
            else
            {
                if ( col != -1 && row != -1 )
                {
                    if ( cols == 1 && rows == 1 )
                    {
#if 0
                        kdError (30508) << "DEBUG - FRAMESET: table " << name << " col, row = "
                                        << col << ", " << row << endl;
#endif

                        FrameAnchor *frameAnchor = findAnchor (grpMgr, *paraList);

                        if ( frameAnchor )
                        {
                            frameAnchor->type = 6;

                            QValueList<ParaData> cellParaList;

                            QValueList<TagProcessing> tagProcessingList;
                            tagProcessingList << TagProcessing ( "FRAME",     NULL,                NULL                   )
                                              << TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) &cellParaList );
                            ProcessSubtags (myNode, tagProcessingList, leader);

                            frameAnchor->table.addCell (col, row, cellParaList);
                        }
                        else
                        {
                            kdError (30508) << "ProcessFramesetTag (): Couldn't find anchor " << name << endl;
                        }
                    }
                    else
                    {
                        kdError (30508) << "Unexpected value for one of, or all FRAMESET attribute cols, rows: "
                                        << cols << ", " << rows << "!" << endl;
                        AllowNoSubtags (myNode);
                    }
                }
                else
                {
                    kdError (30508) << "Unset value for one of, or all FRAMESET attributes col, row: "
                                    << col << ", " << row << "!" << endl;
                    AllowNoSubtags (myNode);
                }
            }
            break;

        case 2:
            {
#if 0
            kdError (30508) << "DEBUG: FRAMESET name of picture is " << name << endl;
#endif

            FrameAnchor *frameAnchor = findAnchor (name, *paraList);

            if ( frameAnchor )
            {
                frameAnchor->type = 2;

                QValueList<TagProcessing> tagProcessingList;
                tagProcessingList << TagProcessing ( "FRAME", NULL,            NULL                                  )
                                  << TagProcessing ( "IMAGE", ProcessImageTag, (void *) &frameAnchor->picture.key );
                ProcessSubtags (myNode, tagProcessingList, leader);

#if 0
                kdError (30508) << "DEBUG: FRAMESET IMAGE KEY filename of picture is " << frameAnchor->picture.key << endl;
#endif

                frameAnchor->name = frameAnchor->picture.key;
            }
            else
            {
                kdError (30508) << "ProcessFramesetTag (): Couldn't find anchor " << name << endl;
            }

            }
            break;

        default:
            kdError (30508) << "Unexpected frametype " << frameType << "!" << endl;
    }

#if 0
    kdError (30508) << "ProcessFramesetTag () - End" << endl;
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
    AllowNoAttributes (myNode);

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


static void ProcessPaperTag (QDomNode myNode, void *, KWEFKWordLeader *leader)
{

    int format      = -1;
    int orientation = -1;
    double width    = -1.0;
    double height   = -1.0;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "format",          "int",    (void *) &format      )
                       << AttrProcessing ( "width",           "double", (void *) &width       )
                       << AttrProcessing ( "height",          "double", (void *) &height      )
                       << AttrProcessing ( "orientation",     "int",    (void *) &orientation )
                       << AttrProcessing ( "columns",         "",       NULL                  )
                       << AttrProcessing ( "columnspacing",   "",       NULL                  )
                       << AttrProcessing ( "hType",           "",       NULL                  )
                       << AttrProcessing ( "fType",           "",       NULL                  )
                       << AttrProcessing ( "spHeadBody",      "",       NULL                  )
                       << AttrProcessing ( "spFootBody",      "",       NULL                  );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode);

    leader->doFullPaperFormat (format, width, height, orientation);
}


static void ProcessPixmapsKeyTag ( QDomNode         myNode,
                                   void            *tagData,
                                   KWEFKWordLeader *leader )
{
    QValueList<ParaData> *paraList = (QValueList<ParaData> *) tagData;

    QString   fileName;   // the name where the picture came from used as the identifier
    QString   name;       // the location where the picture is stored withing the kwd file
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "hour",     NULL,      NULL               )
                       << AttrProcessing ( "msec",     NULL,      NULL               )
                       << AttrProcessing ( "day",      NULL,      NULL               )
                       << AttrProcessing ( "minute",   NULL,      NULL               )
                       << AttrProcessing ( "second",   NULL,      NULL               )
                       << AttrProcessing ( "month",    NULL,      NULL               )
                       << AttrProcessing ( "year",     NULL,      NULL               )
                       << AttrProcessing ( "filename", "QString", (void *) &fileName )
                       << AttrProcessing ( "name",     "QString", (void *) &name     );
    ProcessAttributes (myNode, attrProcessingList);


    FrameAnchor *frameAnchor = findAnchor (fileName, *paraList);

    if ( frameAnchor )
    {
#if 0
        kdError (30508) << "DEBUG: ProcessPixmapsKeyTag (): koStore for picture "
                        << fileName << " is " << name << endl;
#endif

        frameAnchor->picture.koStoreName = name;
    }
    else
    {
        kdError (30508) << "Could find anchor for picture " << fileName << "!" << endl;
    }


    AllowNoSubtags (myNode);
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


static void ProcessDocTag ( QDomNode         myNode,
                            void             *tagData,
                            KWEFKWordLeader  *leader )
{
#if 0
    kdDebug (30508) << "ProcessDocTag () - Begin" << endl;
#endif

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "editor",        "", NULL )
                       << AttrProcessing ( "mime",          "", NULL )
                       << AttrProcessing ( "syntaxVersion", "", NULL );
    ProcessAttributes (myNode, attrProcessingList);

    // TODO: verify syntax version and perhaps mime

    leader->doOpenHead();

    // At first, process <PAPER>, even if mostly the output will need to be delayed.
    QDomNode nodePaper=myNode.namedItem("PAPER");
    if ( nodePaper.isNull () )
        kdWarning (30508) << "No <PAPER>" << endl;
    else
        ProcessPaperTag (nodePaper, NULL, leader);

    // Then we process the styles
    QDomNode nodeStyles=myNode.namedItem("STYLES");
    if ( nodeStyles.isNull () )
        kdWarning (30508) << "No <STYLES>" << endl;
    else
        ProcessStylesPluralTag (nodeStyles, NULL, leader);

    leader->doCloseHead();
    leader->doOpenBody();

    QValueList<TagProcessing> tagProcessingList;
    QValueList<ParaData> paraList;

    tagProcessingList << TagProcessing ( "PAPER",       NULL,                   NULL               ) // Already done
                      << TagProcessing ( "ATTRIBUTES",  NULL,                   NULL               )
                      << TagProcessing ( "FRAMESETS",   ProcessFramesetsTag,    (void *) &paraList )
                      << TagProcessing ( "STYLES",      NULL,                   NULL               ) // Already done
                      << TagProcessing ( "PIXMAPS",     ProcessPixmapsTag,      (void *) &paraList )
                      << TagProcessing ( "EMBEDDED",    NULL,                   NULL               )
                      ;
// TODO: delete (they do not exist anymore in KOffice 1.2)
// tagProcessingList << TagProcessing ( "SERIALL",     NULL,                   NULL               );
// tagProcessingList << TagProcessing ( "FOOTNOTEMGR", NULL,                   NULL               );
// tagProcessingList << TagProcessing ( "CLIPARTS",    NULL,                   NULL               );

    ProcessSubtags (myNode, tagProcessingList, leader);

    leader->doFullDocument (paraList);

    FreeCellParaLists (paraList);

    leader->doCloseBody();

#if 0
    kdDebug (30508) << "ProcessDocTag () - End" << endl;
#endif
}


void KWEFKWordLeader::setWorker ( KWEFBaseWorker *newWorker )
{
    m_worker = newWorker;

    if (newWorker)
        newWorker->registerKWordLeader(this);
}


KWEFBaseWorker *KWEFKWordLeader::getWorker(void) const
{
    return m_worker;
}


// Short simple definition for methods with void parameter
#define DO_VOID_DEFINITION(string) \
    bool KWEFKWordLeader::string() \
    {\
        if (m_worker) \
            return m_worker->string(); \
        return false; \
    }


bool KWEFKWordLeader::doOpenFile ( const QString &filenameOut, const QString &to )
{
    if ( m_worker )
        return m_worker->doOpenFile (filenameOut, to);

    // As it would be the first method to be called, warn if worker is NULL
    kdError (30508) << "No Worker! (in KWEFKWordLeader::doOpenFile)" << endl;

    return false;
}


DO_VOID_DEFINITION (doCloseFile)
DO_VOID_DEFINITION (doAbortFile)
DO_VOID_DEFINITION (doOpenDocument)
DO_VOID_DEFINITION (doCloseDocument)
DO_VOID_DEFINITION (doOpenStyles)
DO_VOID_DEFINITION (doCloseStyles)

bool KWEFKWordLeader::doFullDocumentInfo (const KWEFDocumentInfo &docInfo)
{
    if ( m_worker )
        return m_worker->doFullDocumentInfo (docInfo);

    return false;
}


bool KWEFKWordLeader::doFullDocument (const QValueList<ParaData> &paraList)
{
    if ( m_worker )
        return m_worker->doFullDocument (paraList);

    return false;
}


DO_VOID_DEFINITION (doOpenHead)
DO_VOID_DEFINITION (doCloseHead)
DO_VOID_DEFINITION (doOpenBody)
DO_VOID_DEFINITION (doCloseBody)


bool KWEFKWordLeader::doFullPaperFormat ( const int format, const double width, const double height, const int orientation )
{
    if ( m_worker )
        return m_worker->doFullPaperFormat (format, width, height, orientation);

    return false;
}


bool KWEFKWordLeader::doFullDefineStyle ( LayoutData &layout )
{
    if ( m_worker )
        return m_worker->doFullDefineStyle (layout);

    return false;
}


static bool ProcessStoreFile ( QByteArray       &byteArrayIn,
                               void            (*processor) (QDomNode, void *, KWEFKWordLeader *),
                               KWEFKWordLeader  *leader )
{
    QDomDocument qDomDocumentIn;

    QString errorMsg;
    int errorLine;
    int errorColumn;

    if ( !qDomDocumentIn.setContent (byteArrayIn, &errorMsg, &errorLine, &errorColumn) )
    {
        kdError (30508) << "Parsing Error! Aborting! (in ProcessStoreFile)" << endl
            << "  Line: " << errorLine << " Column: " << errorColumn << endl
            << "  Message: " << errorMsg << endl;

        return false;
    }

#if 0
    kdDebug (30508) << "DOM document type "
                    << qDomDocumentIn.doctype ().name () << "." << endl;
#endif

    QDomNode docNode = qDomDocumentIn.documentElement ();

    processor (docNode, NULL, leader);

    return true;
}

bool KWEFKWordLeader::loadKoStoreFile(const QString& fileName, QByteArray& array)
{
    if (m_filenameIn.isEmpty())
    {
        // We have no KoStore, so we cannot offer anything!
        kdWarning (30508) << "No KoStore to get file from!" << endl;
        return false;
    }

    KoStore koStore (m_filenameIn, KoStore::Read); // TODO

    if ( koStore.open ( fileName ) )
    {
        array = koStore.read ( koStore.size () );
        koStore.close ();
    }
    else
    {
        kdWarning (30508) << "Unable to open " << fileName << " sub-file in KoStore " << m_filenameIn << endl;
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

    // We must save the input file name, as KoFilterChain::inputFile can only be called once!!
    m_filenameIn=chain->inputFile();
    kdDebug(30508) << "Setting m_filenameIn: " << m_filenameIn << endl;

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

    KoStore koStoreIn (m_filenameIn, KoStore::Read);  // TODO
    QByteArray byteArrayIn;

    if ( koStoreIn.open ( "documentinfo.xml" ) )
    {
        byteArrayIn = koStoreIn.read ( koStoreIn.size () );
        koStoreIn.close ();

        kdDebug (30508) << "Processing Document Info..." << endl;
        ProcessStoreFile (byteArrayIn, ProcessDocumentInfoTag, this);
    }
    else
    {
        // Note: we do not worry too much if we cannot open the document info!
        kdWarning (30508) << "Unable to open documentinfo.xml sub-file!" << endl;
    }

    if ( koStoreIn.open ( "root" ) )
    {
        byteArrayIn = koStoreIn.read ( koStoreIn.size () );
        koStoreIn.close ();

        kdDebug (30508) << "Processing KWord File (KoStore)..." << endl;
        ProcessStoreFile (byteArrayIn, ProcessDocTag, this);
    }
    else
    {
        // We were not able to open maindoc.xml
        // But perhaps we have an untarred, uncompressed file
        //  (it might happen with koconverter)
        QFile file (m_filenameIn);
        if (file.open (IO_ReadOnly))
        {
            byteArrayIn = file.readAll ();
            file.close ();

            // Do *not* set m_filenameIn as we have not any KoStore
	    m_filenameIn=QString::null; // PROVISORY

            kdDebug (30508) << "Processing KWord File (QFile)..." << endl;
            ProcessStoreFile (byteArrayIn, ProcessDocTag, this);
        }
        else
        {
            kdError (30508) << "Unable to open input file!" << endl;
            doAbortFile ();
            return KoFilter::StupidError;
        }
    }

    doCloseDocument ();

    doCloseFile ();

    return KoFilter::OK;
}
