/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (c) 2000 ID-PRO Deutschland GmbH. All rights reserved.
                      Contact: Wolf-Michael Bolle <Wolf-Michael.Bolle@gmx.de>
   Copurogjt (C) 2001 Michael Johnson <mikej@xnet.com>


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

#include <kwExport.h>
#include <docinfoExport.h>

// Global variables
QValueList<FormatData> paraFormatDataList; // for processing formats tag
BorderStyle leftBorder;
BorderStyle rightBorder;
BorderStyle topBorder;
BorderStyle bottomBorder;
PaperBorders paperBorders; // initialized by class init

// Every tag has its own processing function. All of those functions
// have the same parameters since the functions are passed to
// ProcessSubtags throuch the TagProcessing class.  The top level
// function is ProcessDocTag and can be called with the node returned
// by QDomDocument::documentElement (). The tagData argument can be
// used to either pass variables down to the subtags or to allow
// subtags to return values. As a bare minimum the tag processing
// functions must handle the tag's attributes and the tag's subtags
// (which it can choose to ignore). Currently implemented is
// processing for the following tags and attributes:
//
//
// maindoc.xml (root):
// DOC
//   ATTRIBUTES  processing standardpage hasHeader hasFooter unit
//   PAPERBORDERS mmLeft mmRight mmTop mmBottom
//   PAPER format width height orientation  columns colSpacing hType fType
//         spHeadBody spFootBody
//   FRAMESETS
//     FRAMESET col= row= cols=1 rows=1 grpMgr=
//       PARAGRAPH
//          TEXT - Text Element
//          FORMATS
//            FORMAT id=1 pos= len=
//              FONT name=
//              ITALIC value=1
//              SIZE value =
//              WEIGHT value=
//              UNDERLINE value=1
//              ANCHOR type=grpMgr instance=
//          LAYOUT
//            NAME value=
//            FLOW align+
//            COUNTER  type depth start lefttext righttext
//            LEFTBORDER
//            RIGHTBORDER
//            TOPBORDER
//            BOTTOMBORDER





/***************************************************************************/

void ProcessBordersStyleTag ( QDomNode    myNode,
                        void       *tagData,
                        QString    &         )
// Gets the attributes in the  tag - information on table cell borders
// called by ProcessLayoutTag()
{
    BorderStyle *border = (BorderStyle *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "red",     "int", (void *) &border->red )
                       << AttrProcessing ( "green", "int", (void *) &border->green )
                       << AttrProcessing ( "blue", "int", (void *) &border->blue   )
                       << AttrProcessing ( "style", "int", (void *) &border->style )
                       << AttrProcessing ( "width", "int", (void *) &border->width );
    ProcessAttributes (myNode, attrProcessingList);



    AllowNoSubtags (myNode);
}   // end ProcessBordersStyleTag

/***************************************************************************/

void ProcessColorTag ( QDomNode    myNode,
                       void       *tagData,
                       QString    &         )
// Gets the attributes in the  COLOR tag - information on text color
// called by ProcessFormatTag()
{
    ColorLayout *color = (ColorLayout *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "red",  "int", (void *) &color->red   )
                       << AttrProcessing ( "green","int", (void *) &color->green )
                       << AttrProcessing ( "blue", "int", (void *) &color->blue  );
    ProcessAttributes (myNode, attrProcessingList);



    AllowNoSubtags (myNode);
}   // end ProcessColorTag

/***************************************************************************/

void ProcessTabulatorTag ( QDomNode    myNode,
                        void       *tagData,
                        QString    &         )
// Gets the attributes in the TABULARTOR tags - positions of tab stops
// called by ProcessLayoutTag()

{
    QValueList <TabularData> *tabulatorData = (QValueList<TabularData> *) tagData;
    TabularData tabData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "ptpos", "int",   (void *) &tabData.ptpos )
                       << AttrProcessing ( "type",  "int",   (void *) &tabData.type  );
    ProcessAttributes (myNode, attrProcessingList);

    *tabulatorData << tabData;

    AllowNoSubtags (myNode);
}   // end ProcessTabulatorTag

/***************************************************************************/

void ProcessAttributesTag ( QDomNode    myNode,
                        void       *tagData,
                        QString    &         )
// Gets the attributes in the ATTRIBUTES tags - information on page layout
// called by ProcessDocTag()

