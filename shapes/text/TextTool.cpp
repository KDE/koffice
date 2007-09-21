/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "TextTool.h"
#include "ChangeTracker.h"
#include "PluginHelperAction.h"
#include "dialogs/SimpleStyleWidget.h"
#include "dialogs/StylesWidget.h"
#include "dialogs/ParagraphSettingsDialog.h"
#include "dialogs/StyleManagerDialog.h"
#include "dialogs/CreateNewBookmarkDialog.h"
#include "dialogs/SelectBookmarkDialog.h"
#include "dialogs/InsertCharacter.h"
#include "commands/TextCommandBase.h"
#include "commands/ChangeListCommand.h"

#include <KoAction.h>
#include <KoExecutePolicy.h>
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoCanvasResourceProvider.h>
#include <KoColor.h>
#include <KoColorSetAction.h>

#include <KoCharacterStyle.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineNote.h>
#include <KoParagraphStyle.h>
#include <KoTextEditingPlugin.h>
#include <KoTextEditingRegistry.h>
#include <KoTextEditingFactory.h>
#include <KoInlineTextObjectManager.h>
#include <KoBookmark.h>
#include <KoBookmarkManager.h>
#include <KoListStyle.h>
#include <KoXmlWriter.h>

#include <kdebug.h>
#include <KStandardShortcut>
#include <KFontSizeAction>
#include <KFontChooser>
#include <KFontAction>
#include <KAction>
#include <KStandardAction>
#include <KMimeType>
#include <KMessageBox>
#include <KRun>
#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QBuffer>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>
#include <QPointer>
#include <QTabWidget>
#include <QTextBlock>
#include <QTextLayout>
#include <QUndoCommand>
#include <KoGenStyles.h>
#include <KoSavingContext.h>
#include <KoShapeSavingContext.h>

static bool hit(const QKeySequence &input, KStandardShortcut::StandardShortcut shortcut) {
    foreach(QKeySequence ks, KStandardShortcut::shortcut(shortcut).toList()) {
        if(input == ks)
            return true;
    }
    return false;
}

static bool isRightToLeft(const QString &text) {
    int ltr = 0, rtl = 0;

    QString::const_iterator iter = text.begin();
    while(iter != text.end()) {
        switch(QChar::direction((*iter).unicode()))
        {
        case QChar::DirL:
        case QChar::DirLRO:
        case QChar::DirLRE:
            ltr++;
            break;
        case QChar::DirR:
        case QChar::DirAL:
        case QChar::DirRLO:
        case QChar::DirRLE:
            rtl++;
        default:
            break;
        }
        ++iter;
    }
    return ltr < rtl;
}

