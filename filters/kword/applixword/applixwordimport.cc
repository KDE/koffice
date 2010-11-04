/* This file is part of the KDE project
   Copyright (C) 2000 Enno Bartels <ebartels@nwn.de>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QMessageBox>
#include <QList>
#include <QTextStream>
#include <QByteArray>
#include <applixwordimport.h>
#include <applixwordimport.moc>
#include <kdebug.h>
#include <KoFilterChain.h>
#include <kpluginfactory.h>

K_PLUGIN_FACTORY(APPLIXWORDImportFactory, registerPlugin<APPLIXWORDImport>();)
K_EXPORT_PLUGIN(APPLIXWORDImportFactory("kofficefilters"))


/******************************************************************************
 *  class: APPLIXWORDImport        function: APPLIXWORDImport                 *
 ******************************************************************************
 *                                                                            *
 *  Short description : Constructor                                           *
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
APPLIXWORDImport::APPLIXWORDImport(QObject *parent, const QVariantList&) :
        KoFilter(parent)
{
}

/******************************************************************************
 *  class: APPLIXWORDImport        function: nextLine                         *
 ******************************************************************************
 *                                                                            *
 *  Short description : Readline and update progressbar                       *
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
QString
APPLIXWORDImport::nextLine(QTextStream & stream)
{
    if (!m_nextPendingLine.isNull()) {
        const QString s = m_nextPendingLine;
        m_nextPendingLine.clear();
        return s;
    }

    // Read one Line
    QString s = stream.readLine();

    m_instep += s.length();
    if (m_instep > m_stepsize) {
        m_instep    = 0;
        m_progress += 2;
        emit sigProgress(m_progress) ;
    }

    return s;
}



/******************************************************************************
 *  class: APPLIXWORDImport        function: filter                           *
 ******************************************************************************
 *                                                                            *
 *  Short description :                                                       *
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
KoFilter::ConversionStatus APPLIXWORDImport::convert(const QByteArray& from, const QByteArray& to)
{

    if (to != "application/x-kword" || from != "application/x-applix-word")
        return KoFilter::NotImplemented;

    QFile in(m_chain->inputFile());
    if (!in.open(QIODevice::ReadOnly)) {
        kError(30517) << "Unable to open input file!" << endl;
        in.close();
        return KoFilter::FileNotFound;
    }


    QString str;

    str += "<?xml version=\"1.0\"?>\n";
    str += "<DOC  author=\"Reginald Stadlbauer and Torben Weis\" email=\"reggie@kde.org and weis@kde.org\" editor=\"KWord\" mime=\"application/x-kword\">\n";
    str += " <PAPER format=\"1\" width=\"595\" height=\"841\" orientation=\"0\" columns=\"1\" columnspacing=\"2\" hType=\"0\" fType=\"0\" spHeadBody=\"9\" spFootBody=\"9\" >\n";
    str += "  <PAPERBORDERS left=\"28\" top=\"42\" right=\"28\" bottom=\"42\" />\n";
    str += " </PAPER>\n";
    str += " <ATTRIBUTES processing=\"0\" standardpage=\"1\" hasHeader=\"0\" hasFooter=\"0\" />\n";
    str += " <FRAMESETS>\n";
    str += "  <FRAMESET frameType=\"1\" autoCreateNewFrame=\"1\" frameInfo=\"0\" removeable=\"0\">\n";
    str += "   <FRAME left=\"28\" top=\"42\" right=\"566\" bottom=\"798\" runaround=\"1\" runaroundGap=\"2\" lWidth=\"1\" lRed=\"255\" lGreen=\"255\" lBlue=\"255\" lStyle=\"0\"  rWidth=\"1\" rRed=\"255\" rGreen=\"255\" rBlue=\"255\" rStyle=\"0\"  tWidth=\"1\" tRed=\"255\" tGreen=\"255\" tBlue=\"255\" tStyle=\"0\"  bWidth=\"1\" bRed=\"255\" bGreen=\"255\" bBlue=\"255\" bStyle=\"0\" bkRed=\"255\" bkGreen=\"255\" bkBlue=\"255\" bleftpt=\"0\" brightpt=\"0\" btoppt=\"0\" bbottompt=\"0\" />\n";


    QTextStream stream(&in);

    m_stepsize = in.size() / 50;
    m_instep   = 0;
    m_progress = 0;

    int  rueck;
    int  pos;
    bool ok;
    char stylename[100];
    QString           mystr, textstr;
    QList<t_mycolor*>  mcol;
    QStringList       mcoltxt;

    /**************************************************************************
     * Read header                                                            *
     **************************************************************************/
    if (! readHeader(stream)) return KoFilter::StupidError;


    while (!stream.atEnd()) {
        // Read one line
        mystr = readTagLine(stream);
        ok = true;

        //kDebug() << "mystr=" << mystr;

        /**********************************************************************
         * jump over start_styles if it exists                                *
         **********************************************************************/
        if (mystr == "<start_styles>") {
            printf("Start styles\n");
            t_mycolor *col = new t_mycolor; // delete is in place
            QString    coltxt ;
            int zaehler = 0; // Note: "zaehler" means "counter" in English
            do {
                mystr = readTagLine(stream);
                if (mystr == "<end_styles>") {
                    ok = false;
                    kDebug(30517) << "End styles";
                } else {
                    if (mystr.startsWith("<color ")) {
                        mystr.remove(0, 8);
                        pos = mystr.indexOf("\"");
                        coltxt = mystr.left(pos);
                        mystr.remove(0, pos + 1);
                        rueck = sscanf((const char *) mystr.toLatin1() ,
                                       ":%d:%d:%d:%d>",
                                       &col->c, &col->m, &col->y, &col->k);
                        kDebug(30517) << "  Color" <<  zaehler << "  :" << col->c << "" << col->m << "" << col->y << "" << col->k << "" << coltxt << "";
                        zaehler ++;

                        // Color transformation cmyk -> rgb
                        col->r = 255 - (col->c + col->k);
                        if (col->r < 0) col->r = 0;

                        col->g = 255 - (col->m + col->k);
                        if (col->g < 0) col->g = 0;

                        col->b = 255 - (col->y + col->k);
                        if (col->b < 0) col->b = 0;

                        mcol.append(col);
                        mcoltxt.append(coltxt);
                    } //end if ...<col...
                } //end else
            } // end while
            while (ok == true);
            delete col;
        } // end if ...<start_styles>...
        /***********************************************************************
         * jump over embedded Applix docs                                      *
         ***********************************************************************/
        else if (mystr == "<start_data Applix>") {
            kDebug(30517) << "\nEmbedded Applix object starts:";
            do {
                mystr = readTagLine(stream);
                if (mystr == "<end_data>") ok = false;
                else {
                    kDebug(30517) << "" << mystr;
                }
            } while (ok == true);
            kDebug(30517) << "Embedded Applix object ends";

        }
        /**********************************************************************
         * jump over header footer                                            *
         **********************************************************************/
        else if (mystr.startsWith("<start_hdrftr ")) {
            kDebug(30517) << "\nHeader/Footer starts:";
            do {
                mystr = readTagLine(stream);
                if (mystr == "<end_hdrftr>") ok = false;
                else {
                    kDebug(30517) << "" << mystr;
                }
            } while (ok == true);
            kDebug(30517) << "\nHeader/Footer ends";
        }
        /**********************************************************************
         * found a paragraph string                                           *
         **********************************************************************/
        else if (mystr.startsWith("<P ")) {
            sscanf((const char *) mystr.toLatin1(), "<P \"%99s\"", stylename);
            mystr.remove(0, 5 + strlen(stylename));
            kDebug(30517) << " Para  Name:" << stylename;
            kDebug(30517) << "       Rest:" << mystr;
        }
        /**********************************************************************
         * found a textstring                                                 *
         **********************************************************************/
        else if (mystr.startsWith("<T ")) {
            QString colname;

            // Remove starting tab info
            mystr.remove(0, 4);

            // Remove ending >
            mystr.remove(mystr.length() - 1, 1);

            // Separate textstring "
            ok = true;
            int y = 0;
            do {
                pos = mystr.indexOf("\"", y);
                kDebug(30517) << "POS:" << pos << " length:" << mystr.length() << " y:" << y;

                kDebug(30517) << "<" << mystr << " >";
                if ((pos - 1 > -1) && (mystr[pos-1] == '\\')) {
                    kDebug(30517) << " No string end - but G�nsef�sschen";
                    y = pos + 1;
                } else {
                    kDebug(30517) << " String end //";
                    ok = false;
                }
            } while (ok == true);

            textstr = mystr.left(pos);
            mystr.remove(0, pos + 1);
            mystr = mystr.trimmed();
            kDebug(30517) << "Text:<" << textstr << " >" << pos << "  Rest:<" << mystr << ">";

            // split format
            const QStringList typeList = mystr.split(' ', QString::SkipEmptyParts);

            int fontsize = 12, bold = 0, italic = 0, underline = 0, colpos = -1;
            QString fontname;
            int nn = 0;
            Q_FOREACH(const QString& type, typeList) {
                kDebug(30517) << "   No:" << nn << "   >" << type << "< =";

                // Looking for bold
                if (type == "bold") {
                    bold = 1;
                    kDebug(30517) << "bold";
                } else if (type == "no-bold") {
                    bold = 0;
                    kDebug(30517) << "no bold";
                } else if (type == "italic") {
                    italic = 1;
                    kDebug(30517) << "italic";
                } else if (type == "no-italic") {
                    italic = 0;
                    kDebug(30517) << "no italic";
                } else if (type == "underline") {
                    underline = 1;
                    kDebug(30517) << "underline";
                } else if (type == "no-underline") {
                    underline = 0;
                    kDebug(30517) << "no underline";
                } else if (type.startsWith("size")) { // e.g. size:14
                    //type.remove(0, 5);
                    fontsize = type.mid(5).toInt();
                    kDebug(30517) << "fontsize:" << fontsize;
                } else if (type.startsWith("face")) { // e.g. face:"Symbol"
                    //type.remove(0, 6);
                    //type.remove(type.length() - 1, 1);
                    fontname = type.mid(6, type.length() - 6 - 1);
                    kDebug(30517) << "fontname:" << fontname;
                } else if (type.startsWith("color:")) { // e.g. color:"Black"
                    //type.remove(0, 7);
                    //type.remove(type.length() - 1, 1);
                    colname = type.mid(7, type.length() - 7 - 1);
                    colpos = mcoltxt.indexOf(colname);
                    kDebug(30517) << "  Color:" << colname << "" << colpos << "";
                } else {
                    kDebug(30517) << "" << type;
                }


            }
            kDebug(30517) << "";

            // Replaces Part for & <>, applixwear special characters and qouts
            replaceSpecial(textstr);


            // add text inside
            str += "    <PARAGRAPH>\n";
            str += "     <TEXT>";
            str += textstr;
            str += "</TEXT>\n";

            if (bold == 1 || underline == 1 || italic == 1 || fontsize != 12 ||
                    colpos != -1 || !fontname.isEmpty()) {
                str += "     <LAYOUT>\n";
                str += "      <FORMAT>\n";
                if (!fontname.isEmpty()) {
                    str += "       <FONT name=\"";
                    str += fontname;
                    str += "\" />\n";
                }

                if (fontsize != 1) {
                    str += "       <SIZE value=\"";
                    str += QString::number(fontsize);
                    str += "\" />\n";
                }

                if (italic == 1) {
                    str += "       <ITALIC value=\"1\" />\n";
                }

                if (bold == 1) {
                    str += "       <WEIGHT value=\"75\" />\n";
                }

                if (underline == 1) {
                    str += "       <UNDERLINE value=\"1\" />\n";
                }

                if (colpos != -1) {
                    t_mycolor *mc = mcol.at(colpos);
                    str += "       <COLOR red=\"";

                    str += QString::number(mc->r);
                    str += "\" green=\"";
                    str += QString::number(mc->g);
                    str += "\" blue=\"";
                    str += QString::number(mc->b);
                    str += "\" />\n";
                }

                str += "      </FORMAT>\n";
                str += "     </LAYOUT>\n";
            }
            str += "    </PARAGRAPH>\n";
        }

    }
    emit sigProgress(100);

    str += "  </FRAMESET>\n";
    str += " </FRAMESETS>\n";
    str += "</DOC>\n";
    kDebug(30517) << "Text" << str;

    KoStoreDevice* out = m_chain->storageFile("root", KoStore::Write);
    if (!out) {
        kError(30517) << "Unable to open output file!" << endl;
        in.close();
        return KoFilter::StorageCreationError;
    }

    QByteArray cstring = str.toUtf8();

    out->write((const char*) cstring, cstring.length());

    in.close();
    return KoFilter::OK;
}


