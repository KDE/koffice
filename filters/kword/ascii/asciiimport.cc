// $Id$

/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2000 Michael Johnson <mikej@xnet.com>
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

#include <config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <qstring.h>
#include <qregexp.h>
#include <qtextstream.h>

#include <kdebug.h>

#include <asciiimport.h>
#include <asciiimport.moc>

#include <algorithm>          // needed for max(a,b)

ASCIIImport::ASCIIImport(KoFilter *parent, const char *name) :
                     KoFilter(parent, name) {
}

bool ASCIIImport::filter(const QString &fileIn, const QString &fileOut,
                         const QString& from, const QString& to,
                         const QString &) {


  QString text1;  // text processing string
  QString text;   // text processing string
  QString Line[MAXLINES];  // lines of the paragraph
  int firstindent;  // amount that the first line of a paragraph is indented
  int secondindent;  // amount the second and remaining lines are indented
  int linecount = 0;  // line counter used to position tables
  int table_no = 0;  // used for table identifiers
  int i; // counter
  int begin;  // beginning line number of a paragraph
  int numLines; // Number of lines of the paragraph


    if(to!="application/x-kword" || from!="text/plain")
        return false;

    QFile in(fileIn);
    if(!in.open(IO_ReadOnly)) {
        kdError(30502) << "Unable to open input file!" << endl;
        in.close();
        return false;
    }

    QString str;
    QString tbl;  // string for table XML

    str += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    //str += "<!DOCTYPE DOC >\n";
    str += "<DOC editor=\"KWord plain text import filter\"";
    // TODO: We claim to be syntax version 2, but we should verify that it is also true.
    str += " mime=\"application/x-kword\" syntaxVersion=\"2\">\n";
    // TODO: other paper formats
    str += "<PAPER format=\"1\" width=\"595\" height=\"841\" orientation=\"0\" columns=\"1\" hType=\"0\" fType=\"0\" >\n";
    str += "<PAPERBORDERS left=\"28\" top=\"42\" right=\"28\" bottom=\"42\" />\n";
    str += "</PAPER>\n";
    str += "<ATTRIBUTES processing=\"0\" standardpage=\"1\" hasHeader=\"0\" hasFooter=\"0\" unit=\"mm\"/>\n";
    str += "<FRAMESETS>\n";
    str += "<FRAMESET frameType=\"1\" autoCreateNewFrame=\"1\" frameInfo=\"0\" removeable=\"0\">\n";
    str += "<FRAME left=\"28\" top=\"42\" right=\"566\" bottom=\"798\" />\n";

    QTextStream stream(&in);
    bool lastCharWasCr=false; // Was the previous character a Carriage Return?
    QString strLine;

    while(!stream.atEnd())
    {
        // Read in paragraph
        for(int line_no = numLines = 0; line_no < MAXLINES; line_no++, numLines++)
        {
            // We need to read a line
            // NOTE: we cannot use QStreamText::readLine,
            //   as it does not know anything about Carriage Returns
            strLine=readLine(stream,lastCharWasCr);
            if (strLine.isEmpty())
            {
                Line[line_no]=QString::null;
                break;
            }

            int length = strLine.length();
            if (strLine.at(length-1) == '-')
                strLine = strLine.left(length-1);  // remove the - at line end
            else
                strLine += ' '; // add space to end of line

            Line[line_no]=strLine;
        } // for(line_no = 0;

         //   process tables
        if ( Table( &Line[0], &linecount, numLines, table_no, tbl, str))
            table_no++;
        else
        {
        // Process bullet and dash lists
        if(ListItem( &Line[0], numLines, str))
            linecount += (numLines + 1); 
        else
           {
           // Paragraph with no tables or lists

           text1 = "";
           begin = 0;  // initial paragraph starts at first line
           for( i = 0; i < numLines; i++)
              {
              if( i > 0)
                 {
                 // check for a short line - if short make it a paragraph
                 if(Line[i].length() <= (uint)shortline)
                    {
                    if(!(i == (numLines - 1) && Line[i-1].length() > (uint)shortline))
                       // skip if short last line of normal paragraph
                       {
                       // write out paragraph begin to (i - 1)
                       firstindent = Indent(Line[begin]);
                       if((i - begin) > 1)
                          secondindent = Indent(Line[begin + 1]);
                       else secondindent = 0;
                       // process the white space to eliminate unwanted spaces
                       text = text1.simplifyWhiteSpace();
                       WriteOutParagraph( "Standard", "", text, firstindent, secondindent, str);
                       text1 = "";  // reinitialize paragraph text
                       begin = i;  // reset paragraph start
                       }  //  if(!(i == (line_no - 1)...
                    }  // if(Line[i].length()....

                 }  // if i > 0)

                 text1 += Line[i]; // add text to paragraph
              }  // for( i = 0; i <= line_no; i++)

              // write out paragraph begin to end
              firstindent = Indent(Line[begin]);
              if((i - begin) > 1)
                 secondindent = Indent(Line[begin + 1]);
              else secondindent = 0;
              // process the white space to eliminate unwanted spaces
              text = text1.simplifyWhiteSpace();
              WriteOutParagraph( "Standard", "", text, firstindent, secondindent, str);

           linecount += ( numLines + 1);  // increment the line count
           }  // else
        }  // else

        if( numLines > 0 )
           {
           // write out a blank line trailing the paragraph
           WriteOutParagraph( "Standard", "", "", 0, 0, str);
           } // if( line[line_no]....

     }  // while(!eof)

    // end document
    str += "</FRAMESET>\n";
    if( table_no > 0) str += tbl;
    str += "</FRAMESETS>\n";
    str += "<!-- We have a problem here somewhere -->\n";
    str+=" <STYLES>\n";
    str+="  <STYLE>\n";
    str+="   <NAME value=\"Standard\" />\n";
    str+="   <FLOW align=\"left\" />\n";
    str+="   <OFFSETS before=\"14.1733\" />\n";
    str+="   <FOLLOWING name=\"Standard\" />\n";
    str+="   <FORMAT id=\"1\" >\n";
    str+="    <WEIGHT value=\"50\" />\n";
    str+="    <COLOR blue=\"0\" red=\"0\" green=\"0\" />\n";
    // TODO: use KControl's default font
    str+="    <FONT name=\"Helvetica\" />\n";
    str+="    <SIZE value=\"12\" />\n";
    str+="    <ITALIC value=\"0\" />\n";
    str+="    <UNDERLINE value=\"0\" />\n";
    str+="    <STRIKEOUT value=\"0\" />\n";
    str+="    <VERTALIGN value=\"0\" />\n";
    str+="   </FORMAT>\n";
    str+="  </STYLE>\n";
    str+=" </STYLES>\n";
    str += "</DOC>\n";

#if 1
    kdDebug(30502) << str << endl;
#endif

    KoStore out=KoStore(QString(fileOut), KoStore::Write);
    if(!out.open("root")) {
        kdError(30502) << "Unable to open output file!" << endl;
        in.close();
        out.close();
        return false;
    }
    QCString cstring=str.utf8();
    // WARNING: we cannot use KoStore::write(const QByteArray&) because it gives an extra NULL character at the end.
    out.write(cstring,cstring.length());
    out.close();
    in.close();
    return true;
}




