// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001 Nicolas GOUTTE <nicog@snafu.de>

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

#ifndef PROCESS_DOCUMENT_H
#define PROCESS_DOCUMENT_H

#include <qdom.h>
#include <KWEFBaseClass.h>

void ProcessLayoutNameTag ( QDomNode myNode, void *tagData, QString &, KWEFBaseClass* );
void ProcessLayoutFlowTag ( QDomNode myNode, void *tagData, QString &, KWEFBaseClass* );
void ProcessCounterTag ( QDomNode myNode, void *tagData, QString &, KWEFBaseClass* );
void ProcessSingleFormatTag (QDomNode myNode, void *tagData, QString &, KWEFBaseClass* exportFilter);
void ProcessIndentsTag (QDomNode myNode, void* tagData , QString&, KWEFBaseClass*);
void ProcessLayoutTag ( QDomNode myNode, void *tagData, QString &outputText, KWEFBaseClass* exportFilter );
void ProcessFormatsTag ( QDomNode myNode, void *tagData, QString &outputText, KWEFBaseClass* exportFilter );
void ProcessTextTag ( QDomNode myNode, void *tagData, QString &, KWEFBaseClass*);

#endif /* PROCESS_DOCUMENT_H */