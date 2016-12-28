

#include "Nodes.h"
#include <Parser/ParserVal.h>
#include <cassert>
#include <iostream>
#include <FileUtils.h>
#include <Pager/Pager.h>
#include <Pager/Page.h>
#include <RecordManage/TableManager.h>
#include <stdlib.h>
#include <cmath>
#include <BTree/BTreePlus.h>
#include <config.h>
#include <ParsingError.h>
#include <regex>
using namespace std;
using namespace tinydbpp;
using namespace tinydbpp::ast;

std::shared_ptr<const tinydbpp::ParserVal> SysManagement::getTarget() const {
  return target;
}

SysManagement::~SysManagement() { }

void Statements::addStatementToFront(std::shared_ptr<Statement> stmt) {
  statements.insert(statements.begin(), stmt);
}

Statements::~Statements() { }

void WhereClause::becomeCompare(const std::string &colname, const std::string &op, Value v) {
  names.push_back(colname);
  ops.push_back(op);
  exprs.push_back(v);
}

void WhereClause::becomeIsNull(const std::string &colname) {
  names.push_back(colname);
  ops.push_back("isnull");
  exprs.push_back(Value("NULL"));
}

void WhereClause::becomeIsNotNull(const std::string &colname) {
  names.push_back(colname);
  ops.push_back("isnotnull");
  exprs.push_back(Value("NULL"));
}

void WhereClause::becomeAnd(std::shared_ptr<WhereClause> w1, std::shared_ptr<WhereClause> w2) {
  names = w1->names;
  ops = w1->ops;
  exprs = w1->exprs;
  for(size_t i = 0;i < w2->names.size();i++){
    names.push_back(w2->names[i]);
    ops.push_back(w2->ops[i]);
    exprs.push_back(w2->exprs[i]);
  }
}

Checker WhereClause::getChecker(std::string table_name) {
    auto td = TableManager::getInstance()->getTableDescription(table_name);
    vector<string> check_ops(td->col_name.size(), "");
    vector<Value> check_value(td->col_name.size(), Value("NULL"));
    for(int i = 0;i < names.size();i++)
        {
            string col_name, col2_name;
            if (names[i].find(".") == string::npos) col_name = names[i];
            else if( names[i].find(table_name + ".") != string::npos)
                col_name = string(names[i].begin() + names[i].find(".") + 1, names[i].end());
            else continue;
            if (exprs[i].type == "col"){
                size_t pos = exprs[i].strVal.find(".");
                if(pos == string::npos)
                    col2_name = exprs[i].strVal;
                else if(string(exprs[i].strVal.begin(), exprs[i].strVal.begin() + pos) == table_name)
                    col2_name = string(exprs[i].strVal.begin() + pos + 1, exprs[i].strVal.end());
                else continue;
            }
            bool found = false;
            for(int j = 0;j < td->col_name.size();j++)
                if(td->col_name[j] == col_name) {
                    found = true;
                    check_ops[j] = ops[i], check_value[j] = exprs[i];
                    if(exprs[i].type == "col") {
                        //type : col, strVal : col_type, iVal: offset
                        check_value[j].strVal = td->col_type[j];
                        bool ffound = false;
                        for (int k = 0; k < td->col_name.size(); k++)
                            if (td->col_name[k] == col2_name)
                                check_value[j].iVal = k, ffound = true;
                        if(!ffound) throw NoThisColumnError(td->name, col2_name);
                    }
                }
            if(!found) throw NoThisColumnError(td->name, col_name);
        }
    Checker ret = [check_ops, check_value](const Item& item)-> bool{
        for(int i = 0;i < check_ops.size();i++)
        if(check_ops[i] != "")
        {
            if(check_value[i].type == "int" || (check_value[i].type == "col" && check_value[i].strVal == "int")){
                int a = *(int *)item[i].c_str();
                char a_null = item[i].back();
                int b = check_value[i].iVal;
                char b_null = 0;
                if(check_value[i].type == "col")
                    b = *(int *)item[check_value[i].iVal].c_str(), b_null = item[check_value[i].iVal].back();

                if(a_null == 1 || b_null == 1) return false;
                if(check_ops[i] == "=" && !(a == b)) return false;
                if(check_ops[i] == "<" && !(a < b)) return false;
                if(check_ops[i] == ">" && !(a > b)) return false;
                if(check_ops[i] == "<>" && !(a != b)) return false;
                if(check_ops[i] == "<=" && !(a <= b)) return false;
                if(check_ops[i] == ">=" && !(a >= b)) return false;

            }else if(check_value[i].type == "varchar" || (check_value[i].type == "col" && check_value[i].strVal == "varchar")){
                string a(item[i].begin(), item[i].end() - 1);
                char a_null = item[i].back();
                string b = check_value[i].strVal;
                char b_null = 0;
                if(check_value[i].type == "col")
                    b = string(item[check_value[i].iVal].begin(), item[check_value[i].iVal].end() - 1), b_null = item[check_value[i].iVal].back();

                if (a_null == 1 || b_null == 1) return false;
                if (check_ops[i] == "=" && !(a == b)) return false;
                if (check_ops[i] == "<" && !(a < b)) return false;
                if (check_ops[i] == ">" && !(a > b)) return false;
                if (check_ops[i] == "<>" && !(a != b)) return false;
                if (check_ops[i] == "<=" && !(a <= b)) return false;
                if (check_ops[i] == ">=" && !(a >= b)) return false;
                cout << "regex: " << b << endl;
                if (check_ops[i] == "like" && !regex_match(a, regex(b))) return false;
            }else {
                char a_null = item[i].back();
                if(check_ops[i] == "isnull")
                {
                    if(a_null != 1) return false;
                }
                else if(check_ops[i] == "isnotnull")
                {
                    if(a_null != 0) return false;
                }
            }
        }
        return true;
    };
    return ret;
}

