/* This file is part of the KDE project
   Copyright (C) 2001 Enno Bartels <ebartels@nwn.de>

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
#include <QStringList>
#include <QRegExp>
#include <q3ptrlist.h>
#include <QTextStream>
#include <QByteArray>
#include <applixspreadimport.h>
#include <kdebug.h>
#include <math.h>
#include <KoFilterChain.h>
#include <kgenericfactory.h>

typedef KGenericFactory<APPLIXSPREADImport> APPLIXSPREADImportFactory;
K_EXPORT_COMPONENT_FACTORY( libapplixspreadimport, APPLIXSPREADImportFactory( "kofficefilters" ) )


APPLIXSPREADImport::APPLIXSPREADImport ( QObject *parent, const QStringList& )
    : KoFilter( parent )
{
}

QString APPLIXSPREADImport::nextLine( QTextStream & stream )
{
    QString s = stream.readLine();
    m_instep += s.length();
    if (m_instep > m_stepsize)
    {
        m_instep = 0;
        m_progress += 2;
        emit sigProgress( m_progress );
    }
    return s;
}

KoFilter::ConversionStatus APPLIXSPREADImport::convert( const QByteArray& from, const QByteArray& to )
{

    if (to != "application/x-kspread" || from != "application/x-applix-spreadsheet")
        return KoFilter::NotImplemented;

    QFile in (m_chain->inputFile());
    if (!in.open(QIODevice::ReadOnly))
    {
        kError(30502) << "Unable to open input file!" << endl;
        in.close();
        return KoFilter::FileNotFound;
    }

    QString str;
    Q3PtrList<t_mycolor> mcol;

    str += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    str += "<!DOCTYPE spreadsheet>\n";
    str += "<spreadsheet mime=\"application/x-kspread\" editor=\"KSpread\" >\n";
    str += " <paper format=\"A4\" orientation=\"Portrait\" >\n";
    str += "  <borders right=\"20\" left=\"20\" bottom=\"20\" top=\"20\" />\n";
    str += "  <head/>\n";
    str += "  <foot/>\n";
    str += " </paper>\n";
    str += " <locale positivePrefixCurrencySymbol=\"True\" negativeMonetarySignPosition=\"1\" negativePrefixCurrencySymbol=\"True\" fracDigits=\"2\" thousandsSeparator=\" \" dateFormat=\"%A, %e. %B %Y\" timeFormat=\"%H:%M:%S\" monetaryDecimalSymbol=\",\" weekStartsMonday=\"True\" currencySymbol=\"DM\" negativeSign=\"-\" positiveSign=\"\" positiveMonetarySignPosition=\"1\" decimalSymbol=\",\" monetaryThousandsSeparator=\" \" dateFormatShort=\"%d.%m.%Y\" />\n";
    str += " <map markerColumn=\"1\" activeTable=\"Table1\" markerRow=\"1\" >\n";
//      str += "  <table columnnumber=\"0\" borders=\"0\" hide=\"0\" hidezero=\"0\" firstletterupper=\"0\" grid=\"1\" formular=\"0\" lcmode=\"0\" name=\"Tabelle1\" >\n";


    // QTextStream
    QTextStream stream (&in);
    m_stepsize = in.size()/50;
    m_instep   = 0;
    m_progress = 0;
    int  pos;
    char ccol;
    int  irow, icol;
    QString  tabctr ;  // Tab control
    QString  mystr, typestr, cellnostr, tabnostr;
    QStringList typefacetab;

    //    QStringList rclist;
    t_rc my_rc;



    /**************************************************************************
     * Read header                                                            *
     **************************************************************************/
    if (! readHeader (stream)) return KoFilter::StupidError;


    while (!stream.atEnd ())
    {
        // Read one line
        mystr = nextLine( stream );

        kDebug()<<"INPUT :"<<mystr;


        /**********************************************************************
         *  Looking for the colormap                                          *
         **********************************************************************/
        if (mystr.startsWith ("COLORMAP") )
	{
          readColormap (stream, mcol);
	}

        /**********************************************************************
         *  Looking for the typeface table                                    *
         **********************************************************************/
        else if (mystr.startsWith ("TYPEFACE TABLE") )
	{
          readTypefaceTable (stream, typefacetab);
	}

        /**********************************************************************
         *  Looking for some View-Information                                 *
         **********************************************************************/
        else if (mystr.startsWith ("View Start, Name:") )
	{
          readView (stream, mystr, my_rc);
	}


        /**********************************************************************
         *   Detect ( at the first place of the Linie                         *
         **********************************************************************/
        else if (mystr[0] == '(')
	{
	  int fg=-1; // fg = foregound

          // Delete  '('
          mystr.remove (0, 1);

          int alllenght;
          QString mystrn;

          // Remember length of the string
          alllenght = mystr.length ();
          if (alllenght >= 80-1)
          {
              kDebug()<<" Line >= 80 chars";
            int ok = true;
            do
	    {
              pos = in.pos ();
              mystrn = nextLine( stream );
              if (mystrn[0] == ' ')
	      {
                mystrn.remove (0, 1);
                mystr += mystrn;
	      }
              else
	      {
                in.seek (pos);
                ok = false;
	      }
	    }
            while (ok == true);

          }


          // Search for ')'
          pos = mystr.indexOf (')');
          typestr = mystr.left (pos);


          // Delete typeformat infos incl. Space
          mystr.remove (0, pos+1);
          // alllenght = alllenght - pos - 1;

          // Search for ':'
          pos = mystr.indexOf (':');

          // Copy cellnumber information
          cellnostr = mystr.left (pos);

          // Delete cellnumber information
          mystr.remove (0, pos+1);
          // alllenght = alllenght - pos - 1;


          // Split Table and Cell Number
          pos = cellnostr.indexOf ('!');

          // Copy tabnumber information
          tabnostr = cellnostr.left (pos);

          // Delete cellnumber information
          cellnostr.remove (0, pos+1);

          int  len = cellnostr.length ();
          char tmp[300], tmp1[300];
          int  leni;



          pos = cellnostr.indexOf (QRegExp ("[0-9]"));
          kDebug()<<" findpos :"<<pos;



          QString rowstr;
          bool ok;
          int bla;
          rowstr = cellnostr.mid (pos, cellnostr.length()-pos);
          irow   = rowstr.toInt(&ok);

          kDebug()<<" findpos :"<< rowstr<<""<<irow;
          sscanf (cellnostr.toLatin1(), "%299s%d",tmp, &bla);
          sprintf (tmp1, "%d", irow);
          leni = strlen (tmp1);
	  QString cellcolstr;
          cellcolstr = cellnostr;
          cellcolstr.remove (cellcolstr.length()-leni, leni);

          kDebug()<<" Info: length :"<<len<<" cellnostr :"<<cellnostr<<" tmp :"<<tmp<<" irow :"<<irow<<" cellcolstr :"<<cellcolstr;

          // Transformat ascii column to int column
          icol = translateColumnNumber (cellcolstr);


	  //  sscanf (cellnostr.toLatin1(), "%c%d",&ccol, &irow);

          // Transformat ascii column to int column
	  //  icol = ccol - 64;

          // Remove first whitespace
          mystr.remove    (0, 1);
          tabnostr.remove (0, 1);


          // Replace part for this characters: <, >, &
          mystr.replace (QRegExp ("&"), "&amp;");
          mystr.replace (QRegExp ("<"), "&lt;");
          mystr.replace (QRegExp (">"), "&gt;");


          // Replace part for Applix Characters
          int   foundSpecialCharakter;
          QChar newchar;

          do
	  {
            // initialize
            foundSpecialCharakter = false;

            pos = mystr.indexOf ("^");

            // is there a special character ?
            if (pos > -1 )
            {
              // i have found a special character !
              foundSpecialCharakter = true;

              // translate the applix special character
              newchar = specCharfind (mystr[pos+1], mystr[pos+2]);

              // replace the character
              mystr.replace (pos, 3, newchar);
	    }

	  }
          while (foundSpecialCharakter == true);


	  // examine the typestring
          // splitt typestring in 3 parts by an |
          QString typeFormStr;
          QString typeCharStr;
          QString typeCellStr;

          int pos1 = typestr.indexOf    ("|");
          int pos2 = typestr.lastIndexOf ("|");


          typeFormStr = typestr.left  (pos1);

          typeCharStr = typestr.mid   (pos1+1,  pos2 - pos1 - 1);

          typeCellStr = typestr.right (typestr.length() - pos2 - 1);

          // Is it a new table
          if (tabctr != tabnostr)
          {
	     // is it not the first table
             if (!(tabctr.isNull()))  str += "  </table>\n";

             str += "  <table columnnumber=\"0\" borders=\"0\" hide=\"0\" hidezero=\"0\" firstletterupper=\"0\" grid=\"1\" formular=\"0\" lcmode=\"0\" name=\"";
             str += tabnostr;
	     str += "\" >\n";

             tabctr = tabnostr;

             // Searching for the rowcol part and adding to the hole string
             pos = my_rc.tabname.indexOf(tabnostr);
             if (pos > -1) str += my_rc.rc[pos];
          }

          //kDebug()<<" Data : Text :"<<mystr<<" tab :"<<tabnostr<<""<< cellnostr <<"" <<ccol<<"" << irow<<""<< typeFormStr<<"" <<typeCharStr<<"" <<typeCellStr;


          /********************************************************************
           * examine charakter format String, splitt it up in basic parts     *
           ********************************************************************/
          QStringList typeCharList;
          int bold=0, italic=0, underline=0, nn=0, fontsize=12, fontnr=-1;

          typeCharList = typeCharStr.split (',');

          for (QStringList::Iterator it = typeCharList.begin();
               it != typeCharList.end(); ++it )
	  {
	    // Output
              kDebug()<<" Char ("<<nn<<" )  >"<<*it<<"<";
            nn++;

            if      ((*it) == "B")
            {
                kDebug()<<" bold";
              bold  = 1;
	    }
            else if ((*it) == "I")
            {

                kDebug()<<"   = italic";
              italic = 1;
	    }
            else if ((*it) == "U")
            {
                kDebug()<<"   = underline";
              underline = 1;
	    }
            else if ((*it).startsWith("FG") )
	    {
              sscanf ((*it).toLatin1(), "FG%d", &fg);
              kDebug()<<"  = Colornr"<< fg;
	    }
            else if ((*it).startsWith("TF") )
	    {
              sscanf ((*it).toLatin1(), "TF%d", &fontnr);
              kDebug()<<" = Font :"<<fontnr<<""<<typefacetab[fontnr];
	    }
            else if ((*it).startsWith('P') )
	    {
              sscanf ((*it).toLatin1(), "P%d", &fontsize);
              kDebug()<<"   = Fontsize"<<fontsize;
	    }
            else
	    {

                kDebug()<<"   = ???";
	    }
	  }
          kDebug()<<"";



          /********************************************************************
           * examine pos format String, splitt it up in basic parts           *
           ********************************************************************/
          QStringList typeFormList;
          int align = 0, valign=0;

          typeFormList = typeFormStr.split (',');
          nn=0;
          for (QStringList::Iterator it = typeFormList.begin(); it != typeFormList.end(); ++it )
          {
	    // Output
              //kDebug()<<"   Type (%2d)   >%s<",
              //    nn, (*it).toLatin1() );
            nn++;
            // Grep horizontal alignment
            if      ( (*it) == "1")
            {
                kDebug()<<" = left align";
              align = 1; // left
	    }
            else if ( (*it) == "2")
            {
                kDebug()<<" = right align";
              align = 3; // right
	    }
            else if ( (*it) == "3")
            {
                kDebug()<<" = center align";
              align = 2; // center
	    }

            // Grep verticale alignment
            else if ( (*it) == "VT")
            {
                kDebug()<<" = top valign";
              valign =  1; // top
	    }
            else if ( (*it) == "VC")
            {
                kDebug()<<" = center valign";
              valign =  0; // center - default (2)
	    }
            else if ( (*it) == "VB")
            {
                kDebug()<<" = bottom valign";
              valign =  3; // bottom
	    }
            else
	    {
                kDebug()<<"   = ???";
	    }
	  }


          /********************************************************************
           * examine cell format String, splitt it up in basic parts          *
           ********************************************************************/
          QStringList typeCellList;
          int topPenWidth=0, bottomPenWidth=0, leftPenWidth = 0, rightPenWidth = 0, fg_bg=-1;
          int topPenStyle=0, bottomPenStyle=0, leftPenStyle = 0, rightPenStyle = 0;
          int brushstyle=0,     brushcolor=1;
          int topbrushstyle=0,  topbrushcolor=1, topfg_bg=1;
          int leftbrushstyle=0, leftbrushcolor=1, leftfg_bg=1;
          int rightbrushstyle=0, rightbrushcolor=1, rightfg_bg=1;
          int bottombrushstyle=0, bottombrushcolor=1, bottomfg_bg=1;

          typeCellList = typeCellStr.split (',');
          nn=0;
          for ( QStringList::Iterator it = typeCellList.begin(); it != typeCellList.end(); ++it )
          {
	    // Output
            printf ("   Cell (%2d)   >%s< ",
                    nn, (*it).toLatin1().data() );
            nn++;

	    if ((*it)[0] == 'T')
	    {
                kDebug()<<" = top";
              transPenFormat ((*it), &topPenWidth, &topPenStyle);

              if ((*it).length() > 2)
	      {
		(*it).remove (0, 2);
                filterSHFGBG ((*it), &topbrushstyle, &topbrushcolor, &topfg_bg);
    	      }

	    }

	    else if ( (*it)[0] == 'B')
	    {
                kDebug()<<" = bottom";
              transPenFormat ((*it), &bottomPenWidth, &bottomPenStyle);

              if ((*it).length() > 2)
	      {
		(*it).remove (0, 2);
                filterSHFGBG ((*it), &bottombrushstyle, &bottombrushcolor, &bottomfg_bg);
    	      }
	    }

	    else if ( (*it)[0] == 'L')
	    {
                kDebug()<<" = left";
              transPenFormat ((*it), &leftPenWidth, &leftPenStyle);

              if ((*it).length() > 2)
	      {
		(*it).remove (0, 2);
                filterSHFGBG ((*it), &leftbrushstyle, &leftbrushcolor, &leftfg_bg);
    	      }
	    }

	    else if ( (*it)[0] == 'R')
	    {
                kDebug()<<" = right";
              transPenFormat ((*it), &rightPenWidth, &rightPenStyle);

              if ((*it).length() > 2)
	      {
		(*it).remove (0, 2);
                filterSHFGBG ((*it), &rightbrushstyle, &rightbrushcolor, &rightfg_bg);
    	      }
	    }

            else if ( ((*it).startsWith ("SH")) || ((*it).startsWith ("FG")) ||
                      ((*it).startsWith ("BG")) )
	    {
                kDebug()<<" =";
              filterSHFGBG ((*it), &brushstyle, &fg_bg, &brushcolor);
	    }

            else
	    {
                kDebug()<<"   = ???";
	    }

          }





          QString col;

          // create kspread fileformat output
          str += "   <cell row=\"" + QString::number (irow) + "\"";
          str += " column=\""      + QString::number (icol) + "\">\n";
	  if (bold == 1  || italic == 1 || underline == 1 ||
              align != 0 || valign != 0 ||
              topPenStyle  != 0  || bottomPenStyle != 0 ||
              leftPenStyle != 0  || rightPenStyle  != 0 || fg !=-1 || fg_bg != -1 ||
              fontsize != 12 || brushstyle != 0 || fontnr != -1)
	  {
            str += "    <format";
            if (brushstyle != 0)
            {
               str += " brushstyle=\""  + QString::number(brushstyle) + "\" ";
               str += " brushcolor=\"";
               str += writeColor (mcol.at(brushcolor));
               str += "\"";
	    }

            if (align   != 0)  str += " align=\""  + QString::number(align) + "\" ";
            if (valign  != 0)  str += " alignY=\"" + QString::number(valign) + "\" ";
            if (fg_bg != -1)
            {
              str += " bgcolor=\"";
              str += writeColor (mcol.at(fg_bg));
              str += "\" ";
	    }
            str += ">\n";

            // Font color
            if (fg != -1)
            {
              str += "    <pen width=\"0\" style=\"1\" color=\"";
              str += writeColor (mcol.at(fg));
	      str += "\" />\n";
	    }

            // Left border
            if (leftPenWidth > 0)
            {
              str += "    <left-border>\n";
              col = writeColor (mcol.at(leftfg_bg));
              writePen (str, leftPenWidth, leftPenStyle, col);
              str += "    </left-border>\n";
	    }

            // Right border
            if (rightPenWidth > 0)
            {
              str += "    <right-border>\n";
              col = writeColor (mcol.at(rightfg_bg));
              writePen (str, rightPenWidth, rightPenStyle, col);
              str += "    </right-border>\n";
	    }

            // Bottom border
            if (bottomPenWidth > 0)
            {
              str += "    <bottom-border>\n";
              col = writeColor (mcol.at(bottomfg_bg));
              writePen (str, bottomPenWidth, bottomPenStyle, col);
              str += "    </bottom-border>\n";
	    }

            // Top border
            if (topPenWidth > 0)
            {
              str += "    <top-border>\n";
              col = writeColor (mcol.at(topfg_bg));
              writePen (str, topPenWidth, topPenStyle, col);
              str += "    </top-border>\n";
	    }

            // Font (size and family)
            if ((fontsize != 12) || (fontnr != -1))
	    {
              str += "     <font ";
              // Fontsize
              if (fontsize != 12)
	      {
                str += "size=\"";
                str += QString::number (fontsize);
                str += "\" ";
	      }
              // Fontfamily
              if (fontnr != -1)
	      {
                str += "family=\"";
                str += typefacetab[fontnr].toLatin1();
                str += "\" ";
	      }
              str += "weight=\"0\"";

              if (italic    == 1) str += " italic=\"yes\"";
              if (bold      == 1) str += " bold=\"yes\"";
              if (underline == 1) str += " underline=\"yes\"";

              str +=" />\n";
	    }
	    str += "    </format>\n";
	  }
          str += "    <text>" + mystr + "</text>\n";
          str += "   </cell>\n";
	}

    }
    emit sigProgress(100);

    str += "  </table>\n";
    str += " </map>\n";
    str += "</spreadsheet>\n";
