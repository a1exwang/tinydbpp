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
%token KW_LEQ KW_NEQ KW_GEQ
%token IDENTIFIER STRING INT FLOAT
%token SEMICOLON
%token UNKNOWN
%token '=' '<' '>' '(' ')' '.' '*' ','

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


table_stmt:  KW_CREATE KW_TABLE IDENTIFIER '(' fieldList ')'{
                $$.makecreateTbNode($3, $5);
            }
           | KW_DROP KW_TABLE IDENTIFIER{
                $$.makeDropTbNode($3);
           }
           | KW_DESC IDENTIFIER{
                $$.makeDescribeTbNode($2);
           }
           | KW_INSERT KW_INTO IDENTIFIER KW_VALUES valueLists{
                $$.makeInsertTbNode($3, $5);
           }
           | KW_DELETE KW_FROM IDENTIFIER KW_WHERE whereClause{
                $$.makeDeleteTbNode($3, $5);
           }
           | KW_UPDATE IDENTIFIER KW_SET setClause KW_WHERE whereClause{
                $$.makeUpdateTbNode($2, $4, $6);
           }
           | KW_SELECT selector KW_FROM tableList KW_WHERE whereClause{
                $$.makeSelectTbNode($2, $4, $6);
                cout << "select " << dynamic_pointer_cast<ast::SelectCols>($2.getNode())->isAll << endl;
           }

idx_stmt  :  KW_CREATE KW_INDEX IDENTIFIER '(' IDENTIFIER ')'{
                $$.makeCreateIdxNode($3, $5);
            }
            | KW_DROP KW_INDEX IDENTIFIER '(' IDENTIFIER ')'{
                $$.makeDropIdxNode($3, $5);
            }

fieldList:  field {
                std::shared_ptr<ast::FieldList> fields(new ast::FieldList());
                fields->vec.push_back(*std::dynamic_pointer_cast<ast::Field>($1.getNode()));
                $$ = ParserVal(fields);
            }
            | fieldList ',' field {
                auto fields = std::dynamic_pointer_cast<ast::FieldList>($1.getNode());
                fields->vec.push_back(*std::dynamic_pointer_cast<ast::Field>($3.getNode()));
            }

field: IDENTIFIER type{
            std::shared_ptr<ast::Field> ptr(new ast::Field($1.strVal, $2.strVal, $2.iVal, true, false));
            $$ = ParserVal(ptr);
         }
         | IDENTIFIER type KW_NOT KW_NULL{
            std::shared_ptr<ast::Field> ptr (new ast::Field($1.strVal, $2.strVal, $2.iVal, false, false));
            $$ = ParserVal(ptr);            
         }
         | KW_PRIMARY KW_KEY '(' IDENTIFIER ')'{
            std::shared_ptr<ast::Field> ptr (new ast::Field($4.strVal, "", 0, false, false, true));
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
        std::shared_ptr<ast::ValueLists> ptr(new ast::ValueLists());
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
        std::shared_ptr<ast::ValueList> ptr(new ast::ValueList());
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
        std::shared_ptr<ast::Value> ptr(new ast::Value("int"));
        ptr->iVal = $1.iVal;
        $$ = ParserVal(ptr);
    }
    | STRING{
        std::shared_ptr<ast::Value> ptr(new ast::Value("varchar"));
        ptr->strVal = $1.strVal;
        $$ = ParserVal(ptr);
    }
    | KW_NULL{
        std::shared_ptr<ast::Value> ptr(new ast::Value("NULL"));
        $$ = ParserVal(ptr);
    }

whereClause : col op expr {
                std::shared_ptr<ast::WhereClause> ptr(new ast::WhereClause());
                ptr->becomeCompare($1.strVal, $2.strVal, *std::dynamic_pointer_cast<ast::Value>($3.getNode()) );
                $$ = ParserVal(ptr);
            }
            | col KW_IS KW_NULL{
                std::shared_ptr<ast::WhereClause> ptr(new ast::WhereClause());
                ptr->becomeIsNull($1.strVal);
                $$ = ParserVal(ptr);
            }
            | col KW_IS KW_NOT KW_NULL{
                std::shared_ptr<ast::WhereClause> ptr(new ast::WhereClause());
                ptr->becomeIsNotNull($1.strVal);
                $$ = ParserVal(ptr);
            }
            | whereClause KW_AND whereClause{
                std::shared_ptr<ast::WhereClause> ptr(new ast::WhereClause());
                ptr->becomeAnd(std::dynamic_pointer_cast<ast::WhereClause>($1.getNode()), std::dynamic_pointer_cast<ast::WhereClause>($3.getNode()));
                $$ = ParserVal(ptr);
            } 
col : IDENTIFIER '.' IDENTIFIER{
        $$.strVal = $1.strVal + "_" + $3.strVal;
    }
    | IDENTIFIER{
        $$.strVal = $1.strVal;
    }

op : '='{$$ = $1;} | KW_NEQ {$$ = $1;}| KW_LEQ {$$ = $1;}| KW_GEQ{$$ = $1;} | '<'{$$ = $1;} | '>'{$$ = $1;}

expr : value{$$ = $1;} | col{
        std::shared_ptr<ast::Value> ptr(new ast::Value("col"));
        ptr->strVal = $1.strVal;
        $$ = ParserVal(ptr);
    }

 setClause : IDENTIFIER '=' value {
            std::shared_ptr<ast::SetClause> ptr(new ast::SetClause());
            ptr->push_back($1.strVal, *std::dynamic_pointer_cast<ast::Value>($3.getNode()) );
            $$ = ParserVal(ptr);
        }
        | setClause ',' IDENTIFIER '=' value {
            auto ptr = std::dynamic_pointer_cast<ast::SetClause>($1.getNode());
            ptr->push_back($3.strVal, *std::dynamic_pointer_cast<ast::Value>($5.getNode()));
            $$ = $1;
        }

 selector   : '*'{ 
                std::shared_ptr<ast::SelectCols> ptr(new ast::SelectCols());
                ptr->setAll();
                $$ = ParserVal(ptr);
            }
            |  colList{
                std::shared_ptr<ast::SelectCols> ptr(new ast::SelectCols());
                auto ptr1 = std::dynamic_pointer_cast<ast::ColList>($1.getNode());
                BOOST_ASSERT(ptr1 != nullptr);
                ptr->setColList(ptr1);
                $$ = ParserVal(ptr);
            }

 colList : col{
        std::shared_ptr<ast::ColList> ptr(new ast::ColList());
        ptr->push_back($1.strVal);
        $$ = ParserVal(ptr);
    } | colList ',' col{
        auto ptr = std::dynamic_pointer_cast<ast::ColList>($1.getNode());
        ptr->push_back($3.strVal);
        $$ = $1;
    }

 tableList  :  IDENTIFIER {
           std::shared_ptr<ast::TableList> ptr(new ast::TableList());
           ptr->push_back($1.strVal);
           $$ = ParserVal(ptr);
        }
        |  tableList ',' IDENTIFIER {
            auto ptr = std::dynamic_pointer_cast<ast::TableList>($1.getNode());
            ptr->push_back($3.strVal);
            $$ = $1;
        }



// id_list: IDENTIFIER
//     | id_list IDENTIFIER;
%%

void tinydbpp::Parser::error(const std::string& msg) {
    throw ParsingError(msg);
}