{
    Attributes *attributes = (Attributes *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "processing",   "int",    (void *) &attributes->processing   )
                       << AttrProcessing ( "standardpage", "int",    (void *) &attributes->standardpage )
                       << AttrProcessing ( "hasHeader",    "int",    (void *) &attributes->hasHeader    )
                       << AttrProcessing ( "hasFooter",    "int",    (void *) &attributes->hasFooter    )
                       << AttrProcessing ( "unit",         "QString",(void *) &attributes->unit         );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode);
}   // end ProcessAttributesTag

/***************************************************************************/

void ProcessPaperBordersTag ( QDomNode    myNode,
                        void       *tagData,
                        QString    &         )
// Gets the attributes in the PAPERBORDERS tag - information on paper margins
// called by ProcessPaperTag()

{
    PaperBorders *paperborders = (PaperBorders *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "left",     "int", (void *) &paperborders->left   )
                       << AttrProcessing ( "right",    "int", (void *) &paperborders->right  )
                       << AttrProcessing ( "top",      "int", (void *) &paperborders->top    )
                       << AttrProcessing ( "bottom",   "int", (void *) &paperborders->bottom );
    ProcessAttributes (myNode, attrProcessingList);


    AllowNoSubtags (myNode);
}   // end ProcessPaperBordersTag


/***************************************************************************/

void ProcessIndentTag ( QDomNode    myNode,
                        void       *tagData,
                        QString    &         )
// Gets the attributes in the indentation tags - first and left
// called by ProcessLayoutTag()

{
    ParaLayout *layout = (ParaLayout *) tagData;

    layout->idFirst = -1;  //  initialize
    layout->idLeft  = -1;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "first", "int", (void *) &layout->idFirst  )
                       << AttrProcessing ( "right", "int", (void *) &layout->idRight  )
                       << AttrProcessing ( "left",  "int", (void *) &layout->idLeft   );
    ProcessAttributes (myNode, attrProcessingList);


    AllowNoSubtags (myNode);
}   // end ProcessIndentTag


void ProcessTimeTag ( QDomNode    myNode,
                      void       *tagData,
                      QString    &         )
// Gets the attributes in the time tags
// called by ProcessFormatTag()

{
    Time *time = (Time *) tagData;


    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "msecond", "int", (void *) &time->msecond )
                       << AttrProcessing ( "second",  "int", (void *) &time->second  )
                       << AttrProcessing ( "minute",  "int", (void *) &time->minute  )
                       << AttrProcessing ( "hour",    "int", (void *) &time->hour    )
                       << AttrProcessing ( "fix",     "int", (void *) &time->fix     );
    ProcessAttributes (myNode, attrProcessingList);


    AllowNoSubtags (myNode);
}   // end ProcessTimeTag


void ProcessDateTag ( QDomNode    myNode,
                      void       *tagData,
                      QString    &         )
// Gets the attributes in the Date tags
// called by ProcessFormatTag()

{
    Date *date = (Date *) tagData;


    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "day",   "int", (void *) &date->day   )
                       << AttrProcessing ( "month", "int", (void *) &date->month )
                       << AttrProcessing ( "year",  "int", (void *) &date->year  )
                       << AttrProcessing ( "fix",   "int", (void *) &date->fix   );
    ProcessAttributes (myNode, attrProcessingList);


    AllowNoSubtags (myNode);
}   // end ProcessDateTag

/***************************************************************************/

// ProcessTypeTag is used to process the type attribute in the TYPE tag
// called by ProcessFormatTat()

void ProcessTypeTag ( QDomNode   myNode,
                      void      *tagData,
                      QString   &         )
{
    int *value = (int *) tagData;

    *value = -1;
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "type", "int", (void *) value );
    ProcessAttributes (myNode, attrProcessingList);
    AllowNoSubtags (myNode);

}  // end ProcessTypeTag()

/***************************************************************************/

void ProcessPaperTag ( QDomNode    myNode,
                        void       *tagData,
                        QString    &outputText         )
// Gets the attributes in the PAPER tag - information on page layout
// called by ProcessDocTag()