//  str += "</DOC>\n";

    kDebug ()<<"Text"<< str;

    KoStoreDevice* out=m_chain->storageFile( "root", KoStore::Write );

    if (!out)
    {
        kError(38000/*30502*/) << "Unable to open output file!" << endl;
        in.close  ();
        return KoFilter::StorageCreationError;
    }

    QByteArray cstring = str.toUtf8();
    out->write ( cstring, cstring.length() );

    in.close  ();
    return KoFilter::OK;
}




/******************************************************************************
 *  function: specCharfind                                                    *
 ******************************************************************************/
QChar
APPLIXSPREADImport::specCharfind (QChar a, QChar b)
{
  QChar chr;

   if      ( (a == 'n') && (b == 'p') ) chr = '�';


   else if ( (a == 'n') && (b == 'c') ) chr = '�';
   else if ( (a == 'p') && (b == 'c') ) chr = '�';

   else if ( (a == 'n') && (b == 'd') ) chr = '�';
   else if ( (a == 'p') && (b == 'd') ) chr = '�';

   else if ( (a == 'n') && (b == 'e') ) chr = '�';
   else if ( (a == 'p') && (b == 'e') ) chr = '�';

   else if ( (a == 'n') && (b == 'f') ) chr = '�';
   else if ( (a == 'p') && (b == 'f') ) chr = '�';

   else if ( (a == 'p') && (b == 'g') ) chr = '�';
   else if ( (a == 'n') && (b == 'g') ) chr = '�';



   else if ( (a == 'n') && (b == 'j') ) chr = '�';
   else if ( (a == 'p') && (b == 'j') ) chr = '�';

   else if ( (a == 'n') && (b == 'k') ) chr = '�';
   else if ( (a == 'p') && (b == 'k') ) chr = '�';

   else if ( (a == 'n') && (b == 'l') ) chr = '�';
   else if ( (a == 'p') && (b == 'l') ) chr = '�';

   else if ( (a == 'p') && (b == 'm') ) chr = '�';
   else if ( (a == 'n') && (b == 'm') ) chr = '�';



   else if ( (a == 'm') && (b == 'a') ) chr = '�';
   else if ( (a == 'o') && (b == 'a') ) chr = '�';

   else if ( (a == 'm') && (b == 'b') ) chr = '�';
   else if ( (a == 'o') && (b == 'b') ) chr = '�';

   else if ( (a == 'm') && (b == 'c') ) chr = '�';
   else if ( (a == 'o') && (b == 'c') ) chr = '�';

   else if ( (a == 'm') && (b == 'd') ) chr = '�';
   else if ( (a == 'o') && (b == 'd') ) chr = '�';

   else if ( (a == 'm') && (b == 'e') ) chr = '�';
   else if ( (a == 'o') && (b == 'e') ) chr = '�';

   else if ( (a == 'm') && (b == 'f') ) chr = '�';
   else if ( (a == 'o') && (b == 'f') ) chr = '�';

   else if ( (a == 'm') && (b == 'g') ) chr = '�';
   else if ( (a == 'o') && (b == 'g') ) chr = '�';



   else if ( (a == 'm') && (b == 'i') ) chr = '�';
   else if ( (a == 'o') && (b == 'i') ) chr = '�';

   else if ( (a == 'm') && (b == 'j') ) chr = '�';
   else if ( (a == 'o') && (b == 'j') ) chr = '�';

   else if ( (a == 'm') && (b == 'k') ) chr = '�';
   else if ( (a == 'o') && (b == 'k') ) chr = '�';

   else if ( (a == 'm') && (b == 'l') ) chr = '�';
   else if ( (a == 'o') && (b == 'l') ) chr = '�';






   else if ( (a == 'm') && (b == 'm') ) chr = '�';
   else if ( (a == 'o') && (b == 'm') ) chr = '�';

   else if ( (a == 'm') && (b == 'n') ) chr = '�';
   else if ( (a == 'o') && (b == 'n') ) chr = '�';

   else if ( (a == 'm') && (b == 'o') ) chr = '�';
   else if ( (a == 'o') && (b == 'o') ) chr = '�';

   else if ( (a == 'm') && (b == 'p') ) chr = '�';
   else if ( (a == 'o') && (b == 'p') ) chr = '�';


   else if ( (a == 'n') && (b == 'b') ) chr = '�';
   else if ( (a == 'p') && (b == 'b') ) chr = '�';


   else if ( (a == 'k') && (b == 'c') ) chr = '�';
   else if ( (a == 'k') && (b == 'j') ) chr = '�';
   else if ( (a == 'l') && (b == 'f') ) chr = '�';
   else if ( (a == 'n') && (b == 'i') ) chr = '�';
   else if ( (a == 'p') && (b == 'i') ) chr = '�';

   else if ( (a == 'l') && (b == 'j') ) chr = '�';
   else if ( (a == 'l') && (b == 'c') ) chr = '�';
   else if ( (a == 'l') && (b == 'd') ) chr = '�';

   else if ( (a == 'l') && (b == 'm') ) chr = '�';
   else if ( (a == 'l') && (b == 'n') ) chr = '�';
   else if ( (a == 'l') && (b == 'o') ) chr = '�';

   else if ( (a == 'l') && (b == 'a') ) chr = '�';

   else if ( (a == 'k') && (b == 'o') ) chr = '�';
   else if ( (a == 'k') && (b == 'h') ) chr = '�';
   else if ( (a == 'k') && (b == 'd') ) chr = '�';

   else if ( (a == 'p') && (b == 'a') ) chr = '�';
   else if ( (a == 'n') && (b == 'a') ) chr = '�';

   else if ( (a == 'l') && (b == 'l') ) chr = '�';
   else if ( (a == 'k') && (b == 'l') ) chr = '�';

   else if ( (a == 'l') && (b == 'k') ) chr = '�';

   else if ( (a == 'l') && (b == 'h') ) chr = '�';

   else if ( (a == 'k') && (b == 'b') ) chr = '�';

   else if ( (a == 'k') && (b == 'e') ) chr = '�';

   else if ( (a == 'l') && (b == 'b') ) chr = '�';

   else if ( (a == 'l') && (b == 'p') ) chr = '�';

   else if ( (a == 'k') && (b == 'f') ) chr = '�';

   else if ( (a == 'p') && (b == 'o') ) chr = '�';
   else if ( (a == 'n') && (b == 'o') ) chr = '�';

   else if ( (a == 'n') && (b == 'n') ) chr = '�';
   else if ( (a == 'p') && (b == 'n') ) chr = '�';
   else if ( (a == 'p') && (b == 'p') ) chr = '�';

   else if ( (a == 'k') && (b == 'k') ) chr = '�';

   else if ( (a == 'k') && (b == 'm') ) chr = '�';
   else if ( (a == 'p') && (b == 'h') ) chr = '�';

   else if ( (a == 'k') && (b == 'g') ) chr = '|';

   else if ( (a == 'l') && (b == 'e') ) chr = '\'';

   else if ( (a == 'k') && (b == 'i') ) chr = '�';

   else if ( (a == 'k') && (b == 'n') ) chr = '�';

   else if ( (a == 'k') && (b == 'p') ) chr = '�';

   else if ( (a == 'l') && (b == 'g') ) chr = '�';

   else if ( (a == 'l') && (b == 'i') ) chr = '�';

   else if ( (a == 'm') && (b == 'h') ) chr = '�';
   else if ( (a == 'o') && (b == 'h') ) chr = '�';

   else if ( (a == 'n') && (b == 'h') ) chr = '�';

   else if ( (a == 'k') && (b == 'a') ) chr = ' ';

   else if ( (a == 'a') && (b == 'j') ) chr = '!';

   else  chr = '#';

   return chr;
}



