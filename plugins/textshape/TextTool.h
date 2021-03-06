/* This file is part of the KDE project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KODFTEXTTOOL_H
#define KODFTEXTTOOL_H

#include "TextShape.h"

#include <KToolBase.h>

#include <QtGui/QClipboard>
#include <QtCore/QHash>
#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>
#include <QtCore/QTimer>
#include <QtCore/QWeakPointer>

class TextEditingPluginContainer;
class InsertCharacter;
class KChangeTracker;
class KCharacterStyle;
class KoColor;
class KoColorPopupAction;
class KParagraphStyle;
class KStyleManager;
class KTextEditingPlugin;
class KoTextEditor;
class UndoTextCommand;

class KAction;
class KFontSizeAction;
class KFontAction;

class QUndoCommand;

class MockCanvas;

/**
 * This is the tool for the text-shape (which is a flake-based plugin).
 */
class TextTool : public KToolBase
{
    Q_OBJECT
public:
    explicit TextTool(KCanvasBase *canvas);
#ifndef NDEBUG
    explicit TextTool(MockCanvas *canvas);
#endif
    ~TextTool();

    /// reimplemented from superclass
    virtual void paint(QPainter &painter, const KViewConverter &converter);

    /// reimplemented from superclass
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
    /// reimplemented from superclass
    virtual void deactivate();
    /// reimplemented from superclass
    virtual void copy() const;
    ///reimplemented
    virtual void deleteSelection();
    /// reimplemented from superclass
    virtual void cut();
    /// reimplemented from superclass
    virtual bool paste();
    /// reimplemented from superclass
    virtual void repaintDecorations();

    /// reimplemented from superclass
    virtual KToolSelection* selection();
    /// reimplemented from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query, const KViewConverter &converter) const;
    /// reimplemented from superclass
    virtual void inputMethodEvent(QInputMethodEvent * event);

    bool isBidiDocument() const;


    /// The following two methods allow an undo/redo command to tell the tool, it will modify the QTextDocument and wants to be parent of the undo/redo commands resulting from these changes.

    void startEditing(QUndoCommand* command);

    void stopEditing();

    const QTextCursor cursor();

    void setShapeData(KTextShapeData *data);

    KoTextEditor *textEditor() { return m_textEditor.data(); }

    QRectF textRect(int startPosition, int endPosition) const;

protected:
    /// reimplemented from superclass
    virtual void shortcutOverride(QKeyEvent *event);
    /// reimplemented from superclass
    virtual void mousePressEvent(KPointerEvent *event) ;
    /// reimplemented from superclass
    virtual void mouseDoubleClickEvent(KPointerEvent *event);
    /// reimplemented from superclass
    virtual void mouseMoveEvent(KPointerEvent *event);
    /// reimplemented from superclass
    virtual void mouseReleaseEvent(KPointerEvent *event);
    /// reimplemented from superclass
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented from superclass
    virtual void keyReleaseEvent(QKeyEvent *event);
    /// reimplemented from superclass
    virtual QWidget *createOptionWidget();


public slots:
    /// start the textedit-plugin.
    void startTextEditingPlugin(const QString &pluginId);
    /// add a command to the undo stack, executing it as well.
    void addCommand(QUndoCommand *command);
    /// reimplemented from KToolBase
    virtual void resourceChanged(int key, const QVariant &res);
    //When enabled, display changes
    void toggleShowChanges(bool);
    /// When enabled, make the change tracker record changes made while typing
    void toggleRecordChanges(bool);
    /// Configure Change Tracking
    void configureChangeTracking();
    /// call this when the 'is-bidi' boolean has been changed.
    void isBidiUpdated();

signals:
    /// emitted every time a different styleManager is set.
    void styleManagerChanged(KStyleManager *manager);
    /// emitted every time a caret move leads to a different character format being under the caret
    void charFormatChanged(const QTextCharFormat &format);
    /// emitted every time a caret move leads to a different paragraph format being under the caret
    void blockFormatChanged(const QTextBlockFormat &format);
    /// emitted every time a caret move leads to a different paragraph format being under the caret
    void blockChanged(const QTextBlock &block);

private slots:
    /// make the selected text bold or not
    void bold(bool);
    /// make the selected text italic or not
    void italic(bool);
    /// underline of the selected text
    void underline(bool underline);
    /// strikethrough of the selected text
    void strikeOut(bool strikeOut);
    /// insert a non breaking space at the caret position
    void nonbreakingSpace();
    /// insert a non breaking hyphen at the caret position
    void nonbreakingHyphen();
    /// insert a soft hyphen at the caret position
    void softHyphen();
    /// insert a linebreak at the caret position
    void lineBreak();
    /// align all of the selected text left
    void alignLeft();
    /// align all of the selected text right
    void alignRight();
    /// align all of the selected text centered
    void alignCenter();
    /// align all of the selected text block-justified
    void alignBlock();
    /// make the selected text switch to be super-script
    void superScript(bool);
    /// make the selected text switch to be sub-script
    void subScript(bool);
    /// move the paragraph indent of the selected text to be less (left on LtR text)
    void decreaseIndent();
    /// move the paragraph indent of the selected text to be more (right on LtR text)
    void increaseIndent();
    /// Increase the font size. This will preserve eventual difference in font size within the selection.
    void increaseFontSize();
    /// Decrease font size. See above.
    void decreaseFontSize();
    /// Set font family
    void setFontFamily(const QString &);
    /// Set Font size
    void setFontSize(int size);
    /// Default Format
    void setDefaultFormat();
    /// see KoTextSelectionHandler::insertIndexMarker
    void insertIndexMarker();
    /// shows a dialog to insert a table
    void insertTable();
    /// insert a row above
    void insertTableRowAbove();
    /// insert a row below
    void insertTableRowBelow();
    /// insert a column left
    void insertTableColumnLeft();
    /// insert a column right
    void insertTableColumnRight();
    /// delete a column
    void deleteTableColumn();
    /// delete a row
    void deleteTableRow();
    /// merge table cells
    void mergeTableCells();
    /// split previous merged table cells
    void splitTableCells();
    /// shows a dialog to alter the paragraph properties
    void formatParagraph();
    /// select all text in the current document.
    void selectAll();
    /// show the style manager
    void showStyleManager();
    /// show the insert bookmark dialog
    void insertBookmark();
    /// change color of a selected text
    void setTextColor(const KoColor &color);
    /// change background color of a selected text
    void setBackgroundColor(const KoColor &color);
    /// set Paragraph style of current selection. Exisiting style will be completely overridden.
    void setStyle(KParagraphStyle *syle);
    /// set the characterStyle of the current selection. see above.
    void setStyle(KCharacterStyle *style);
    /// show dialog with markers (bookmarks, pages)
    void jumpToText();