/******************************************************************************
 *  function: specCharfind                                                    *
 ******************************************************************************/
QChar
APPLIXWORDImport::specCharfind(QChar a, QChar b)
{
    QChar chr;

    if ((a == 'n') && (b == 'p')) chr = '�';


    else if ((a == 'n') && (b == 'c')) chr = '�';
    else if ((a == 'p') && (b == 'c')) chr = '�';

    else if ((a == 'n') && (b == 'd')) chr = '�';
    else if ((a == 'p') && (b == 'd')) chr = '�';

    else if ((a == 'n') && (b == 'e')) chr = '�';
    else if ((a == 'p') && (b == 'e')) chr = '�';

    else if ((a == 'n') && (b == 'f')) chr = '�';
    else if ((a == 'p') && (b == 'f')) chr = '�';

    else if ((a == 'p') && (b == 'g')) chr = '�';
    else if ((a == 'n') && (b == 'g')) chr = '�';



    else if ((a == 'n') && (b == 'j')) chr = '�';
    else if ((a == 'p') && (b == 'j')) chr = '�';

    else if ((a == 'n') && (b == 'k')) chr = '�';
    else if ((a == 'p') && (b == 'k')) chr = '�';

    else if ((a == 'n') && (b == 'l')) chr = '�';
    else if ((a == 'p') && (b == 'l')) chr = '�';

    else if ((a == 'p') && (b == 'm')) chr = '�';
    else if ((a == 'n') && (b == 'm')) chr = '�';



    else if ((a == 'm') && (b == 'a')) chr = '�';
    else if ((a == 'o') && (b == 'a')) chr = '�';

    else if ((a == 'm') && (b == 'b')) chr = '�';
    else if ((a == 'o') && (b == 'b')) chr = '�';

    else if ((a == 'm') && (b == 'c')) chr = '�';
    else if ((a == 'o') && (b == 'c')) chr = '�';

    else if ((a == 'm') && (b == 'd')) chr = '�';
    else if ((a == 'o') && (b == 'd')) chr = '�';

    else if ((a == 'm') && (b == 'e')) chr = '�';
    else if ((a == 'o') && (b == 'e')) chr = '�';

    else if ((a == 'm') && (b == 'f')) chr = '�';
    else if ((a == 'o') && (b == 'f')) chr = '�';

    else if ((a == 'm') && (b == 'g')) chr = '�';
    else if ((a == 'o') && (b == 'g')) chr = '�';



    else if ((a == 'm') && (b == 'i')) chr = '�';
    else if ((a == 'o') && (b == 'i')) chr = '�';

    else if ((a == 'm') && (b == 'j')) chr = '�';
    else if ((a == 'o') && (b == 'j')) chr = '�';

    else if ((a == 'm') && (b == 'k')) chr = '�';
    else if ((a == 'o') && (b == 'k')) chr = '�';

    else if ((a == 'm') && (b == 'l')) chr = '�';
    else if ((a == 'o') && (b == 'l')) chr = '�';






    else if ((a == 'm') && (b == 'm')) chr = '�';
    else if ((a == 'o') && (b == 'm')) chr = '�';

    else if ((a == 'm') && (b == 'n')) chr = '�';
    else if ((a == 'o') && (b == 'n')) chr = '�';

    else if ((a == 'm') && (b == 'o')) chr = '�';
    else if ((a == 'o') && (b == 'o')) chr = '�';

    else if ((a == 'm') && (b == 'p')) chr = '�';
    else if ((a == 'o') && (b == 'p')) chr = '�';


    else if ((a == 'n') && (b == 'b')) chr = '�';
    else if ((a == 'p') && (b == 'b')) chr = '�';


    else if ((a == 'k') && (b == 'c')) chr = '�';
    else if ((a == 'k') && (b == 'j')) chr = '�';
    else if ((a == 'l') && (b == 'f')) chr = '�';
    else if ((a == 'n') && (b == 'i')) chr = '�';
    else if ((a == 'p') && (b == 'i')) chr = '�';

    else if ((a == 'l') && (b == 'j')) chr = '�';
    else if ((a == 'l') && (b == 'c')) chr = '�';
    else if ((a == 'l') && (b == 'd')) chr = '�';

    else if ((a == 'l') && (b == 'm')) chr = '�';
    else if ((a == 'l') && (b == 'n')) chr = '�';
    else if ((a == 'l') && (b == 'o')) chr = '�';

    else if ((a == 'l') && (b == 'a')) chr = '�';

    else if ((a == 'k') && (b == 'o')) chr = '�';
    else if ((a == 'k') && (b == 'h')) chr = '�';
    else if ((a == 'k') && (b == 'd')) chr = '�';

    else if ((a == 'p') && (b == 'a')) chr = '�';
    else if ((a == 'n') && (b == 'a')) chr = '�';

    else if ((a == 'l') && (b == 'l')) chr = '�';
    else if ((a == 'k') && (b == 'l')) chr = '�';

    else if ((a == 'l') && (b == 'k')) chr = '�';

    else if ((a == 'l') && (b == 'h')) chr = '�';

    else if ((a == 'k') && (b == 'b')) chr = '�';

    else if ((a == 'k') && (b == 'e')) chr = '�';

    else if ((a == 'l') && (b == 'b')) chr = '�';

    else if ((a == 'l') && (b == 'p')) chr = '�';

    else if ((a == 'k') && (b == 'f')) chr = '�';

    else if ((a == 'p') && (b == 'o')) chr = '�';
    else if ((a == 'n') && (b == 'o')) chr = '�';

    else if ((a == 'n') && (b == 'n')) chr = '�';
    else if ((a == 'p') && (b == 'n')) chr = '�';
    else if ((a == 'p') && (b == 'p')) chr = '�';

    else if ((a == 'k') && (b == 'k')) chr = '�';

    else if ((a == 'k') && (b == 'm')) chr = '�';
    else if ((a == 'p') && (b == 'h')) chr = '�';

    else if ((a == 'k') && (b == 'g')) chr = '|';

    else if ((a == 'l') && (b == 'e')) chr = '\'';

    else if ((a == 'k') && (b == 'i')) chr = '�';

    else if ((a == 'k') && (b == 'n')) chr = '�';

    else if ((a == 'k') && (b == 'p')) chr = '�';

    else if ((a == 'l') && (b == 'g')) chr = '�';

    else if ((a == 'l') && (b == 'i')) chr = '�';

    else if ((a == 'm') && (b == 'h')) chr = '�';
    else if ((a == 'o') && (b == 'h')) chr = '�';

    else if ((a == 'n') && (b == 'h')) chr = '�';

    else if ((a == 'k') && (b == 'a')) chr = ' ';

    else if ((a == 'a') && (b == 'j')) chr = '!';

    else  chr = '#';

    return chr;
}



