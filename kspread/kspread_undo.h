/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __kspread_undo_h__
#define __kspread_undo_h__

class KSpreadUndo;
class KSpreadUndoAction;
class KSpreadTable;
class KSpreadLayout;
class KSpreadDoc;
class ColumnLayout;
class RowLayout;

#include "kspread_cell.h"
#include <qptrstack.h>
#include <qstring.h>
#include <qrect.h>
#include <qptrlist.h>
#include <qvaluelist.h>

struct rowSize {
int rowNumber;
double rowHeight;
};

struct columnSize {
int columnNumber;
double columnWidth;
};

struct textOfCell {
int row;
int col;
QString text;
};

struct layoutCell {
int row;
int col;
KSpreadLayout *l;
};

struct layoutColumn {
int col;
ColumnLayout *l;
};

struct layoutRow {
int row;
RowLayout *l;
};

struct styleCell {
  int row;
  int col;
  KSpreadCell::Style style;
  QString action;
};

/**
 * Abstract base class. Every undo/redo action must
 * derive from this class.
 */
class KSpreadUndoAction
{
public:
    KSpreadUndoAction( KSpreadDoc *_doc ) { m_pDoc = _doc; }
    virtual ~KSpreadUndoAction() { }

    virtual void undo() = 0;
    virtual void redo() = 0;

    KSpreadDoc* doc() { return m_pDoc; }

    QString getName() {return name ;}

// #### To be private

protected:
    KSpreadDoc *m_pDoc;
    QString name;
};

class KSpreadMacroUndoAction : public KSpreadUndoAction
{
public:
    KSpreadMacroUndoAction( KSpreadDoc *_doc,const QString& _name );
    virtual ~KSpreadMacroUndoAction();

    void addCommand(KSpreadUndoAction *command);

    virtual void undo();
    virtual void redo();

protected:
    QPtrList<KSpreadUndoAction> m_commands;
};

class KSpreadUndoRemoveColumn : public KSpreadUndoAction
{
public:
    KSpreadUndoRemoveColumn( KSpreadDoc *_doc, KSpreadTable *_table, int _column,int _nbCol=0 );
    virtual ~KSpreadUndoRemoveColumn();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
    QCString m_data;
    int m_iColumn;
    int m_iNbCol;
};

class KSpreadUndoInsertColumn : public KSpreadUndoAction
{
public:
    KSpreadUndoInsertColumn( KSpreadDoc *_doc, KSpreadTable *_table, int _column,int _nbCol=0 );
    virtual ~KSpreadUndoInsertColumn();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
    int m_iColumn;
    int m_iNbCol;
};

class KSpreadUndoRemoveRow : public KSpreadUndoAction
{
public:
    KSpreadUndoRemoveRow( KSpreadDoc *_doc, KSpreadTable *_table, int _row,int _nbRow=0 );
    virtual ~KSpreadUndoRemoveRow();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
    QCString m_data;
    int m_iRow;
    int m_iNbRow;
};

class KSpreadUndoInsertRow : public KSpreadUndoAction
{
public:
    KSpreadUndoInsertRow( KSpreadDoc *_doc, KSpreadTable *_table, int _row,int _nbRow=0 );
    virtual ~KSpreadUndoInsertRow();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
    int m_iRow;
    int m_iNbRow;
};


class KSpreadUndoHideColumn : public KSpreadUndoAction
{
public:
    KSpreadUndoHideColumn( KSpreadDoc *_doc, KSpreadTable *_table, int _column,int _nbCol=0, QValueList<int>listCol=QValueList<int>() );
    virtual ~KSpreadUndoHideColumn();

    virtual void undo();
    virtual void redo();
    void createList( QValueList<int>&list,KSpreadTable *_tab );

protected:
    QString m_tableName;
    int m_iColumn;
    int m_iNbCol;
    QValueList<int> listCol;
};

class KSpreadUndoHideRow : public KSpreadUndoAction
{
public:
    KSpreadUndoHideRow( KSpreadDoc *_doc, KSpreadTable *_table, int _column,int _nbCol=0, QValueList<int>_listRow=QValueList<int>() );
    virtual ~KSpreadUndoHideRow();

    virtual void undo();
    virtual void redo();
    void createList( QValueList<int>&list,KSpreadTable *_tab );

protected:
    QString m_tableName;
    int m_iRow;
    int m_iNbRow;
    QValueList<int> listRow;
};

class KSpreadUndoShowColumn : public KSpreadUndoAction
{
public:
    KSpreadUndoShowColumn( KSpreadDoc *_doc, KSpreadTable *_table, int _column,int _nbCol=0, QValueList<int>_list=QValueList<int>() );
    virtual ~KSpreadUndoShowColumn();

    virtual void undo();
    virtual void redo();
    void createList( QValueList<int>&list,KSpreadTable *_tab );

protected:
    QString m_tableName;
    int m_iColumn;
    int m_iNbCol;
    QValueList<int> listCol;
};