/******************************************************************************
 *  function:   writePen                                                      *
 ******************************************************************************/
void
APPLIXSPREADImport::writePen (QString &str, int penwidth, int penstyle, QString framecolor)
{
  str += "     <pen width=\"";

  // width of the pen
  str += QString::number (penwidth);
  str += "\" style=\"";

  // style of the pen
  str += QString::number (penstyle);
  str += "\" color=\"";

  // color of the pen
  str += framecolor;
  str += "\" />\n";

}



/******************************************************************************
 *  function:   writeColor                                                    *
 ******************************************************************************/
QString
APPLIXSPREADImport::writeColor (t_mycolor *mc)
{
  char rgb[20];

//    printf ("                 WriteColor: <%d>-<%d>-<%d>   <%d>-<%d>-<%d>-<%d>\n",
//            mc->r, mc->g, mc->b,
//            mc->c, mc->m, mc->y, mc->k);

  sprintf (rgb, "#%02X%02X%02X", mc->r, mc->g, mc->b);
  QString bla = rgb;


  return bla;
}




/******************************************************************************
 *  function:   readTypefaceTable                                             *
 ******************************************************************************/
void
APPLIXSPREADImport::readTypefaceTable  (QTextStream &stream, QStringList &typefacetab)
{
   int tftabCounter=0, ok;
   QString mystr;

   // Read the colormap
   kDebug()<<"Reading typeface table:";

   ok = true;
   do
   {
     mystr = nextLine( stream );
     if (mystr == "END TYPEFACE TABLE" ) ok = false;
     else
     {
         //printf ("  %2d: <%s>\n", tftabCounter, mystr.toLatin1());
       typefacetab.append(mystr);
       tftabCounter++;
     }
    }
    while (ok == true );

   kDebug()<<"... done";
}



