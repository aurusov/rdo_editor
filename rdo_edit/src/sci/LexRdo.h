#ifndef LEXRDO_H
#define LEXRDO_H

#define SCN_RDO_POSCHANGED 20000
#define SCN_RDO_CLICK      20001
#define SCN_RDO_BUFFERKEY  20002

static inline bool isRDOLexerOperator( char ch )
{
	if ( ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == ':' ||
	     ch == '[' || ch == ']' || ch == '(' || ch == ')' || ch == ',' ||
	     ch == '<' || ch == '>' || ch == '=' || ch == '.' ) return true;
	return false;
}

static inline bool isRDOLexerIdentifier( char ch )
{
	if ( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
	     (ch >= '�' && ch <= '�') || (ch >= '�' && ch <= '�') ||
	     ch == '_' || ch == '$' || isdigit(ch) ) return true;
	return false;
}

static inline bool isRDOUpper( char ch )
{
	if ( ch >= '�' && ch <= '�' ) return false;
	if ( ch >= '�' && ch <= '�' ) return true;
	return isupper( ch ) ? true : false;
}

static inline bool isRDOLower( char ch )
{
	if ( ch >= '�' && ch <= '�' ) return true;
	if ( ch >= '�' && ch <= '�' ) return false;
	return islower( ch ) ? true : false;
}

static inline char RDOMakeUpperCase( char ch )
{
	if ( ch >= 'a' && ch <= 'z' ) return static_cast<char>( ch - 'a' + 'A' );
	if ( ch >= '�' && ch <= '�' ) return static_cast<char>( ch - '�' + '�' );
	return ch;
}

static inline char RDOMakeLowerCase( char ch )
{
	if ( ch >= 'A' && ch <= 'Z' ) return static_cast<char>( ch - 'A' + 'a' );
	if ( ch >= '�' && ch <= '�' ) return static_cast<char>( ch - '�' + '�' );
	return ch;
}

extern bool useFold;

#endif
