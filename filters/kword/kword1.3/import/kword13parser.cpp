// 

#include "kwordparser.h"
#include "kwordframeset.h"
#include "kworddocument.h"

StackItem::StackItem() : elementType( ElementTypeUnknown ), m_currentFrameset( 0 )
{
}

StackItem::~StackItem()
{
}

KWordParser::KWordParser( KWordDocument* kwordDocument ) : m_kwordDocument(kwordDocument), m_currentParagraph( 0 )
{
    parserStack.setAutoDelete( true );
    StackItem* bottom = new StackItem;
    bottom->elementType = ElementTypeBottom;
    parserStack.push( bottom ); //Security item (not to empty the stack)
}

KWordParser::~KWordParser( void )
{
    parserStack.clear();
    delete m_currentParagraph;
}

bool KWordParser::startElementParagraph( const QString&, const QXmlAttributes&, StackItem *stackItem )
{
    if ( stackItem->elementType == ElementTypeUnknownFrameset )
    {
        stackItem->elementType = ElementTypeIgnore;
        return true;
    }

    stackItem->elementType = ElementTypeParagraph;
    
    if ( m_currentParagraph )
    {
        // Delete an evenetually already existing paragraph (should not happen)
        qDebug("Current paragraph already defined!");
        delete m_currentParagraph;
    }
        
    m_currentParagraph = new KWordParagraph;
    
    return true;
}

bool KWordParser::startElementFrame( const QString& name, const QXmlAttributes& attributes, StackItem *stackItem )
{
    if ( stackItem->elementType == ElementTypeFrameset )
    {
        stackItem->elementType = ElementTypeEmpty;
        if ( stackItem->m_currentFrameset )
        {
            const int num = ++stackItem->m_currentFrameset->m_numFrames;
            for (int i = 0; i < attributes.count(); ++i )
            {
                QString attrName ( name );
                attrName += ':';
                attrName += QString::number( num );
                attrName += ':';
                attrName += attributes.qName( i );
                stackItem->m_currentFrameset->m_frameData[ attrName ] = attributes.value( i );
                qDebug("FrameData: %s = %s", attrName.latin1(), attributes.value( i ).latin1() );
            }
            
        }
        else
        {
            //kdError(30520) << "Data of <FRAMESET> not found" << endl;
            qDebug("Data of <FRAMESET> not found");
            return false;
        }
    }
    else if ( stackItem->elementType != ElementTypeUnknownFrameset )
    {
        //kdError(30520) << "<FRAME> not child of <FRAMESET>" << endl;
        qDebug("<FRAME> not child of <FRAMESET>");
        return false;
    }
    return true;
}

bool KWordParser::startElementFrameset( const QString& name, const QXmlAttributes& attributes, StackItem *stackItem )
{
    const QString frameTypeStr( attributes.value( "frameType" ) );
    const QString frameInfoStr( attributes.value( "frameInfo" ) );
    
    if ( frameTypeStr.isEmpty() || frameInfoStr.isEmpty() )
    {
        // kdError(30520) << "<FRAMESET> without frameType or frameInfo attribute!" << endl;
        qDebug("<FRAMESET> without frameType or frameInfo attribute!");
        return false;
    }
    
    const int frameType = frameTypeStr.toInt();
    const int frameInfo = frameInfoStr.toInt();
    
    if ( frameType == 1 && frameInfo == 0 )
    {
        // Normal text frame (in or outside a table)
        if ( attributes.value( "grpMgr" ).isEmpty() )
        {
            stackItem->elementType = ElementTypeFrameset;
            
            KWordNormalTextFrameset* frameset = new KWordNormalTextFrameset( frameType, frameInfo, attributes.value( "name" ) );
            m_kwordDocument->m_normalTextFramesetList.append( frameset );
            stackItem->m_currentFrameset = m_kwordDocument->m_normalTextFramesetList.current();
        }
        else
        {
            qDebug("Tables are not supported yet!");
            stackItem->elementType = ElementTypeUnknownFrameset;
        }
    }
    else
    {
        // Frame of unknown/unsupported type
        //kdWarning(30520) << "Unknown/unsupported <FRAMESET> type! Type: " << frameTypeStr << " Info: " << frameInfoStr << emdl;
        qDebug("Unknown <FRAMESET> type! Type: %i Info: %i", frameType, frameInfo);
        stackItem->elementType = ElementTypeUnknownFrameset;
    }
    return true;
}


bool KWordParser::startElementDocumentAttributes( const QString& name, const QXmlAttributes& attributes, StackItem *stackItem,
     const StackItemElementType& allowedParentType, const StackItemElementType& newType )
{
    if ( parserStack.current()->elementType == allowedParentType )
    {
        stackItem->elementType = newType;
        for (int i = 0; i < attributes.count(); ++i )
        {
            QString attrName ( name );
            attrName += ':';
            attrName += attributes.qName( i );
            m_kwordDocument->m_documentProperties[ attrName ] = attributes.value( i );
            qDebug("DocAttr: %s = %s", attrName.latin1(), attributes.value( i ).latin1() );
        }
        return true;
    }
    else
    {
        qDebug("Wrong parent!");
        return false;
    }
}

