//
// Created by chord on 16/10/14.
//

#ifndef TINYDBPP_RecordMANAGER_H
#define TINYDBPP_RecordMANAGER_H


#include <cstdlib>
#include "TableManager.h"
#include <functional>
#include <string>
#include <vector>

namespace tinydbpp {
    struct Location {
        int pageNumber;
        int loc;
        Location(int pageNumber, int loc) : pageNumber(pageNumber), loc(loc) {}

        std::string toString() const;
        bool operator==(const Location &rhs) const;
        bool operator!=(const Location &rhs) const;
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
        void updateOneRecord(const std::string &table_name, std::vector<std::string> &vec,
                             int pageID, int now);
        Location updateOneRecord(const std::string &table_name, Location loc, const std::string &res, bool fixed_res);

        void del(const std::string &table_name, std::function<bool(const std::vector<std::string>&)> &c,
                 std::function<void(const std::vector<std::string>&)>&);
        void delOneRecord(const std::string &table_name, int pageID, int now, bool fixed);

        void select(const std::string &table_name, std::function<bool(const std::vector<std::string>&)> &c,
                    std::function<void(std::vector<std::string>&, int, int)>&);
        std::string getRecord(const std::string &table_name, Location loc) const;
        void deleteRecord(const std::string &table_name, Location loc);
        void updateRecordNoResize(const std::string &table_name, Location loc, std::function<bool (std::string &record)> callback) const;
    };

}
#endif //TINYDBPP_RecordMANAGER_H
