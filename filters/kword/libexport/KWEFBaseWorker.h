// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001, 2002 Nicolas GOUTTE <nicog@snafu.de>

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

#ifndef KWEF_BASEWORKER_H
#define KWEF_BASEWORKER_H

#include <qstring.h>
#include <qiodevice.h>

#include <KWEFStructures.h>

class KWEFKWordLeader;

class KWEFBaseWorker
{
    public:
        KWEFBaseWorker(void) : m_kwordLeader(NULL) {}
        virtual ~KWEFBaseWorker(void) {}
    public:
        void registerKWordLeader(KWEFKWordLeader* leader);
    public: // callbacks to Leader
        bool loadKoStoreFile(const QString& fileName, QByteArray& array); // DEPRECATED (use loadSubFile)
        bool loadSubFile(const QString& fileName, QByteArray& array);
        QIODevice* getSubFileDevice(const QString& fileName);
    public: // leader/worker functions
        virtual bool doOpenFile (const QString& filenameOut, const QString& to);
        virtual bool doCloseFile (void); // Close file in normal conditions
        virtual bool doAbortFile (void); // Close file after errors
        virtual bool doOpenDocument (void); // Like HTML's <html>
        virtual bool doCloseDocument (void); // Like HTML's </html>
        virtual bool doOpenTextFrameSet (void); // Like AbiWord's <section>
        virtual bool doCloseTextFrameSet (void); // Like AbiWord's </section>
        virtual bool doFullDocumentInfo (const KWEFDocumentInfo &docInfo);
        virtual bool doFullDocument (const QValueList<ParaData> &);
        virtual bool doFullAllParagraphs (const QValueList<ParaData>& paraList);
        virtual bool doFullParagraph(const QString& paraText, const LayoutData& layout,
            const ValueListFormatData& paraFormatDataList);
        virtual bool doFullPaperFormat (const int format,
            const double width, const double height, const int orientation); // Like AbiWord's <papersize>
        virtual bool doFullPaperBorders (const double top, const double left,
            const double bottom, const double right); // Like KWord's <PAPERBORDERS>
        virtual bool doOpenHead (void); // Like HTML's <HEAD>
        virtual bool doCloseHead (void); // Like HTML's </HEAD>
        virtual bool doOpenBody (void); // Like HTML's <BODY>
        virtual bool doCloseBody (void); // Like HTML's </BODY>
        virtual bool doOpenStyles (void); // Like HTML's <style>
        virtual bool doCloseStyles (void); // Like HTML's </style>
        virtual bool doFullDefineStyle (LayoutData& layout); // Defines a single style
    protected:
        KWEFKWordLeader* m_kwordLeader;
};

#endif /* KWEF_BASEWORKER_H */