std::string WhereClause::getNextAssignTableName(bool &can_index, int &col_index, string & v_str, const std::vector<std::string>& tables) {
    string default_table = tables[0];
    string rank2_table = default_table;
    for(auto & table_name : tables){
        auto td = TableManager::getInstance()->getTableDescription(table_name);
        for(int i = 0;i < names.size();i++)
        if(ops[i] == "=" && exprs[i].type != "col")
        {
            size_t pos = names[i].find(".");
            string this_table =  pos == string::npos? default_table : string(names[i].begin(), names[i].begin() + pos);
            if(table_name != this_table) continue;
            string col_name = pos == string::npos? names[i] : string(names[i].begin() + pos + 1, names[i].end());
            int j = 0;
            for(;j < td->col_name.size();j++)
                if(td->col_name[j] == col_name)
                    break;
            if(j >= td->col_name.size())
                throw NoThisColumnError(td->name, col_name);
            if(td->col_has_index[j])
            {
                can_index = true;
                col_index = j;
                if(exprs[i].type == "int"){
                    v_str = string(5, '\0');
                    v_str.replace(v_str.begin(), v_str.begin() + 4, string((char*)&(exprs[i].iVal), (char*)&(exprs[i].iVal)+ 4));
                }else v_str = exprs[i].strVal + string(1,0);
                return table_name;
            }
        }else if(ops[i] == "=" && exprs[i].type == "col"){
            string this_table, col_name;
            ColList::split(names[i], default_table, this_table, col_name);
            string this_table2, col_name2;
            ColList::split(exprs[i].strVal, default_table, this_table2, col_name2);
            auto td2 = TableManager::getInstance()->getTableDescription(this_table2);
            auto td = TableManager::getInstance()->getTableDescription(this_table);
            if(table_name == this_table && table_name != this_table2 && td2->col_has_index[td2->getOffset(col_name2)])
                rank2_table = table_name;
            else if(table_name != this_table && table_name == this_table2 && td->col_has_index[td->getOffset(col_name)])
                rank2_table = table_name;
        }
    }
    can_index = false;
    col_index = 0;
    return rank2_table;
}

