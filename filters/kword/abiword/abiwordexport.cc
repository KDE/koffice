// $Header$

/* This file is part of the KDE project
   Copyright (C) 2001, 2002 Nicolas GOUTTE <nicog@snafu.de>

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

/*
   This file is based on the old file:
    /home/kde/koffice/filters/kword/ascii/asciiexport.cc

   The old file was copyrighted by
    Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
    Copyright (c) 2000 ID-PRO Deutschland GmbH. All rights reserved.
                       Contact: Wolf-Michael Bolle <Bolle@ID-PRO.de>

   The old file was licensed under the terms of the GNU Library General Public
   License version 2.
*/

#include <qmap.h>
#include <qregexp.h>
#include <qiodevice.h>
#include <qbuffer.h>
#include <qimage.h>
#include <qpicture.h>
#include <qtextstream.h>
#include <qdom.h>

#include <kdebug.h>
#include <kmdcodec.h>
#include <kfilterdev.h>
#include <kgenericfactory.h>
#include <kimageio.h>

#include <koGlobal.h>
#include <koFilterChain.h>
#include <koPictureKey.h>

#include <KWEFStructures.h>
#include <KWEFUtil.h>
#include <KWEFBaseWorker.h>
#include <KWEFKWordLeader.h>

#include <abiwordexport.h>
#include <abiwordexport.moc>

typedef KGenericFactory<ABIWORDExport, KoFilter> ABIWORDExportFactory;
K_EXPORT_COMPONENT_FACTORY( libabiwordexport, ABIWORDExportFactory( "kwordabiwordexport" ) );

class StyleMap : public QMap<QString,LayoutData>
{
public:
    StyleMap(void) {}
    ~StyleMap(void) {}
};

class AbiWordWorker : public KWEFBaseWorker
{
public:
    AbiWordWorker(void);
    virtual ~AbiWordWorker(void) { delete m_streamOut;}
public:
    virtual bool doOpenFile(const QString& filenameOut, const QString& to);
    virtual bool doCloseFile(void); // Close file in normal conditions
    virtual bool doOpenDocument(void);
    virtual bool doCloseDocument(void);
    virtual bool doFullParagraph(const QString& paraText, const LayoutData& layout,
        const ValueListFormatData& paraFormatDataList);
    virtual bool doOpenTextFrameSet(void); // AbiWord's <section>
    virtual bool doCloseTextFrameSet(void); // AbiWord's </section>
    virtual bool doFullPaperFormat(const int format,
        const double width, const double height, const int orientation); // Calc AbiWord's <papersize>
    virtual bool doFullPaperBorders (const double top, const double left,
        const double bottom, const double right); // Like KWord's <PAPERBORDERS>
    virtual bool doCloseHead(void); // Write <papersize>
    virtual bool doOpenStyles(void); // AbiWord's <styles>
    virtual bool doCloseStyles(void); // AbiWord's </styles>
    virtual bool doFullDefineStyle(LayoutData& layout); // AbiWord's <s></s>
    virtual bool doOpenSpellCheckIgnoreList (void); // AbiWord's <ignorewords>
    virtual bool doCloseSpellCheckIgnoreList (void); // AbiWord's </ignorewords>
    virtual bool doFullSpellCheckIgnoreWord (const QString& ignoreword); // AbiWord's <iw>
private:
    void processParagraphData (const QString& paraText,
        const TextFormatting& formatLayout,
        const ValueListFormatData& paraFormatDataList);
    void processNormalText ( const QString& paraText,
        const TextFormatting& formatLayout,
        const FormatData& formatData);
    void processVariable ( const QString& paraText,
        const TextFormatting& formatLayout,
        const FormatData& formatData);
    void processAnchor ( const QString& paraText,
        const TextFormatting& formatLayout,
        const FormatData& formatData);
    QString textFormatToAbiProps(const TextFormatting& formatOrigin,
        const TextFormatting& formatData, const bool force) const;
    QString layoutToCss(const LayoutData& layoutOrigin,
        const LayoutData& layout, const bool force) const;
    QString escapeAbiWordText(const QString& strText) const;
    bool makeImage(const FrameAnchor& anchor, const bool isImage);
    bool convertUnknownImage(const QString& name, QByteArray& image);
    void writeAbiProps(const TextFormatting& formatLayout, const TextFormatting& format);
    void writeImageData(const QString& koStoreName, const QString& keyName);
    void writeClipartData(const QString& koStoreName, const QString& keyName);
    void writeSvgData(const QString& koStoreName, const QString& keyName);
    void writePictureData(const QString& koStoreName, const QString& keyName);

private:
    QIODevice* m_ioDevice;
    QTextStream* m_streamOut;
    QString m_pagesize; // Buffer for the <pagesize> tag
    QMap<QString,KoPictureKey> m_mapPictureData;
    StyleMap m_styleMap;
    double m_paperBorderTop,m_paperBorderLeft,m_paperBorderBottom,m_paperBorderRight;
    bool m_inIgnoreWords; // true if <ignorewords> has been written
};