void ASCIIImport::WriteOutParagraph( QString name, QString type, QString text,
    int firstindent, int secondindent, QString &str)
{
    /* This method writes out a paragraph or a list item (Bullet, Dash)
        in Kword XML to the output string str. The paragraph is indented according
       to the first and second line indentation amounts.
       Arguments
         QString name - name of the paragraph type
         Qstring type - attribute in the COUNTER element to designate a list type
         QString text - text of the paragraph
         int firstindent - amount in spaces to indent the first line
         int secondindent - amount to indent the following lines
         QString str - the output string.

       Returns - void
        */

    QString bullet;

    str += "<PARAGRAPH>\n";
    str += "<TEXT>";
    str += escapeXmlText(text);  // Insert text into string
    str += "</TEXT>\n";
    str += "<FORMATS>\n";
    // we do not need any <FORMAT> child, we are using layout and style
    str += "</FORMATS>\n";
    str += "<LAYOUT>\n";

    str += "<NAME value=\"";
    str += name; // Name of paragraph type
    str += "\"/>\n";

    str += "<FOLLOWING name=\"";
    str += name;
    str += "\"/>\n";

    // if center justified write out a FLOW command
    //if( justified == center) str += "<FLOW value=\"2\"/>\n";

    // If the paragraph is indented, write out indentation elements.
    // TODO: why not always write identation?
    if (firstindent > 0 || secondindent > 0)         \
             WriteOutIndents( firstindent, secondindent, str);

    // If this is a bullet or a dash list, write out a COUNTER element
    if(type.isEmpty())
       {
       str += "<COUNTER type=\"";
       str += type;  // "6" = bullet "7" = dash
       str += "\" depth=\"0\" bullet=\"";

       if(type == "7") bullet = "183";  // symbol for a dash
       else bullet = "176";    // symbol for a bullet list
       str += bullet;

       // FIXME: not sure if times is the right font for symbols
       str+= "\" start=\"1\" numberingtype=\"0\" lefttext=\"\" righttext=\".\" bulletfont=\"times\" customdef=\"\" />\n";
       }

    str += "<FORMAT>\n";
    // for now we try with style (TODO: verify that KWord 1.2 works correctly)
    //str += "<FONT name=\"times\"/>\n";
    str += "</FORMAT>\n";
    str += "</LAYOUT>\n";
    str += "</PARAGRAPH>\n";

    return;

}  // WriteOutParagraph