{
    PaperAttributes *paperattributes = (PaperAttributes *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "format",        "int", (void *) &paperattributes->format     )
                       << AttrProcessing ( "width",         "int", (void *) &paperattributes->width      )
                       << AttrProcessing ( "height",        "int", (void *) &paperattributes->height     )
                       << AttrProcessing ( "orientation",   "int", (void *) &paperattributes->orientation)
                       << AttrProcessing ( "columns",       "int", (void *) &paperattributes->columns    )
                       << AttrProcessing ( "columnspacing", "int", (void *) &paperattributes->colSpacing )
                       << AttrProcessing ( "hType",         "int", (void *) &paperattributes->hType      )
                       << AttrProcessing ( "fType",         "int", (void *) &paperattributes->fType      )
                       << AttrProcessing ( "zoom",          "",    NULL                                  )
                       << AttrProcessing ( "spHeadBody",    "",    NULL                                  )
                       << AttrProcessing ( "spFootBody",    "",    NULL                                  );
    ProcessAttributes (myNode, attrProcessingList);

    // set global variables
    GhType = paperattributes->hType;  // set document indicator for page header
    GfType = paperattributes->fType;

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "PAPERBORDERS", ProcessPaperBordersTag,(void *) &paperBorders);
    ProcessSubtags (myNode, tagProcessingList, outputText);

}   // end ProcessPaperTag

/***************************************************************************/

// ProcessValueTag is the processing function for both the FILENAME tag
// and the LAYOUT NAME tag.

void ProcessValueTag ( QDomNode   myNode,
                       void      *tagData,
                       QString   &         )
{
    QString *value = (QString *) tagData;

    *value = "";
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "value", "QString", (void *) value );
    ProcessAttributes (myNode, attrProcessingList);
    AllowNoSubtags (myNode);

}  // end ProcessValueTag()

/***************************************************************************/

// ProcessIntValueTag is used to process value attributes with a numeric value.

void ProcessIntValueTag ( QDomNode   myNode,
                       void      *tagData,
                       QString   &         )
{
    int *value = (int *) tagData;

    *value = -1;
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "value", "int", (void *) value );
    ProcessAttributes (myNode, attrProcessingList);
    AllowNoSubtags (myNode);

}  // end ProcessIntValueTag()

/***************************************************************************/

// ProcessTextTag is a processing function for several different tags
// that are text elements, and have no attributes and no subtags.

void ProcessTextTag ( QDomNode    myNode,
                      void       *tagData,
                      QString    &         )
{
    QString *tagText = (QString *) tagData;

    QDomText myText ( myNode.firstChild ().toText () );  // extract text

    if ( !myText.isNull () )
    {
        *tagText = myText.data ();   // transfer text to return location
    }
    else
    {
        *tagText = "";
    }

    AllowNoAttributes (myNode);

    AllowNoSubtags (myNode);
}  // ProcessTextTag


/***************************************************************************/

void ProcessCounterTag ( QDomNode    myNode,
                        void       *tagData,
                        QString    &         )
// Gets the attributes in the counter tag - information on list counters
{
    ParaLayout *layout = (ParaLayout *) tagData;
    int type = -1;
    int depth = -1;
    int start = -1;
    QString lefttext = "";
    QString righttext = "";

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "type",          "int",     (void *) &type      )
                       << AttrProcessing ( "depth",         "int",     (void *) &depth     )
                       << AttrProcessing ( "start",         "int",     (void *) &start     )
                       << AttrProcessing ( "bullet",        "",        NULL                )
                       << AttrProcessing ( "numberingtype", "",        NULL                )
                       << AttrProcessing ( "bulletfont",    "",        NULL                )
                       << AttrProcessing ( "customdef",     "",        NULL                )
                       << AttrProcessing ( "lefttext",      "QString", (void *) &lefttext  )
                       << AttrProcessing ( "righttext",     "QString", (void *) &righttext );
    ProcessAttributes (myNode, attrProcessingList);

    // Put info into layout structure
    layout->type = type;
    layout->depth = depth;
    layout->start = start;
    layout->lefttext = lefttext;
    layout->righttext = righttext;


    AllowNoSubtags (myNode);
}   // end ProcessCounterTag

/***************************************************************************/

void   ProcessFlowTag ( QDomNode    myNode,
                        void       *tagData,
                        QString    &         )
// Gets the align attribute in the flow tag
{
    ParaLayout *layout = (ParaLayout *) tagData;
    QString flow = "";

    QValueList<AttrProcessing> attrProcessingList;
//    attrProcessingList << AttrProcessing ( "align", "QString", (void *) &flow );
    attrProcessingList << AttrProcessing ( "align", "QString", (void *) &flow );
    ProcessAttributes (myNode, attrProcessingList);

    // Put flow into layout structure
    layout->flow = flow;

    AllowNoSubtags (myNode);
}   // end ProcessFlowTag

/***************************************************************************/