TextTool::TextTool(KoCanvasBase *canvas)
: KoTool(canvas),
    m_textShape(0),
    m_textShapeData(0),
    m_changeTracker(0),
    m_allowActions(true),
    m_allowAddUndoCommand(true),
    m_trackChanges(false),
    m_allowResourceProviderUpdates(true),
    m_needSpellChecking(true),
    m_prevCursorPosition(-1),
    m_spellcheckPlugin(0),
    m_currentCommand(0),
    m_currentCommandHasChildren(false),
    m_specialCharacterDocker(0)
{
    m_actionFormatBold  = new QAction(KIcon("format-text-bold"), i18n("Bold"), this);
    addAction("format_bold", m_actionFormatBold );
    m_actionFormatBold->setShortcut(Qt::CTRL + Qt::Key_B);
    m_actionFormatBold->setCheckable(true);
    connect( m_actionFormatBold, SIGNAL(triggered(bool)), &m_selectionHandler, SLOT(bold(bool)) );

    m_actionFormatItalic  = new QAction(KIcon("format-text-italic"), i18n("Italic"), this);
    addAction("format_italic", m_actionFormatItalic );
    m_actionFormatItalic->setShortcut( Qt::CTRL + Qt::Key_I);
    m_actionFormatItalic->setCheckable(true);
    connect( m_actionFormatItalic, SIGNAL(triggered(bool)), &m_selectionHandler, SLOT(italic(bool)) );

    m_actionFormatUnderline  = new QAction(KIcon("format-text-underline"), i18nc("Text formatting", "Underline"), this);
    addAction("format_underline", m_actionFormatUnderline );
    m_actionFormatUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    m_actionFormatUnderline->setCheckable(true);
    connect( m_actionFormatUnderline, SIGNAL(triggered(bool)), &m_selectionHandler, SLOT(underline(bool)) );

    m_actionFormatStrikeOut  = new QAction(KIcon("format-text-strikethrough"), i18n("Strike Out"), this);
    addAction("format_strike", m_actionFormatStrikeOut );
    m_actionFormatStrikeOut->setCheckable(true);
    connect( m_actionFormatStrikeOut, SIGNAL(triggered(bool)), &m_selectionHandler, SLOT(strikeOut(bool)) );

    QActionGroup *alignmentGroup = new QActionGroup(this);
    m_actionAlignLeft  = new QAction(KIcon("text-left"), i18n("Align Left"), this);
    addAction("format_alignleft", m_actionAlignLeft );
    m_actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    m_actionAlignLeft->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignLeft);
    connect(m_actionAlignLeft, SIGNAL(triggered(bool)), this, SLOT(alignLeft()));

    m_actionAlignRight  = new QAction(KIcon("text-right"), i18n("Align Right"), this);
    addAction("format_alignright", m_actionAlignRight );
    m_actionAlignRight->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_R);
    m_actionAlignRight->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignRight);
    connect(m_actionAlignRight, SIGNAL(triggered(bool)), this, SLOT(alignRight()));

    m_actionAlignCenter  = new QAction(KIcon("text-center"), i18n("Align Center"), this);
    addAction("format_aligncenter", m_actionAlignCenter );
    m_actionAlignCenter->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_C);
    m_actionAlignCenter->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignCenter);
    connect(m_actionAlignCenter, SIGNAL(triggered(bool)), this, SLOT(alignCenter()));

    m_actionAlignBlock  = new QAction(KIcon("format-justify-fill"), i18n("Align Block"), this);
    addAction("format_alignblock", m_actionAlignBlock );
    m_actionAlignBlock->setShortcut(Qt::CTRL + Qt::ALT + Qt::Key_R);
    m_actionAlignBlock->setCheckable(true);
    alignmentGroup->addAction(m_actionAlignBlock);
    connect(m_actionAlignBlock, SIGNAL(triggered(bool)), this, SLOT(alignBlock()));

    m_actionFormatSuper = new QAction(KIcon("text-super"), i18n("Superscript"), this);
    addAction("format_super", m_actionFormatSuper );
    m_actionFormatSuper->setCheckable(true);
    connect(m_actionFormatSuper, SIGNAL(triggered(bool)), this, SLOT(superScript(bool)));

    m_actionFormatSub = new QAction(KIcon("text-sub"), i18n("Subscript"), this);
    addAction("format_sub", m_actionFormatSub );
    m_actionFormatSub->setCheckable(true);
    connect(m_actionFormatSub, SIGNAL(triggered(bool)), this, SLOT(subScript(bool)));

    QAction *action = new QAction(
            KIcon(QApplication::isRightToLeft() ? "format-indent-less" : "format-indent-more"),
            i18n("Increase Indent"), this);
    addAction("format_increaseindent", action );
    connect(action, SIGNAL(triggered()), this, SLOT(increaseIndent()));

    m_actionFormatDecreaseIndent = new QAction(
            KIcon(QApplication::isRightToLeft() ? "format-indent-more" :"format-indent-less"),
            i18n("Decrease Indent"), this);
    addAction("format_decreaseindent", m_actionFormatDecreaseIndent );
    connect(m_actionFormatDecreaseIndent, SIGNAL(triggered()), this, SLOT(decreaseIndent()));

    action = new QAction(i18n("Increase Font Size"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_Greater);
    addAction("fontsizeup", action);
    connect(action, SIGNAL(triggered()), &m_selectionHandler, SLOT(increaseFontSize()));

    action = new QAction(i18n("Decrease Font Size"), this);
    action->setShortcut(Qt::CTRL + Qt::Key_Less);
    addAction("fontsizedown", action);
    connect(action, SIGNAL(triggered()), &m_selectionHandler, SLOT(decreaseFontSize()));

    m_actionFormatFontFamily = new KFontAction( KFontChooser::SmoothScalableFonts, this);
    addAction( "format_fontfamily", m_actionFormatFontFamily );
    connect( m_actionFormatFontFamily, SIGNAL( triggered( const QString & ) ),
            &m_selectionHandler, SLOT( setFontFamily( const QString & ) ) );

/*
    m_actionFormatStyleMenu  = new KActionMenu(i18n("Style"), this);
    addAction("format_stylemenu", m_actionFormatStyleMenu );
    m_actionFormatStyle  = new KSelectAction(i18n("Style"), this);
    addAction("format_style", m_actionFormatStyle );
    connect( m_actionFormatStyle, SIGNAL( activated( int ) ),
            this, SLOT( textStyleSelected( int ) ) );
    updateStyleList();

    // ----------------------- More format actions, for the toolbar only
    QActionGroup* spacingActionGroup = new QActionGroup( this );
    spacingActionGroup->setExclusive( true );
    m_actionFormatSpacingSingle = new KToggleAction( i18n( "Line Spacing 1" ), "spacesimple", Qt::CTRL + Qt::Key_1,
            this, SLOT( textSpacingSingle() ),
            actionCollection(), "format_spacingsingle" );
    m_actionFormatSpacingSingle->setActionGroup( spacingActionGroup );
    m_actionFormatSpacingOneAndHalf = new KToggleAction( i18n( "Line Spacing 1.5" ), "spacedouble", Qt::CTRL + Qt::Key_5,
            this, SLOT( textSpacingOneAndHalf() ),
            actionCollection(), "format_spacing15" );
    m_actionFormatSpacingOneAndHalf->setActionGroup( spacingActionGroup );
    m_actionFormatSpacingDouble = new KToggleAction( i18n( "Line Spacing 2" ), "spacetriple", Qt::CTRL + Qt::Key_2,
            this, SLOT( textSpacingDouble() ),
            actionCollection(), "format_spacingdouble" );
    m_actionFormatSpacingDouble->setActionGroup( spacingActionGroup );

    m_actionFormatColor = new TKSelectColorAction( i18n( "Text Color..." ), TKSelectColorAction::TextColor,
            this, SLOT( textColor() ),
            actionCollection(), "format_color", true );
    m_actionFormatColor->setDefaultColor(QColor());


    m_actionFormatNumber  = new KActionMenu(KIcon( "enumList" ), i18n("Number"), this);
    addAction("format_number", m_actionFormatNumber );
    m_actionFormatNumber->setDelayed( false );
    m_actionFormatBullet  = new KActionMenu(KIcon( "unsortedList" ), i18n("Bullet"), this);
    addAction("format_bullet", m_actionFormatBullet );
    m_actionFormatBullet->setDelayed( false );
    QActionGroup* counterStyleActionGroup = new QActionGroup( this );
    counterStyleActionGroup->setExclusive( true );
    Q3PtrList<KoCounterStyleWidget::StyleRepresenter> stylesList;
    KoCounterStyleWidget::makeCounterRepresenterList( stylesList );
    Q3PtrListIterator<KoCounterStyleWidget::StyleRepresenter> styleIt( stylesList );
    for ( ; styleIt.current() ; ++styleIt ) {
        // Dynamically create toggle-actions for each list style.
        // This approach allows to edit toolbars and extract separate actions from this menu
        KToggleAction* act = new KToggleAction( styleIt.current()->name(), // TODO icon
                actionCollection(),
                QString("counterstyle_%1").arg( styleIt.current()->style() ) );
        connect( act, SIGNAL( triggered(bool) ), this, SLOT( slotCounterStyleSelected() ) );
        act->setActionGroup( counterStyleActionGroup );
        // Add to the right menu: both for "none", bullet for bullets, numbers otherwise
        if ( styleIt.current()->style() == KoParagCounter::STYLE_NONE ) {
            m_actionFormatBullet->insert( act );
            m_actionFormatNumber->insert( act );
        } else if ( styleIt.current()->isBullet() )
            m_actionFormatBullet->insert( act );
        else
            m_actionFormatNumber->insert( act );
    }
*/


    // ------------------- Actions with a key binding and no GUI item
    action  = new QAction(i18n("Insert Non-Breaking Space"), this);
    addAction("nonbreaking_space", action );
    action->setShortcut( Qt::CTRL+Qt::Key_Space);
    connect(action, SIGNAL(triggered()), this, SLOT( nonbreakingSpace() ));

    action  = new QAction(i18n("Insert Non-Breaking Hyphen"), this);
    addAction("nonbreaking_hyphen", action );
    action->setShortcut( Qt::CTRL+Qt::SHIFT+Qt::Key_Minus);
    connect(action, SIGNAL(triggered()), this, SLOT( nonbreakingHyphen() ));

    action  = new QAction(i18n("Insert Index"), this);
action->setShortcut( Qt::CTRL+ Qt::Key_T);
    addAction("insert_index", action );
    connect(action, SIGNAL(triggered()), this, SLOT( insertIndexMarker() ));

    action  = new QAction(i18n("Insert Soft Hyphen"), this);
    addAction("soft_hyphen", action );
    //action->setShortcut( Qt::CTRL+Qt::Key_Minus); // TODO this one is also used for the kde-global zoom-out :(
    connect(action, SIGNAL(triggered()), this, SLOT( softHyphen() ));

    action  = new QAction(i18n("Line Break"), this);
    addAction("line_break", action );
    action->setShortcut( Qt::SHIFT+Qt::Key_Return);
    connect(action, SIGNAL(triggered()), this, SLOT( lineBreak() ));

    action = new QAction(i18n("Bookmark..."), this);
    //action->setIcon(KIcon("bookmark-new"));
    addAction("add_bookmark", action);
    connect(action, SIGNAL(triggered()), this, SLOT( addBookmark() ));

    action = new QAction(i18n("Select Bookmark..."), this);
    //action->setIcon(KIcon("bookmark"));
    action->setShortcut( Qt::CTRL+ Qt::Key_G);
    addAction("select_bookmark", action);
    connect(action, SIGNAL(triggered()), this, SLOT( selectBookmark() ));

    action  = new QAction(i18n("Font..."), this);
    addAction("format_font", action );
    action->setShortcut( Qt::ALT + Qt::CTRL + Qt::Key_F);
    action->setToolTip( i18n( "Change character size, font, boldface, italics etc." ) );
    action->setWhatsThis( i18n( "Change the attributes of the currently selected characters." ) );
    connect(action, SIGNAL(triggered()), &m_selectionHandler, SLOT( selectFont() ));

    m_actionFormatFontSize = new KFontSizeAction( i18n( "Font Size" ), this);
    addAction("format_fontsize", m_actionFormatFontSize );
    connect( m_actionFormatFontSize, SIGNAL(fontSizeChanged( int )), &m_selectionHandler, SLOT(setFontSize(int)) );

    m_actionFormatTextColor = new KoColorSetAction(this);
    addAction("format_color", m_actionFormatTextColor);
    connect(m_actionFormatTextColor, SIGNAL(colorChanged(const KoColor &)), this, SLOT(setTextColor(const KoColor &)) );

    action = new QAction(i18n("Default Format"), this);
    addAction("text_default", action);
    action->setToolTip( i18n( "Change font and paragraph attributes to their default values" ) );
    connect(action, SIGNAL(triggered()), this, SLOT( textDefaultFormat() ));

    foreach(QString key, KoTextEditingRegistry::instance()->keys()) {
        KoTextEditingFactory *factory =  KoTextEditingRegistry::instance()->value(key);
        Q_ASSERT(factory);
        if(m_textEditingPlugins.contains(factory->id())) {
            kWarning(32500) << "Duplicate id for textEditingPlugin, ignoring one! (" << factory->id() << ")\n";
            continue;
        }
        QString factoryId = factory->id();
        KoTextEditingPlugin *plugin = factory->create();
        if (factoryId == "spellcheck") {
            kDebug() << "KOffice SpellCheck plugin found";
            m_spellcheckPlugin = plugin;
        }
        m_textEditingPlugins.insert(factory->id(), plugin);
    }

    foreach (KoTextEditingPlugin* plugin, m_textEditingPlugins.values()) {
        connect(plugin, SIGNAL(startMacro(const QString &)), this, SLOT(startMacro(const QString &)));
        connect(plugin, SIGNAL(stopMacro()), this, SLOT(stopMacro()));
        QHash<QString, QAction*> actions = plugin->actions();
        QHash<QString, QAction*>::iterator i = actions.begin();
        while (i != actions.end()) {
            addAction(i.key(), i.value());
            i++;
        }
    }

    action = new QAction(i18n("Paragraph..."), this);
    addAction("format_paragraph", action);
    action->setShortcut(Qt::ALT + Qt::CTRL + Qt::Key_P);
    action->setToolTip( i18n( "Change paragraph margins, text flow, borders, bullets, numbering etc." ) );
    action->setWhatsThis( i18n( "Change paragraph margins, text flow, borders, bullets, numbering etc.<p>Select text in multiple paragraphs to change the formatting of all selected paragraphs.<p>If no text is selected, the paragraph where the cursor is located will be changed.</p>" ) );
    connect(action, SIGNAL(triggered()), this, SLOT(formatParagraph()));

    action = new QAction(i18n("Record"), this);
    action->setCheckable(true);
    addAction("edit_record_changes", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(toggleTrackChanges(bool)));

    action = new QAction(i18n("Style Manager"), this);
    action->setShortcut( Qt::ALT + Qt::CTRL + Qt::Key_S);
    action->setToolTip( i18n( "Change attributes of styles" ) );
    action->setWhatsThis( i18n( "Change font and paragraph attributes of styles.<p>Multiple styles can be changed using the dialog box." ) );
    addAction("format_stylist", action);
    connect(action, SIGNAL(triggered()), this, SLOT(showStyleManager()));

    action = KStandardAction::selectAll(this, SLOT(selectAll()), this);
    addAction("edit_selectall", action);

    action = new QAction(i18n( "Special Character..." ), this);
    action->setIcon(KIcon("character-set"));
    action->setShortcut(Qt::ALT + Qt::SHIFT + Qt::Key_C);
    addAction("insert_specialchar", action);
    action->setToolTip( i18n( "Insert one or more symbols or characters not found on the keyboard" ) );
    action->setWhatsThis( i18n( "Insert one or more symbols or characters not found on the keyboard." ) );
    connect(action, SIGNAL(triggered()), this, SLOT( insertSpecialCharacter() ));

    m_updateParagDirection.action = new KoAction(this);
    m_updateParagDirection.action->setExecutePolicy(KoExecutePolicy::onlyLastPolicy);
    connect(m_updateParagDirection.action, SIGNAL(triggered(const QVariant &)),
            this, SLOT(updateParagraphDirection(const QVariant&)), Qt::DirectConnection);
    connect(m_updateParagDirection.action, SIGNAL(updateUi(const QVariant &)),
            this, SLOT(updateParagraphDirectionUi()));


    // setup the context list.
    QList<QAction*> list;
    list.append(this->action("text_default"));
    list.append(this->action("format_font"));
    foreach(QString key, KoTextEditingRegistry::instance()->keys()) {
        KoTextEditingFactory *factory =  KoTextEditingRegistry::instance()->value(key);
        if(factory->showInMenu())
            list.append(new PluginHelperAction(factory->title(), this, factory->id()));
    }
    setPopupActionList(list);

    connect(&m_selectionHandler, SIGNAL(startMacro(const QString&)), this, SLOT(startMacro(const QString&)));
    connect(&m_selectionHandler, SIGNAL(stopMacro()), this, SLOT(stopMacro()));
}