    /// add a KoDocument wide undo command which will call undo on the qtextdocument.
    void addUndoCommand();

    /// slot to call when a series of commands is started that together need to become 1 undo action.
    void startMacro(const QString &title);
    /// slot to call when a series of commands has ended that together should be 1 undo action.
    void stopMacro();

    /// show the insert special character docker.
    void insertSpecialCharacter();
    /// insert string
    void insertString(const QString &string);

    /// returns the focus to canvas when styles are selected in the optionDocker
    void returnFocusToCanvas();

    void selectFont();
    void shapeAddedToCanvas();

    void blinkCaret();

    // called when the m_textShapeData has been deleted.
    void shapeDataRemoved();

    //Show tooltip with change info
    void showChangeTip();

    /// print debug about the details of the text document
    void debugTextDocument();
    /// print debug about the details of the styles on the current text document
    void debugTextStyles();
    /// the document we are editing has received an extra shape
    void shapeAddedToDoc(KShape *shape);
    void ensureCursorVisible();

    void showEditVariableDialog();

private:
    void repaintCaret();
    void repaintSelection();
    void repaintSelection(int from, int to);
    int pointToPosition(const QPointF & point) const;
    void updateActions();
    void updateStyleManager();
    void updateSelectedShape(const QPointF &point);
    void updateSelectionHandler();
    void editingPluginEvents();
    void finishedWord();
    void finishedParagraph();
    void readConfig();
    void writeConfig();

private:
    friend class UndoTextCommand;
    friend class TextCommandBase;
    friend class ChangeTracker;
    friend class TextPasteCommand;
    friend class TextCutCommand;
    friend class ShowChangesCommand;
    friend class ChangeTrackedDeleteCommand;
    friend class DeleteCommand;
    TextShape *m_textShape;
    KTextShapeData *m_textShapeData;
    QWeakPointer<KoTextEditor> m_textEditor;
    KChangeTracker *m_changeTracker;
    bool m_allowActions;
    bool m_allowAddUndoCommand;
    bool m_trackChanges;
    bool m_allowResourceManagerUpdates;
    int m_prevCursorPosition; /// used by editingPluginEvents

    QTimer m_caretTimer;
    bool m_caretTimerState;
    KAction *m_actionFormatBold;
    KAction *m_actionFormatItalic;
    KAction *m_actionFormatUnderline;
    KAction *m_actionFormatStrikeOut;
    KAction *m_actionAlignLeft;
    KAction *m_actionAlignRight;
    KAction *m_actionAlignCenter;
    KAction *m_actionAlignBlock;
    KAction *m_actionFormatSuper;
    KAction *m_actionFormatSub;
    KAction *m_actionFormatIncreaseIndent;
    KAction *m_actionFormatDecreaseIndent;
    KAction *m_actionShowChanges;
    KAction *m_actionRecordChanges;
    KAction *m_configureChangeTracking;
    KFontSizeAction *m_actionFormatFontSize;
    KFontAction *m_actionFormatFontFamily;
    KoColorPopupAction *m_actionFormatTextColor;
    KoColorPopupAction *m_actionFormatBackgroundColor;

    QUndoCommand *m_currentCommand; //this command will be the direct parent of undoCommands generated as the result of QTextDocument changes

    bool m_currentCommandHasChildren;

    /// structure that allows us to remember the text position and selection of previously edited documents.
    struct TextSelection {
        QTextDocument *document; // be warned that this may end up being a dangling pointer, so don't rereference.
        int position;
        int anchor;
    };
    QList<TextSelection> m_previousSelections;

    InsertCharacter *m_specialCharacterDocker;

    TextEditingPluginContainer *m_textEditingPlugins;

    bool m_textTyping;
    bool m_textDeleting;

    QTimer m_changeTipTimer;
    int m_changeTipCursorPos;
    QPoint m_changeTipPos;
};

#endif