void ProcessLayoutTag ( QDomNode   myNode,
                        void      *tagData,
                        QString   &outputText )
{
    ParaLayout *layout = (ParaLayout *) tagData;
    QString name;

    AllowNoAttributes (myNode);

    paraFormatDataList.clear(); // clear global value list for new layout

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "NAME",         ProcessValueTag,        (void *) &name                )
                      << TagProcessing ( "COUNTER",      ProcessCounterTag,      (void *) layout               )
                      << TagProcessing ( "TABULATOR",    ProcessTabulatorTag,    (void *) &layout->tabularData )
                      << TagProcessing ( "FLOW",         ProcessFlowTag,         (void *) layout               )
                      << TagProcessing ( "INDENTS",      ProcessIndentTag,       (void *) layout               )
                      << TagProcessing ( "OFFSETS",      NULL,                   NULL                          )
                      << TagProcessing ( "PAGEBREAKING", NULL,                   NULL                          )
                      << TagProcessing ( "LINESPACING",  ProcessIntValueTag,     (void *) &layout->lineSpacing )
                      << TagProcessing ( "FORMAT",       ProcessFormatTag,       (void *) &paraFormatDataList  )
                      << TagProcessing ( "FOLLOWING",    NULL,                   NULL                          )
                      << TagProcessing ( "LEFTBORDER",   ProcessBordersStyleTag, (void *) &layout->leftBorder  )
                      << TagProcessing ( "RIGHTBORDER",  ProcessBordersStyleTag, (void *) &layout->rightBorder )
                      << TagProcessing ( "TOPBORDER",    ProcessBordersStyleTag, (void *) &layout->topBorder   )
                      << TagProcessing ( "BOTTOMBORDER", ProcessBordersStyleTag, (void *) &layout->bottomBorder);
    ProcessSubtags (myNode, tagProcessingList, outputText);

    // enter result into global variables
    leftBorder =   layout->leftBorder;
    rightBorder =  layout->rightBorder;
    topBorder =    layout->topBorder;
    bottomBorder = layout->bottomBorder;

    if ( (name).length () == 0 )
    {
        name = "Standard";  // No name is same as standard

//        kdError (KDEBUG_KWFILTER) << "Bad layout name value!" << endl;
    }
    layout->layout = name;
}  // end ProcessLayoutTag

/***************************************************************************/

void ProcessItalicTag ( QDomNode   myNode,
                        void      *tagData,
                        QString   &         )

// used for italic and underline tags - sets indicator if value=1
{
    bool *italic = (bool *) tagData;

    *italic = false;

    int value (-1);
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "value", "int", (void *) &value );
    ProcessAttributes (myNode, attrProcessingList);

    switch ( value )
    {
        case 1:
            *italic = true;
            break;

        case -1:
            kdError (KDEBUG_KWFILTER) << "Bad attributes in ITALIC tag!" << endl;
            break;

        case 0:
            *italic = false;
            break;

        default:
            kdError (KDEBUG_KWFILTER) << "Unexpected ITALIC attribute value value "
                                           << value << "!" << endl;
    }

    AllowNoSubtags (myNode);
}  // end ProcessItalicTag

/***************************************************************************/


void ProcessFontTag ( QDomNode    myNode,
                      void       *tagData,
                      QString    &         )
// gets text string of FONT value=
{
    QString *fontName = (QString *) tagData;

    *fontName = "";
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "name", "QString", (void *) fontName );
    ProcessAttributes (myNode, attrProcessingList);

    if ( (*fontName).length () == 0 )
    {
        kdError (KDEBUG_KWFILTER) << "Bad font name!" << endl;
    }

    AllowNoSubtags (myNode);
}  // end ProcessFontTag

/***************************************************************************/

void ProcessAnchorTag ( QDomNode    myNode,
                        void       *tagData,
                        QString    &         )
// gets and passes back the grpMgr in instance name
{
    QString *instance = (QString *) tagData;

    QString type;
    *instance = "";
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "type",     "QString", (void *) &type    )
                       << AttrProcessing ( "instance", "QString", (void *) instance );
    ProcessAttributes (myNode, attrProcessingList);

    if ( type != "grpMgr" )
    {
       kdError (KDEBUG_KWFILTER) << "Unknown anchor type " << type << "!" << endl;
    }

    if ( (*instance).length () == 0 )
    {
        kdError (KDEBUG_KWFILTER) << "Bad instance name!" << endl;
    }

    AllowNoSubtags (myNode);
}   // end ProcessAnchorTag

/***************************************************************************/