TextTool::~TextTool() {
    qDeleteAll(m_textEditingPlugins);
}

void TextTool::paint( QPainter &painter, const KoViewConverter &converter) {
    QTextBlock block = m_caret.block();
    if(! block.layout()) // not layouted yet.  The Shape paint method will trigger a layout
        return;

    if(painter.hasClipping()) {
        QRect shape = converter.documentToView(m_textShape->boundingRect()).toRect();
        if(painter.clipRegion().intersect( QRegion(shape) ).isEmpty())
            return;
    }

    painter.setMatrix( painter.matrix() * m_textShape->absoluteTransformation(&converter) );
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);
    Q_ASSERT(m_textShapeData);
    painter.translate(0, -m_textShapeData->documentOffset());

    if(m_textShapeData && m_caret.hasSelection()) {
        QAbstractTextDocumentLayout::PaintContext pc;
        QAbstractTextDocumentLayout::Selection selection;
        selection.cursor = m_caret;
        selection.format.setBackground(QBrush(Qt::yellow)); // TODO use configured selection color
        selection.format.setForeground(QBrush(Qt::black)); // TODO use configured selected-text color
        pc.selections.append(selection);

        QRectF clip = textRect(m_caret.position(), m_caret.anchor());
        painter.save();
        painter.setClipRect(clip, Qt::IntersectClip);
        m_textShapeData->document()->documentLayout()->draw( &painter, pc);
        painter.restore();
    }

    // paint caret
    QPen pen(Qt::black);
    if(! m_textShape->hasTransparency()) {
        QColor bg = m_textShape->background().color();
        QColor invert = QColor(255 - bg.red(), 255 - bg.green(), 255 - bg.blue());
        pen.setColor(invert);
    }
    painter.setPen(pen);
    const int posInParag = m_caret.position() - block.position();
    KoTextDocumentLayout::updateTabsForLine(block, block.layout()->lineForTextPosition(posInParag).lineNumber());
    block.layout()->drawCursor(&painter, QPointF(0,0), posInParag);
}

