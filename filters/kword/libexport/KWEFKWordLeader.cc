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

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList
        << AttrProcessing ( "runaround", NULL,      NULL                 )
        << AttrProcessing ( "top",       "double",  &frameAnchor->top    )
        << AttrProcessing ( "bottom",    "double",  &frameAnchor->bottom )
        << AttrProcessing ( "left",      "double",  &frameAnchor->left   )
        << AttrProcessing ( "right",     "double",  &frameAnchor->right  )
        ;
    ProcessAttributes (myNode, attrProcessingList);

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
                       << AttrProcessing ( "protectSize","",       NULL                )
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
                        kdDebug (30508) << "DEBUG - FRAMESET: table " << name << " col, row = "
                                        << col << ", " << row << endl;
#endif

                        FrameAnchor *frameAnchor = findAnchor (grpMgr, *paraList);

                        if ( frameAnchor )
                        {
                            frameAnchor->type = 6;

                            QValueList<ParaData> cellParaList;

                            QValueList<TagProcessing> tagProcessingList;
                            tagProcessingList << TagProcessing ( "FRAME",     ProcessFrameTag,     frameAnchor   )
                                              << TagProcessing ( "PARAGRAPH", ProcessParagraphTag, &cellParaList );
                            ProcessSubtags (myNode, tagProcessingList, leader);

                            frameAnchor->table.addCell (col, row, cellParaList);
                        }
                        else
                        {
                            kdWarning (30508) << "ProcessFramesetTag (): Couldn't find anchor " << name << endl;
                        }
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

    leader->doFullPaperFormat (format, width, height, orientation);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList
        << TagProcessing ( "PAPERBORDERS", ProcessPaperBordersTag, NULL )
        ;

    ProcessSubtags (myNode, tagProcessingList, leader);
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


/*static*/ void ProcessDocTag ( QDomNode         myNode,
    void* /*tagData*/, KWEFKWordLeader* leader )
{
    //kdDebug (30508) << "Entering ProcessDocTag" << endl;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "editor",        "", NULL )
                       << AttrProcessing ( "mime",          "", NULL )
                       << AttrProcessing ( "syntaxVersion", "", NULL );
    ProcessAttributes (myNode, attrProcessingList);

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

    leader->doFullDocument (paraList);

    FreeCellParaLists (paraList);

    leader->doCloseBody();

    //kdDebug (30508) << "Exiting ProcessDocTag" << endl;
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
DO_VOID_DEFINITION (doOpenHead)
DO_VOID_DEFINITION (doCloseHead)
DO_VOID_DEFINITION (doOpenBody)
DO_VOID_DEFINITION (doCloseBody)
DO_VOID_DEFINITION (doOpenSpellCheckIgnoreList)
DO_VOID_DEFINITION (doCloseSpellCheckIgnoreList)

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


bool KWEFKWordLeader::doFullPaperFormat ( const int format, const double width, const double height, const int orientation )
{
    if ( m_worker )
        return m_worker->doFullPaperFormat (format, width, height, orientation);

    return false;
}

bool KWEFKWordLeader::doFullPaperBorders (const double top, const double left, const double bottom, const double right)
{
    if ( m_worker )
        return m_worker->doFullPaperBorders (top, left, bottom, right);

    return false;
}


bool KWEFKWordLeader::doFullDefineStyle ( LayoutData &layout )
{
    if ( m_worker )
        return m_worker->doFullDefineStyle (layout);

    return false;
}

bool KWEFKWordLeader::doFullSpellCheckIgnoreWord (const QString& ignoreword)
{
    if ( m_worker )
        return m_worker->doFullSpellCheckIgnoreWord (ignoreword);

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

QIODevice* KWEFKWordLeader::getSubFileDevice(const QString& fileName)
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
