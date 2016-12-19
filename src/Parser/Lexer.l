%{
#include <string>
#include <boost/log/trivial.hpp>

#include <Parser/Lexer.h>
#include <Parser.tab.hpp>

// This is token enum in Bison, declared in bison as '%token IDENTIFIER'
%}

%option debug
%option nodefault
%option yyclass="tinydbpp::Lexer"
%option noyywrap
%option c++
%option prefix="SQL"
%option stack

SIMPLE_OPERATOR = ("="|"<"|">"|"("|")"|"."|"*"|",")
%%
[ \t\r\n\v]+ {
    // Ignore all spaces
}

(?i:database) { yylval.becomeKeyword("database"); return Token::KW_DATABASE; }
(?i:databases) { yylval.becomeKeyword("databases"); return Token::KW_DATABASES; }
(?i:table) { yylval.becomeKeyword("table"); return Token::KW_TABLE; }
(?i:tables) { yylval.becomeKeyword("tables"); return Token::KW_TABLES; }
(?i:show) { yylval.becomeKeyword("show"); return Token::KW_SHOW; }
(?i:create) { yylval.becomeKeyword("create"); return Token::KW_CREATE; }
(?i:drop) { yylval.becomeKeyword("drop"); return Token::KW_DROP; }
(?i:use) { yylval.becomeKeyword("use"); return Token::KW_USE; }
(?i:primary) { yylval.becomeKeyword("primary"); return Token::KW_PRIMARY; }
(?i:key) { yylval.becomeKeyword("key"); return Token::KW_KEY; }
(?i:not) { yylval.becomeKeyword("not"); return Token::KW_NOT; }
(?i:null) { yylval.becomeKeyword("null"); return Token::KW_NULL; }
(?i:insert) { yylval.becomeKeyword("insert"); return Token::KW_INSERT; }
(?i:into) { yylval.becomeKeyword("into"); return Token::KW_INTO; }
(?i:values) { yylval.becomeKeyword("values"); return Token::KW_VALUES; }
(?i:delete) { yylval.becomeKeyword("delete"); return Token::KW_DELETE; }
(?i:from) { yylval.becomeKeyword("from"); return Token::KW_FROM; }
(?i:where) { yylval.becomeKeyword("where"); return Token::KW_WHERE; }
(?i:update) { yylval.becomeKeyword("update"); return Token::KW_UPDATE; }
(?i:set) { yylval.becomeKeyword("set"); return Token::KW_SET; }
(?i:select) { yylval.becomeKeyword("select"); return Token::KW_SELECT; }
(?i:is) { yylval.becomeKeyword("is"); return Token::KW_IS; }
(?i:int) { yylval.becomeKeyword("int"); return Token::KW_INT; }
(?i:varchar) { yylval.becomeKeyword("varchar"); return Token::KW_VARCHAR; }
(?i:desc) { yylval.becomeKeyword("desc"); return Token::KW_DESC; }
(?i:index) { yylval.becomeKeyword("index"); return Token::KW_INDEX; }
(?i:and) { yylval.becomeKeyword("and"); return Token::KW_AND; }

'[^']*' {
    yylval.becomeStringLiteral(yytext, yyleng);
    BOOST_LOG_TRIVIAL(info) << "Lexer. String literal" <<  yytext;
    return Token::STRING;
}

0x[0-9a-fA-F]+ {
    yylval.becomeHexInt(yytext, yyleng);
    BOOST_LOG_TRIVIAL(info) << "Lexer. Hex Number " <<  yytext;
    return Token::INT;
}
[0123456789]+           {
    yylval.becomeInt(yytext, yyleng);
    BOOST_LOG_TRIVIAL(info) << "Lexer. Decimal Number " << yytext;
    return Token::INT;
}
[A-Za-z][_0-9A-Za-z]*    {
    yylval.becomeIdentifier(yytext, yyleng);
    BOOST_LOG_TRIVIAL(info) << "Lexer. Identifier " << yytext;
    return Token::IDENTIFIER;
}

; {
    return Token::SEMICOLON;
}
SIMPLE_OPERATOR {
    yylval.becomeBasicSymbol(yytext, yyleng);
    return yytext[0];
}

"<>" {  yylval.becomeBasicSymbol(yytext, yyleng);return Token::KW_NEQ; }
"<=" {  yylval.becomeBasicSymbol(yytext, yyleng);return Token::KW_LEQ; }
">=" {  yylval.becomeBasicSymbol(yytext, yyleng);return Token::KW_GEQ; }

%%