AbiWordWorker::AbiWordWorker(void) : m_ioDevice(NULL), m_streamOut(NULL),
    m_paperBorderTop(0.0),m_paperBorderLeft(0.0),
    m_paperBorderBottom(0.0),m_paperBorderRight(0.0)
{
}

QString AbiWordWorker::escapeAbiWordText(const QString& strText) const
{
    // Escape quotes (needed in attributes)
    // Escape apostrophs (allowed by XML)
    return KWEFUtil::EscapeSgmlText(NULL,strText,true,true);
}

bool AbiWordWorker::doOpenFile(const QString& filenameOut, const QString& )
{
    kdDebug(30506) << "Opening file: " << filenameOut
        << " (in AbiWordWorker::doOpenFile)" << endl;
    //Find the last extension
    QString strExt;
    const int result=filenameOut.findRev('.');
    if (result>=0)
    {
        strExt=filenameOut.mid(result);
    }

    QString strMimeType; // Mime type of the compressor

    if ((strExt==".gz")||(strExt==".GZ")        //in case of .abw.gz (logical extension)
        ||(strExt==".zabw")||(strExt==".ZABW")) //in case of .zabw (extension used prioritary with AbiWord)
    {
        // Compressed with gzip
        strMimeType="application/x-gzip";
    }
    else if ((strExt==".bz2")||(strExt==".BZ2") //in case of .abw.bz2 (logical extension)
        ||(strExt==".bzabw")||(strExt==".BZABW")) //in case of .bzabw (extension used prioritary with AbiWord)
    {
        // Compressed with bzip2

        // It seems that bzip2-compressed AbiWord files were planned
        //   but AbiWord CVS 2001-12-15 does not have import and export filters for them anymore.
        //   We leave this code but leave the .desktop file without bzip2 files
        strMimeType="application/x-bzip2";
    }
    else
    {
        // No compression
        strMimeType="text/plain";
    }

    kdDebug(30506) << "Compression: " << strMimeType << endl;

    m_ioDevice = KFilterDev::deviceForFile(filenameOut,strMimeType);

    if (!m_ioDevice)
    {
        kdError(30506) << "No output file! Aborting!" << endl;
        return false;
    }

    if ( !m_ioDevice->open (IO_WriteOnly) )
    {
        kdError(30506) << "Unable to open output file! Aborting!" << endl;
        return false;
    }

    m_streamOut=new QTextStream(m_ioDevice);
    if (!m_ioDevice)
    {
        kdError(30506) << "Could not create output stream! Aborting!" << endl;
        m_ioDevice->close();
        return false;
    }

    // We only export in UTF-8 (are there AbiWord ports that cannot read UTF-8? Be careful SVG uses UTF-8 too!)
    m_streamOut->setEncoding( QTextStream::UnicodeUTF8 );
    return true;
}

bool AbiWordWorker::doCloseFile(void)
{
    if (m_ioDevice)
        m_ioDevice->close();
    return (m_ioDevice);
}

bool AbiWordWorker::doOpenDocument(void)
{
    // Make the file header

    // First the XML header in UTF-8 version
    // (AbiWord and QT handle UTF-8 well, so we stay with this encoding!)
    *m_streamOut << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

    // NOTE: AbiWord CVS 2002-02-?? has a new DOCTYPE
    *m_streamOut << "<!DOCTYPE abiword PUBLIC \"-//ABISOURCE//DTD AWML 1.0 Strict//EN\"";
    *m_streamOut << " \"http://www.abisource.com/awml.dtd\">\n";

    // First magic: "<abiword"
    *m_streamOut << "<abiword";
    // AbiWord CVS 2002-02-23 defines a default namespace.
    *m_streamOut << " xmlns=\"http://www.abisource.com/awml.dtd\"";
    // As we do not use xmlns:awml, do we need to define it?
    // *m_streamOut << " xmlns:awml=\"http://www.abisource.com/awml.dtd\"";
    *m_streamOut << " xmlns:xlink=\"http://www.w3.org/1999/xlink\"";
    // AbiWord CVS 2002-02-22 defines other namesapces, which we are not using.
    *m_streamOut << " version=\"\" fileformat=\"1.0\" styles=\"unlocked\">\n";
    // Second magic: "<!-- This file is an AbiWord document."
    // TODO/FIXME: write as much spaces as AbiWord does for the following line.
    *m_streamOut << "<!-- This file is an AbiWord document. -->\n";
    // We have chosen NOT to have the full comment header that AbiWord files normally have.
    *m_streamOut << "\n";

    // Put the rest of the information in the way AbiWord puts its debug info!

    // Say who we are (with the CVS revision number) in case we have a bug in our filter output!
    *m_streamOut << "<!-- KWord_Export_Filter_Version =";

    QString strVersion("$Revision$");
    // Remove the dollar signs
    //  (We don't want that the version number changes if the AbiWord file is itself put in a CVS storage.)
    *m_streamOut << strVersion.mid(10).replace(QRegExp("\\$"),""); // Note: double escape character (one for C++, one for QRegExp!)

    *m_streamOut << " -->\n\n";

    return true;
}

