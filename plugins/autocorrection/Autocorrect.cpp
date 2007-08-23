/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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
#include "Autocorrect.h"
#include "AutocorrectConfigDialog.h"

#include <QTextBlock>
#include <QAction>
#include <QFile>
#include <QDomDocument>

#include <KLocale>
#include <KConfigGroup>
#include <KCalendarSystem>
#include <KStandardDirs>
#include <KDebug>

#include <KoGlobal.h>

Autocorrect::Autocorrect() {
    /* setup actions for this plugin */
    QAction *configureAction = new QAction(i18n("Configure &Autocorrection..."), this);
    connect(configureAction, SIGNAL(triggered()), this, SLOT(configureAutocorrect()));
    addAction("configure_autocorrection", configureAction);

    m_singleSpaces = true;
    m_uppercaseFirstCharOfSentence = false;
    m_fixTwoUppercaseChars = false;
    m_autoFormatURLs = false;
    m_trimParagraphs = true;
    m_autoBoldUnderline = false;
    m_autoFractions = true;
    m_autoNumbering = false;
    m_capitalizeWeekDays = false;
    m_autoFormatBulletList = false;
    m_replaceDoubleQuotes = false;
    m_replaceSingleQuotes = false;

    readConfig();

    KLocale *locale = KGlobal::locale();
    for (int i = 1; i <=7; i++)
        m_cacheNameOfDays.append(locale->calendar()->weekDayName(i).toLower());

    // default double quote open 0x201c
    // default double quote close 0x201d
}

Autocorrect::~Autocorrect()
{
    writeConfig();
}

void Autocorrect::finishedWord(QTextDocument *document, int cursorPosition) {
    m_cursor = QTextCursor(document);
    selectWord(m_cursor, cursorPosition);
    m_word = m_cursor.selectedText();
    if (m_word.isEmpty()) return;

    emit startMacro(i18n("Autocorrection"));

    bool done = autoFormatURLs();
    if(!done) done = singleSpaces();
    if(!done) done = autoBoldUnderline();
    if(!done) done = autoFractions();
    if(!done) advancedAutocorrect();
    if(!done) uppercaseFirstCharOfSentence();
    if(!done) fixTwoUppercaseChars();
    if(!done) autoNumbering();
    if(!done) superscriptAppendix();
    if(!done) capitalizeWeekDays();
    if(!done) autoFormatBulletList();
    if(!done) replaceDoubleQuotes();
    if(!done) replaceSingleQuotes();

    if(m_cursor.selectedText() != m_word)
        m_cursor.insertText(m_word);

    emit stopMacro();
}

void Autocorrect::finishedParagraph(QTextDocument *document, int cursorPosition) {
    if(! m_trimParagraphs) return;
    // TODO
}

void Autocorrect::setUpperCaseExceptions(QSet<QString> exceptions) { m_upperCaseExceptions = exceptions; }
void Autocorrect::setTwoUpperLetterExceptions(QSet<QString> exceptions) { m_twoUpperLetterExceptions = exceptions; }
void Autocorrect::setAutocorrectEntries(QHash<QString, QString> entries) { m_autocorrectEntries = entries; }

QSet<QString> Autocorrect::getUpperCaseExceptions() { return m_upperCaseExceptions; }
QSet<QString> Autocorrect::getTwoUpperLetterExceptions() { return m_twoUpperLetterExceptions; }
QHash<QString, QString> Autocorrect::getAutocorrectEntries() { return m_autocorrectEntries; }

void Autocorrect::configureAutocorrect()
{
    AutocorrectConfigDialog *cfgDlg = new AutocorrectConfigDialog(this);
    if (cfgDlg->exec()) {
        // TODO
    }
}

// ******************** individual features;