void ASCIIImport::WriteOutIndents(const int firstindent,const int secondindent, QString &str)
{
    double size;

    str += "<INDENTS ";

    size = firstindent-secondindent;
    size *= ptsperchar;  // convert indent spaces to points

    str += "first=\"";
    str += QString::number(size);
    str += "\" ";


    size = secondindent;
    size *= ptsperchar;  // convert indent spaces to points

    str += "left=\"";
    str += QString::number(size);
    str += "\" ";

    str += "right=\"0\"/>\n";

}  // WriteOutIndents


   /* The Indent method determines the equivalent number of spaces
      at the beginning of a line   */

int ASCIIImport::Indent(const QString& line) const
{

    QChar c;  // for reading string a character at a time
    int count=0;  // amount of white space at the begining of the line

    for( uint i = 0; i < line.length(); i++ )
    {
         c = line.at(i);
         if( c == QChar(' '))
            count++;
         else if( c == QChar('\t'))
            count += spacespertab;
         else
            break;
    }

   return count;

}  // Indent

QString ASCIIImport::escapeXmlText(const QString& strIn) const
{
    QString strReturn;
    QChar ch;

    for (uint i=0; i<strIn.length(); i++)
    {
        ch=strIn[i];
        switch (ch.unicode())
        {
        case 38: // &
            {
                strReturn+="&amp;";
                break;
            }
        case 60: // <
            {
                strReturn+="&lt;";
                break;
            }
        case 62: // >
            {
                strReturn+="&gt;";
                break;
            }
        default:
            {
                strReturn+=ch;
                break;
            }
        }
    }

    return strReturn;
}

    /* The WriteOutTableCell method writes out a single table cell
        in Kword XML to the output string str. The table is sized according
       to the spacing in the ascii document.
       Arguments
         int table_no - used to keep the group number count
         int row - row number
         int col - column number
         Position *pos - pointer to the table position
         QString str - the output string.

       Returns - void
        */


   void ASCIIImport::WriteOutTableCell( int table_no, int row,
                         int col, Position *pos, QString &str)
   {

   str += "<FRAMESET frameType=\"1\" frameInfo=\"0\" grpMgr=\"grpmgr_";

   QString buf = QString::number(table_no); // convert int to string
   str += buf;

   str += "\" row=\"";
   buf = QString::number(row); // convert row to string
   str += buf;

   str += "\" col=\"";
   buf = QString::number(col); // convert col to string
   str += buf;

   str += "\" rows=\"1\" cols=\"1\" removable=\"0\" visible=\"1\" name=\"Cell ";
   buf = QString::number(row); // convert row to string
   str += buf;
   str += ",";
   buf = QString::number(col); // convert col to string
   str += buf;

   str += "\">\n";

   str += "<FRAME left=\"";
   buf = QString::number(pos->left); // convert left coordinate to string
   str += buf;

   str += "\" top=\"";
   buf = QString::number(pos->top); // convert top coordinate to string
   str += buf;

   str += "\" right=\"";
   buf = QString::number(pos->right); // convert right coordinate to string
   str += buf;

   str += "\" bottom=\"";
   buf = QString::number(pos->bottom); // convert bottom coordinate to string
   str += buf;

   str += "\" runaround=\"0\" autoCreateNewFrame=\"0\" newFrameBehavior=\"1\" />\n";

   return;

   }  // WriteOutTableCell




