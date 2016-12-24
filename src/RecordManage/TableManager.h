//
// Created by chord on 16/10/14.
//

#ifndef TINYDBPP_TABLEMANAGER_H
#define TINYDBPP_TABLEMANAGER_H
#include <string>
#include <config.h>
#include <Pager/Pager.h>
#include <vector>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <FileUtils.h>
#include <Pager/Page.h>
#include <functional>
namespace tinydbpp {
        class TheBTree;
        typedef std::vector< std::string> Item;
        typedef std::function<bool(const Item &)> Checker;
        typedef std::function<void(Item &)> Changer;
        struct TableDescription {
            int len;
            /*
             * if pattern[x] > 0, it represents a fixed length of pattern[x] bytes
             * if pattern[x] == -1, it represents a varying length which the first 4 bytes is a int representing length
             */
            std::vector<int> pattern, col_not_null, col_unique, col_has_index, col_width;
            std::vector<std::string> col_name, col_type;
            std::string name, path;
            std::shared_ptr<Pager> my_pager;
            TableDescription(): len(0), my_pager(nullptr){}
            std::string getPath(){return path;}
            void addPattern(int x);
            std::shared_ptr<Pager> getPager(Pager::OpenFlag flag = Pager::ReadWrite);
            Item read(char* buf, int len, int& now, bool fixed);
            std::string embed(const Item & list, bool & fixed_res);
            std::shared_ptr<TheBTree> getIndex(int offset);
            int getOffset(const std::string&);
            bool insertInTable(const Item& item);
            std::vector< Item > selectUseIndex(int offset, std::string v,const Checker &checker = nullptr);
            std::vector< Item > deleteAndCollectItems(const Checker &checker);
            void updateItems(Checker &checker,Changer &changer);
            std::vector< Item > selectUseChecker(Checker &checker);
        };

    class TableManager {
        static TableManager *ins;
        TableManager(){}
        struct Garbo{
            Garbo(){}
            ~Garbo(){
                delete TableManager::getInstance();
            }
        };
        static Garbo garbo;
    public:
        std::vector<std::shared_ptr<TableDescription>> table_map;
        static std::string base_dir;
        std::string dbname;
        std::string dir;
        static TableManager *getInstance() {
            if (!ins) {
                ins = new TableManager();
                if(base_dir == "") {
                    base_dir = DEFAULT_DATABASE_DIR;
                }
                FileUtils::createDir(base_dir.c_str());
                ins->dbname = "";
                ins->changeDB(TEST_DATABASE_NAME, true);//TODO delete it, this is just for compatibility of previous unit tests
                return ins;
            } else return ins;
        }
        static bool setBaseDir(std::string _dir){
            if(ins) return false;
            base_dir = _dir;
            return true;
        }
        bool hasDB(){return getInstance()->dbname != "";}
        bool changeDB(std::string db, bool auto_create = true);
        void createDB(std::string db);
        std::shared_ptr<TableDescription> getTableDescription(std::string);
        bool buildTable(std::string name, std::function<void(Pager *)> callback = nullptr);
        bool dropTable(std::string name);
        bool DropDB(std::string db);
        static std::string createIndexName(const std::string &tableName, const std::string &colName);
        static bool parseIndex(const std::string &indexName, const std::string &tableName, std::string &colName);
    };

}

#endif //TINYDBPP_TABLEMANAGER_H