void Autocorrect::uppercaseFirstCharOfSentence() {
    if(! m_uppercaseFirstCharOfSentence) return;

    int startPos = m_cursor.selectionStart();
    QTextBlock block = m_cursor.block();

    m_cursor.setPosition(block.position());
    m_cursor.setPosition(startPos, QTextCursor::KeepAnchor);

    int position = m_cursor.selectionEnd();

    QString text = m_cursor.selectedText();

    if (text.isEmpty()) // start of a paragraph
        m_word.replace(0, 1, m_word.at(0).toUpper());
    else {
        QString::ConstIterator constIter = text.constEnd();
        constIter--;

        while (constIter != text.constBegin()) {
            while (constIter != text.begin() && constIter->isSpace()) {
                constIter--;
                position--;
            }

            if (constIter != text.constBegin() && (*constIter == QChar('.') || *constIter == QChar('!') || *constIter == QChar('?'))) {
                constIter--;
                while (constIter != text.constBegin() && !(constIter->isLetter())) {
                    position--;
                    constIter--;
                }
                bool replace = true;
                selectWord(m_cursor, --position);
                QString prevWord = m_cursor.selectedText();

                // search for exception
                if (m_upperCaseExceptions.contains(prevWord.trimmed()))
                    replace = false;

                if (replace)
                    m_word.replace(0, 1, m_word.at(0).toUpper());
                break;
            }
            else
                break;
        }
    }

    m_cursor.setPosition(startPos);
    m_cursor.setPosition(startPos + m_word.length(), QTextCursor::KeepAnchor);
}

void Autocorrect::fixTwoUppercaseChars() {
    if(! m_fixTwoUppercaseChars) return;
    if (m_word.length() <= 2) return;

    if (m_twoUpperLetterExceptions.contains(m_word.trimmed()))
        return;

    QChar secondChar = m_word.at(1);

    if (secondChar.isUpper()) {
        QChar thirdChar = m_word.at(2);

        if (thirdChar.isLower())
            m_word.replace(1, 1, secondChar.toLower());
    }
}

bool Autocorrect::autoFormatURLs() {
    if(! m_autoFormatURLs) return false;

    QString link = autoDetectURL(m_word);
    if (link.isNull()) return false;

    QString trimmed = m_word.trimmed();
    int startPos = m_cursor.selectionStart();
    m_cursor.setPosition(startPos);
    m_cursor.setPosition(startPos + trimmed.length(), QTextCursor::KeepAnchor);
    
    QTextCharFormat format;
    format.setAnchor(true);
    format.setAnchorHref(link);
    format.setFontItalic(true); // TODO: formatting
    m_cursor.mergeCharFormat(format);

    m_word = m_cursor.selectedText();
    return true;
}

bool Autocorrect::singleSpaces() {
    if(! m_singleSpaces) return false;
    if(!m_cursor.atBlockStart() && m_word.length() == 1 && m_word.at(0) == ' ') {
        // then when the prev char is also a space, don't insert one.
        QTextBlock block = m_cursor.block();
        QString text = block.text();
        if(text.at(m_cursor.position() -1 - block.position()) == ' ') {
            m_word.clear();
            return true;
        }
    }
    return false;
}

bool Autocorrect::autoBoldUnderline() {
    if(! m_autoBoldUnderline) return false;

    QString trimmed = m_word.trimmed();

    if (trimmed.length() < 3) return false;

    bool underline = (trimmed.at(0) == '_' && trimmed.at(trimmed.length() - 1) == '_');
    bool bold = (trimmed.at(0) == '*' && trimmed.at(trimmed.length() - 1) == '*');

    if (underline || bold) {
        int startPos = m_cursor.selectionStart();
        QString replacement = trimmed.mid(1, trimmed.length() - 2);
        bool foundLetterNumber = false;

        QString::ConstIterator constIter = replacement.constBegin();
        while (constIter != replacement.constEnd()) {
            if (constIter->isLetterOrNumber()) {
                foundLetterNumber = true;
                break;
            }
            constIter++;
        }

        // if no letter/number found, don't apply autocorrection like in OOo 2.x
        if (!foundLetterNumber)
            return false;

        m_cursor.setPosition(startPos);
        m_cursor.setPosition(startPos + trimmed.length(), QTextCursor::KeepAnchor);
        m_cursor.insertText(replacement);
        m_cursor.setPosition(startPos);
        m_cursor.setPosition(startPos + replacement.length(), QTextCursor::KeepAnchor);

        QTextCharFormat format;
        format.setFontUnderline(underline);
        format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
        m_cursor.mergeCharFormat(format);

        // to avoid the selection being replaced by m_word
        m_word = m_cursor.selectedText();
    }
    else
        return false;

    return true;
}

