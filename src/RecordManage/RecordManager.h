//
// Created by chord on 16/10/14.
//

#ifndef TINYDBPP_RecordMANAGER_H
#define TINYDBPP_RecordMANAGER_H


#include <cstdlib>
#include <TableManager.h>
#include <functional>
#include <string>
#include <vector>

namespace tinydbpp {
    struct Location {
        int pageNumber;
        int loc;
        Location(int _a, int _b) : pageNumber(_a), loc(_b) {}
    };

    class RecordManager {
        static RecordManager *ins;

        RecordManager() {}

    public:
        static RecordManager *getInstance() {
            if (!ins) return ins = new RecordManager();
            else return ins;
        }

        Location insert(const std::string &table_name, const std::string &record, bool);
        Location* tryInsert(std::shared_ptr<Page>, const std::string &record, bool fixed);
        void update(const std::string &table_name, std::function<bool(const std::vector<std::string>&)> &c,
                    std::function<void(std::vector<std::string>&)>&);

        void del(const std::string &table_name, std::function<bool(const std::vector<std::string>&)> &c,
                 std::function<void(const std::vector<std::string>&)>&);
        void delOneRecord(const std::string &table_name, int pageID, int now, bool fixed);

        void select(const std::string &table_name, std::function<bool(const std::vector<std::string>&)> &c,
                    std::function<void(std::vector<std::string>&, int, int)>&);
    };

}
#endif //TINYDBPP_RecordMANAGER_H