bool AbiWordWorker::convertUnknownImage(const QString& strName, QByteArray& image)
{
    QIODevice* io=getSubFileDevice(strName);
    if (!io)
    {
        // NO message error, as there must be already one
        return false;
    }

    kdDebug(30506) << "Image " << strName << " has size: " << io->size() << endl;

    QImageIO imageIO(io,NULL);

    if (!imageIO.read())
    {
        kdWarning(30506) << "Could not read image: " << strName << " (AbiWordWorker::convertUnknownImage)" << endl;
        return false;
    }

    kdDebug(30506) << "Image: " << strName << " (AbiWordWorker::convertUnknownImage)" << endl;

    QBuffer buffer(image); // A QBuffer is a QIODevice
    if (!buffer.open(IO_WriteOnly))
    {
        kdWarning(30506) << "Could not open buffer! (AbiWordWorker::convertUnknownImage)" << endl;
        return false;
    }

    imageIO.setIODevice(&buffer);
    imageIO.setFormat("PNG");

    if (!imageIO.write())
    {
        kdWarning(30506) << "Could not write converted image! (AbiWordWorker::convertUnknownImage)" << endl;
        return false;
    }
    buffer.close();

    return true;
}

void AbiWordWorker::writeImageData(const QString& koStoreName, const QString& keyName)
{
    kdDebug(30506) << "Picture is Image " << endl;

    QByteArray image;

    QString strExtension(koStoreName);
    const int result=koStoreName.findRev(".");
    if (result>=0)
    {
        strExtension=koStoreName.mid(result+1);
    }

    bool isImageLoaded=false;

    QString strMime;

    if (strExtension=="png")
    {
        strMime="image/png";
        isImageLoaded=loadKoStoreFile(koStoreName,image);
    }
    else
    {
        // All other image types must be converted to PNG
        //   (yes, even JPEG!)
        strMime="image/png";
        isImageLoaded=convertUnknownImage(koStoreName,image);
    }

    if (isImageLoaded)
    {
        *m_streamOut << "<d name=\"" << keyName << "\""
            << " base64=\"yes\""
            << " mime=\"" << strMime << "\">\n";

        QCString base64=KCodecs::base64Encode(image,true);

        *m_streamOut << base64 << "\n"; // QCString is taken as Latin1 by QTextStream

        *m_streamOut << "</d>\n";
    }
    else
    {
        kdWarning(30506) << "Unable to load image: " << koStoreName << endl;
    }
}

void AbiWordWorker::writeClipartData(const QString& koStoreName, const QString& keyName)
{
    // We must always convert the image to SVG
    QString strMime="image/svg-xml"; // Yes, it is -xml not +xml

    kdDebug(30506) << "Picture is QPicture " << koStoreName << endl;

    QPicture picture;

    QIODevice* io=getSubFileDevice(koStoreName);
    if (!io)
    {
        // NO message error, as there must be already one
        return;
    }

    if (picture.load(io,NULL))
    {
        *m_streamOut << "<d name=\"" << keyName << "\""
            << " base64=\"no\""
            << " mime=\"" << strMime << "\">\n"
            << "<![CDATA["; // Open CDATA section
        // Do not add a new line after <![CDATA[ (No white space before a XML declaration)
        // TODO/FIXME. aarrgghh, we cannot have a XML declaration at all!

        // Save picture as SVG

        // Not sure what is better: call the QIODevice directly or through QTextStream::device()?
        // QPicture::save saves in UTF-8, so we have no problem!
        // TODO/FIXME. aarrgghh, we cannot have a XML declaration at all!
        if (!picture.save(m_streamOut->device(),"svg"))
        {
            kdWarning(30506) << "Could not save clipart: "  << koStoreName << endl;
        }

        *m_streamOut << "]]>\n" // Close CDATA section
            << "</d>\n";
    }
    else
    {
        kdWarning(30506) << "Unable to load clipart: " << koStoreName << endl;
    }
}

void AbiWordWorker::writeSvgData(const QString& koStoreName, const QString& keyName)
{
    QByteArray svg;
    QString strMime="image/svg-xml"; // Yes, it is -xml not +xml

    kdDebug(30506) << "Picture is SVG" << endl;

    if (!loadKoStoreFile(koStoreName,svg))
    {
        // NO message error, as there must be already one
        return;
    }

    QDomDocument svgDoc;
    if (!svgDoc.setContent(svg))
    {
        kdWarning(30506) << "Parsing of SVG failed..." << endl;
        return; // TODO: full error line/column/message
    }

    *m_streamOut << "<d name=\"" << keyName << "\""
        << " base64=\"no\""
        << " mime=\"" << strMime << "\">\n"
        << "<![CDATA["; // Open CDATA section

    // Do not add a new line after <![CDATA[ (No white space before a XML declaration)
    // TODO/FIXME. aarrgghh, we cannot have a XML declaration at all!
    *m_streamOut << svgDoc;

    *m_streamOut << "]]>\n" // Close CDATA section
        << "</d>\n";
}