/******************************************************************************
 *  function:   readColormap                                                  *
 ******************************************************************************/
void
APPLIXSPREADImport::readColormap (QTextStream &stream,  Q3PtrList<t_mycolor> &mcol)
{
  int contcount, ok, pos;

  QString colstr, mystr;
  kDebug ()<<"Reading colormap:";

  ok = true;

  do
  {

     mystr = nextLine( stream );
     mystr.trimmed ();

     if (mystr == "END COLORMAP") ok = false;
     else
     {
         kDebug()<<"  ->"<< mystr;

       // Count the number of  whitespaces
       contcount = mystr.count (' ');
       kDebug()<<"contcount:"<< contcount;
       contcount -= 5;

       // Begin off interest
       pos = mystr.indexOf (" 0 ");

       // get colorname
       colstr = mystr.left (pos);
       mystr.remove (0, pos+1);
       mystr.trimmed ();

       t_mycolor *tmc = new t_mycolor;

       // get sub colors
       pos = sscanf (mystr.toLatin1(), "0 %d %d %d %d 0",
                     &tmc->c, &tmc->m, &tmc->y, &tmc->k);

       printf ("  - <%-20s> <%-15s> <%3d> <%3d> <%3d> <%3d>  pos: %d\n",
                mystr.toLatin1().data(),
                colstr.toLatin1().data(),
                tmc->c, tmc->m, tmc->y, tmc->k, pos);

       // Color transformation cmyk -> rgb
       tmc->r = 255 - (tmc->c + tmc->k);
       if (tmc->r < 0) tmc->r = 0;

       tmc->g = 255 - (tmc->m + tmc->k);
       if (tmc->g < 0) tmc->g = 0;

       tmc->b = 255 - (tmc->y + tmc->k);
       if (tmc->b < 0) tmc->b = 0;

       mcol.append (tmc);
     }

   }
   while (ok == true );

  kDebug()<<"... done"<< mcol.count();


   t_mycolor *emp;
   for (emp=mcol.first(); emp != 0; emp=mcol.next() )
   {
     printf (" c:%3d m:%3d y:%3d k:%3d   r:%3d g:%3d b:%3d\n",
            emp->c, emp->m, emp->y, emp->k, emp->r, emp->g, emp->b);
   }
}




