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
%token '=' '<' '>' '(' ')' '.'

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
    | idx_stmt SEMICOLON { $$ = $1;}
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


table_stmt:  KW_CREATE KW_TABLE IDENTIFIER '(' fieldLists ')'{
                $$.makecreateTbNode($5);
            }
           | KW_DROP KW_TABLE IDENTIFIER{
                $$.makeDropTbNode($3);
           }
           //| KW_DESC IDENTIFIER
           | KW_INSERT KW_INTO IDENTIFIER KW_VALUES valueLists{
                $$.makeInsertTbNode($3, $5);
           }
           | KW_DELETE KW_FROM IDENTIFIER KW_WHERE whereClause{
                $$.makeDeleteTbNode($3, $5);
           }
           | KW_UPDATE IDENTIFIER KW_SET setClause KW_WHERE whereClause{
                $$.makeUpdateTbNode($3, $5);
           }
           | KW_SELECT selector FROM tableList KW_WHERE whereClause{
                $$.makeSelectTbNode($2, $4, $6);
           }

idx_stmt  :  KW_CREATE KW_INDEX IDENTIFIER '(' IDENTIFIER ')'{
                $$.makeCreateIdxNode($5);
            }
            | KW_DROP KW_INDEX IDENTIFIER '(' IDENTIFIER ')'{
                $$.makeDropIdxNode($5);
            }

fieldList:  field {
                $$ = ParserVal(std::shared_ptr<FieldList>(new FieldList()));
            }
            | fieldList ',' field {
                auto fields = std::dynamic_pointer_cast<ast::FieldList>($1.getNode());
                fields->vec.push_back(*($3.getNode()));
            }

field: IDENTIFIER type{
            std::shared_ptr<Field> ptr(new Field($1.strVal, $2.strVal, $2.iVal, true, false));
            $$ = ParserVal(ptr);
         }
         | IDENTIFIER type NOT NULL{
            auto ptr = new Field($1.strVal, $2.strVal, $2.iVal, false, false);
            $$ = ParserVal(ptr);            
         }
         | KW_PRIMARY KW_KEY '(' IDENTIFIER ')'{
            auto ptr = new Field($4.strVal, "", 0, false, false, true);
            $$ = ParserVal(ptr);
         }

type: 
    KW_INT '(' INT ')'{
        $$.iVal = $3.iVal;
        $$.strVal = "int";
    }
    | KW_VARCHAR '(' INT ')'{
        $$.iVal = $3.iVal;
        $$.strVal = "varchar";        
    }

valueLists  : '(' valueList ')'{
        std::shared_ptr<ValueLists> ptr(new ValueLists());
        auto vlist = std::dynamic_pointer_cast<ast::ValueList>($2.getNode());
        ptr->push_back(vlist);
        $$ = ParserVal(ptr);
    }
    | valueLists ',' '(' valueList ')'{
        auto vlists = std::dynamic_pointer_cast<ast::ValueLists>($1.getNode());
        auto vlist = std::dynamic_pointer_cast<ast::ValueList>($4.getNode());
        vlists->push_back(vlist);
        $$ = $1;
    }

valueList : value{
        std::shared_ptr<ValueList> ptr(new ValueList());
        auto v = std::dynamic_pointer_cast<ast::Value>($1.getNode());
        ptr->push_back(v);
        $$ = ParserVal(ptr);
    }
    | valueList ',' value{
        auto vlist = std::dynamic_pointer_cast<ast::ValueList>($1.getNode());
        auto v = std::dynamic_pointer_cast<ast::Value>($3.getNode());
        vlist->push_back(v);
        $$ = $1;
    }

value : INT{
        std::shared_ptr<Value> ptr(new Value("int"));
        ptr->iVal = $1.iVal;
        $$ = ParserVal(ptr);
    }
    | STRING{
        std::shared_ptr<Value> ptr(new Value("string"));
        ptr->strVal = $1.strVal;
        $$ = ParserVal(ptr);
    }
    | KW_NULL{
        std::shared_ptr<Value> ptr(new Value("NULL"));
        $$ = ParserVal(ptr);
    }

whereClause : col op expr {
                
            }
            | col KW_IS KW_NULL
            | col KW_IS KW_NOT KW_NULL
            | whereClause AND whereClause 
col : IDENTIFIER '.' IDENTIFIER{
        $$.strVal = $1.strVal + "." + $3.strVal;
    }
    | IDENTIFIER{
        $$.strVal = $1.strVal;
    }

op : '='{$$ = $1;} | KW_NEQ {$$ = $1;}| KW_LEQ {$$ = $1;}| KW_GEQ{$$ = $1;} | '<'{$$ = $1;} | '>'{$$ = $1;}


// id_list: IDENTIFIER
//     | id_list IDENTIFIER;
%%

void tinydbpp::Parser::error(const std::string& msg) {
    throw ParsingError(msg);
}