void WhereClause::dfs(std::vector< vector<Value> > &ans, const std::vector<std::string> &table_names, const vector<Value>& prefix,
                      const Selector & selector, std::vector<std::string>& assigned_table) {
    bool can_index;
    int col_offset;
    string v;
    string table_name = this->getNextAssignTableName(can_index, col_offset, v, table_names);
    vector<string> next_table_names;
    for(int i = 0;i < table_names.size();i++)
        if(table_names[i] != table_name)
            next_table_names.push_back(table_names[i]);
        else if(assigned_table[table_names.size() - 1] == "")
            assigned_table[table_names.size() - 1] = table_name;
    auto checker = this->getChecker(table_name);
    vector< Item > this_table_items;
    if(can_index)
        this_table_items = TableManager::getInstance()->getTableDescription(table_name)->selectUseIndex(col_offset, v, checker);
    else this_table_items = TableManager::getInstance()->getTableDescription(table_name)->selectUseChecker(checker);
    for(auto &item : this_table_items) {
        vector<Value> selected_item = selector(item, table_name);
        vector<Value> merged_item(prefix);
        for(auto & v : selected_item)
            merged_item.push_back(v);
        if (table_names.size() > 1)
            this->assign(table_name, item).dfs(ans, next_table_names, merged_item, selector, assigned_table);
        else
            ans.push_back(merged_item);
    }
}

WhereClause WhereClause::assign(const std::string &table_name, const Item &item) {
    WhereClause ret;
    auto td = TableManager::getInstance()->getTableDescription(table_name);
    for(int i = 0;i < names.size();i++)
    {
        size_t pos = names[i].find(".");
        string this_table =  pos == string::npos? table_name : string(names[i].begin(), names[i].begin() + pos);
        string col_name = pos == string::npos? names[i] : string(names[i].begin() + pos + 1, names[i].end());
        if(exprs[i].type != "col"){
            if(this_table != table_name) {
                ret.names.push_back(names[i]);
                ret.ops.push_back(ops[i]);
                ret.exprs.push_back(exprs[i]);
            }
        }else{
            pos = exprs[i].strVal.find(".");
            string this_table2 = pos == string::npos? table_name : string(exprs[i].strVal.begin(), exprs[i].strVal.begin() + pos);
            string col_name2 = pos == string::npos? exprs[i].strVal : string(exprs[i].strVal.begin() + pos + 1, exprs[i].strVal.end());
            if(this_table == table_name && this_table2 == table_name) continue;
            else if(this_table != table_name && this_table2 == table_name){
                ret.names.push_back(names[i]);
                ret.ops.push_back(ops[i]);
                int offset = td->getOffset(col_name2);
                ret.exprs.push_back(Value(td->col_type[offset], item[offset]));
            }
            else if(this_table != table_name && this_table2 != table_name){
                ret.names.push_back(names[i]);
                ret.ops.push_back(ops[i]);
                ret.exprs.push_back(exprs[i]);
            }else if(this_table == table_name && this_table2 != table_name){
                ret.names.push_back(exprs[i].strVal);
                string op = ops[i];
                if(ops[i] == "<") op = ">";
                else if (ops[i] == ">") op = "<";
                else if (ops[i] == "<=") op = ">=";
                else if (ops[i] == ">=") op = "<=";
                ret.ops.push_back(op);
                int offset = td->getOffset(col_name);
                ret.exprs.push_back(Value(td->col_type[offset], item[offset]));
            }
        }
    }
    return ret;
}

void WhereClause::becomeLike(const std::string &colname, const string &regexp) {
    names.push_back(colname);
    ops.push_back("like");
    string escapedRegexp;
    for (int i = 0; i < regexp.size(); ++i) {
        if (!(isdigit(regexp[i]) || isalpha(regexp[i]) || regexp[i] == '%' || regexp[i] == '_')) {
            escapedRegexp += "\\";
        }
        escapedRegexp += regexp[i];
    }
    escapedRegexp = regex_replace(escapedRegexp, regex("%"), ".*");
    escapedRegexp = regex_replace(escapedRegexp, regex("_"), ".");
    Value v("varchar");
    v.strVal = escapedRegexp;
    exprs.push_back(v);
}

