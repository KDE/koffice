// -*- Mode: c++-mode; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2001 Toshitaka Fujioka <fujioka@kde.org>

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

#ifndef NOTEBAR_H
#define NOTEBAR_H

#include <qwidget.h>

class QPainter;
class QLabel;

class KPresenterDoc;
class KPresenterView;
class KPrinter;
class KTextEdit;

class NoteBar : public QWidget
{
    Q_OBJECT

public:
    NoteBar( QWidget *_parent, KPresenterView *_view );
    ~NoteBar();

    void setCurrentNoteText( const QString &_text );

    // print
    void printNotes( QPainter *_painter, KPrinter *_printer, QValueList<int> );

    QString getNotesTextForPrinting(QValueList<int>) const;

private slots:
    void slotTextChanged();
    void slotSelectionChanged();
    void slotCopyAvailable( bool );
    void slotUndoAvailable( bool );
    void slotRedoAvailable( bool );

private:
    ::KTextEdit *textEdit;
    QLabel *label;

    KPresenterView *view;

    bool initialize;
};

#endif