/******************************************************************************
 *  function:   readColormap                                                  *
 ******************************************************************************/
void
APPLIXSPREADImport::readView (QTextStream &stream, QString instr, t_rc &rc)
{
  QString rowcolstr;
  QString mystr, tabname;
  int ok;

  kDebug()<<"Reading View";

   tabname = instr;

   tabname.remove (0, 19);
   tabname.remove (tabname.length()-2, 2);
   kDebug()<<"  - Table name:"<< tabname;

   ok = true;
   do
   {
     mystr = nextLine( stream );

     kDebug()<<""<< mystr;
     if (mystr.startsWith ("View End, Name:")) ok = false;
     else
     {
       // COLUMN Widths
       if  (mystr.startsWith ("View Column Widths"))
       {
           kDebug()<<"   - Column Widths";
	  mystr.remove (0, 20);
          kDebug()<<""<<mystr;

          int  colwidth, icolumn;
          char ccolumn;

          // loop
          QStringList ColumnList;
          ColumnList = mystr.split(' ');

          for ( QStringList::Iterator it = ColumnList.begin(); it != ColumnList.end(); ++it )
          {

            sscanf ((*it).toLatin1(), "%c:%d", &ccolumn, &colwidth);
            int len = (*it).length ();
            int pos = (*it).indexOf (":");
            (*it).remove (pos, len-pos);

            printf( "     >%s<- -<%c><%d>  \n", (*it).toLatin1().data(), ccolumn, colwidth);

            // Transformat ascii column to int column
            icolumn = translateColumnNumber (*it);

            //icolumn = ccolumn - 64;
            // Translate the column width right from applix to kspread
            icolumn = icolumn * 5;


            rowcolstr += "  <column width=\"";
            rowcolstr += QString::number (colwidth);
            rowcolstr += "\" column=\"";
            rowcolstr += QString::number (icolumn);
            rowcolstr += "\" >\n";
            rowcolstr += "   <format/>\n";
            rowcolstr += "  </column>\n";
          }
       }

       // ROW Heights
       else if  (mystr.startsWith ("View Row Heights"))
       {
           kDebug()<<"   - Row Heights";
	 mystr.remove (0, 17);
         kDebug()<<""<< mystr;

         int irow, rowheight;

         // loop
         QStringList RowList;
         RowList = mystr.split(' ');

         for ( QStringList::Iterator it = RowList.begin(); it != RowList.end(); ++it )
         {
            sscanf ((*it).toLatin1(), " %d:%d",
                    &irow, &rowheight);
            printf ("   row: %2d   height: %2d\n", irow, rowheight);
            if (rowheight > 32768) rowheight -= 32768;
            printf ("              height: %2d\n", rowheight);
            rowcolstr += "  <row row=\"";
            rowcolstr += QString::number (irow);
            rowcolstr += "\" height=\"";
            rowcolstr += QString::number (rowheight);
            rowcolstr += "\" >\n";
            rowcolstr += "   <format/>\n";
            rowcolstr += "  </row>\n";
         }


       }
     } // else != END COLORMAP
   }
   while (ok == true );

   // tabname append to my list
   // tabname append to my list
   rc.tabname.append (tabname);
   rc.rc.append (rowcolstr);

   printf ("%s %s\n", tabname.toLatin1().data(),
                      rowcolstr.toLatin1().data());

   printf ("...done \n\n");
}





