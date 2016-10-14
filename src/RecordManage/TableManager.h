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
        }
        std::shared_ptr<Pager> my_pager;
        std::shared_ptr<Pager> getPager(Pager::OpenFlag flag = Pager::ReadWrite){
            if(my_pager == nullptr)
                return my_pager = new Pager(std::string(DEFAULT_DATABASE_DIR) + getRelativePath(), flag);
            else return my_pager;
        }
        std::vector<std::string> read(char* buf, int len, int& now){
            auto ret = std::vector<std::string>();
            for(int x : pattern){
                if(x > 0){
                    BOOST_ASSERT(now + x <= len);
                    ret.push_back(std::string(buf + now, x));
                    now += x;
                }else if(x == -1){
                    BOOST_ASSERT(now + 4 <= len);
                    int next_len = atoi(std::string(buf + now, 4));
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
        static std::vector<std::shared_ptr<TableDescription>> table_map;
        static TableDescription sysTableDescription;
        static TableManager *getInstance() {
            if (!ins) {
                sysTableDescription.name = SYS_TABLE_NAME;
                sysTableDescription.addPattern(20); //table_name & path can be deduced
                sysTableDescription.addPattern(-1); //condensed define of columns
                return ins = new TableManager();
            } else return ins;
        }

        std::shared_ptr<TableDescription> getTableDescription(std::string);
    };
}

#endif //TINYDBPP_TABLEMANAGER_H