bool ASCIIImport::Table( QString *Line, int *linecount, int no_lines,
                         int table_no, QString &tbl, QString &str )
{
    return false; // this method is disabled

   /* This method examines a paragraph for embedded tables.
      If none are found, it returns. If tables are found then it
      writes out the paragraph including the tables. It does not
      check for embedded bullet or dash lists which is a seperate
      method.
      Arguments
         QString *Line - pointer to the array of text lines in the paragraph.
         int *linecount - pointer to the line counter used to position tables
         int no_lines - the number of lines in the paragraph
         int *table_no - pointer to the table number counter
         QString str  - the output string
      Returns - enum value indicating wheterer a table was processed. */

      enum LiType{paragraph, table} linetype[MAXLINES];
      struct Tabs tabs[MAXLINES];  // array of structures for tables
      struct Position pos;  // struct to pass cell position
      int width[MAXCOLUMNS]; // array of column widths
      QString *currentline;  // pointer to the current line
      QString *beginline;  // pointer to the beginning line
      QString text;  // text buffer
      QString text1;  // another text buffer
      QString No_indents;  // Line with leading white space removed
      int index, index1, index2, index3; // used to define column positions in a line

      int no_cols;  // column counter
      int firstindent;
      int secondindent;
      int i;  // counter
      int j;  // counter
      int k;  // counter
      QString *firstline;
      firstline = Line;
      int begin = 0;  // beginning line number
      int tabcount = 0;   // counts tab characters

      // check all lines for tabs
      for(i = 0; i < no_lines; i++)
         {
         no_cols = 0;  // inilialize the number of columns
         index = 0;  // initialize the line index

         // check the line for leading tabs and indentation
         if( (tabs[i].indent = Indent( *Line)) > 0)
             {
             // save and remove tabs at beginning of the line
             text = *Line;  // copy
             Line->replace(QRegExp("^[\\ \t]*"), "");
             }

         // find column positions and record text fields
         while((index2 = Line->find( QRegExp("\t"),index)) > index
         || (index3 = MultSpaces( *Line, index)) > index )
           {
           index1 = max(index2, index3);
           if( index2 > index3)
           index1 = Line->find( QRegExp("[^\t]"), index1);
           tabcount++;
           tabs[i].field[no_cols] = Line->mid(index, (index1 - index -1));
           tabs[i].width[no_cols] = index1 - index + spacespertab - 1;

           no_cols++;
           index = index1;
           }  // while( (index1 = ...

         // record last column
         if( no_cols > 0)
            {
            index1 = Line->length() - (index + 1);  // +1 for space at line end
            tabs[i].field[no_cols] = Line->mid(index,index1);
            tabs[i].width[no_cols] = index1;
            }

         if(tabs[i].indent > 0) *Line = text;  // restore original line
         Line++;  // increment the line pointer
         tabs[i].columns = no_cols;
         }  // for(i = 0; ...
         // All lines of the paragraph have been checked for tabs

     // check for a tab in at least one line
     if( tabcount == 0) return false;
     tabcount = 0;

     Line = firstline;  // reset the line pointer

     for(j = 0; j < MAXCOLUMNS; j++) width[j] = 0;
     for( i = 0; i <= no_lines; i++)
        {
        if(tabs[i].columns == 0 )  // indicates no tabs in line
           {
           text += *Line;  // add line of text to the paragraph
           linetype[i] = paragraph;
           }

        else  // this means a tabulated line
           {
           linetype[i] = table;

           // calculate the column widths
           for(j = 0; j <= tabs[i].columns; j++)
              width[j] = max(tabs[i].width[j], width[j] );
           }  // else

        if(i > 0)
           {
           if( linetype[i] != linetype[(i - 1)] || i == no_lines) // change of type
              {
              if( linetype[(begin)] == paragraph )  // write out a paragraph
                 {
                 text = "";  // initialize a null string for paragraph text
                 currentline = firstline;
                 currentline += begin;  //starting line
                 beginline = currentline;
                 firstindent = Indent(*currentline);
                 currentline += 1;  // second line
                 if( (i - begin) > 1)
                        secondindent = Indent( *currentline );
                 else secondindent = 0;

                 for( j= begin; j < i; j++)
                    {
                    text += *beginline;  // form text entry
                    beginline++;   // increment pointer
                    }
                 // process the white space to eliminate unwanted spaces
                 QString text1 = text.simplifyWhiteSpace();
                 WriteOutParagraph( "Standard", "", text1 , firstindent, secondindent, str);
                 *linecount += (i - begin);

                 }  // if( linetype[(i - 1)]

              else       // this implies a table
                 {
                 // Write out a blank line to align text and table
                 WriteOutParagraph( "Standard", "", "" , 0, 0, str);
                 for( j = begin; j < i; j++ )
                    {
                    int row = j - begin;
                    pos.top = (double)((*linecount) + 1) * ptsperline;
                    pos.bottom = pos.top + ptsperline;
                    // initialize position of first col
                    pos.left = leftmargin + (double)tabs[i].indent * ptsperchar;

                    // write out a cell for each column
                    for( k = 0; k <= tabs[j].columns; k++)
                       {

                       text = tabs[j].field[k];
                       // process the white space to eliminate unwanted spaces
                       text1 = text.simplifyWhiteSpace();

                       // calculate position of table cell
                       pos.right = pos.left + (double)width[k] * ptsperchar;

                       WriteOutTableCell( table_no, row, k, &pos, tbl);
                       WriteOutParagraph( "Standard", "", text1 , 0, 0, tbl);
                       tbl += "</FRAMESET>\n";
                       pos.left += ptsperchar * (double)width[k];
                       }  // for( k = 0; ...
                    (*linecount)  += 1;
                    }   // for( j = begin; ...


                 }  // else...
              begin = i;
              }  // if( linetype[i]...

           }  // if(i > 0)
        Line++;
        }  // for( i = 0; ...

   (*linecount) += 1;

   return true;
   } // end of Table()

   // the following method finds the location of multiple spaces in a string