/******************************************************************************
 *  function:   filterSHFGBG                                                  *
 ******************************************************************************/
void
APPLIXSPREADImport::filterSHFGBG (QString it, int *style, int *bgcolor,
                                  int *fgcolor)
{
  QString tmpstr;
  int     pos;
  int     m2=0, m3=0;

  // filter SH = Brushstyle Background
  pos = it.indexOf ("SH");
  if (pos > -1)
  {
     tmpstr = it;
     if (pos > 0)   tmpstr.remove(0, pos);
     pos = sscanf (tmpstr.toLatin1(), "SH%d",
                   style);

     printf ("style: %d(%d)  ",
             *style, pos);
  }


  // filter FG = FGCOLOR
  pos = it.indexOf ("FG");
  if (pos > -1)
  {
    tmpstr = it;
    if (pos > 0)   tmpstr.remove(0, pos);
    pos = sscanf (tmpstr.toLatin1(), "FG%d",
                  fgcolor);
    printf ("fg: %d(%d)  ",
            *fgcolor, pos);
     m2=1;
  }


  // filter BG = BGCOLOR
  pos = it.indexOf ("BG");
  if (pos > -1)
  {
    tmpstr = it;
    if (pos > 0)   tmpstr.remove(0, pos);
    pos = sscanf (tmpstr.toLatin1(), "BG%d",
                  bgcolor);
    printf ("bgcolor: %d(%d)  ",
            *bgcolor, pos);
    m3=1;
  }


  printf ("\n");


  // correct the bgcolor to the fgcolor if the background is plain
  if ((*style == 8) && (m2 == 1) && (m3 == 0))
  {
    *bgcolor = *fgcolor;
  }


  // Translate brushstyle to kspread brushstyle
  if (*style != 0)
  {
    if       (*style ==  1) *style =  0;
    else if  (*style ==  2) *style =  7;
    else if  (*style ==  3) *style =  0;
    else if  (*style ==  4) *style =  4;
    else if  (*style ==  5) *style =  3;
    else if  (*style ==  6) *style =  2;
    else if  (*style ==  7) *style =  0;
    else if  (*style ==  8) *style =  0;
    else if  (*style ==  9) *style = 10;
    else if  (*style == 10) *style =  9;
    else if  (*style == 11) *style = 11;
    else if  (*style == 12) *style = 12;
    else if  (*style == 13) *style = 13;
    else if  (*style == 14) *style = 14;
    else if  (*style == 15) *style =  0;
    else if  (*style == 16) *style =  0;
    else if  (*style == 17) *style =  0;
    else if  (*style == 18) *style =  0;
    else if  (*style == 19) *style =  0;
  }
}