void AbiWordWorker::writePictureData(const QString& koStoreName, const QString& keyName)
{
    kdDebug(30506) << "Picture " << koStoreName << endl;
    QString strExtension(koStoreName);
    const int result=koStoreName.findRev(".");
    if (result>=0)
    {
        strExtension=koStoreName.mid(result+1).lower();
    }

    if (strExtension=="wmf")
    {
        kdDebug(30506) << "Type " << strExtension << " not supported!" << endl;
    }
    else if (strExtension=="svg")
    {
        writeSvgData(koStoreName,keyName);
    }
    else if (strExtension=="qpic")
    {
        writeClipartData(koStoreName,keyName);
    }
    else
    {
        writeImageData(koStoreName,keyName);
    }
}


bool AbiWordWorker::doCloseDocument(void)
{
    // Before writing the <data> element,
    //  we must be sure that we have data and that we can retrieve it.

    if (m_kwordLeader && !m_mapPictureData.isEmpty())
    {
        *m_streamOut << "<data>\n";

        QMap<QString,KoPictureKey>::ConstIterator it;

        // all images first
        for (it=m_mapPictureData.begin(); it!=m_mapPictureData.end(); it++)
        {
            // Warning: do not mix up KWord's key and the iterator's key!
            writeImageData(it.key(),it.data().filename());
            writeClipartData(it.key(),it.data().filename());
        }

        *m_streamOut << "</data>\n";
    }

    *m_streamOut << "</abiword>\n"; //Close the file for XML
    return true;
}

bool AbiWordWorker::doOpenTextFrameSet(void)
{
    *m_streamOut << "<section props=\"";
    *m_streamOut << "page-margin-top: ";
    *m_streamOut << m_paperBorderTop;
    *m_streamOut << "pt; ";
    *m_streamOut << "page-margin-left: ";
    *m_streamOut << m_paperBorderLeft;
    *m_streamOut << "pt; ";
    *m_streamOut << "page-margin-bottom: ";
    *m_streamOut << m_paperBorderBottom;
    *m_streamOut << "pt; ";
    *m_streamOut << "page-margin-right: ";
    *m_streamOut << m_paperBorderRight;
    *m_streamOut << "pt"; // Last one, so no semi-comma
    *m_streamOut << "\">\n";
    return true;
}

bool AbiWordWorker::doCloseTextFrameSet(void)
{
    *m_streamOut << "</section>\n";
    return true;
}

bool AbiWordWorker::doOpenStyles(void)
{
    *m_streamOut << "<styles>\n";
    return true;
}

bool AbiWordWorker::doCloseStyles(void)
{
    *m_streamOut << "</styles>\n";
    return true;
}

QString AbiWordWorker::textFormatToAbiProps(const TextFormatting& formatOrigin,
    const TextFormatting& formatData, const bool force) const
{
    // TODO: rename variable formatData
    QString strElement; // TODO: rename this variable

    // Font name
    QString fontName = formatData.fontName;
    if ( !fontName.isEmpty()
        && (force || (formatOrigin.fontName!=formatData.fontName)))
    {
        strElement+="font-family: ";
        strElement+=fontName; // TODO: add alternative font names
        strElement+="; ";
    }

    if (force || (formatOrigin.italic!=formatData.italic))
    {
        // Font style
        strElement+="font-style: ";
        if ( formatData.italic )
        {
            strElement+="italic";
        }
        else
        {
            strElement+="normal";
        }
        strElement+="; ";
    }

    if (force || ((formatOrigin.weight>=75)!=(formatData.weight>=75)))
    {
        strElement+="font-weight: ";
        if ( formatData.weight >= 75 )
        {
            strElement+="bold";
        }
        else
        {
            strElement+="normal";
        }
        strElement+="; ";
    }

    if (force || (formatOrigin.fontSize!=formatData.fontSize))
    {
        const int size=formatData.fontSize;
        if (size>0)
        {
            // We use absolute font sizes.
            strElement+="font-size: ";
            strElement+=QString::number(size,10);
            strElement+="pt; ";
        }
    }

    if (force || (formatOrigin.fgColor!=formatData.fgColor))
    {
        if ( formatData.fgColor.isValid() )
        {
            // Give colour
            strElement+="color: ";

            // No leading # (unlike CSS2)
            // We must have two hex digits for each colour channel!
            const int red=formatData.fgColor.red();
            strElement += QString::number((red&0xf0)>>4,16);
            strElement += QString::number(red&0x0f,16);

            const int green=formatData.fgColor.green();
            strElement += QString::number((green&0xf0)>>4,16);
            strElement += QString::number(green&0x0f,16);

            const int blue=formatData.fgColor.blue();
            strElement += QString::number((blue&0xf0)>>4,16);
            strElement += QString::number(blue&0x0f,16);

            strElement+="; ";
        }
    }

    if (force || (formatOrigin.bgColor!=formatData.bgColor))
    {
        if ( formatData.bgColor.isValid() )
        {
            // Give background colour
            strElement+="bgcolor: ";

            // No leading # (unlike CSS2)
            // We must have two hex digits for each colour channel!
            const int red=formatData.bgColor.red();
            strElement += QString::number((red&0xf0)>>4,16);
            strElement += QString::number(red&0x0f,16);

            const int green=formatData.bgColor.green();
            strElement += QString::number((green&0xf0)>>4,16);
            strElement += QString::number(green&0x0f,16);

            const int blue=formatData.bgColor.blue();
            strElement += QString::number((blue&0xf0)>>4,16);
            strElement += QString::number(blue&0x0f,16);

            strElement+="; ";
        }
    }

    if (force || (formatOrigin.underline!=formatData.underline)
        || (formatOrigin.strikeout!=formatData.strikeout))
    {
        strElement+="text-decoration: ";
        if ( formatData.underline )
        {
            strElement+="underline";
        }
        else if ( formatData.strikeout )
        {
            strElement+="line-through";
        }
        else
        {
            strElement+="none";
        }
        strElement+="; ";
    }

    return strElement;
}