/******************************************************************************
 *  class: APPLIXWORDImport        function: readTagLine                      *
 ******************************************************************************
 *                                                                            *
 *  Short description :                                                       *
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
QString
APPLIXWORDImport::readTagLine(QTextStream &stream)
{
    // Read one line
    QString mystr = nextLine(stream);

    // Delete whitespaces
    mystr = mystr.trimmed();

    // Look if the tag continues on the next line
    if ((mystr.length() == 80) && (mystr[mystr.length()-1] == '\\')) {
        bool ok = true;
        do {
            // Read next line
            QString mystrn = nextLine(stream);

            // Is the new line a new tag line
            if (mystrn[0] == ' ') {
                // remove the whitespace at the start of the new line
                mystrn.remove(0, 1);

                // remove the '\' at the end of the old line
                mystr.remove(mystr.length() - 1, 1);

                // append the new line
                mystr += mystrn;
            } else {
                // keep this line for later
                m_nextPendingLine = mystrn;
                ok = false;
            }
        } while (ok == true);
    }

    return mystr;
}




/******************************************************************************
 *  class: APPLIXWORDImport        function: replaceSpecial                   *
 ******************************************************************************
 *                                                                            *
 *  Short description :                                                       *
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
void
APPLIXWORDImport::replaceSpecial(QString &textstr)
{
    int ok, pos;

    // 1. Replace Part for this characters: <, >, &
    textstr.replace('&', "&amp;");
    textstr.replace('<', "&lt;");
    textstr.replace('>', "&gt;");


    // 2. Replace part for this characters: applixwear qoutes
    ok = true;
    pos = 0;
    do {
        // Searching for an quote
        pos = textstr.indexOf('\"', pos);

        // Is it a textquote ?
        if ((pos > -1) && (textstr[pos-1] == '\\')) {
            textstr.replace(pos - 1, 2, "\"");
        } else {
            ok = false;
        }
    } while (ok == true);



    // 3. Replace part for Applix Characters
    int   foundSpecialCharakter;
    QChar newchar;

    do {
        // initialize
        foundSpecialCharakter = false;

        pos = textstr.indexOf('^');

        // is there a special character ?
        if (pos > -1) {
            // i have found a special character !
            foundSpecialCharakter = true;

            // translate the applix special character
            newchar = specCharfind(textstr[pos+1], textstr[pos+2]);

            // replace the character
            textstr.replace(pos, 3, newchar);
        }
    } while (foundSpecialCharakter == true);
}



/******************************************************************************
 *  class: APPLIXWORDImport       function: readHeader                        *
 ******************************************************************************
 *                                                                            *
 *  Short description :                                                       *
 *                                                                            *
 *                                                                            *
 ******************************************************************************/
int
APPLIXWORDImport::readHeader(QTextStream &stream)
{
    int     rueck;
    int     vers[3] = { 0, 0, 0 };

    // Read Headline
    QString mystr = readTagLine(stream);

    rueck = sscanf((const char *) mystr.toLatin1() ,
                   "*BEGIN WORDS VERSION=%d/%d ENCODING=%dBIT",
                   &vers[0], &vers[1], &vers[2]);
    printf("Versions info: %d %d %d\n", vers[0], vers[1], vers[2]);

    // Check the headline
    if (rueck <= 0) {
        printf("Header not correkt - May be it is not an applixword file\n");
        printf("Headerline: <%s>\n", (const char *) mystr.toLatin1());

        QMessageBox::critical(0L, "Applixword header problem",
                              QString("The Applixword header is not correct. "
                                      "May be it is not an applixword file! <BR>"
                                      "This is the header line I did read:<BR><B>%1</B>").arg(mystr),
                              "Okay");

        // i18n( "What is the separator used in this file ? First line is \n%1" ).arg(firstLine),
        return false;
    } else return true;
}
