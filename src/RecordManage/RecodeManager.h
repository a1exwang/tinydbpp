//
// Created by chord on 16/10/14.
//

#ifndef TINYDBPP_RECODEMANAGER_H
#define TINYDBPP_RECODEMANAGER_H


#include <cstdlib>
#include <TableManager.h>
#include <functional>
#include <string>
#include <vector>

namespace tinydbpp {
    struct Location {
        int loc;
        int pageNumber;

        Location(int _a, int _b) : pageNumber(_a), loc(_b) {}
    };

    class RecodeManager {
        static RecodeManager *ins = NULL;

        RecodeManager() {}

    public:
        static RecodeManager *getInstance() {
            if (!ins) return ins = new RecodeManager();
            else return ins;
        }

        Location insert(const std::string &table_name, const std::string &record, bool);
        Location* tryInsert(std::shared_ptr<Page>, const std::string &record, bool fixed);
        void update(const std::string &table_name, std::function<bool(std::vector<std::string>)> &c,
                    std::function<void(std::vector<std::string>, char *)>);

        void del(const std::string &table_name, std::function<bool(std::vector<std::string>)> &c,
                 std::function<void(std::vector<std::string>, char *)>);

        void select(const std::string &table_name, std::function<bool(std::vector<std::string>)> &c,
                    std::function<void(std::vector<std::string>, char *)>);
    };

}
#endif //TINYDBPP_RECODEMANAGER_H