int ASCIIImport::MultSpaces(const QString& text, const int index) const
{

    QChar c;
    QChar lastchar = 'c'; // previous character - initialize non blank
    bool found = false;
    // kdDebug(30502) << "length = "  << text.length() << endl;
    for (uint i = index; i < text.length(); i++)
    {
        c = text.at(i);
    // kdeDebug(30502) << "i = " << i << " found = " << found << " c = " << c << " lastchar = " << lastchar << endl;
        if ( (c != ' ') && found)
            return i;
        else if (c == ' ' && lastchar == ' ')
            found = true;
        lastchar = c;
    }
    return -1;
} // MultSpaces

   bool ASCIIImport::ListItem( QString *Line, int no_lines,
             QString &str )
      {

   /* This method examines a paragraph for embedded lists.
      If none are found, it returns. If lists are found then it
      writes out the paragraph including the lists.
      Arguments
         QString *Line - pointer to the array of text lines in the paragraph.
         int *linecount - pointer to the line counter used to position tables
         int no_lines - the number of lines in the paragraph
         QString &str  - the output string
      Returns - bool value indicating whether a list item was processed. */


      QString *currentline;  // pointer to the current line
      enum Listtype{none, bullet, dash} listtype[MAXLINES];
      QString type;  // type of list - bullet or dash
      QString *firstline;
      int i;  // counter
      int firstindent;
      int secondindent;
      int begin = 0;  // beginning line number
      int listcount = 0;   // counts lines with list items
      firstline = Line;

      // check all lines for list item indicators - or *
      for(i = 0; i < no_lines; i++)
         {
         listtype[i] = none;  // initialize

         // check for a bullet list item
         if( IsListItem( *Line, QChar('*')))
            {
            listtype[i] = bullet;
            listcount++;
            }
         // check for a dash list item
         if( IsListItem( *Line, QChar('-')))
            {
            listtype[i] = dash;
            listcount++;
            }

         Line++;  // increment the line pointer
         }  // for(i = 0; ...
         // All lines of the paragraph have been checked for list items

         // check for at least one list
         if( listcount == 0) return false;

         QString text = "";  // initialize a null string for paragraph text
         Line = firstline;  // reset the line pointer

         for( i = 0; i < no_lines; i++)
            {

            if(i > 0)
               {
               if( listtype[i] != none || Line->isEmpty()) // new list item
                  {
                  if( listtype[begin] == bullet)  // write out a bullet list
                     {
                     type = "6";
                     // delete * at beginning of line
                     text.replace( QRegExp("^[ \t]*\\* "), "");
                     }
                  else if( listtype[begin] == dash) // write out a dash list
                     {
                     type = "7";
                     // delete - at beginning of line
                     text.replace( QRegExp("^[ \t]*\\- "), "");
                     }
                  else if( listtype[begin] == none) // write out a paragraph
                     type = "";

                     currentline = firstline;
                     currentline += begin;  //starting line
                     firstindent = Indent(*currentline);
                     if( (i - begin) > 1)
                        {
                        currentline += 1;  // second line
                        secondindent = Indent( *currentline );
                        }
                     else secondindent = 0;

                     // process the white space to eliminate unwanted spaces
                     QString text1 = text.simplifyWhiteSpace();
                     WriteOutParagraph( "Standard", type, text1 , firstindent, secondindent, str);

                     begin = i;
                     text = "";  // reset text


                  }  // if( listtype[i] != none


               }  // if(i > 0)

            text += *Line;  // add text to paragraph
            Line++;

            }  // for( i = 0; ...

         // write out the last paragraph
         if( listtype[begin] == bullet)  // write out a bullet list
            {
            type = "6";
            // delete * at beginning of line
            text.replace( QRegExp("^[ \t]*\\* "), "");
            }
         else if( listtype[begin] == dash) // write out a dash list
            {
            type = "7";
            // delete - at beginning of line
            text.replace( QRegExp("^[ \t]*\\- "), "");
            }
         else if( listtype[begin] == none) // write out a paragraph
            type = "";

         currentline = firstline;
         currentline += begin;  //starting line
         firstindent = Indent(*currentline);
         if( (i - begin) > 1)
            {
            currentline += 1;  // second line
            secondindent = Indent( *currentline );
            }
         else secondindent = 0;

         // process the white space to eliminate unwanted spaces
         QString text1 = text.simplifyWhiteSpace();
         WriteOutParagraph( "Standard", type, text1 , firstindent, secondindent, str);



   return true;
   } // end of ListItem()