// findAnchoredInsert () searches for the anchored insert in the list
// of all anchored inserts of the document that fits to the picture or
// table data that was found during the tag processing so that data
// can be added to the object.

AnchoredInsert *findAnchoredInsert (
                                   AnchoredInsert searchElement,
                                   QValueList<AnchoredInsert> &list)
{


    QValueList<AnchoredInsert>::Iterator it;
    QValueList<AnchoredInsert>::Iterator listElement;
    bool                        found = false;

    for ( it = list.begin (); it != list.end (); it++ )  // check list for a match
    {
        if ( (*it).type == searchElement.type )
        {
            bool equal = false;

            switch ( searchElement.type ) // try to match names
            {
                case 2:
                    equal = (*it).picture.name == searchElement.picture.name;
                    break;

                case 6:
                    equal = (*it).table.name == searchElement.table.name;
                    break;
            } // end switch

            if ( equal )
            {
                if ( !found )
                {
                    found = true;
                    listElement = it;
                }
                else
                {
                    kdError (KDEBUG_KWFILTER) << "More than one anchor"
                                               << endl;
                }
            } // end if(equao)
        }  // end if( (*it).type == ...)
    }  // end for(  ...)

    if ( !found )
    {
        kdError (KDEBUG_KWFILTER) << "No anchor found "
                                       << "! Will append one at the end." << endl;

        list.prepend ( searchElement );

        listElement = list.begin ();
    }

    return &(*listElement);
}   // end findAnchoredInsert()



// FormatData is a container for data retreived from the FORMAT tag
// and its subtags to be used in the PARAGRAPH tag.

/***************************************************************************/
void Table::addCell ( int     c,
                      int     r,
                      QString t,
                      BorderStyle l,
                      BorderStyle ri,
                      BorderStyle tp,
                      BorderStyle b,
                      Frame f  )

{
   if ( c + 1 > cols )
   {
      cols = c + 1;
   }

   cellList << TableCell ( c, r, t, l, ri, tp, b, f );
}   // end addCell()




/***************************************************************************/
// This function processes the FORMAT tag retrieving the format id, position,
// length, size, weight, font name, italic or underline and the anchor indicator.s
// The type of processing done depends on the format id. Only id's 1, 2
// and 6 are processed at this time. Ids 3 (tabular), 4 (variable page no), and 5
// (footnote) are not processed at this time.

void ProcessFormatTag ( QDomNode   myNode,
                        void      *tagData,
                        QString   &outputText )
{
    QValueList<FormatData> *formatDataList = (QValueList<FormatData> *) tagData;

    ColorLayout color;
    int formatId  = -1;
    int formatPos = -1;
    int formatLen = -1;
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "id",  "int", (void *) &formatId  )
                       << AttrProcessing ( "pos", "int", (void *) &formatPos )
                       << AttrProcessing ( "len", "int", (void *) &formatLen );
    ProcessAttributes (myNode, attrProcessingList);

    switch ( formatId )
    {
       case 1:   // regular text
       case 4:   // variable items time, date, page number
          if ( formatPos != -1 && formatLen != -1 )
          {
             int     fontSize     = -1;
             int     fontWeight   = -1;
             int     vertalign    = -1;
             int     pageNum      = -1;
             int     varType      = -1;
             Time    time;
             Date    date;
             QString fontName = "";
             bool    italic    = false;
             bool    underline = false;
             bool    strikeout = false;
             QValueList<TagProcessing> tagProcessingList;
             tagProcessingList << TagProcessing ( "SIZE",      ProcessIntValueTag,  (void *) &fontSize   )
                               << TagProcessing ( "WEIGHT",    ProcessIntValueTag,  (void *) &fontWeight )
                               << TagProcessing ( "UNDERLINE", ProcessItalicTag,    (void *) &underline  )
                               << TagProcessing ( "STRIKEOUT", ProcessItalicTag,    (void *) &strikeout  )
                               << TagProcessing ( "FONT",      ProcessFontTag,      (void *) &fontName   )
                               << TagProcessing ( "VERTALIGN", ProcessIntValueTag,  (void *) &vertalign  )
                               << TagProcessing ( "COLOR",     ProcessColorTag,     (void *) &color      )
                               << TagProcessing ( "DATE",      ProcessDateTag,      (void *) &date       )
                               << TagProcessing ( "TIME",      ProcessTimeTag,      (void *) &time       )
                               << TagProcessing ( "PGNUM",     ProcessIntValueTag,  (void *) &pageNum    )
                               << TagProcessing ( "TYPE",      ProcessTypeTag,      (void *) &varType    )
                               << TagProcessing ( "ITALIC",    ProcessItalicTag,    (void *) &italic     );
             ProcessSubtags (myNode, tagProcessingList, outputText);

             (*formatDataList) << FormatData ( TextFormatting (formatId, formatPos,
             formatLen, fontSize, fontWeight, fontName, italic, underline, strikeout,
             vertalign, color.red, color.blue, color.green, pageNum, time, date, varType ));
          }

          break;

       case 2:   // pictures
          if ( formatPos != -1 && formatLen == -1 )
          {
             QString pictureName;
             QValueList<TagProcessing> tagProcessingList;
             tagProcessingList << TagProcessing ( "FILENAME", ProcessValueTag, &pictureName );
             ProcessSubtags (myNode, tagProcessingList, outputText);

             (*formatDataList) << FormatData ( PictureAnchor (formatPos, pictureName ) );
          }
          else
          {
             kdError (KDEBUG_KWFILTER) << "Missing or bad picture formatting!" << endl;
          }
          break;

       case 6:   // tables
          if ( formatPos != -1 && formatLen == -1 )
          {
             QString instance = "";

             QValueList<TagProcessing> tagProcessingList;
             tagProcessingList << TagProcessing ( "ANCHOR", ProcessAnchorTag, (void *) &instance );
             ProcessSubtags (myNode, tagProcessingList, outputText);

             (*formatDataList) << FormatData ( TableAnchor (formatPos, instance) );
          }
          else
          {
             kdError (KDEBUG_KWFILTER) << "Missing or bad table anchor formatting!" << endl;
          }
          break;

       case -1:
          kdError (KDEBUG_KWFILTER) << "FORMAT attribute id value not set!" << endl;

          AllowNoSubtags (myNode);
          break;

       default:
          kdError (KDEBUG_KWFILTER) << "Unexpected FORMAT attribute id "
                                         << formatId << "!" << endl;

          AllowNoSubtags (myNode);
#if 0
          (*formatDataList) << FormatData (formatId);
#endif
    }
}  // end ProcessFormatTag