json Statement::exec() {
    try {
        if (type == ShowDbs) {
            TableManager::getInstance(); // create base dir
            auto file_name_list = FileUtils::listFiles(TableManager::base_dir.c_str());
            return json({{"result", file_name_list}});
        } else if (type == CreateDb) {
            TableManager::getInstance()->createDB(ch[0]->strVal);
            return json({{"result", "Succeed."}});
        } else if (type == DropDb) {
            if (TableManager::getInstance()->DropDB(ch[0]->strVal))
                return json({{"result", "Succeed."}});
            else return json({{"result", "No such Database."}});
        } else if (type == UseDb) {
            if (TableManager::getInstance()->changeDB(ch[0]->strVal, false))
                return json({{"result", "Succeed."}});
            else
                return json({{"result", "No such Database."}});
        } else if (type == ShowTables) {
            if (TableManager::getInstance()->hasDB()) {
                vector<string> ret;
                auto file_name_list = FileUtils::listFiles(TableManager::getInstance()->dir.c_str());
                for (auto &str : file_name_list)
                    if (str.find(".") == str.npos && str != SYS_TABLE_NAME) {
                        ret.push_back(str);
                    }
                return json({{"result", ret}});
            } else return json({{"result", "No Database was specified."}});
        } else if (type == CreateTable) {
            if (TableManager::getInstance()->hasDB()) {
                auto fl = std::dynamic_pointer_cast<FieldList>(ch[1]->getNode());
                fl->checkPrimaryKey();
                function<void(tinydbpp::Pager *)> writeScheme = [this, &fl](tinydbpp::Pager *ptr) {
                    auto p = ptr->getPage(0);
                    char *buf = p->getBuf();
                    char size_str[20];
                    int num = 0;
                    for (auto &f : fl->vec)
                        if (!f.is_primary_key_stmt)
                            num++;
                    sprintf(size_str, "%d", num);
                    string tmp = string("    ") + size_str + " ";
                    for (auto &f : fl->vec) {
                        if (f.is_primary_key_stmt) continue;
                        string pattern = f.type == "varchar" ? "-1" : "5"; //bits to bytes +1 is null
                        sprintf(size_str, "%d", f.size);
                        //can null 0 not null 1
                        tmp = tmp + f.name + " " + f.type + " " + pattern + " " +
                              ((f.is_key | !f.can_null) ? string("1 ") : string("0 "))
                              + (f.is_key ? string("1 ") : string("0 ")) + size_str + " "; // unique

                        // create index here
                        auto indexName = TableManager::createIndexName(ch[0]->strVal, f.name);
                        if (f.is_key)
                            TheBTree::BT::setupBTree(indexName);
                    }
                    sprintf(buf, "%s", tmp.c_str());
                    p->markDirty();
                    p->releaseBuf(buf);
                };
                if (TableManager::getInstance()->buildTable(ch[0]->strVal, writeScheme))
                    return json({{"result", "Succeed."}});
                else return json({{"result", "Create table failed."}});
            } else return json({{"result", "No Database was specified."}});
        } else if (type == DropTable) {
            if (!TableManager::getInstance()->dropTable(ch[0]->strVal))
                return json({{"result", "There's no this table."}});
        } else if (type == DesribeTable) {
            if (!TableManager::getInstance()->hasDB())
                return json({{"result", "No Database was specified."}});
            auto td = TableManager::getInstance()->getTableDescription(ch[0]->strVal);
            if (td == nullptr) return json({{"result", "There's no this table."}});
            return json({{"result", {{"name", td->col_name}, {"type", td->col_type}, {"pattern", td->pattern},
                                            {"not null", td->col_not_null}, {"unique", td->col_unique}, {"has index", td->col_has_index}, {"width", td->col_width}}}});
        } else if (type == InsertItem) {
            if (!TableManager::getInstance()->hasDB())
                return json({{"result", "No Database was specified."}});
            auto vlists = dynamic_pointer_cast<ValueLists>(ch[1]->getNode());
            auto td = TableManager::getInstance()->getTableDescription(ch[0]->strVal);
            for (auto &vlist : vlists->vec) {
                if (vlist->vec.size() != td->col_name.size()) return json({{"result", "Wrong Dimension."}});
                vector<string> item;
                for (int i = 0; i < vlist->vec.size(); i++) {
                    string v_str(td->pattern[i] > 0 ? td->pattern[i] : 1, 0);
                    auto v = vlist->vec[i];
                    if(td->col_type[i] != v->type && v->type != "NULL")
                        throw TypeError("TypeError!",td->col_type[i], v->type);
                    if (v->type == "NULL")
                        *(v_str.end() - 1) = 1;
                    else if (v->type == "int")
                        v_str.replace(v_str.begin(), v_str.begin() + 4,
                                      string((char *) &(v->iVal), (char *) &(v->iVal) + 4));
                    else if (v->type == "varchar")
                        v_str = v->strVal + string(1, 0);
                    item.push_back(v_str);
                    //TODO special check
                }
                if (!td->insertInTable(item))
                    return json({{"result", "Insert failed."}});
            }
            return json({{"result", "Succeed."}});
        } else if (type == UpdateItem) {
            if (!TableManager::getInstance()->hasDB())
                return json({{"result", "No Database was specified."}});
            vector<string> tables({ch[0]->strVal});
            bool can_index;
            int col_offset;
            string v;
            auto where = dynamic_pointer_cast<WhereClause>(ch[2]->getNode());
            string table_name = where->getNextAssignTableName(can_index, col_offset, v, tables);
            auto checker = where->getChecker(table_name);
            auto td = TableManager::getInstance()->getTableDescription(table_name);
            Changer changer = dynamic_pointer_cast<SetClause>(ch[1]->getNode())->getChanger(table_name);
            vector <Item> deleted_items;
            if (can_index) {
                deleted_items = td->deleteAndCollectUseIndex(col_offset, v, checker);
                td->updateItems(deleted_items, changer);
            } else {
                deleted_items = td->deleteAndCollectItems(checker);
                td->updateItems(deleted_items, changer);
            }
            return json({{"result", "Finished."}});
        } else if (type == DeleteItem) {
            if (!TableManager::getInstance()->hasDB())
                return json({{"result", "No Database was specified."}});
            vector<string> tables({ch[0]->strVal});
            bool can_index;
            int col_offset;
            string v;
            auto where = dynamic_pointer_cast<WhereClause>(ch[1]->getNode());
            string table_name = where->getNextAssignTableName(can_index, col_offset, v, tables);
            auto checker = where->getChecker(table_name);
            auto td = TableManager::getInstance()->getTableDescription(table_name);
            vector<Item> res;
            if (can_index) {
                res = td->deleteAndCollectUseIndex(col_offset, v, checker);
            } else
                res = td->deleteAndCollectItems(checker);
            return json({{"result", res}});
        } else if (type == SelectItem) {
            if (!TableManager::getInstance()->hasDB())
                return json({{"result", "No Database was specified."}});
            vector<string> tables = dynamic_pointer_cast<TableList>(ch[1]->getNode())->tables;
            auto where = dynamic_pointer_cast<WhereClause>(ch[2]->getNode());
            auto scols = dynamic_pointer_cast<SelectCols>(ch[0]->getNode());
            auto selector = scols->getSelector(tables);
            vector<vector<Value> > ans;
            vector<string> assigned_tables(tables.size(), "");
            where->dfs(ans, tables, vector<Value>(), selector, assigned_tables);
            cout << "Select Finished! Dumping results to JSON..." <<endl;
            vector<string> assigned_cols;
            for (int k = (int) assigned_tables.size() - 1; k >= 0; k--) {
                auto ts = assigned_tables[k];
                if (scols->isAll) {
                    auto td = TableManager::getInstance()->getTableDescription(ts);
                    for (auto &col : td->col_name)
                        assigned_cols.push_back(td->name + "." + col);
                } else
                    for (auto &name : scols->col_list->cols) {
                        string table, col;
                        ColList::split(name, tables[0], table, col);
                        if (table == ts)
                            assigned_cols.push_back(name);
                    }
            }
            json ret;
            int t = 0;
            for (int i = 0; i < assigned_cols.size(); i++) {
                string table, col;
                ColList::split(assigned_cols[i], "", table, col);
                auto td = TableManager::getInstance()->getTableDescription(table);
                auto a = json::array();
                int j = td->getOffset(col) + t;
                for (auto &item : ans) {
                    if (item[j].type == "int")
                        a.push_back(item[j].iVal);
                    else if (item[j].type == "varchar")
                        a.push_back(item[j].strVal);
                    else if (item[j].type == "NULL")
                        a.push_back("NULL");
                    else
                        BOOST_ASSERT(0);
                }
                t += td->col_name.size();
                ret["result"][assigned_cols[i]] = a;
            }
            return ret;

        } else if (type == CreateIdx) {
            if (!TableManager::getInstance()->hasDB())
                return json({{"result", "No Database was specified."}});
            // 1. has index
            string tableName = ch[0]->strVal;
            string colName = ch[1]->strVal;
            auto indexName = TableManager::createIndexName(tableName, colName);
            auto td = TableManager::getInstance()->getTableDescription(tableName);
            BOOST_ASSERT(td != nullptr);
            bool colFound = false;
            int colId = 0;
            for (int i = 0; i < td->col_name.size(); ++i) {
                if (td->col_name[i] == colName) {
                    if (td->col_has_index[i]) {
                        return json({{"result", "Index already exists."}});
                    }
                    td->col_has_index[i] = true;
                    // 2. write back
                    colFound = true;
                    colId = i;
                    break;
                }
            }
            if (!colFound) {
                return json({{"result", "Column name does not exists."}});
            }

            // 3. create index file
            TheBTree::BT::setupBTree(indexName);
            shared_ptr<TheBTree> btree(new TheBTree(indexName));

            // 4. select and insert index
            td->traverseWithLocation([btree, colId](const Item &item, Location loc) -> void {
                btree->insert(std::hash<std::string>()(item[colId]), btree->locationToString(loc), false);
            });

        } else if (type == DropIdx) {
            if (!TableManager::getInstance()->hasDB())
                return json({{"result", "No Database was specified."}});
            string tableName = ch[0]->strVal;
            string colName = ch[1]->strVal;
            auto indexName = TableManager::createIndexName(tableName, colName);
            auto td = TableManager::getInstance()->getTableDescription(tableName);
            BOOST_ASSERT(td != nullptr);
            bool colFound = false;
            for (int i = 0; i < (int) td->col_name.size(); ++i) {
                if (td->col_name[i] == colName) {
                    if (!td->col_has_index[i]) {
                        return json({{"result", "Index does not exists."}});
                    }
                    td->col_has_index[i] = false;
                    // 2. write back
                    colFound = true;
                    break;
                }
            }
            if (!colFound) {
                return json({{"result", "Column name does not exists."}});
            }
            FileUtils fu;
            fu.deleteFile((TableManager::getInstance()->base_dir + "/" + indexName).c_str());
            return json({{"result", "Success"}});
        }
    }catch(TypeError e)
    {
        return json({{"result", e.toString()}});
    }catch(NoThisColumnError e){
        return json({{"result", e.toString()}});
    }
    return nullptr;
}

Statement::~Statement() { }

Statement::Statement(Statement::Type type) :type(type) { }

TableManagement::~TableManagement() { }

TableManagement::TableManagement(Statement::Type type) :Statement(type) { }

Field::~Field() {}