bool Autocorrect::autoFractions() {
    if(! m_autoFractions) return false;

    QString trimmed = m_word.trimmed();

    if (trimmed == QString("1/2"))
        m_word.replace(0, 3, QString("½"));
    else if (trimmed == QString("1/4"))
        m_word.replace(0, 3, QString("¼"));
    else if (trimmed == QString("3/4"))
        m_word.replace(0, 3, QString("¾"));
    else
        return false;

    return true;
}

void Autocorrect::autoNumbering() {
    if(! m_autoNumbering) return;
    // TODO
}

void Autocorrect::superscriptAppendix() {
    if(! m_superscriptAppendix) return;
    // TODO
}

void Autocorrect::capitalizeWeekDays() {
    if(! m_capitalizeWeekDays) return;

    QString trimmed = m_word.trimmed();
    foreach (QString name, m_cacheNameOfDays) {
        if (trimmed == name) {
            int pos = m_word.indexOf(name);
            m_word.replace(pos, 1, name.at(0).toUpper());
            return;
        }
    }
}

void Autocorrect::autoFormatBulletList() {
    if(! m_autoFormatBulletList) return;
    // TODO
}

void Autocorrect::replaceDoubleQuotes() {
    if(! m_replaceDoubleQuotes) return;
    // TODO
}

void Autocorrect::replaceSingleQuotes() {
    if(! m_replaceSingleQuotes) return;
    // TODO
}

void Autocorrect::advancedAutocorrect()
{
    if (!m_advancedAutocorrect) return;

    int startPos = m_cursor.selectionStart();
    int length = m_word.length();

    QString word = m_word.toLower().trimmed();
    if (m_autocorrectEntries.contains(word)) {
        int pos = m_word.toLower().indexOf(word);
        QString replacement = m_autocorrectEntries.value(word);
        m_word.replace(pos, pos + word.length(), replacement);
    }

    // We do replacement here, since the length of new word might be different from length of
    // the old world. Length difference might affect other type of autocorrection
    m_cursor.setPosition(startPos);
    m_cursor.setPosition(startPos + length, QTextCursor::KeepAnchor);
    m_cursor.insertText(m_word);
    m_cursor.setPosition(startPos); // also restore the selection
    m_cursor.setPosition(startPos + m_word.length(), QTextCursor::KeepAnchor);
}

QString Autocorrect::autoDetectURL(const QString &_word) const
{
    QString word = _word;

    /* this method is ported from lib/kotext/KoAutoFormat.cpp KoAutoFormat::doAutoDetectUrl
     * from KOffice 1.x branch */
    // kDebug() <<"link:" << word;

    char link_type = 0;
    int pos = word.indexOf("http://");
    int tmp_pos = word.indexOf("https://");

    if (tmp_pos < pos && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf("mailto:/");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf("ftp://");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf("ftp.");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1) {
          pos = tmp_pos;
          link_type = 3;
    }
    tmp_pos = word.indexOf("file:/");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf("news:");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf("www.");
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1 && word.indexOf('.', tmp_pos+4) != -1 ) {
          pos = tmp_pos;
          link_type = 2;
    }
    tmp_pos = word.indexOf('@');
    if (pos == -1 && tmp_pos != -1) {
        pos = tmp_pos-1;
        QChar c;

        while (pos >= 0) {
            c = word.at(pos);
            if (c.isPunct() && c != '.' && c != '_') break;
            else --pos;
        }
        if (pos == tmp_pos - 1) // not a valid address
            pos = -1;
        else
            ++pos;
        link_type = 1;
    }

    if (pos != -1) {
        // A URL inside e.g. quotes (like "http://www.koffice.org" with the quotes) shouldn't include the quote in the URL.
	    while (!word.at(word.length()-1).isLetter() &&  !word.at(word.length()-1).isDigit() && word.at(word.length()-1) != '/')
            word.truncate(word.length() - 1);
        word.remove(0, pos);
        QString newWord = word;

        if (link_type == 1)
            newWord = QString("mailto:") + word;
        else if (link_type == 2)
            newWord = QString("http://") + word;
        else if (link_type == 3)
            newWord = QString("ftp://") + word;

        kDebug() <<"newWord:" << newWord;
        return newWord;
    }

    return QString();
}