bool AbiWordWorker::makeImage(const FrameAnchor& anchor, const bool isImage)
{
    kdDebug(30506) << "New image/clipart: " << anchor.picture.koStoreName
        << " , " << anchor.picture.key.toString() << " (is image:" << isImage << ")" <<endl;

    const double height=anchor.bottom - anchor.top;
    const double width =anchor.right  - anchor.left;

    // TODO: we are only using the filename, not the rest of the key
    // TODO:  (bad if they are two images of the same name, but of different key)
    *m_streamOut << "<image dataid=\"" << anchor.picture.key.filename() << "\"";
    *m_streamOut << " props= \"height:" << height << "pt;width:" << width << "pt\"";
    *m_streamOut << "/>"; // NO end of line!
    // TODO: other props for image

    m_mapPictureData[anchor.picture.koStoreName]=anchor.picture.key;

    return true;
}

void AbiWordWorker::writeAbiProps (const TextFormatting& formatLayout, const TextFormatting& format)
{
    QString abiprops=textFormatToAbiProps(formatLayout,format,false);

    // Erase the last semi-comma (as in CSS2, semi-commas only separate instructions and do not terminate them)
    const int result=abiprops.findRev(";");

    if (result>=0)
    {
        // Remove the last semi-comma and the space thereafter
        abiprops.remove(result,2);
    }

    if (!abiprops.isEmpty())
    {
        *m_streamOut << " props=\"" << abiprops << "\"";
    }
}

void AbiWordWorker::processNormalText ( const QString &paraText,
    const TextFormatting& formatLayout,
    const FormatData& formatData)
{
    // Retrieve text and escape it
    QString partialText=escapeAbiWordText(paraText.mid(formatData.pos,formatData.len));

    // Replace line feeds by line breaks
    int pos;
    while ((pos=partialText.find(QChar(10)))>-1)
    {
        partialText.replace(pos,1,"<br/>");
    }

    if (formatData.text.missing)
    {
        // It's just normal text, so we do not need a <c> element!
        *m_streamOut << partialText;
    }
    else
    { // Text with properties, so use a <c> element!
        *m_streamOut << "<c";
        writeAbiProps(formatLayout,formatData.text);
        *m_streamOut << ">" << partialText << "</c>";
    }
}

void AbiWordWorker::processVariable ( const QString&,
    const TextFormatting& formatLayout,
    const FormatData& formatData)
{
    if (0==formatData.variable.m_type)
    {
        // As AbiWord's field is inflexible, we cannot make the date custom
        *m_streamOut << "<field type=\"date_ntdfl\"";
        writeAbiProps(formatLayout,formatData.text);
        *m_streamOut << "/>";
    }
    else if (2==formatData.variable.m_type)
    {
        // As AbiWord's field is inflexible, we cannot make the time custom
        *m_streamOut << "<field type=\"time\"";
        writeAbiProps(formatLayout,formatData.text);
        *m_streamOut << "/>";
    }
    else if (4==formatData.variable.m_type)
    {
        // As AbiWord's field is inflexible, we cannot make the time custom
        QString strFieldType;
        if (formatData.variable.isPageNumber())
        {
            strFieldType="page_number";
        }
        else if (formatData.variable.isPageCount())
        {
            strFieldType="page_count";
        }
        if (strFieldType.isEmpty())
        {
            // Unknown subtype, therefore write out the result
            *m_streamOut << formatData.variable.m_text;
        }
        else
        {
            *m_streamOut << "<field type=\"" << strFieldType <<"\"";
            writeAbiProps(formatLayout,formatData.text);
            *m_streamOut << "/>";
        }
    }
    else if (9==formatData.variable.m_type)
    {
        // A link
        *m_streamOut << "<a xlink:href=\""
            << escapeAbiWordText(formatData.variable.getHrefName())
            << "\"><c";  // In AbiWord, an anchor <a> has always a <c> child
        writeAbiProps(formatLayout,formatData.text);
        *m_streamOut << ">"
            << escapeAbiWordText(formatData.variable.getLinkName())
            << "</c></a>";
    }
    else
    {
        // Generic variable
        *m_streamOut << formatData.variable.m_text;
    }
}

