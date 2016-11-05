//
// Created by chord on 16/10/14.
//

#ifndef TINYDBPP_TABLEMANAGER_H
#define TINYDBPP_TABLEMANAGER_H
#include <string>
#include <config.h>
#include <Pager.h>
#include <vector>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <FileUtils.h>

namespace tinydbpp {

        struct TableDescription {
            int len;
            /*
             * if pattern[x] > 0, it represents a fixed length of pattern[x] bytes
             * if pattern[x] == -1, it represents a varying length which the first 4 bytes is a int representing length
             */
            std::vector<int> pattern;
            std::string name, path;
            std::shared_ptr<Pager> my_pager;
            TableDescription(): len(0), my_pager(nullptr){}
            std::string getPath(){return path;}
            void addPattern(int x);
            std::shared_ptr<Pager> getPager(Pager::OpenFlag flag = Pager::ReadWrite);
            std::vector<std::string> read(char* buf, int len, int& now, bool fixed);
            std::string embed(const std::vector<std::string> list, bool & fixed_res);
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
        bool changeDB(std::string db, bool auto_create = true);
        std::shared_ptr<TableDescription> getTableDescription(std::string);
        bool buildTable(std::string name, std::function<void(Pager *)> callback = nullptr);
    };


}

#endif //TINYDBPP_TABLEMANAGER_H