/***************************************************************************/

void ProcessFormatsTag ( QDomNode   myNode,
                         void      *tagData,
                         QString   &outputText )

// This function processes multiple FORMAT subtags and through function
//  calls outputs the rtf markup corresponding to the FORMAT tags.

{
    QValueList<FormatData> *formatDataList = (QValueList<FormatData> *) tagData;

    AllowNoAttributes (myNode);

    (*formatDataList).clear ();
    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "FORMAT", ProcessFormatTag, (void *) formatDataList );
    ProcessSubtags (myNode, tagProcessingList, outputText);
}  // end ProcessFormatsTag

/***************************************************************************/

void ProcessFrameTag ( QDomNode    myNode,
                        void       *tagData,
                        QString    &         )
// Gets the attributes in the FRAME tags - Frame positions
// called by ProcessFramesetTag()

{
    Frame *frame = (Frame *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "right",             "int",    (void *) &frame->right              )
                       << AttrProcessing ( "left",              "int",    (void *) &frame->left               )
                       << AttrProcessing ( "top",               "int",    (void *) &frame->top                )
                       << AttrProcessing ( "bottom",            "int",    (void *) &frame->bottom             )
                       // The following four are not in the dtd
                       << AttrProcessing ( "bleftpt",           "",       NULL                                )
                       << AttrProcessing ( "brightpt",          "",       NULL                                )
                       << AttrProcessing ( "bbottompt",         "",       NULL                                )
                       << AttrProcessing ( "btoppt",            "",       NULL                                )
                       << AttrProcessing ( "runaround",         "int",    (void *) &frame->runaround          )
                       << AttrProcessing ( "runaroundGap",      "int",    (void *) &frame->runaroundGap       )
                       << AttrProcessing ( "autoCreateNewFrame","int",    (void *) &frame->autoCreateNewFrame )
                       << AttrProcessing ( "newFrameBehaviour", "int",    (void *) &frame->newFrameBehaviour  )
                       << AttrProcessing ( "sheetSide",         "int",    (void *) &frame->sheetSide          );
    ProcessAttributes (myNode, attrProcessingList);

    AllowNoSubtags (myNode);
}   // end ProcessFrameTag


/***************************************************************************/

