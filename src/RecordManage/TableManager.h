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
        TableDescription(){
            len = 0;
        }
        /*
         * if pattern[x] > 0, it represents a fixed length of pattern[x] bytes
         * if pattern[x] == -1, it represents a varying length which the first 4 bytes is a int representing length
         */
        std::vector<int> pattern;
        //path after DEFAULT_DATABASE_DIR
        std::string name;
        std::string getRelativePath(){
            return name;
        }
        void addPattern(int x){
            pattern.push_back(x);
            if(x == -1) len += 4 + DEFAULT_VARCHAR_LEN;
        }
        std::shared_ptr<Pager> my_pager;
        std::shared_ptr<Pager> getPager(Pager::OpenFlag flag = Pager::ReadWrite);
        std::vector<std::string> read(char* buf, int len, int& now){
            auto ret = std::vector<std::string>();
            for(int x : pattern){
                if(x > 0){
                    BOOST_ASSERT(now + x <= len);
                    ret.push_back(std::string(buf + now, x));
                    now += x;
                }else if(x == -1){
                    BOOST_ASSERT(now + 4 <= len);
                    int next_len = *(int*)(std::string(buf + now, 4).c_str());
                    now += 4;
                    BOOST_ASSERT(now + next_len <= len);
                    ret.push_back(std::string(buf + now, next_len));
                }else
                    BOOST_ASSERT(0);
            }
        }
    };

    class TableManager {
        static TableManager *ins = NULL;
        TableManager() {
        }

    public:
        std::vector<std::shared_ptr<TableDescription>> table_map;
        static std::string dir;
        std::string dbname;
        std::shared_ptr<Pager> dbtable;
        static TableManager *getInstance() {
            if (!ins) {
                ins = new TableManager();
                if(dir == "")
                    dir = DEFAULT_DATABASE_DIR;
                ins->dbtable = new Pager(dir + "/" + SYS_TABLE_NAME, Pager::ReadWrite);
                return ins;
            } else return ins;
        }
        bool setDir(std::string _dir){
            if(ins) return false;
            dir = _dir;
            return true;
        }
        void changeDB(std::string db);
        std::shared_ptr<TableDescription> getTableDescription(std::string);

        bool isExist(std::string basic_string);
    };
}

#endif //TINYDBPP_TABLEMANAGER_H
