%code requires {
#include <Parser/ParserVal.h>
#include <Parser/AST/Nodes.h>
#include <Parser/ParsingError.h>
#include <boost/assert.hpp>
#include <memory>
}

%{
#include <iostream>
#include <string>
#include <cstdlib>
#include <Parser/Lexer.h>
#include <Parser/ParserVal.h>
#include <Parser/AST/Nodes.h>
using namespace tinydbpp;
using namespace std;
#undef yylex
#define yylex lexer.lex
%}

%require "3.0.4"
%debug
%start root

%defines
%skeleton "lalr1.cc"
%define api.namespace {tinydbpp}
%define api.value.type {ParserVal}
%define parser_class_name {Parser}

%token KW_DATABASE KW_DATABASES KW_TABLE KW_TABLES KW_SHOW KW_CREATE KW_DROP KW_USE KW_PRIMARY KW_KEY KW_NOT KW_NULL KW_INSERT KW_INTO KW_VALUES KW_DELETE KW_FROM KW_WHERE KW_UPDATE KW_SET KW_SELECT KW_IS KW_INT KW_VARCHAR KW_DESC KW_INDEX KW_AND
%token IDENTIFIER STRING INT FLOAT
%token SEMICOLON
%token UNKNOWN

%parse-param { class Lexer &lexer } { std::shared_ptr<tinydbpp::ast::Node> &rootNode }

%% /* The grammar follows.  */
root: stmts { rootNode = $1.getNode(); }

stmts: %empty { $$ = ParserVal(std::make_shared<ast::Statements>()); }
    | stmt stmts {
        $$ = $2;
        auto stmts = std::dynamic_pointer_cast<ast::Statements>($2.getNode());
        BOOST_ASSERT(stmts != nullptr);
        auto stmt = std::dynamic_pointer_cast<ast::Statement>($1.getNode());
        stmts->addStatementToFront(stmt);
      }
;
stmt: sys_stmt SEMICOLON   { $$ = $1; }
    | table_stmt SEMICOLON { $$ = $1; }
;

sys_stmt: KW_SHOW KW_DATABASES {
        $$.makeShowDbsNode();
    }
    | KW_CREATE KW_DATABASE IDENTIFIER {
        $$.makeCreateDbNode($3);
    }
    | KW_DROP KW_DATABASE IDENTIFIER {
        $$.makeDropDbNode($3);
    }
    | KW_USE KW_DATABASE IDENTIFIER {
        $$.makeUseDbNode($3);
    }
    | KW_SHOW KW_TABLES {
        $$.makeShowTablesNode();
    }


table_stmt: id_list;

id_list: IDENTIFIER
    | id_list IDENTIFIER;
%%

void tinydbpp::Parser::error(const std::string& msg) {
    throw ParsingError(msg);
}