class KSpreadUndoShowRow : public KSpreadUndoAction
{
public:
    KSpreadUndoShowRow( KSpreadDoc *_doc, KSpreadTable *_table, int _column,int _nbCol=0, QValueList<int>list=QValueList<int>() );
    virtual ~KSpreadUndoShowRow();

    virtual void undo();
    virtual void redo();
    void createList( QValueList<int>&list,KSpreadTable *_tab );

protected:
    QString m_tableName;
    int m_iRow;
    int m_iNbRow;
    QValueList<int> listRow;
};



class KSpreadUndoSetText : public KSpreadUndoAction
{
public:
    KSpreadUndoSetText( KSpreadDoc *_doc, KSpreadTable *_table, const QString& _text, int _column, int _row,KSpreadCell::FormatType _formatType );
    virtual ~KSpreadUndoSetText();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
    int m_iRow;
    int m_iColumn;
    QString m_strText;
    QString m_strRedoText;
    KSpreadCell::FormatType m_eFormatType;
    KSpreadCell::FormatType m_eFormatTypeRedo;
};

class KSpreadUndoCellLayout : public KSpreadUndoAction
{
public:
    KSpreadUndoCellLayout( KSpreadDoc *_doc, KSpreadTable *_table, QRect &_selection, QString &_title );
    virtual ~KSpreadUndoCellLayout();

    virtual void undo();
    virtual void redo();

    void copyLayout( QValueList<layoutCell> &list,QValueList<layoutColumn> &listCol,QValueList<layoutRow> &listRow, KSpreadTable* table );

protected:
    QRect m_rctRect;
    QValueList<layoutCell> m_lstLayouts;
    QValueList<layoutCell> m_lstRedoLayouts;
    QValueList<layoutColumn> m_lstColLayouts;
    QValueList<layoutColumn> m_lstRedoColLayouts;
    QValueList<layoutRow> m_lstRowLayouts;
    QValueList<layoutRow> m_lstRedoRowLayouts;

    QString m_tableName;
};

class KSpreadUndoDelete : public KSpreadUndoAction
{
public:
    KSpreadUndoDelete( KSpreadDoc *_doc, KSpreadTable *_table, QRect &_rect );
    virtual ~KSpreadUndoDelete();

    virtual void undo();
    virtual void redo();

    void createListCell( QCString &listCell,QValueList<columnSize> &listCol,QValueList<rowSize> &listRow, KSpreadTable* table );

protected:
    QRect m_selection;
    QCString m_data;
    QCString m_dataRedo;
    QValueList<columnSize> m_lstColumn;
    QValueList<columnSize> m_lstRedoColumn;
    QValueList<rowSize> m_lstRow;
    QValueList<rowSize> m_lstRedoRow;
    QString m_tableName;
};

class KSpreadUndoSetTableName : public KSpreadUndoAction
{
public:
    KSpreadUndoSetTableName( KSpreadDoc *doc, KSpreadTable *table, const QString& name );
    virtual ~KSpreadUndoSetTableName();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
    QString m_name;
    QString m_redoName;
};

class KSpreadUndoResizeColRow : public KSpreadUndoAction
{
public:
    KSpreadUndoResizeColRow( KSpreadDoc *_doc, KSpreadTable *_table, QRect &_selection );
    virtual ~KSpreadUndoResizeColRow();

    virtual void undo();
    virtual void redo();

    void createList( QValueList<columnSize> &listCol,QValueList<rowSize> &listRow, KSpreadTable* table );

protected:
    QRect m_rctRect;
    QValueList<columnSize> m_lstColumn;
    QValueList<columnSize> m_lstRedoColumn;
    QValueList<rowSize> m_lstRow;
    QValueList<rowSize> m_lstRedoRow;
    QString m_tableName;
};

class KSpreadUndoChangeAreaTextCell : public KSpreadUndoAction
{
public:
    KSpreadUndoChangeAreaTextCell( KSpreadDoc *_doc, KSpreadTable *_table, QRect &_selection );
    virtual ~KSpreadUndoChangeAreaTextCell();

    virtual void undo();
    virtual void redo();

    void createList( QValueList<textOfCell> &list, KSpreadTable* table );

protected:
    QRect m_rctRect;
    QValueList<textOfCell> m_lstTextCell;
    QValueList<textOfCell> m_lstRedoTextCell;
    QString m_tableName;
};

class KSpreadUndoMergedCell : public KSpreadUndoAction
{
public:
    KSpreadUndoMergedCell( KSpreadDoc *_doc, KSpreadTable *_table, int _column, int _row, int _extraX,int _extraY);
    virtual ~KSpreadUndoMergedCell();

    virtual void undo();
    virtual void redo();

protected:
    int m_iRow;
    int m_iCol;
    int m_iExtraX;
    int m_iExtraY;
    int m_iExtraRedoX;
    int m_iExtraRedoY;
    QString m_tableName;
};


class KSpreadUndoAutofill : public KSpreadUndoAction
{
public:
    KSpreadUndoAutofill( KSpreadDoc *_doc, KSpreadTable *_table, QRect &_rect );
    virtual ~KSpreadUndoAutofill();