void AbiWordWorker::processAnchor ( const QString&,
    const TextFormatting& /*formatLayout*/, //TODO
    const FormatData& formatData)
{
    // We have an image or a table
    // However, AbiWord does not support tables
    if (2==formatData.frameAnchor.type)
    {   // <IMAGE>
        makeImage(formatData.frameAnchor,true);
    }
    else if (5==formatData.frameAnchor.type)
    {   // <CLIPART>
        makeImage(formatData.frameAnchor,false);
    }
    else
    {
        kdWarning(30506) << "Unsupported anchor type: "
            << formatData.frameAnchor.type << endl;
    }
}

void AbiWordWorker::processParagraphData ( const QString &paraText,
    const TextFormatting& formatLayout,
    const ValueListFormatData &paraFormatDataList)
{
    if ( paraText.length () > 0 )
    {
        ValueListFormatData::ConstIterator  paraFormatDataIt;

        for ( paraFormatDataIt = paraFormatDataList.begin ();
              paraFormatDataIt != paraFormatDataList.end ();
              paraFormatDataIt++ )
        {
            if (1==(*paraFormatDataIt).id)
            {
                processNormalText(paraText, formatLayout, (*paraFormatDataIt));
            }
            else if (4==(*paraFormatDataIt).id)
            {
                processVariable(paraText, formatLayout, (*paraFormatDataIt));
            }
            else if (6==(*paraFormatDataIt).id)
            {
                processAnchor(paraText, formatLayout, (*paraFormatDataIt));
            }
        }
    }
}

QString AbiWordWorker::layoutToCss(const LayoutData& layoutOrigin,
    const LayoutData& layout, const bool force) const
{
    QString props;

    if (force || (layoutOrigin.alignment!=layout.alignment))
    {
        // Check if the current alignment is a valid one for AbiWord.
        if ((layout.alignment == "left") || (layout.alignment == "right")
            || (layout.alignment == "center")  || (layout.alignment == "justify"))
        {
            props += "text-align:";
            props += layout.alignment;
            props += "; ";
        }
        else if (layout.alignment == "auto")
        {
            // We assume a left alignment as AbiWord is not really bi-di (and this filter even less.)
            props += "text-align:left; ";
        }
        else
        {
            kdWarning(30506) << "Unknown alignment: " << layout.alignment << endl;
        }
    }

    // TODO/FIXME: what if all tabulators must be erased?
#if 0
    // DEPRECATED!
    if (!layout.tabulator.isEmpty()
        && (force || (layoutOrigin.tabulator!=layout.tabulator)))
    {
        props += "tabstops:";
        props += layout.tabulator;
        props += "; ";
    }
#endif
    if (!layout.tabulatorList.isEmpty()
        && (force || (layoutOrigin.tabulatorList!=layout.tabulatorList) ))
    {
        props += "tabstops:";
        bool first=true;
        TabulatorList::ConstIterator it;
        for (it=layout.tabulatorList.begin();it!=layout.tabulatorList.end();it++)
        {
            if (first)
            {
                first=false;
            }
            else
            {
                props += ",";
            }
            props += QString::number((*it).m_ptpos);
            props += "pt";

            switch ((*it).m_type)
            {
                case 0:  props += "/L"; break;
                case 1:  props += "/C"; break;
                case 2:  props += "/R"; break;
                case 3:  props += "/D"; break;
                default: props += "/L";
            }

            props += "0"; // No filling
        }
        props += "; ";
    }

    if ((layout.indentLeft>=0.0)
        && (force || (layoutOrigin.indentLeft!=layout.indentLeft)))
    {
        props += QString("margin-left:%1pt; ").arg(layout.indentLeft);
    }

    if ((layout.indentRight>=0.0)
        && (force || (layoutOrigin.indentRight!=layout.indentRight)))
    {
        props += QString("margin-right:%1pt; ").arg(layout.indentRight);
    }

    if (force || (layoutOrigin.indentLeft!=layout.indentLeft))
    {
        props += "text-indent: ";
        props += QString::number(layout.indentFirst);
        props += "; ";
    }

    if ((layout.marginBottom>=0.0)
        && (force || (layoutOrigin.indentRight!=layout.indentRight)))
    {
       props += QString("margin-bottom:%1pt; ").arg(layout.marginBottom);
    }

    if ((layout.marginTop>=0.0)
        && (force || (layoutOrigin.indentRight!=layout.indentRight)))
    {
       props += QString("margin-top:%1pt; ").arg(layout.marginTop);
    }

    if (!force
        && (layoutOrigin.lineSpacingType==layoutOrigin.lineSpacingType)
        && (layoutOrigin.lineSpacing==layoutOrigin.lineSpacing))
    {
        // Do nothing!
    }
    else if (!layout.lineSpacingType)
    {
        // We have a custom line spacing (in points)
        props += QString("line-height:%1pt; ").arg(layout.lineSpacing);
    }
    else if ( 15==layout.lineSpacingType  )
    {
        props += "line-height:1.5; "; // One-and-half
    }
    else if ( 20==layout.lineSpacingType  )
    {
        props += "line-height:2.0; "; // Two
    }
    else if ( layout.lineSpacingType!=10  )
    {
        kdWarning(30506) << "Curious lineSpacingType: " << layout.lineSpacingType << " (Ignoring!)" << endl;
    }

    // Add all AbiWord properties collected in the <FORMAT> element
    props += textFormatToAbiProps(layoutOrigin.formatData.text,layout.formatData.text,force);

    return props;
}