void Autocorrect::readConfig()
{
    KConfigGroup interface = KoGlobal::kofficeConfig()->group("Autocorrect");

    m_uppercaseFirstCharOfSentence = interface.readEntry("UppercaseFirstCharOfSentence", m_uppercaseFirstCharOfSentence);
    m_fixTwoUppercaseChars = interface.readEntry("FixTwoUppercaseChars", m_fixTwoUppercaseChars);
    m_autoFormatURLs = interface.readEntry("AutoFormatURLs", m_autoFormatURLs);
    m_singleSpaces = interface.readEntry("SingleSpaces", m_singleSpaces);
    m_trimParagraphs = interface.readEntry("TrimParagraphs", m_trimParagraphs);
    m_autoBoldUnderline = interface.readEntry("AutoBoldUnderline", m_autoBoldUnderline);
    m_autoFractions = interface.readEntry("AutoFractions", m_autoFractions);
    m_autoNumbering = interface.readEntry("AutoNumbering", m_autoNumbering);
    m_superscriptAppendix = interface.readEntry("SuperscriptAppendix", m_superscriptAppendix);
    m_capitalizeWeekDays = interface.readEntry("CapitalizeWeekDays", m_capitalizeWeekDays);
    m_autoFormatBulletList = interface.readEntry("AutoFormatBulletList", m_autoFormatBulletList);
    m_advancedAutocorrect = interface.readEntry("AdvancedAutocorrect", m_advancedAutocorrect);

    m_replaceDoubleQuotes = interface.readEntry("ReplaceDoubleQuotes", m_replaceDoubleQuotes);
    m_replaceSingleQuotes = interface.readEntry("ReplaceSingleQuotes", m_replaceSingleQuotes);

    m_autocorrectLang = interface.readEntry("formatLanguage", m_autocorrectLang);

    readAutocorrectXmlEntry();
}

void Autocorrect::writeConfig()
{
    KConfigGroup interface = KoGlobal::kofficeConfig()->group("Autocorrect");
    interface.writeEntry("UppercaseFirstCharOfSentence", m_uppercaseFirstCharOfSentence);
    interface.writeEntry("FixTwoUppercaseChars", m_fixTwoUppercaseChars);
    interface.writeEntry("AutoFormatURLs", m_autoFormatURLs);
    interface.writeEntry("SingleSpaces", m_singleSpaces);
    interface.writeEntry("TrimParagraphs", m_trimParagraphs);
    interface.writeEntry("AutoBoldUnderline", m_autoBoldUnderline);
    interface.writeEntry("AutoFractions", m_autoFractions);
    interface.writeEntry("AutoNumbering", m_autoNumbering);
    interface.writeEntry("SuperscriptAppendix", m_superscriptAppendix);
    interface.writeEntry("CapitalizeWeekDays", m_capitalizeWeekDays);
    interface.writeEntry("AutoFormatBulletList", m_autoFormatBulletList);
    interface.writeEntry("AdvancedAutocorrect", m_advancedAutocorrect);

    interface.writeEntry("ReplaceDoubleQuotes", m_replaceDoubleQuotes);
    interface.writeEntry("ReplaceSingleQuotes", m_replaceSingleQuotes);

    interface.writeEntry("formatLanguage", m_autocorrectLang);
}