    virtual void undo();
    virtual void redo();
    void createListCell( QCString &list, KSpreadTable* table );
protected:
    QRect m_selection;
    QCString m_data;
    QCString m_dataRedo;
    QString m_tableName;
};

class KSpreadUndoInsertCellCol : public KSpreadUndoAction
{
public:
    KSpreadUndoInsertCellCol( KSpreadDoc *_doc, KSpreadTable *_table, QRect _rect );
    virtual ~KSpreadUndoInsertCellCol();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
    QRect m_rect;
};

class KSpreadUndoInsertCellRow : public KSpreadUndoAction
{
public:
    KSpreadUndoInsertCellRow( KSpreadDoc *_doc, KSpreadTable *_table,QRect _rect );
    virtual ~KSpreadUndoInsertCellRow();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
    QRect m_rect;
};

class KSpreadUndoRemoveCellCol : public KSpreadUndoAction
{
public:
    KSpreadUndoRemoveCellCol( KSpreadDoc *_doc, KSpreadTable *_table, QRect _rect );
    virtual ~KSpreadUndoRemoveCellCol();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
    QRect m_rect;
    QCString m_data;
};

class KSpreadUndoRemoveCellRow : public KSpreadUndoAction
{
public:
    KSpreadUndoRemoveCellRow( KSpreadDoc *_doc, KSpreadTable *_table, QRect _rect );
    virtual ~KSpreadUndoRemoveCellRow();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
    QRect m_rect;
    QCString m_data;
};

class KSpreadUndoConditional : public KSpreadUndoAction
{
public:
    KSpreadUndoConditional( KSpreadDoc *_doc, KSpreadTable *_table, QRect &_rect );
    virtual ~KSpreadUndoConditional();

    virtual void undo();
    virtual void redo();
    void createListCell( QCString &list, KSpreadTable* table );
protected:
    QRect m_selection;
    QCString m_data;
    QCString m_dataRedo;
    QString m_tableName;
};

class KSpreadUndoHideTable : public KSpreadUndoAction
{
public:
    KSpreadUndoHideTable( KSpreadDoc *_doc, KSpreadTable *_table );
    virtual ~KSpreadUndoHideTable();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
};

class KSpreadUndoShowTable : public KSpreadUndoAction
{
public:
    KSpreadUndoShowTable( KSpreadDoc *_doc, KSpreadTable *_table );
    virtual ~KSpreadUndoShowTable();

    virtual void undo();
    virtual void redo();

protected:
    QString m_tableName;
};


class KSpreadUndoCellPaste : public KSpreadUndoAction
{
public:
    KSpreadUndoCellPaste( KSpreadDoc *_doc, KSpreadTable *_table,int _nbCol,int _nbRow, int _xshift,int _yshift, QRect &_selection,bool insert,int insertTo=0 );
    virtual ~KSpreadUndoCellPaste();

    virtual void undo();
    virtual void redo();
    void createListCell( QCString &listCell,QValueList<columnSize> &listCol,QValueList<rowSize> &listRow, KSpreadTable* table );

protected:
    QRect m_selection;
    QCString m_data;
    QCString m_dataRedo;
    QValueList<columnSize> m_lstColumn;
    QValueList<columnSize> m_lstRedoColumn;
    QValueList<rowSize> m_lstRow;
    QValueList<rowSize> m_lstRedoRow;
    int nbCol;
    int nbRow;
    int xshift;
    int yshift;
    bool  b_insert;
    int m_iInsertTo;
    QString m_tableName;
};


class KSpreadUndoStyleCell : public KSpreadUndoAction
{
public:
    KSpreadUndoStyleCell( KSpreadDoc *_doc, KSpreadTable *_table, QRect &_rect );
    virtual ~KSpreadUndoStyleCell();

    virtual void undo();
    virtual void redo();

    void createListCell( QValueList<styleCell> &listCell, KSpreadTable* table );

protected:
    QRect m_selection;
    QValueList<styleCell> m_lstStyleCell;
    QValueList<styleCell> m_lstRedoStyleCell;
    QString m_tableName;
};



class KSpreadUndo
{
public:
    KSpreadUndo( KSpreadDoc *_doc );
    ~KSpreadUndo();

    void undo();
    void redo();
    void clear();

    void lock() { m_bLocked = TRUE; }
    void unlock() { m_bLocked = FALSE; }
    bool isLocked() { return m_bLocked; }

    bool hasUndoActions() { return !m_stckUndo.isEmpty(); }
    bool hasRedoActions() { return !m_stckRedo.isEmpty(); }

    void appendUndo( KSpreadUndoAction *_action );

    QString getUndoName();
    QString getRedoName();

protected:
    QPtrStack<KSpreadUndoAction> m_stckUndo;
    QPtrStack<KSpreadUndoAction> m_stckRedo;

    KSpreadDoc *m_pDoc;

    bool m_bLocked;
};

#endif