bool AbiWordWorker::doFullParagraph(const QString& paraText, const LayoutData& layout,
    const ValueListFormatData& paraFormatDataList)
{
    QString style=layout.styleName;

    const LayoutData& styleLayout=m_styleMap[style];

    QString props=layoutToCss(styleLayout,layout,false);

    *m_streamOut << "<p";
    if (!style.isEmpty())
    {
        *m_streamOut << " style=\"" << EscapeXmlText(style,true,true) << "\"";
    }
    if (!props.isEmpty())
    {
        // Find the last semi-comma
        // Note: as in CSS2, semi-commas only separates instructions (like in PASCAL) and do not terminate them (like in C)
        const int result=props.findRev(";");
        if (result>=0)
        {
            // Remove the last semi-comma and the space thereafter
            props.remove(result,2);
        }

        *m_streamOut << " props=\"" << props << "\"";
    }
    *m_streamOut << ">";  //Warning: No trailing white space or else it's in the text!!!

    // Before processing the text, test if we have a page break
    if (layout.pageBreakBefore)
    {
        // We have a page break before the paragraph
        *m_streamOut << "<pbr/>";
    }

    processParagraphData(paraText, layout.formatData.text, paraFormatDataList);

    // Before closing the paragraph, test if we have a page break
    if (layout.pageBreakAfter)
    {
        // We have a page break after the paragraph
        *m_streamOut << "<pbr/>";
    }

    *m_streamOut << "</p>\n";
    return true;
}

bool AbiWordWorker::doFullDefineStyle(LayoutData& layout)
{
    //Register style in the style map
    m_styleMap[layout.styleName]=layout;

    *m_streamOut << "<s";

    // TODO: cook the style name to the standard style names in AbiWord
    *m_streamOut << " name=\"" << EscapeXmlText(layout.styleName,true,true) << "\"";
    *m_streamOut << " followedby=\"" << EscapeXmlText(layout.styleFollowing,true,true) << "\"";

    if ( (layout.counter.numbering == CounterData::NUM_CHAPTER)
        && (layout.counter.depth<10) )
    {
        *m_streamOut << " level=\"";
        *m_streamOut << QString::number(layout.counter.depth+1,10);
        *m_streamOut << "\"";
    }

    QString abiprops=layoutToCss(layout,layout,true);

    const int result=abiprops.findRev(";");
    if (result>=0)
    {
        // Remove the last semi-comma and the space thereafter
        abiprops.remove(result,2);
    }

    *m_streamOut << " props=\"" << abiprops << "\"";

    *m_streamOut << "/>\n";

    return true;
}