void ProcessFramesetTag ( QDomNode   myNode,
                          void      *tagData,
                          QString   &outputText )
{
    DocData *docData = (DocData *) tagData;

    int col (-1);
    int row (-1);
    int cols (-1);
    int rows (-1);
    QString grpMgr = "";
    int frameInfo = -1;
    Frame frame;
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "col",       "int",     (void *) &col       )
                       << AttrProcessing ( "row",       "int",     (void *) &row       )
                       << AttrProcessing ( "cols",      "int",     (void *) &cols      )
                       << AttrProcessing ( "rows",      "int",     (void *) &rows      )
                       << AttrProcessing ( "grpMgr",    "QString", (void *) &grpMgr    )
                       << AttrProcessing ( "frameType", "",        NULL                )
                       << AttrProcessing ( "frameInfo", "int",     (void *) &frameInfo )
                       << AttrProcessing ( "removable", "",        NULL                )
                       << AttrProcessing ( "visible",   "",        NULL                )
                       << AttrProcessing ( "name",      "",        NULL                );
    ProcessAttributes  (myNode, attrProcessingList);
    (*docData).frameInfo = frameInfo;
    (*docData).grpMgr = false;  // indicate no group manager
    if ( grpMgr.length () == 0 )  // no grpMgr, not a table
    {
        // process as a paragraph
        QValueList<TagProcessing> tagProcessingList;
        tagProcessingList << TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) docData )
                          << TagProcessing ( "FORMULA",   NULL,                NULL             )
                          << TagProcessing ( "IMAGE",     NULL,                NULL             )
                          << TagProcessing ( "FRAME",     ProcessFrameTag,     (void *) &frame  );
        ProcessSubtags (myNode, tagProcessingList, outputText);
    }
    else  // grpMgr indicates a table
    {
    (*docData).grpMgr = true;
        if ( col != -1 && row != -1 )
        {
            if ( cols == 1 && rows == 1 )
            {
#if 0
                kdError (KDEBUG_KWFILTER) << "Debug - FRAMESET: table " << grpMgr << " col, row = "
                                               << col << ", " << row << endl;
#endif

                // process cell text and formats
                QString cellText;

                QValueList<TagProcessing> tagProcessingList;
                tagProcessingList << TagProcessing ( "PARAGRAPH", ProcessParagraphTag, (void *) docData )
                                  << TagProcessing ( "FORMULA",   NULL,                NULL             )
                                  << TagProcessing ( "IMAGE",     NULL,                NULL             )
                                  << TagProcessing ( "FRAME",     ProcessFrameTag,     (void *) &frame   );
                ProcessSubtags ( myNode, tagProcessingList, cellText );

                AnchoredInsert *anchoredInsert = findAnchoredInsert ( AnchoredInsert ( Table (grpMgr),
                                                                                       outputText.length () ),
                                                                      docData->anchoredInsertList              );

                if ( anchoredInsert )  // add cell info to list
                {
                    anchoredInsert->table.addCell ( col, row, cellText,
                                                    leftBorder, rightBorder,
                                                    topBorder, bottomBorder, frame );
                }
                else
                {
                    kdError (KDEBUG_KWFILTER) << "Could find anchored insert for table "
                                                   << grpMgr << "!" << endl;
                }
            }
            else  // not if(cols == 1 && rows == 1)
            {
                kdError (KDEBUG_KWFILTER) << "Unexpected value for one of, or all FRAMESET attribute cols, rows: "
                                               << cols << ", " << rows << "!" << endl;
                AllowNoSubtags (myNode);
            }
        }
        else   // not if( col != -1 && row != -1)
        {
           kdError (KDEBUG_KWFILTER) << "Unset value for one of, or all FRAMESET attributes col, row: "
                                          << col << ", " << row << "!" << endl;
           AllowNoSubtags (myNode);
        }
    }
}   // end ProcessFramesetTag

/***************************************************************************/

void ProcessFramesetsTag ( QDomNode   myNode,
                           void      *tagData,
                           QString   &outputText  )
// FRAMESETS is a container tag for one or more framesets
{
    AllowNoAttributes (myNode);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "FRAMESET", ProcessFramesetTag, tagData );
    ProcessSubtags (myNode, tagProcessingList, outputText);

}  // end ProcessFramesetsTag

/***************************************************************************/

