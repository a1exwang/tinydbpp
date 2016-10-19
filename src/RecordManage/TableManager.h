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
            TableDescription():my_pager(nullptr), len(0){}
            //path after DEFAULT_DATABASE_DIR
            std::string getPath(){
                return path;
            }
            void addPattern(int x){
                pattern.push_back(x);
                if(x == -1) len += 4 + DEFAULT_VARCHAR_LEN;
                else len += x;
            }
            std::shared_ptr<Pager> getPager(Pager::OpenFlag flag = Pager::ReadWrite);
            std::vector<std::string> read(char* buf, int len, int& now, bool fixed);
            std::string embed(const std::vector<std::string> list, bool & fixed_res);
    };

    class TableManager {
        static TableManager *ins;
        TableManager():dbtable(nullptr) {
        }

    public:
        std::vector<std::shared_ptr<TableDescription>> table_map;
        static std::string dir;
        std::string dbname;
        std::shared_ptr<Pager> dbtable;
        static TableManager *getInstance() {
            if (!ins) {
                ins = new TableManager();
                if(dir == "") {
                    //TODO create directory
                    dir = DEFAULT_DATABASE_DIR;
                }
                ins->dbtable = std::shared_ptr<Pager>(new Pager(dir + "/" + SYS_TABLE_NAME, Pager::ReadWrite));
                return ins;
            } else return ins;
        }
        static bool setDir(std::string _dir){
            if(ins) return false;
            dir = _dir;
            return true;
        }
        void changeDB(std::string db);
        std::shared_ptr<TableDescription> getTableDescription(std::string);
        bool buildTable(std::string);
        bool isExist(std::string);
    };
}

#endif //TINYDBPP_TABLEMANAGER_H