/* The IsListItem method checks a paragraph's first line and determines if
   the text appears to be a list item (bullet or dash).

   Arguments
     QString FirstLine  the first line of text in a paragraph
     QChar mark         the mark - or * that indicates a list item

   Return
     true if the line appears as a list item, false if not. */

bool ASCIIImport::IsListItem( QString FirstLine, QChar mark )
  {

     int i;  // counter
     QChar c; // to stream into one character at a time



     int k = FirstLine.find(mark);

     if( k < 0) return false;  // list item mark not on line

     // find first non white character on the line
     for( i=0; IsWhiteSpace( c = FirstLine.at((uint)i) ); i++);

     if(i != k ) return false;  // mark is in wrong position

     c = FirstLine.at((uint)++i);

     return IsWhiteSpace(c);  // character after the mark must be a white space

  }  // IsListItem


bool ASCIIImport::IsWhiteSpace(const QChar& c) const
{
     if ( c == QChar(' ') || c == QChar('\t') )
        return true;
     else
        return false;
} // IsWhiteSpace

QString ASCIIImport::readLine(QTextStream& textstream, bool& lastCharWasCr)
{
    // We need to read a line, character by character
    // NOTE: we cannot use QStreamText::readLine,
    //   as it does not know anything about Carriage Returns
    QString strLine;
    QChar ch;
    while (!textstream.atEnd())
    {
        textstream >> ch; // Read one character
        if (ch=="\n")
        {
            if (lastCharWasCr)
            {
                // We have a line feed following a Carriage Return
                // As the Carriage Return has already ended the previous line,
                // discard this Line Feed.
                lastCharWasCr=false;
            }
            else
            {
                // We have a normal Line Feed, therefore end the line
                break;
            }
        }
        else if (ch=="\r")
        {
            // We have a Carraiage Return, therefore end the line
            lastCharWasCr=true;
            break;
        }
        else
        {
            strLine+=ch;
            lastCharWasCr=false;
        }
    } // while
    return strLine;
}