void ProcessPixmapsKeyTag ( QDomNode   myNode,
                            void      *tagData,
                            QString   &outputText )
{
    DocData  *docData = (DocData *) tagData;

    QString   key;
    QString   name;
    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "key",  "QString", (void *) &key  )
                       << AttrProcessing ( "name", "QString", (void *) &name );
    ProcessAttributes (myNode, attrProcessingList);

    AnchoredInsert *anchoredInsert = findAnchoredInsert ( AnchoredInsert ( Picture (key),
                                                                           outputText.length () ),
                                                          docData->anchoredInsertList              );

    if ( anchoredInsert )
    {
        anchoredInsert->picture.koStoreName = name;
    }
    else
    {
        kdError (KDEBUG_KWFILTER) << "Could find anchored insert for picture "
                                       << name << "!" << endl;
    }

    AllowNoSubtags (myNode);
}

/***************************************************************************/

void ProcessPixmapsTag ( QDomNode   myNode,
                         void      *tagData,
                         QString   &outputText )
{
    AllowNoAttributes (myNode);

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "KEY", ProcessPixmapsKeyTag, tagData );
    ProcessSubtags (myNode, tagProcessingList, outputText);
}

/***************************************************************************/

// The following function is called by the top level function
// filter. In turn it processes all the tags and subtags in
// the root document.


/***************************************************************************/

void ProcessParagraphTag ( QDomNode   myNode,
                           void      *tagData,
                           QString   &outputText )
{
    DocData  *docData = (DocData *) tagData;

    AllowNoAttributes (myNode);

    QString paraText;
    ParaLayout layout;
    QValueList<FormatData> paraFormatDataFormats;
    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "TEXT",    ProcessTextTag,    (void *) &paraText )
                      << TagProcessing ( "FORMATS", ProcessFormatsTag, (void *) &paraFormatDataFormats )
                      << TagProcessing ( "LAYOUT",  ProcessLayoutTag,  (void *) &layout   );
    ProcessSubtags (myNode, tagProcessingList, outputText);

ProcessParagraph ( paraText, paraFormatDataList, paraFormatDataFormats,
                   outputText, layout, docData );

}   // end ProcessParagraphTag()




/***************************************************************************/

void ProcessDocTag ( QDomNode   myNode,
                     void      */*tagData*/,
                     QString   &outputText )
{
    //FilterData *filterData = (FilterData *) tagData;

    QValueList<AttrProcessing> attrProcessingList;
    attrProcessingList << AttrProcessing ( "editor",        "", NULL )
                       << AttrProcessing ( "mime",          "", NULL )
                       << AttrProcessing ( "syntaxVersion", "", NULL );
    ProcessAttributes (myNode, attrProcessingList);

    DocData docData;   // hopefully also initializes docData.anchoredInsertList
    docData.article          = false;
    docData.head1            = false;
    docData.head2            = false;
    docData.head3            = false;
    docData.bulletList       = false;
    docData.enumeratedList   = false;
    docData.alphabeticalList = false;
    PaperAttributes paper;  // initialized by class initializer
    Attributes attributes; // initialized by class init

    QValueList<TagProcessing> tagProcessingList;
    tagProcessingList << TagProcessing ( "PAPER",        ProcessPaperTag,       (void *) &paper       )
                      << TagProcessing ( "ATTRIBUTES",   ProcessAttributesTag,  (void *) &attributes  )
                      << TagProcessing ( "PIXMAPS",      ProcessPixmapsTag,     (void *) &docData     )
                      << TagProcessing ( "STYLES",       NULL,                  NULL                  )
                      << TagProcessing ( "EMBEDDED",     NULL,                  NULL                  )
                      << TagProcessing ( "FRAMESETS",    ProcessFramesetsTag,   (void *) &docData     );
    ProcessSubtags (myNode, tagProcessingList, outputText);

    // Process the paper size and margins
    paperSize( paper, paperBorders );

    QValueList<AnchoredInsert>::Iterator anchoredInsert;

    for ( anchoredInsert = docData.anchoredInsertList.begin ();
          anchoredInsert != docData.anchoredInsertList.end ();
          anchoredInsert++ )
    {
        switch ( (*anchoredInsert).type )
        {
            case 2:
//                ProcessPictureData ( (*anchoredInsert).picture, (*anchoredInsert).pos, filterData->storeFileName, filterData->exportFileName, outputText );
                  ProcessPictureData ();
                 break;

            case 6:
                ProcessTableData ( (*anchoredInsert).table, (*anchoredInsert).pos, outputText );
                break;

            default:
                kdError (KDEBUG_KWFILTER) << "Unhandled anchored insert type "
                                               << (*anchoredInsert).type << "!" << endl;
        }
    }

} // end ProcessDocTag()