/******************************************************************************
 *  function:   filterSHFGBG                                                  *
 ******************************************************************************/
void
APPLIXSPREADImport::transPenFormat (QString it, int *PenWidth, int *PenStyle)
{

   if       ( it[1] == '1' )
   {
     *PenWidth = 1;
     *PenStyle = 1;
   }

   else if  ( it[1] == '2' )
   {
     *PenWidth = 2;
     *PenStyle = 1;
   }

   else if  ( it[1] == '3' )
   {
     *PenWidth = 3;
     *PenStyle = 1;
   }

   else if  ( it[1] == '4' )
   {
     *PenWidth = 1;
     *PenStyle = 3;
   }

   else if  ( it[1] == '5' )
   {
     *PenWidth = 5;
     *PenStyle = 1;
   }

   printf ("frame (w:%d - s:%d) \n", *PenWidth, *PenStyle);
}




/******************************************************************************
 *  function: readHeader                                                       *
 ******************************************************************************/
int
APPLIXSPREADImport::readHeader (QTextStream &stream)
{
  QString mystr;
  int     vers[3] = { 0, 0, 0 };
  int     rueck;


    // Read Headline
    mystr = nextLine (stream);
    rueck = sscanf (mystr.toLatin1(),
                    "*BEGIN SPREADSHEETS VERSION=%d/%d ENCODING=%dBIT",
	             &vers[0], &vers[1], &vers[2]);
    printf ("Versions info: %d %d %d\n", vers[0], vers[1], vers[2]);

    // Check the headline
    if (rueck <= 0)
    {
      printf ("Header not correct - May be it is not an applixspreadsheet file\n");
      printf ("Headerline: <%s>\n", mystr.toLatin1().data());

      QMessageBox::critical (0L, "Applix spreadsheet header problem",
              QString ("The Applix Spreadsheet header is not correct. "
                       "May be it is not an applix spreadsheet file! <BR>"
                       "This is the header line I did read:<BR><B>%1</B>").arg(mystr),
                       "Okay");


      return false;
    }
    else
    {
      return true;
    }
}



/******************************************************************************
 *  function: translateRowNumber                                              *
 ******************************************************************************/
int
APPLIXSPREADImport::translateColumnNumber (QString colstr)
{
  int icol=0;
  int p, x, len;


  len = colstr.length ();
  p = len-1;
  x = 1;

  printf ("HI 0 len:%d\n", len );
  while ((p >= 0))
  {
     printf ("HI 1 x:%d p:%d char:<%c>\n", x, p, colstr[p].toLatin1());
     // Upper chars
     if      ((colstr[p] >= 'A') && (colstr[p] <= 'Z'))
     {
         kDebug ()<<" UPPER";
       icol = icol + ((int)pow ((double)x, 26) * (colstr[p].toLatin1() - 'A' + 1)  );
       x++;
     }
     // lower chars
     else if ((colstr[p] >= 'a') && (colstr[p] <= 'z'))
     {
         kDebug()<<" lower";
       icol = icol + ((int)pow ((double)x, 26) * (colstr[p].toLatin1() - 'a' + 1)  );
       x++;
     }
     p--;
     kDebug ()<<"HI 2";

   }

   printf ("translateColumnNumber : <%s> -> %d\n", colstr.toLatin1().data(), icol);
   return icol;
}

#include <applixspreadimport.moc>
