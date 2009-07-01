/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOTEXTTOOL_H
#define KOTEXTTOOL_H

#include "TextShape.h"

#include <KoTextSelectionHandler.h>
#include <KoTool.h>

#include <QTextCursor>
#include <QHash>
#include <QTextBlock>
#include <QTimer>
#include <QClipboard>

class KAction;
class KoAction;
class KoStyleManager;
class KoTextEditingPlugin;
class KoColor;
class KoColorPopupAction;
class UndoTextCommand;
class InsertCharacter;
class KoChangeTracker;

class QUndoCommand;
class KFontSizeAction;
class KFontAction;

class MockCanvas;

/**
 * This is the tool for the text-shape (which is a flake-based plugin).
 */
class TextTool : public KoTool
{
    Q_OBJECT
public:
    explicit TextTool(KoCanvasBase *canvas);
    ~TextTool();

    /// reimplemented from superclass
    virtual void paint(QPainter &painter, const KoViewConverter &converter);

    /// reimplemented from superclass
    virtual void mousePressEvent(KoPointerEvent *event) ;
    /// reimplemented from superclass
    virtual void mouseDoubleClickEvent(KoPointerEvent *event);
    /// reimplemented from superclass
    virtual void mouseMoveEvent(KoPointerEvent *event);
    /// reimplemented from superclass
    virtual void mouseReleaseEvent(KoPointerEvent *event);
    /// reimplemented from superclass
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented from superclass
    virtual void keyReleaseEvent(QKeyEvent *event);

    /// reimplemented from superclass
    virtual void activate(bool temporary = false);
    /// reimplemented from superclass
    virtual void deactivate();
    /// reimplemented from superclass
    virtual void copy() const;
    ///reimplemented
    virtual void deleteSelection();
    /// reimplemented from superclass
    virtual bool paste();
    /// reimplemented from superclass
    virtual QStringList supportedPasteMimeTypes() const;
    /// reimplemented from superclass
    virtual void repaintDecorations();

    /// reimplemented from superclass
    virtual KoToolSelection* selection();
    /// reimplemented from superclass
    virtual QWidget *createOptionWidget();
    /// reimplemented from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const;
    /// reimplemented from superclass
    virtual void inputMethodEvent(QInputMethodEvent * event);

    bool isBidiDocument() const;


    /// The following two methods allow an undo/redo command to tell the tool, it will modify the QTextDocument and wants to be parent of the undo/redo commands resulting from these changes.

    void startEditing(QUndoCommand* command);

    void stopEditing();

    const QTextCursor cursor();


public slots:
    /// start the textedit-plugin.
    void startTextEditingPlugin(const QString &pluginId);
    /// add a command to the undo stack, executing it as well.
    void addCommand(QUndoCommand *command);
    /// reimplemented from KoTool
    virtual void resourceChanged(int key, const QVariant &res);

signals:
    /// emitted every time a different styleManager is set.
    void styleManagerChanged(KoStyleManager *manager);
    /// emitted every time a caret move leads to a different character format being under the caret
    void charFormatChanged(const QTextCharFormat &format);
    /// emitted every time a caret move leads to a different paragraph format being under the caret
    void blockFormatChanged(const QTextBlockFormat &format);
    /// emitted every time a caret move leads to a different paragraph format being under the caret
    void blockChanged(const QTextBlock &block);

private slots:
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
    /// Default Format
    void setDefaultFormat();
    /// see KoTextSelectionHandler::insertIndexMarker
    void insertIndexMarker();
    /// shows a dialog to alter the paragraph properties
    void formatParagraph();
    /// When enabled, make the change tracker record changes made while typing
    void toggleTrackChanges(bool);
    /// select all text in the current document.
    void selectAll();
    /// show the style manager
    void showStyleManager();
    /// change color of a selected text
    void setTextColor(const KoColor &color);
    /// change background color of a selected text
    void setBackgroundColor(const KoColor &color);

    /// add a KoDocument wide undo command which will call undo on the qtextdocument.
    void addUndoCommand();

    /// slot to call when a series of commands is started that together need to become 1 undo action.
    void startMacro(const QString &title);
    /// slot to call when a series of commands has ended that together should be 1 undo action.
    void stopMacro();

    /// show the insert special character docker.
    void insertSpecialCharacter();

    /// method that will be called in an alternative thread for updating the paragraph direction at a character pos
    void updateParagraphDirection(const QVariant &variant);
    /// method that will be called in the UI thread directly after the one above
    void updateParagraphDirectionUi();

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

#ifndef NDEBUG
protected:
    explicit TextTool(MockCanvas *canvas);
#endif

private:
    bool pasteHelper(QClipboard::Mode mode);
    void repaintCaret();
    void repaintSelection();
    void repaintSelection(int from, int to);
    void ensureCursorVisible();
    QRectF textRect(int startPosition, int endPosition) const;
    int pointToPosition(const QPointF & point) const;
    void updateSelectionHandler();
    void updateActions();
    void updateStyleManager();
    void setShapeData(KoTextShapeData *data);
    void updateSelectedShape(const QPointF &point);

    void editingPluginEvents();
    void finishedWord();
    void finishedParagraph();

private:
    friend class UndoTextCommand;
    friend class TextCommandBase;
    friend class ChangeTracker;
    TextShape *m_textShape;
    KoTextShapeData *m_textShapeData;
    QTextCursor m_caret;
    KoChangeTracker *m_changeTracker;
    KoTextSelectionHandler m_selectionHandler;
    bool m_allowActions;
    bool m_allowAddUndoCommand;
    bool m_trackChanges;
    bool m_allowResourceProviderUpdates;
    bool m_needSpellChecking;
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
    KFontSizeAction *m_actionFormatFontSize;
    KFontAction *m_actionFormatFontFamily;
    KoColorPopupAction *m_actionFormatTextColor;
    KoColorPopupAction *m_actionFormatBackgroundColor;

    QHash<QString, KoTextEditingPlugin*> m_textEditingPlugins;
    KoTextEditingPlugin *m_spellcheckPlugin;

    QUndoCommand *m_currentCommand; //this command will be the direct parent of undoCommands generated as the result of QTextDocument changes

    bool m_currentCommandHasChildren;

    // update Parag direction will be multi-threaded.
    struct UpdatePageDirection {
        KoAction *action;
        QTextBlock block;
        KoText::Direction direction;
    };
    UpdatePageDirection m_updateParagDirection;

    /// structure that allows us to remember the text position and selection of previously edited documents.
    struct TextSelection {
        QTextDocument *document; // be warned that this may end up being a dangling pointer, so don't use.
        int position;
        int anchor;
    };
    QList<TextSelection> m_previousSelections;

    InsertCharacter *m_specialCharacterDocker;

    bool m_textTyping;
    bool m_textDeleting;

    QTimer m_changeTipTimer;
    int m_changeTipCursorPos;
    QPoint m_changeTipPos;
};

#endif