bool AbiWordWorker::doFullPaperFormat(const int format,
            const double width, const double height, const int orientation)
{
    QString outputText = "<pagesize ";

    switch (format)
    {
        // ISO A formats
        case PG_DIN_A0: // ISO A0
        case PG_DIN_A1: // ISO A1
        case PG_DIN_A2: // ISO A2
        case PG_DIN_A3: // ISO A3
        case PG_DIN_A4: // ISO A4
        case PG_DIN_A5: // ISO A5
        case PG_DIN_A6: // ISO A6
        // ISO B formats
        case PG_DIN_B0: // ISO B0
        case PG_DIN_B1: // ISO B1
        case PG_DIN_B2: // ISO B2
        case PG_DIN_B3: // ISO B3
        case PG_DIN_B4: // ISO B4
        case PG_DIN_B5: // ISO B5
        case PG_DIN_B6: // ISO B6
        // American formats
        case PG_US_LETTER: // US Letter
        case PG_US_LEGAL:  // US Legal
        {
            QString pagetype=KoPageFormat::formatString(KoFormat(format));
            outputText+="pagetype=\"";
            outputText+=pagetype;

            QString strWidth, strHeight, strUnits;
            KWEFUtil::GetNativePaperFormat(format, strWidth, strHeight, strUnits);
            outputText+="\" width=\"";
            outputText+=strWidth;
            outputText+="\" height=\"";
            outputText+=strHeight;
            outputText+="\" units=\"";
            outputText+=strUnits;
            outputText+="\" ";
            break;
        }
        case PG_US_EXECUTIVE: // US Executive (does not exists in AbiWord!)
        {
            // FIXME/TODO: AbiWord (CVS 2001-04-25) seems not to like custom formats, so avoid them for now!
#if 0
            outputText += "pagetype=\"Custom\" width=\"7.5\" height=\"10.0\" units=\"inch\" ";
#else
            // As replacement, use the slightly bigger "letter" format.
            outputText += "pagetype=\"Letter\" width=\"8.5\" height=\"11.0\" units=\"inch\" ";
#endif
            break;
        }
        // Other format not supported yet by AbiWord CVS 2001-04-25)
        case PG_DIN_A7: // ISO A7
        case PG_DIN_A8: // ISO A8
        case PG_DIN_A9: // ISO A9
        case PG_DIN_B10: // ISO B10
        // Other formats
        case PG_SCREEN: // Screen
        case PG_CUSTOM: // Custom
        default:
        {
             // FIXME/TODO: AbiWord (CVS 2001-04-25) seems not to like custom formats, so avoid them for now!
            if ((width<=1.0) || (height<=1.0) || true)
            {
                // Height or width is ridiculous, so assume A4 format
                outputText += "pagetype=\"A4\" width=\"21.0\" height=\"29.7\" units=\"cm\" ";
            }
            else
            {   // We prefer to use inches, as to limit rounding errors (page size is in points!)
                outputText += QString("pagetype=\"Custom\" width=\"%1\" height=\"%2\" units=\"inch\" ").arg(width/72.0).arg(height/72.0);
            }
            break;
        }
    }

    outputText += "orientation=\"";
    if (1==orientation)
    {
        outputText += "landscape";
    }
    else
    {
        outputText += "portrait";
    }
    outputText += "\" ";

    outputText += "page-scale=\"1.0\"/>\n"; // KWord has no page scale, so assume 100%

    m_pagesize=outputText;
    return true;
}

bool AbiWordWorker::doFullPaperBorders (const double top, const double left,
    const double bottom, const double right)
{
    m_paperBorderTop=top;
    m_paperBorderLeft=left;
    m_paperBorderBottom=bottom;
    m_paperBorderRight=right;
    return true;
}

bool AbiWordWorker::doCloseHead(void)
{
    if (!m_pagesize.isEmpty())
    {
        *m_streamOut << m_pagesize;
    }
    return true;
}

bool AbiWordWorker::doOpenSpellCheckIgnoreList (void)
{
    kdDebug(30506) << "AbiWordWorker::doOpenSpellCheckIgnoreList" << endl;
    m_inIgnoreWords=false; // reset
    return true;
}

bool AbiWordWorker::doCloseSpellCheckIgnoreList (void)
{
    kdDebug(30506) << "AbiWordWorker::doCloseSpellCheckIgnoreList" << endl;
    if (m_inIgnoreWords)
        *m_streamOut << "</ignorewords>\n";
    return true;
}

bool AbiWordWorker::doFullSpellCheckIgnoreWord (const QString& ignoreword)
{
    kdDebug(30506) << "AbiWordWorker::doFullSpellCheckIgnoreWord: " << ignoreword << endl;
    if (!m_inIgnoreWords)
    {
        *m_streamOut << "<ignorewords>\n";
        m_inIgnoreWords=true;
    }
    *m_streamOut << " <iw>" << ignoreword << "</iw>\n";
    return true;
}


ABIWORDExport::ABIWORDExport(KoFilter */*parent*/, const char */*name*/, const QStringList &) :
                     KoFilter() {
}

KoFilter::ConversionStatus ABIWORDExport::convert( const QCString& from, const QCString& to )
{
    if ( to != "application/x-abiword" || from != "application/x-kword" )
    {
        return KoFilter::NotImplemented;
    }

    // We need KimageIO's help in AbiWordWorker::convertUnknownImage
    KImageIO::registerFormats();

    AbiWordWorker* worker=new AbiWordWorker();

    if (!worker)
    {
        kdError(30506) << "Cannot create Worker! Aborting!" << endl;
        return KoFilter::StupidError;
    }

    KWEFKWordLeader* leader=new KWEFKWordLeader(worker);

    if (!leader)
    {
        kdError(30506) << "Cannot create Worker! Aborting!" << endl;
        delete worker;
        return KoFilter::StupidError;
    }

    KoFilter::ConversionStatus result=leader->convert(m_chain,from,to);

    delete leader;
    delete worker;

    return result;
}