void Autocorrect::readAutocorrectXmlEntry()
{
    // Taken from KOffice 1.x KoAutoFormat.cpp
    KLocale *locale = KGlobal::locale();
    QString kdelang = locale->languageList().first();
    kdelang.remove(QRegExp("@.*"));

    QString fname;
    if (!m_autocorrectLang.isEmpty())
        fname = KGlobal::dirs()->findResource("data", "koffice/autocorrect/" + m_autocorrectLang + ".xml");
    if (m_autocorrectLang != "all_languages") {
        if (fname.isEmpty() && !kdelang.isEmpty())
            fname = KGlobal::dirs()->findResource("data", "koffice/autocorrect/" + kdelang + ".xml");
        if (fname.isEmpty() && kdelang.contains("_")) {
            kdelang.remove( QRegExp( "_.*" ) );
            fname = KGlobal::dirs()->findResource("data", "koffice/autocorrect/" + kdelang + ".xml");
        }
        if (fname.isEmpty())
            fname = KGlobal::dirs()->findResource("data", "koffice/autocorrect/autocorrect.xml");
    }
    if (m_autocorrectLang.isEmpty())
        m_autocorrectLang = kdelang;

    if (fname.isEmpty())
        return;

    QFile xmlFile(fname);
    if (!xmlFile.open(IO_ReadOnly))
        return;

    QDomDocument doc;
    if (!doc.setContent(&xmlFile))
        return;

    if (doc.doctype().name() != "autocorrection")
        return;
    
    QDomElement de = doc.documentElement();

    QDomElement upper = de.namedItem("UpperCaseExceptions").toElement();
    if (!upper.isNull()) {
        QDomNodeList nl = upper.childNodes();
        for (int i = 0; i < nl.count(); i++) 
            m_upperCaseExceptions += nl.item(i).toElement().attribute("exception");
    }

    QDomElement twoUpper = de.namedItem("TwoUpperLetterExceptions").toElement();
    if (!twoUpper.isNull()) {
        QDomNodeList nl = twoUpper.childNodes();
        for(int i = 0; i < nl.count(); i++)
            m_twoUpperLetterExceptions += nl.item(i).toElement().attribute("exception");
    }

    QDomElement superScript = de.namedItem("SuperScript").toElement();
    if (!superScript.isNull()) {
        QDomNodeList nl = superScript.childNodes();
        for(int i = 0; i < nl.count() ; i++)
            m_superScriptEntries.insert(nl.item(i).toElement().attribute("find"), nl.item(i).toElement().attribute("super"));
    }

    /* Load advanced autocorrect entry, including the format */
    QDomElement item = de.namedItem("items").toElement();
    if(!item.isNull())
    {
        QDomNodeList nl = item.childNodes();
        for (int i = 0; i < nl.count(); i++) {
            QDomElement element = nl.item(i).toElement();
            QString find = element.attribute("find");
            QString replace = element.attribute("replace");
            /* AutocorrectEntry entry;
            entry.replace = element.attribute("replace");
            if (element.hasAttribute("FONT"))
                entry.format.setFontFamily(element.attribute("FONT"));
            if (element.hasAttribute("SIZE"))
                entry.format.setFontPointSize(element.attribute("SIZE").toInt());
            if (element.hasAttribute("BOLD"))
                entry.format.setFontWeight(QFont::Bold);
            if (element.hasAttribute("ITALIC"))
                entry.format.setFontItalic(true);
            if (element.hasAttribute("UNDERLINE"))
                entry.format.setFontUnderline(true);
            if (element.hasAttribute("STRIKEOUT"))
                entry.format.setFontStrikeOut(true);
            if (element.hasAttribute("VERTALIGN"))
                entry.format.setVerticalAlignment(static_cast<QTextCharFormat::VerticalAlignment>(element.attribute("VERTALIGN").toInt()));
            if (element.hasAttribute("TEXTCOLOR"))
                ; // entry.format.setForeground(QBrush(QColor::(element.attribute("TEXTCOLOR"))));
            if (element.hasAttribute("TEXTBGCOLOR"))
                ; // entry.format.setBackground(QBrush(QColor(element.attribute("TEXTBGCOLOR"))));
            */
            m_autocorrectEntries.insert(find, replace);
        }
    }
}