void TextTool::mousePressEvent( KoPointerEvent *event ) {
    const bool canMoveCaret = !m_caret.hasSelection() || event->button() !=  Qt::RightButton;

    if(canMoveCaret && ! m_textShape->boundingRect().contains(event->point)) {
        QRectF area(event->point, QSizeF(1,1));
        repaintSelection();
        foreach(KoShape *shape, m_canvas->shapeManager()->shapesAt(area, true)) {
            TextShape *textShape = dynamic_cast<TextShape*> (shape);
            if(textShape) {
                m_textShape = textShape;
                KoTextShapeData *d = static_cast<KoTextShapeData*> (textShape->userData());
                if(d->document() == m_textShapeData->document())
                    break; // stop looking.
            }
        }
        setShapeData(static_cast<KoTextShapeData*> (m_textShape->userData()));
    }
    KoSelection *selection = m_canvas->shapeManager()->selection();
    if(!selection->isSelected(m_textShape)) {
        selection->deselectAll();
        selection->select(m_textShape);
    }

    if(canMoveCaret) {
        bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
        if(m_caret.hasSelection() && !shiftPressed)
            repaintSelection(); // will erase selection
        else if(! m_caret.hasSelection())
            repaintCaret();
        int prevPosition = m_caret.position();
        int position = pointToPosition(event->point);
        m_caret.setPosition(position, shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        if(shiftPressed) // altered selection.
            repaintSelection(prevPosition, m_caret.position());
        else
            repaintCaret();

        updateSelectionHandler();
        updateStyleManager();
    }
    updateActions();

    if(event->button() ==  Qt::MidButton) { // Paste
        QClipboard *clipboard = QApplication::clipboard();
        QString paste = clipboard->text(QClipboard::Selection);
        if(! paste.isEmpty()) {
            if (m_caret.hasSelection())
                m_selectionHandler.deleteInlineObjects();
            m_caret.insertText(paste);
            ensureCursorVisible();
            editingPluginEvents();
            emit blockChanged(m_caret.block());
        }
    } else {
        // Is there an anchor here ?
        if (m_caret.charFormat().isAnchor()) {
            QString anchor = m_caret.charFormat().anchorHref();
            bool isLocalLink = (anchor.indexOf("file:") == 0);
            QString type = KMimeType::findByUrl(anchor, 0, isLocalLink)->name();

            if ( KRun::isExecutableFile( anchor, type ) )
            {
                QString question = i18n("This link points to the program or script '%1'.\n"
                        "Malicious programs can harm your computer. "
                        "Are you sure that you want to run this program?", anchor);
                // this will also start local programs, so adding a "don't warn again"
                // checkbox will probably be too dangerous
                int choice = KMessageBox::warningYesNo(0, question, i18n("Open Link?"));
                if ( choice != KMessageBox::Yes )
                    return;
            }
            new KRun(m_caret.charFormat().anchorHref(), 0);
        }
        else
            event->ignore(); // allow the event to be used by another
    }
}

void TextTool::setShapeData(KoTextShapeData *data) {
    bool docChanged = data == 0 || m_textShapeData == 0 || m_textShapeData->document() != data->document();
    if(m_textShapeData && docChanged)
        disconnect(m_textShapeData->document(), SIGNAL(undoAvailable(bool)), this, SLOT(addUndoCommand()));
    m_textShapeData = data;
    if(m_textShapeData && docChanged) {
        connect(m_textShapeData->document(), SIGNAL(undoAvailable(bool)), this, SLOT(addUndoCommand()));
        m_caret = QTextCursor(m_textShapeData->document());

        if(m_textShape->demoText()) {
            m_textShapeData->document()->setUndoRedoEnabled(false); // removes undo history
            m_textShape->setDemoText(false); // remove demo text
        }
        m_textShapeData->document()->setUndoRedoEnabled(true); // allow undo history
    }
    if(m_textShapeData && m_trackChanges) {
        if(m_changeTracker == 0)
            m_changeTracker = new ChangeTracker(this);
        m_changeTracker->setDocument(m_textShapeData->document());
    }
    if(m_textShapeData && m_spellcheckPlugin)
        m_spellcheckPlugin->checkSection(m_textShapeData->document(), 0, 0);
}

void TextTool::updateSelectionHandler() {
    m_selectionHandler.setShape(m_textShape);
    m_selectionHandler.setShapeData(m_textShapeData);
    m_selectionHandler.setCaret(&m_caret);
    emit selectionChanged(m_caret.hasSelection());

    if(m_caret.hasSelection()) {
        QClipboard *clipboard = QApplication::clipboard();
        if(clipboard->supportsSelection())
            clipboard->setText(m_caret.selectedText(), QClipboard::Selection);
    }
    KoCanvasResourceProvider *p = m_canvas->resourceProvider();
    m_allowResourceProviderUpdates = false;
    if(m_textShapeData) {
        p->setResource(KoText::CurrentTextPosition, m_caret.position());
        p->setResource(KoText::CurrentTextAnchor, m_caret.anchor());
        QVariant variant;
        variant.setValue<void*>(m_textShapeData->document());
        p->setResource(KoText::CurrentTextDocument, variant);
    }
    else {
        p->clearResource(KoText::CurrentTextPosition);
        p->clearResource(KoText::CurrentTextAnchor);
        p->clearResource(KoText::CurrentTextDocument);
    }
    m_allowResourceProviderUpdates = true;
}

void TextTool::copy() const {
    if(m_textShapeData == 0 || !m_caret.hasSelection()) return;
    int from = m_caret.position();
    int to = m_caret.anchor();
    if(to < from)
        qSwap(to, from);

    QByteArray bytes;
    QBuffer buffer( &bytes );
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter writer( &buffer );
    KoGenStyles mainStyles;
    KoSavingContext savingContext( mainStyles, KoSavingContext::Store );
    KoShapeSavingContext context (writer, savingContext);
    //writer.startDocument( "foobar" );
    m_textShapeData->saveOdf(context, from, to);
    //writer.endDocument();
    buffer.putChar( '\0' ); // null-terminate
    QMimeData *data = new QMimeData();
    data->setData("application/vnd.oasis.opendocument.text", bytes);
    QApplication::clipboard()->setMimeData(data);
kDebug() <<"output:" << QString::fromUtf8(bytes);
}

bool TextTool::paste() {
    const QMimeData *data = QApplication::clipboard()->mimeData();

    if(data->hasFormat("application/vnd.oasis.opendocument.text")) {
        kDebug() <<"TODO load ODF style text!";
        // TODO create a KoTextShapeData::loadOdf() method and call it here.
    }
    else if(data->hasHtml()) {
        if (m_caret.hasSelection())
            m_selectionHandler.deleteInlineObjects();
        m_caret.insertHtml(data->html());
    }
    else if(data->hasText()) {
        if (m_caret.hasSelection())
            m_selectionHandler.deleteInlineObjects();
        m_caret.insertText(data->text());
    }
    else
        return false;

    ensureCursorVisible();
    editingPluginEvents();
    emit blockChanged(m_caret.block());
    return true;
}

int TextTool::pointToPosition(const QPointF & point) const {
    QPointF p = m_textShape->convertScreenPos(point);
    int caretPos = m_caret.block().document()->documentLayout()->hitTest(p, Qt::FuzzyHit);
    caretPos = qMax(caretPos, m_textShapeData->position());
    if(m_textShapeData->endPosition() == -1) {
        kWarning(32500) << "Clicking in not fully laid-out textframe\n";
        m_textShapeData->fireResizeEvent(); // requests a layout run ;)
    }
    caretPos = qMin(caretPos, m_textShapeData->endPosition());
    return caretPos;
}

void TextTool::mouseDoubleClickEvent( KoPointerEvent *event ) {
    if(m_canvas->shapeManager()->shapeAt(event->point) != m_textShape) {
        event->ignore(); // allow the event to be used by another
        return;
    }
    m_caret.clearSelection();
    int pos = m_caret.position();
    m_caret.movePosition(QTextCursor::WordLeft);
    m_caret.movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor);
    if(qAbs(pos - m_caret.position()) <= 1) // clicked between two words
        m_caret.movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor);

    repaintSelection();
    updateSelectionHandler();
}

void TextTool::mouseMoveEvent( KoPointerEvent *event ) {
    useCursor(Qt::IBeamCursor);
    int position = pointToPosition(event->point);

    if(event->buttons() == Qt::NoButton) {
        QTextCursor cursor(m_caret);
        cursor.setPosition(position);

        if (cursor.charFormat().isAnchor())
            useCursor(Qt::PointingHandCursor);
        else
            useCursor(Qt::IBeamCursor);

        return;
    }

    if(position == m_caret.position()) return;
    if(position >= 0) {
        repaintCaret();
        int prevPos = m_caret.position();
        m_caret.setPosition(position, QTextCursor::KeepAnchor);
        repaintSelection(prevPos, m_caret.position());
    }

    updateSelectionHandler();
}

void TextTool::mouseReleaseEvent( KoPointerEvent *event ) {
    event->ignore();
    editingPluginEvents();
}

