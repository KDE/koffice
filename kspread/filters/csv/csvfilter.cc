/*
 *  koffice/filters/csv/csvfilter.cc
 *
 *  Copyright (C) 1999 David Faure <faure@kde.org>
 *  Covered by the GPL license
 */

#include <qstring.h>
#include <kdebug.h>

#include <csvfilter.h>
#include <csvfilter.moc>

CSVFilter::CSVFilter(const QTextStream &_inputStream) 
  : inputStream ( _inputStream ),
    bReady( false )
{
}

CSVFilter::~CSVFilter() 
{
}

const bool CSVFilter::filter()
{
  bReady = true; // note that filter() has been called
  // TODO !

  tree.cell( "Trying...." );
  tree.newline();
  tree.cell("Line 2 !");

  bSuccess = true; // TODO : only if file parsed correctly
  return bSuccess;
}

const QString CSVFilter::part() 
{

  if(bReady && bSuccess) {
    kdebug(KDEBUG_INFO, 31000, tree.part());
    return tree.part();
  }
  else {
    QString str;
    str+="<?xml version=\"1.0\"?>\n"
      "<DOC author=\"Torben Weis\" email=\"weis@kde.org\" editor=\"KSpread\" mime=\"application/x-kspread\" >\n"
      "<PAPER format=\"A4\" orientation=\"Portrait\">\n"
      "<PAPERBORDERS left=\"20\" top=\"20\" right=\"20\" bottom=\"20\"/>\n"
      "<HEAD left=\"\" center=\"\" right=\"\"/>\n"
      "<FOOT left=\"\" center=\"\" right=\"\"/>\n"
      "</PAPER>\n"
      "<MAP>\n"
      "<TABLE name=\"Table1\">\n"
      "<CELL row=\"1\" column=\"1\">\n"
      "<FORMAT align=\"4\" precision=\"-1\" float=\"3\" floatcolor=\"2\" faktor=\"1\"/>\n"
      "Sorry :(\n"
      "</CELL>\n"
      "</TABLE>\n"
      "</MAP>\n"
      "</DOC>";
    return str;
  }
}
