#include <iostream.h>

#include <qstring.h>

#include "kscript_parser.h"
#include "kscript_parsenode.h"

extern FILE* yyin;
extern char* yytext;
extern int yylex();

KSParser *theParser;

QString idl_lexFile;
QString toplevelFile;
int idl_line_no;

// Imported from yacc.yy
extern void kscriptParse( const char *_code );

KSParser::KSParser()
{
  rootNode = 0;
}

KSParser::~KSParser()
{
  if ( rootNode )
    delete rootNode;
}

bool KSParser::parse( FILE *inp_file, const char *filename )
{
  idl_lexFile = toplevelFile = (char *) filename;
  yyin = inp_file;

  m_errorMessage = "";
  theParser = this;
  idl_line_no = 1;
  yyparse();

  return m_errorMessage.isEmpty();
}

bool KSParser::parse( const char* code )
{
  m_errorMessage = "";
  theParser = this;
  idl_line_no = 1;
  kscriptParse( code );

  return m_errorMessage.isEmpty();
}

void KSParser::setRootNode( KSParseNode *node )
{
  rootNode = node;
}


KSParseNode *KSParser::getRootNode()
{
  return rootNode;
}


void KSParser::parse_error( const char *file, const char *err, int line )
{
  m_errorMessage = "%1:%2: %3 before '%4'";
  m_errorMessage = m_errorMessage.arg( file ).arg( line).arg( err ).arg( yytext );
}

void KSParser::print( bool detailed )
{
  if ( rootNode )
    rootNode->print( detailed );
}

bool KSParser::eval( KSContext& context )
{
  if ( !rootNode )
    return FALSE;

  return rootNode->eval( context );
}

KSParseNode* KSParser::donateParseTree()
{
  KSParseNode* n = rootNode;
  rootNode = 0;
  return n;
}