void TextTool::keyPressEvent(QKeyEvent *event) {
    int destinationPosition = -1; // for those cases where the moveOperation is not relevant;
    QTextCursor::MoveOperation moveOperation = QTextCursor::NoMove;
    if(event->key() == Qt::Key_Backspace) {
        if(! m_caret.hasSelection() && m_caret.block().textList() && m_caret.block().length() == 1) {
            // backspace on numbered, empty parag, removes numbering.
            ChangeListCommand *clc = new ChangeListCommand(m_caret.block(), KoListStyle::NoItem);
            addCommand(clc);
        }
        else {
            if(!m_caret.hasSelection() && event->modifiers() & Qt::ControlModifier) // delete prev word.
                m_caret.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
            // if the cursor position (no selection) has inline object, the character + inline object
            // is deleted by the InlineTextObjectManager
            if (!m_selectionHandler.deleteInlineObjects(true) || m_caret.hasSelection())
                m_caret.deletePreviousChar();
            editingPluginEvents();
        }
        ensureCursorVisible();
    }
    else if(event->key() == Qt::Key_Delete) {
        if(!m_caret.hasSelection() && event->modifiers() & Qt::ControlModifier) // delete next word.
            m_caret.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
        if (!m_selectionHandler.deleteInlineObjects(false) || m_caret.hasSelection())
            m_caret.deleteChar();
        editingPluginEvents();
    }
    else if((event->key() == Qt::Key_Left) && (event->modifiers() | Qt::ShiftModifier) == Qt::ShiftModifier)
        moveOperation = QTextCursor::Left;
    else if((event->key() == Qt::Key_Right) && (event->modifiers() | Qt::ShiftModifier) == Qt::ShiftModifier)
        moveOperation = QTextCursor::Right;
    else if((event->key() == Qt::Key_Up) && (event->modifiers() | Qt::ShiftModifier) == Qt::ShiftModifier)
        moveOperation = QTextCursor::Up;
    else if((event->key() == Qt::Key_Down) && (event->modifiers() | Qt::ShiftModifier) == Qt::ShiftModifier)
        moveOperation = QTextCursor::Down;
    else {
        // check for shortcuts.
        QKeySequence item(event->key() | ((Qt::ControlModifier | Qt::AltModifier) & event->modifiers()));
        if(hit(item, KStandardShortcut::Home))
            // Goto beginning of the document. Default: Ctrl-Home
            destinationPosition = 0;
        else if(hit(item, KStandardShortcut::End)) {
            // Goto end of the document. Default: Ctrl-End
            QTextBlock last = m_textShapeData->document()->end().previous();
            destinationPosition = last.position() + last.length() -1;
        }
        else if(hit(item, KStandardShortcut::Prior)) // page up
            // Scroll up one page. Default: Prior
            moveOperation = QTextCursor::StartOfLine; // TODO
        else if(hit(item, KStandardShortcut::Next))
            // Scroll down one page. Default: Next
            moveOperation = QTextCursor::StartOfLine; // TODO
        else if(hit(item, KStandardShortcut::BeginningOfLine))
            // Goto beginning of current line. Default: Home
            moveOperation = QTextCursor::StartOfLine;
        else if(hit(item, KStandardShortcut::EndOfLine))
            // Goto end of current line. Default: End
            moveOperation = QTextCursor::EndOfLine;
        else if(hit(item, KStandardShortcut::BackwardWord))
            moveOperation = QTextCursor::WordLeft;
        else if(hit(item, KStandardShortcut::ForwardWord))
            moveOperation = QTextCursor::WordRight;
#ifndef NDEBUG
        else if(event->key() == Qt::Key_F12) {
            KoInlineNote *fn = new KoInlineNote(KoInlineNote::Footnote);
            fn->setText("Lorem ipsum dolor sit amet, XgXgectetuer adiXiscing elit, sed diam nonummy");
            fn->setLabel("1");
            KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
           lay->inlineObjectTextManager()->insertInlineObject(m_caret, fn);
        }
#endif
        else if((event->modifiers() & (Qt::ControlModifier | Qt::AltModifier)) || event->text().length() == 0) {
            event->ignore();
            return;
        }
        else if(event->text().at(0) == '\r') {
            if (m_caret.hasSelection())
                m_selectionHandler.deleteInlineObjects();
            QTextBlockFormat format = m_caret.blockFormat();
            m_selectionHandler.nextParagraph();

            QVariant direction = format.property(KoParagraphStyle::TextProgressionDirection);
            format = m_caret.blockFormat();
            if(m_textShapeData->pageDirection() != KoText::AutoDirection) { // inherit from shape
                KoText::Direction dir;
                switch(m_textShapeData->pageDirection()) {
                    case KoText::RightLeftTopBottom:
                        dir = KoText::PerhapsRightLeftTopBottom;
                        break;
                    case KoText::LeftRightTopBottom:
                    default:
                        dir = KoText::PerhapsLeftRightTopBottom;
                }
                format.setProperty(KoParagraphStyle::TextProgressionDirection, dir);
            }
            else if(! direction.isNull()) // then we inherit from the previous paragraph.
                format.setProperty(KoParagraphStyle::TextProgressionDirection, direction);
            m_caret.setBlockFormat(format);
            updateActions();
            editingPluginEvents();
            ensureCursorVisible();
        }
        else if(! (event->text().length() == 1 && !event->text().at(0).isPrint())) { // insert the text
            if (m_caret.hasSelection())
                m_selectionHandler.deleteInlineObjects();
            m_prevCursorPosition = m_caret.position();
            ensureCursorVisible();
            m_caret.insertText(event->text());
            if(m_textShapeData->pageDirection() == KoText::AutoDirection)
                m_updateParagDirection.action->execute(m_prevCursorPosition);
            editingPluginEvents();
            emit blockChanged(m_caret.block());
        }
    }
    if(moveOperation != QTextCursor::NoMove || destinationPosition != -1) {
        useCursor(Qt::BlankCursor);
        bool shiftPressed = event->modifiers() & Qt::ShiftModifier;
        if(m_caret.hasSelection() && !shiftPressed)
            repaintSelection(); // will erase selection
        else if(! m_caret.hasSelection())
            repaintCaret();
        QTextBlockFormat format = m_caret.blockFormat();


        KoText::Direction dir = static_cast<KoText::Direction> (format.intProperty(KoParagraphStyle::TextProgressionDirection));
        bool isRtl;
        if(dir == KoText::AutoDirection)
            isRtl = m_caret.block().text().isRightToLeft();
        else
            isRtl =  dir == KoText::RightLeftTopBottom;

        if(isRtl) { // if RTL toggle direction of cursor movement.
            switch(moveOperation) {
            case QTextCursor::Left: moveOperation = QTextCursor::Right; break;
            case QTextCursor::Right: moveOperation = QTextCursor::Left; break;
            case QTextCursor::WordRight: moveOperation = QTextCursor::WordLeft; break;
            case QTextCursor::WordLeft: moveOperation = QTextCursor::WordRight; break;
            default: break;
            }
        }
        int prevPosition = m_caret.position();
        if(moveOperation != QTextCursor::NoMove)
            m_caret.movePosition(moveOperation,
                shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        else
            m_caret.setPosition(destinationPosition,
                shiftPressed ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        if(shiftPressed) // altered selection.
            repaintSelection(prevPosition, m_caret.position());
        else
            repaintCaret();
        updateActions();
        editingPluginEvents();
        ensureCursorVisible();
    }

    updateSelectionHandler();
}

QVariant TextTool::inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const {
    switch(query) {
    case Qt::ImMicroFocus: {
        // The rectangle covering the area of the input cursor in widget coordinates.
        QRectF rect = textRect(m_caret.position(), m_caret.position());
        rect.moveTop(rect.top() - m_textShapeData->documentOffset());
        rect = m_textShape->absoluteTransformation(&converter).mapRect(rect);
        return rect.toRect();
    }
    case Qt::ImFont:
        // The currently used font for text input.
        return m_caret.charFormat().font();
    case Qt::ImCursorPosition:
        // The logical position of the cursor within the text surrounding the input area (see ImSurroundingText).
        return m_caret.position() - m_caret.block().position();
    case Qt::ImSurroundingText:
        // The plain text around the input area, for example the current paragraph.
        return m_caret.block().text();
    case Qt::ImCurrentSelection:
        // The currently selected text.
        return m_caret.selectedText();
    }
    return QVariant();
}

void TextTool::ensureCursorVisible() {
    if(m_textShapeData->endPosition() < m_caret.position() || m_textShapeData->position() > m_caret.position()) {
        KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
        Q_ASSERT(lay);
        foreach(KoShape* shape, lay->shapes()) {
            TextShape *textShape = dynamic_cast<TextShape*> (shape);
            Q_ASSERT(textShape);
            KoTextShapeData *d = static_cast<KoTextShapeData*> (textShape->userData());
            if(m_caret.position() >= d->position() && m_caret.position() <= d->endPosition()) {
                m_textShapeData = d;
                m_textShape = textShape;
                break;
            }
        }
    }

    QRectF cursorPos = textRect(m_caret.position(), m_caret.position());
    if(! cursorPos.isValid()) { // paragraph is not yet layouted.
        // The number one usecase for this is when the user pressed enter.
        // So take bottom of last paragraph.
        QTextBlock block = m_caret.block().previous();
        if(block.isValid()) {
            double y = block.layout()->boundingRect().bottom();
            cursorPos = QRectF(0, y, 1, 10);
        }
    }
    cursorPos.moveTop(cursorPos.top() - m_textShapeData->documentOffset());
    m_canvas->ensureVisible(m_textShape->absoluteTransformation(0).mapRect(cursorPos));
}

void TextTool::keyReleaseEvent(QKeyEvent *event) {
    event->ignore();
}

void TextTool::updateActions() {
    m_allowActions = false;
    QTextCharFormat cf = m_caret.charFormat();
    m_actionFormatBold->setChecked(cf.fontWeight() > QFont::Normal);
    m_actionFormatItalic->setChecked(cf.fontItalic());
    m_actionFormatUnderline->setChecked(cf.intProperty(KoCharacterStyle::UnderlineType) != KoCharacterStyle::NoLineType);
    m_actionFormatStrikeOut->setChecked(cf.intProperty(KoCharacterStyle::StrikeOutType) != KoCharacterStyle::NoLineType);
    bool super=false, sub=false;
    switch(cf.verticalAlignment()) {
        case QTextCharFormat::AlignSuperScript: super = true; break;
        case QTextCharFormat::AlignSubScript: sub = true; break;
        default:;
    }
    m_actionFormatSuper->setChecked(super);
    m_actionFormatSub->setChecked(sub);
    m_actionFormatFontSize->setFontSize(qRound(cf.fontPointSize()));
    m_actionFormatFontFamily->setFont(cf.font().family());

    QTextBlockFormat bf = m_caret.blockFormat();
    if(bf.alignment() == Qt::AlignLeading || bf.alignment() == Qt::AlignTrailing) {
        bool revert = (m_caret.block().layout()->textOption().textDirection() == Qt::LeftToRight) != QApplication::isLeftToRight();
        if(bf.alignment() == Qt::AlignLeading ^ revert)
            m_actionAlignLeft->setChecked(true);
        else
            m_actionAlignRight->setChecked(true);
    }
    else if(bf.alignment() == Qt::AlignHCenter)
        m_actionAlignCenter->setChecked(true);
    if(bf.alignment() == Qt::AlignJustify)
        m_actionAlignBlock->setChecked(true);
    else if(bf.alignment() == (Qt::AlignLeft | Qt::AlignAbsolute))
        m_actionAlignLeft->setChecked(true);
    else if(bf.alignment() == (Qt::AlignRight | Qt::AlignAbsolute))
        m_actionAlignRight->setChecked(true);

    m_actionFormatDecreaseIndent->setEnabled(m_caret.blockFormat().leftMargin() > 0.);
    m_allowActions = true;

    //action("text_default")->setEnabled(m_caret.hasSelection());

    emit charFormatChanged(cf);
    emit blockFormatChanged(bf);
    emit blockChanged(m_caret.block());
}

void TextTool::updateStyleManager() {
    Q_ASSERT(m_textShapeData);
    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
    if(lay)
        emit styleManagerChanged(lay->styleManager());
    else {
        emit styleManagerChanged(0);
        kWarning(32500) << "Shape does not have a KoTextDocumentLayout\n";
    }
}

void TextTool::activate (bool temporary) {
    Q_UNUSED(temporary);
    KoSelection *selection = m_canvas->shapeManager()->selection();
    foreach(KoShape *shape, selection->selectedShapes()) {
        m_textShape = dynamic_cast<TextShape*> (shape);
        if(m_textShape)
            break;
    }
    if(m_textShape == 0) { // none found
        emit done();
        return;
    }
    foreach(KoShape *shape, selection->selectedShapes()) {
        // deselect others.
        if(m_textShape == shape) continue;
        selection->deselect(shape);
    }
    setShapeData(static_cast<KoTextShapeData*> (m_textShape->userData()));
    useCursor(Qt::IBeamCursor, true);

    // restore the selection from a previous time we edited this document.
    for(int i=0; i < m_previousSelections.count(); i++ ) {
        TextSelection selection = m_previousSelections.at(i);
        if(selection.document == m_textShapeData->document()) {
            m_caret.setPosition(selection.anchor);
            m_caret.setPosition(selection.position, QTextCursor::KeepAnchor);
            m_previousSelections.removeAt(i);
            break;
        }
    }

    if (m_needSpellChecking && m_spellcheckPlugin) {
        foreach (KoShape *shape, m_canvas->shapeManager()->shapes()) {
            TextShape *textShape = dynamic_cast<TextShape*>(shape);
            if (textShape) {
                KoTextShapeData *textShapeData = static_cast<KoTextShapeData*>(textShape->userData());
                m_spellcheckPlugin->checkSection(textShapeData->document(), 0, -1);
            }
        }
        m_needSpellChecking = false;
    }

    m_textShape->repaint();

    updateSelectionHandler();
    updateActions();
    updateStyleManager();
    if(m_specialCharacterDocker)
        m_specialCharacterDocker->setEnabled(true);
}

void TextTool::deactivate() {
    m_textShape = 0;
    if(m_textShapeData) {
        m_textShapeData->document()->setUndoRedoEnabled(false); // erase undo history.
        TextSelection selection;
        selection.document = m_textShapeData->document();
        selection.position = m_caret.position();
        selection.anchor = m_caret.anchor();
        m_previousSelections.append(selection);
    }
    setShapeData(0);
    if(m_previousSelections.count() > 20) // don't let it grow indefinitely
        m_previousSelections.removeAt(0);

    updateSelectionHandler();
    if(m_specialCharacterDocker) {
        m_specialCharacterDocker->setEnabled(false);
        m_specialCharacterDocker->setVisible(false);
    }
}

void TextTool::repaintDecorations() {
    if(m_textShapeData)
        repaintSelection();
}

void TextTool::repaintCaret() {
    QTextBlock block = m_caret.block();
    if(block.isValid()) {
        QTextLine tl = block.layout()->lineForTextPosition(m_caret.position() - block.position());
        QRectF repaintRect;
        if(tl.isValid()) {
            repaintRect = tl.rect();
            KoTextDocumentLayout::updateTabsForLine(block, tl.lineNumber());
            repaintRect.setX(tl.cursorToX(m_caret.position() - block.position()) - 2);
            repaintRect.setWidth(6);
        }
        repaintRect.moveTop(repaintRect.y() - m_textShapeData->documentOffset());
        repaintRect = m_textShape->absoluteTransformation(0).mapRect(repaintRect);
        m_canvas->updateCanvas(repaintRect);
    }
}

void TextTool::repaintSelection() {
    repaintSelection(m_caret.position(), m_caret.anchor());
}

void TextTool::repaintSelection(int startPosition, int endPosition) {
    QList<TextShape *> shapes;
    if(m_textShapeData->position() > startPosition || m_textShapeData->endPosition() < endPosition) {
        KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
        Q_ASSERT(lay);
        foreach(KoShape* shape, lay->shapes()) {
            TextShape *textShape = dynamic_cast<TextShape*> (shape);
            Q_ASSERT(textShape);
            if(textShape->textShapeData()->position() >= startPosition &&
                    textShape->textShapeData()->endPosition() <= endPosition)
                shapes.append(textShape);
        }
    }
    else // the simple case; the full selection is inside the current shape.
        shapes.append(m_textShape);

    // loop over all shapes that contain the text and update per shape.
    QRectF repaintRect = textRect(startPosition, endPosition);
    foreach(TextShape *ts, shapes) {
        QRectF rect = repaintRect;
        rect.moveTop(rect.y() - ts->textShapeData()->documentOffset());
        rect = ts->absoluteTransformation(0).mapRect(rect);
        m_canvas->updateCanvas(ts->boundingRect().intersected(rect));
    }
}

QRectF TextTool::textRect(int startPosition, int endPosition) const {
    if(startPosition > endPosition)
        qSwap(startPosition, endPosition);
    QTextBlock block = m_textShapeData->document()->findBlock(startPosition);
    QTextLine line1 = block.layout()->lineForTextPosition(startPosition - block.position());
    if(! line1.isValid())
        return QRectF();
    KoTextDocumentLayout::updateTabsForLine(block, line1.lineNumber());
    double startX = line1.cursorToX(startPosition - block.position());
    if(startPosition == endPosition)
        return QRectF(startX, line1.y(), 1, line1.height());

    QTextBlock block2 = m_textShapeData->document()->findBlock(endPosition);
    QTextLine line2 = block2.layout()->lineForTextPosition(endPosition - block2.position());
    if(! line2.isValid())
        return QRectF();
    KoTextDocumentLayout::updateTabsForLine(block2, line2.lineNumber());
    double endX = line2.cursorToX(endPosition - block2.position());

    if(line1.textStart() + block.position() == line2.textStart()+ block2.position() )
        return QRectF(qMin(startX, endX), line1.y(), qAbs(startX - endX), line1.height());
    return QRectF(0, line1.y(), 10E6, line2.y() + line2.height() - line1.y());
}

KoToolSelection* TextTool::selection() {
    return &m_selectionHandler;
}

QWidget *TextTool::createOptionWidget() {
    QTabWidget *widget = new QTabWidget();
    SimpleStyleWidget *ssw = new SimpleStyleWidget(this, widget);
    widget->addTab(ssw, i18n("Style"));
    StylesWidget *paragTab = new StylesWidget(StylesWidget::ParagraphStyle, widget);
    widget->addTab(paragTab, i18n("Paragraph"));
    StylesWidget *charTab =new StylesWidget(StylesWidget::CharacterStyle, widget);
    widget->addTab(charTab, i18n("Character"));

    connect(this, SIGNAL(styleManagerChanged(KoStyleManager *)), ssw, SLOT(setStyleManager(KoStyleManager *)));
    connect(this, SIGNAL(blockChanged(const QTextBlock&)), ssw, SLOT(setCurrentBlock(const QTextBlock&)));
    connect(this, SIGNAL(styleManagerChanged(KoStyleManager *)), paragTab, SLOT(setStyleManager(KoStyleManager *)));
    connect(this, SIGNAL(styleManagerChanged(KoStyleManager *)), charTab, SLOT(setStyleManager(KoStyleManager *)));
    connect(this, SIGNAL(charFormatChanged(const QTextCharFormat &)),
            paragTab, SLOT(setCurrentFormat(const QTextCharFormat &)));
    connect(this, SIGNAL(blockFormatChanged(const QTextBlockFormat &)),
            paragTab, SLOT(setCurrentFormat(const QTextBlockFormat &)));
    connect(this, SIGNAL(charFormatChanged(const QTextCharFormat &)),
            charTab, SLOT(setCurrentFormat(const QTextCharFormat &)));

    connect(paragTab, SIGNAL(paragraphStyleSelected(KoParagraphStyle *)),
            &m_selectionHandler, SLOT(setStyle(KoParagraphStyle*)));
    connect(charTab, SIGNAL(characterStyleSelected(KoCharacterStyle *)),
            &m_selectionHandler, SLOT(setStyle(KoCharacterStyle*)));

    updateStyleManager();
    if(m_textShape)
        updateActions();
    return widget;
}

void TextTool::addUndoCommand() {
    if(! m_allowAddUndoCommand) return;
    class UndoTextCommand : public QUndoCommand {
      public:
        UndoTextCommand(QTextDocument *document, TextTool *tool, QUndoCommand *parent = 0)
            : QUndoCommand(i18n("Text"), parent),
            m_document(document),
            m_tool(tool)
        {
        }

        void undo () {
            if(m_document.isNull())
                return;
            if(! m_tool.isNull()) {
                m_tool->m_allowAddUndoCommand = false;
                if(m_tool->m_changeTracker)
                    m_tool->m_changeTracker->notifyForUndo();
                m_document->undo(&m_tool->m_caret);
            }
            else
                m_document->undo();
            if(! m_tool.isNull())
                m_tool->m_allowAddUndoCommand = true;
        }

        void redo () {
            if(m_document.isNull())
                return;
            if(! m_tool.isNull()) {
                m_tool->m_allowAddUndoCommand = false;
                 m_document->redo(&m_tool->m_caret);
            }
            else
                m_document->redo();
            if(! m_tool.isNull())
                m_tool->m_allowAddUndoCommand = true;
        }

        QPointer<QTextDocument> m_document;
        QPointer<TextTool> m_tool;
    };
    if(m_currentCommand) {
        new UndoTextCommand(m_textShapeData->document(), this, m_currentCommand);
        m_currentCommandHasChildren = true;
    }
    else
        m_canvas->addCommand(new UndoTextCommand(m_textShapeData->document(), this));
}

void TextTool::addCommand(QUndoCommand *command) {
    m_currentCommand = command;
    TextCommandBase *cmd = dynamic_cast<TextCommandBase*> (command);
    m_canvas->addCommand(command); // will execute it.
    if(cmd)
        cmd->setTool(this);
    m_currentCommand = 0;
}

void TextTool::nonbreakingSpace() {
    if(! m_allowActions) return;
    m_selectionHandler.insert(QString(QChar(Qt::Key_nobreakspace)));
}

void TextTool::nonbreakingHyphen() {
    if(! m_allowActions) return;
    m_selectionHandler.insert(QString(QChar(0x2013)));
}

void TextTool::softHyphen() {
    if(! m_allowActions) return;
    m_selectionHandler.insert(QString(QChar(Qt::Key_hyphen)));
}

void TextTool::lineBreak() {
    if(! m_allowActions) return;
    m_selectionHandler.insert(QString(QChar(0x2028)));
}

void TextTool::alignLeft() {
    if(! m_allowActions) return;
    Qt::Alignment align = Qt::AlignLeading;
    if(m_caret.block().layout()->textOption().textDirection() != Qt::LeftToRight)
        align |= Qt::AlignTrailing;
    m_selectionHandler.setHorizontalTextAlignment(align);
}

void TextTool::alignRight() {
    if(! m_allowActions) return;
    Qt::Alignment align = Qt::AlignTrailing;
    if(m_caret.block().layout()->textOption().textDirection() == Qt::RightToLeft)
        align = Qt::AlignLeading;
    m_selectionHandler.setHorizontalTextAlignment(align);
}

void TextTool::alignCenter() {
    if(! m_allowActions) return;
    m_selectionHandler.setHorizontalTextAlignment(Qt::AlignHCenter);
}

void TextTool::alignBlock() {
    if(! m_allowActions) return;
    m_selectionHandler.setHorizontalTextAlignment(Qt::AlignJustify);
}

void TextTool::superScript(bool on) {
    if(! m_allowActions) return;
    if(on)
        m_actionFormatSub->setChecked(false);
    m_selectionHandler.setVerticalTextAlignment(on ? Qt::AlignTop : Qt::AlignVCenter);
}

void TextTool::subScript(bool on) {
    if(! m_allowActions) return;
    if(on)
        m_actionFormatSuper->setChecked(false);
    m_selectionHandler.setVerticalTextAlignment(on ? Qt::AlignBottom : Qt::AlignVCenter);
}

void TextTool::increaseIndent() {
    if(! m_allowActions) return;
    m_selectionHandler.increaseIndent();
    m_actionFormatDecreaseIndent->setEnabled(m_caret.blockFormat().leftMargin() > 0.);
}

void TextTool::decreaseIndent() {
    if(! m_allowActions) return;
    m_selectionHandler.decreaseIndent();
    m_actionFormatDecreaseIndent->setEnabled(m_caret.blockFormat().leftMargin() > 0.);
}

void TextTool::textDefaultFormat() {
    // TODO
    kDebug() <<"TextTool::textDefaultFormat";
}

void TextTool::insertIndexMarker() {
    // TODO handle result when we figure out how to report errors from a tool.
    m_selectionHandler.insertIndexMarker();
}

void TextTool::addBookmark() {
    QString name, suggestedName;
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout *>(m_textShapeData->document()->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineObjectTextManager());
    KoBookmarkManager *manager = layout->inlineObjectTextManager()->bookmarkManager();
    if (m_caret.hasSelection())
        suggestedName = m_caret.selectedText();

    CreateNewBookmarkDialog *dia = new CreateNewBookmarkDialog(manager->bookmarkNameList(), suggestedName, m_canvas->canvasWidget());
    if (dia->exec() == QDialog::Accepted) {
        name = dia->newBookmarkName();
    }
    else {
        delete dia;
        return;
    }
    delete dia;

    m_selectionHandler.addBookmark(name, m_textShape);
}

void TextTool::selectBookmark() {
    QString name;
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout *>(m_textShapeData->document()->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineObjectTextManager());
    KoBookmarkManager *manager = layout->inlineObjectTextManager()->bookmarkManager();

    SelectBookmarkDialog *dia = new SelectBookmarkDialog(manager->bookmarkNameList(), m_canvas->canvasWidget());
    connect( dia, SIGNAL( nameChanged(const QString &, const QString &) ),
            manager, SLOT( rename(const QString &, const QString &) ) );
    connect( dia, SIGNAL( bookmarkDeleted(const QString &) ),
            this, SLOT( deleteBookmark(const QString &) ) );
    if (dia->exec() == QDialog::Accepted) {
        name = dia->selectedBookmarkName();
    }
    else {
        delete dia;
        return;
    }
    delete dia;

    if (m_caret.hasSelection())
        repaintSelection(m_caret.position(), m_caret.anchor()); // erase selection
    else
        repaintCaret();

    KoBookmark *bookmark = manager->retrieveBookmark(name);
    KoShape *shape = bookmark->shape();
    if (!(shape == m_textShape)) {
        m_textShape = static_cast<TextShape *>(shape);
        setShapeData(m_textShape->textShapeData());
        updateSelectionHandler();
    }

    if (bookmark->hasSelection()) {
        m_caret.setPosition(bookmark->position());
        m_caret.setPosition(bookmark->endBookmark()->position() + 1, QTextCursor::KeepAnchor);
        repaintSelection(m_caret.selectionStart(), m_caret.selectionEnd());
    }
    else {
        m_caret.setPosition(bookmark->position() + 1);
        repaintCaret();
    }
    ensureCursorVisible();
}

void TextTool::deleteBookmark(const QString &name) {
    if (!name.isNull()) {
        TextShape *prevShape = m_textShape;
        QTextCursor copyCursor = m_caret;
        int startPosition;
        int endPosition = -1;

        KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout *>(m_textShapeData->document()->documentLayout());
        Q_ASSERT(layout);
        Q_ASSERT(layout->inlineObjectTextManager());
        KoBookmarkManager *manager = layout->inlineObjectTextManager()->bookmarkManager();
        KoBookmark *bookmark = manager->retrieveBookmark(name);

        KoShape *shape = bookmark->shape();
        if (!(shape == m_textShape)) {
            m_textShape = static_cast<TextShape *>(shape);
            setShapeData(m_textShape->textShapeData());
            updateSelectionHandler();
        }

        if (bookmark->hasSelection()) {
            KoBookmark *endBookmark = bookmark->endBookmark();
            endPosition = endBookmark->position() - 1;
        }

        m_caret.setPosition(bookmark->position());
        m_selectionHandler.deleteInlineObjects(false);

        if (endPosition != -1) {
            m_caret.setPosition(endPosition);
            m_selectionHandler.deleteInlineObjects(false);
        }

        startPosition = copyCursor.selectionStart();
        endPosition = copyCursor.selectionEnd();

        // restore cursor / selection to original position
        if (!(prevShape == m_textShape)) {
            m_textShape = prevShape;
            setShapeData(m_textShape->textShapeData());
            updateSelectionHandler();
        }
        m_caret.setPosition(startPosition);

        if (startPosition != endPosition) {
            m_caret.setPosition(endPosition, QTextCursor::KeepAnchor);
            repaintSelection(startPosition, endPosition);
        }
        else {
            repaintCaret();
        }
    }
}

void TextTool::formatParagraph() {
    ParagraphSettingsDialog *dia = new ParagraphSettingsDialog(m_canvas->canvasWidget(), this);
    dia->open(m_caret);
    dia->setUnit(m_canvas->unit());
    connect(dia, SIGNAL(startMacro(const QString&)), this, SLOT(startMacro(const QString&)));
    connect(dia, SIGNAL(stopMacro()), this, SLOT(stopMacro()));

    dia->show();
}

void TextTool::toggleTrackChanges(bool on) {
    m_trackChanges = on;
    if(m_textShapeData && on){
        if(m_changeTracker == 0)
            m_changeTracker = new ChangeTracker(this);
        if(m_changeTracker)
            m_changeTracker->setDocument(m_textShapeData->document());
    }
    else if(m_changeTracker)
        m_changeTracker->setDocument(0);
}

void TextTool::selectAll() {
    if(m_textShapeData == 0) return;
    QTextBlock lastBlock = m_textShapeData->document()->end().previous();
    m_caret.setPosition(lastBlock.position() + lastBlock.length() - 1);
    m_caret.setPosition(0, QTextCursor::KeepAnchor);
    repaintSelection(0, m_caret.anchor());
}

void TextTool::startMacro(const QString &title) {
    if(m_currentCommand) return;
    class MacroCommand : public QUndoCommand {
      public:
        MacroCommand(const QString &title) : QUndoCommand(title), m_first(true) {}
        virtual void redo() {
            if(! m_first)
                QUndoCommand::redo();
            m_first = false;
        }
        virtual bool mergeWith (const QUndoCommand *) { return false; }
        bool m_first;
    };
    m_currentCommand = new MacroCommand(title);
    m_currentCommandHasChildren = false;
}

void TextTool::stopMacro() {
    if(m_currentCommand == 0) return;
    if(m_currentCommandHasChildren)
        m_canvas->addCommand(m_currentCommand);
    else
        delete m_currentCommand;
    m_currentCommand = 0;
}

void TextTool::showStyleManager() {
    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (m_textShapeData->document()->documentLayout());
    if(lay) {
        Q_ASSERT( lay->styleManager() );
        if( ! lay->styleManager() ) return; //don't crash
        StyleManagerDialog *dia = new StyleManagerDialog(m_canvas->canvasWidget());
        dia->setStyleManager(lay->styleManager());
        dia->setUnit(m_canvas->unit());
        dia->show();
    }
}

void TextTool::startTextEditingPlugin(const QString &pluginId) {
    KoTextEditingPlugin *plugin = m_textEditingPlugins.value(pluginId);
    if(plugin) {
        if(m_caret.hasSelection()) {
            int from = m_caret.position();
            int to = m_caret.anchor();
            if(from > to) // make sure we call the plugin consistently
                qSwap(from, to);
            plugin->checkSection(m_textShapeData->document(), from, to);
        }
        else
            plugin->finishedWord(m_textShapeData->document(), m_caret.position());
    }
}

bool TextTool::isBidiDocument() const {
    if(m_canvas->resourceProvider())
        return m_canvas->resourceProvider()->boolResource(KoText::BidiDocument);
    return false;
}

void TextTool::updateParagraphDirection(const QVariant &variant) {
    int position = variant.toInt();
    KoTextShapeData *data = m_textShapeData;
    if(data == 0) // tools is deactivated already
        return;
    m_updateParagDirection.block = data->document()->findBlock(position);
    m_updateParagDirection.direction = KoText::AutoDirection;
    if(! m_updateParagDirection.block.isValid())
        return;
    QTextBlockFormat format = m_updateParagDirection.block.blockFormat();
    KoText::Direction dir = static_cast<KoText::Direction> (format.intProperty(
                KoParagraphStyle::TextProgressionDirection));
    if(dir == KoText::AutoDirection || dir == KoText::PerhapsLeftRightTopBottom ||
            dir == KoText::PerhapsRightLeftTopBottom) {
        bool rtl = isRightToLeft(m_updateParagDirection.block.text());
        if(rtl && (dir != KoText::AutoDirection || QApplication::isLeftToRight()))
            m_updateParagDirection.direction = KoText::PerhapsRightLeftTopBottom;
        else if(!rtl && (dir != KoText::AutoDirection || QApplication::isRightToLeft())) // remove previously set one if needed.
            m_updateParagDirection.direction = KoText::PerhapsLeftRightTopBottom;
    }
    else
        m_updateParagDirection.block = QTextBlock();
}

void TextTool::updateParagraphDirectionUi() {
    if(! m_updateParagDirection.block.isValid())
        return;
    QTextCursor cursor(m_updateParagDirection.block);
    QTextBlockFormat format = cursor.blockFormat();
    if(format.property(KoParagraphStyle::TextProgressionDirection).toInt() != m_updateParagDirection.direction) {
        format.setProperty(KoParagraphStyle::TextProgressionDirection, m_updateParagDirection.direction);
        cursor.setBlockFormat(format); // note that setting this causes a re-layout.
    }

    if(m_canvas->resourceProvider() && ! isBidiDocument()) {
        if((QApplication::isLeftToRight() &&
                (m_updateParagDirection.direction == KoText::RightLeftTopBottom ||
                 m_updateParagDirection.direction == KoText::PerhapsRightLeftTopBottom)) ||
                (QApplication::isRightToLeft() &&
                 (m_updateParagDirection.direction == KoText::LeftRightTopBottom ||
                  m_updateParagDirection.direction == KoText::PerhapsLeftRightTopBottom))) {
            m_canvas->resourceProvider()->setResource(KoText::BidiDocument, true);

            emit blockChanged(m_caret.block()); // make sure that the dialogs follow this change
        }
    }
    updateActions();
}

void TextTool::resourceChanged(int key, const QVariant &var) {
    if(m_allowResourceProviderUpdates == false)
        return;
    if(key == KoText::CurrentTextPosition) {
        repaintSelection();
        m_caret.setPosition(var.toInt());
        ensureCursorVisible();
    }
    else if(key == KoText::CurrentTextAnchor) {
        repaintSelection();
        int pos = m_caret.position();
        m_caret.setPosition(var.toInt());
        m_caret.setPosition(pos, QTextCursor::KeepAnchor);
    }
    else return;

    repaintSelection();
}

void TextTool::insertSpecialCharacter() {
    if(m_specialCharacterDocker == 0) {
        m_specialCharacterDocker = new InsertCharacter(m_canvas->canvasWidget());
        connect(m_specialCharacterDocker, SIGNAL(insertCharacter(const QString&)),
                &m_selectionHandler, SLOT(insert(const QString&)));
    }

    m_specialCharacterDocker->show();
}

// ---------- editing plugins methods.
void TextTool::editingPluginEvents() {
    if(m_prevCursorPosition == -1 || m_prevCursorPosition == m_caret.position())
        return;

    QTextBlock block = m_caret.block();
    if(! block.contains(m_prevCursorPosition)) {
        finishedWord();
        finishedParagraph();
        m_prevCursorPosition = -1;
    }
    else {
        int from = m_prevCursorPosition;
        int to = m_caret.position();
        if(from > to)
            qSwap(from, to);
        QString section = block.text().mid(from - block.position(), to - from);
        if(section.contains(' ')) {
            finishedWord();
            m_prevCursorPosition = -1;
        }
    }
}

void TextTool::finishedWord() {
    foreach(KoTextEditingPlugin* plugin, m_textEditingPlugins.values())
        plugin->finishedWord(m_textShapeData->document(), m_prevCursorPosition);
}

void TextTool::finishedParagraph() {
    foreach(KoTextEditingPlugin* plugin, m_textEditingPlugins.values())
        plugin->finishedParagraph(m_textShapeData->document(), m_prevCursorPosition);
}

void TextTool::setTextColor(const KoColor &color)
{
    m_selectionHandler.setTextColor(color.toQColor());
}

#include "TextTool.moc"