bool KWordParser::startElement( const QString&, const QString&, const QString& name, const QXmlAttributes& attributes )
{
    qDebug("%s<%s>", indent.latin1(), name.latin1() );
    indent += "*"; //DEBUG
    if (parserStack.isEmpty())
    {
        //kdError(30520) << "Stack is empty!! Aborting! (in KWordParser::startElement)" << endl;
        qDebug("Stack is empty!! Aborting! (in KWordParser::startElement)");
        return false;
    }
    
    // Create a new stack element copying the top of the stack.
    StackItem *stackItem=new StackItem(*parserStack.current());

    if (!stackItem)
    {
        //kdError(30506) << "Could not create Stack Item! Aborting! (in StructureParser::startElement)" << endl;
        qDebug("Could not create Stack Item! Aborting! (in StructureParser::startElement)");
        return false;
    }

    stackItem->itemName=name;

    bool success=false;

    if ( name == "TEXT" )
    {
        if ( stackItem->elementType == ElementTypeParagraph && m_currentParagraph )
        {
            stackItem->elementType = ElementTypeText;
            m_currentParagraph->setText( QString::null );
        }
        else
        {
            stackItem->elementType = ElementTypeIgnore;
        }
        success = true;
    }
    else if ( name == "PARAGRAPH" )
    {
        success = startElementParagraph( name, attributes, stackItem );
    }
    else if ( name == "FRAME" )
    {
        success = startElementFrame( name, attributes, stackItem );
    }
    else if ( name == "FRAMESET" )
    {
        success = startElementFrameset( name, attributes, stackItem );
    }
    else if ( name == "DOC" )
    {
        success = startElementDocumentAttributes( name, attributes, stackItem, ElementTypeBottom, ElementTypeDocument );
    }
    else if  ( name == "PAPER") 
    {
        success = startElementDocumentAttributes( name, attributes, stackItem, ElementTypeDocument, ElementTypePaper );
    }
    else if ( name == "PAPERBORDERS" )
    {
        success = startElementDocumentAttributes( name, attributes, stackItem, ElementTypePaper, ElementTypeEmpty );
    }
    else if ( ( name == "ATTRIBUTES" ) || ( name == "VARIABLESETTINGS" )
         || ( name == "FOOTNOTESETTINGS" ) || ( name == "ENDNOTESETTINGS" ) )
    {
        success = startElementDocumentAttributes( name, attributes, stackItem, ElementTypeDocument, ElementTypeEmpty );
    }
    else
    {
        stackItem->elementType = ElementTypeUnknown;
        success = true;
    }

    if ( success )
    {
        parserStack.push( stackItem );
    }
    else
    {   // We have a problem so destroy our resources.
        delete stackItem;
    }
    
    return success;
}

bool KWordParser :: endElement( const QString&, const QString& , const QString& name)
{
    indent.remove( 0, 1 ); // DEBUG
    //qDebug("%s</%s>", indent.latin1(), name.latin1() );
    if (parserStack.isEmpty())
    {
        //kdError(30506) << "Stack is empty!! Aborting! (in StructureParser::endElement)" << endl;
        qDebug("Stack is empty!! Aborting! (in StructureParser::endElement)");
        return false;
    }

    bool success=false;
    
    StackItem *stackItem=parserStack.pop();
        
    if ( name == "PARAGRAPH" )
    {
        if ( stackItem->m_currentFrameset && m_currentParagraph )
        {
            if ( stackItem->m_currentFrameset->addParagraph( *m_currentParagraph ) )
            {
                success = true;
            }
        }
        else if ( stackItem->elementType == ElementTypeIgnore )
        {
            success = true;
        }
        delete m_currentParagraph;
        m_currentParagraph = 0;
    }
    else if ( name == "DOC" )
    {
        success = true;
    }
    else
    {
        success = true; // No problem, so authorisation to continue parsing
    }
    
    if (!success)
    {
        // If we have no success, then it was surely a tag mismatch. Help debugging!
        //kdError(30506) << "Found closing tag name: " << name << " expected: " << stackItem->itemName << endl;
        qDebug("Found closing tag name: %s expected: %s", name.latin1(), stackItem->itemName.latin1() );
    }
    
    delete stackItem;
    
    return success;
}

bool KWordParser :: characters ( const QString & ch )
{
#if 0
    // DEBUG start
    if (ch=="\n")
    {
        kdDebug(30520) << indent << " (LINEFEED)" << endl;
    }
    else if (ch.length()> 40)
    {   // 40 characters are enough (especially for image data)
        kdDebug(30520) << indent << " :" << ch.left(40) << "..." << endl;
    }
    else
    {
        kdDebug(30520) << indent << " :" << ch << ":" << endl;
    }
    // DEBUG end
#endif

    if (parserStack.isEmpty())
    {
        //kdError(30520) << "Stack is empty!! Aborting! (in StructureParser::characters)" << endl;
        qDebug("Stack is empty!! Aborting! (in KWordParser::characters)");
        return false;
    }

    bool success=false;

    StackItem *stackItem = parserStack.current();

    if ( stackItem->elementType == ElementTypeText )
    { 
        // <TEXT>
        if ( m_currentParagraph )
        {
            m_currentParagraph->appendText( ch );
            success = true;
        }
        else
        {
            qDebug("No current paragraph defined! Tag mismatch?");
            success = false;
        }
    }
    else if (stackItem->elementType==ElementTypeEmpty)
    {
        success=ch.stripWhiteSpace().isEmpty();
        if (!success)
        {
            // We have a parsing error, so abort!
            // kdError(30520) << "Empty element "<< stackItem->itemName <<" is not empty! Aborting! (in KWordParser::characters)" << endl;
            qDebug("Empty element %s  is not empty! Aborting! (in KWordParser::characters)", stackItem->itemName.latin1());
        }
    }
    else
    {
        success=true;
    }

    return success;
}
