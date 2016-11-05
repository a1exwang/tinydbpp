//
// Created by chord on 16/10/14.
//

#include <Pager.h>
#include <Page.h>
#include <iostream>
#include <sstream>
#include <boost/log/trivial.hpp>
#include "RecordManager.h"
#include "TableManager.h"

using namespace std;
namespace tinydbpp {

    std::string Location::toString() const {
        std::stringstream ss;
        ss << "(" << this->pageNumber << ", " << this->loc << ")";
        return ss.str();
    }

    bool Location::operator==(const Location &rhs) const {
        return this->pageNumber == rhs.pageNumber && this->loc == rhs.loc;
    }
    bool Location::operator!=(const Location &rhs) const {
        return !this->operator ==(rhs);
    }

    RecordManager * RecordManager::ins = nullptr;

    Location* RecordManager::tryInsert(shared_ptr<Page> data_page, const std::string &record, bool fixed){
        char* data = data_page->getBuf();
        if(fixed){
            short& free_loc = *(short*)data;
            if(free_loc == 0) {
                data_page->releaseBuf(data);
                return NULL;
            }
            short len = *(short*)(data + free_loc + 1);
            if(len < (short)record.length()) {
                BOOST_ASSERT(free_loc + 1 + record.length() >= PAGER_PAGE_SIZE);
                data_page->releaseBuf(data);
                return NULL;
            }
            short this_loc = free_loc;
            //del from free block list
            if(len - record.length() < 1)
                free_loc = *(short*)(data + this_loc + 3);
            else{
                free_loc = this_loc + (short)1 + (short)record.length();
                //unnecessary
                *(data + free_loc) = 0;
                *(short*)(data + free_loc + 3) = *(short*)(data + this_loc + 3);//next
                *(short*)(data + free_loc + 1) = len - (short)1 - (short)record.length();//length
            }
            memcpy(data + this_loc + 1, record.c_str(), record.length());
            *(data + this_loc) = 1;
            data_page->markDirty();
            data_page->releaseBuf(data);
            return new Location(data_page->getID(), this_loc);
        }else{
            int now = 0;
            while(now + 3 < (int)PAGER_PAGE_SIZE){
                short len = *(short*)(data + now + 1);
                if(*(data + now) == 0 && (len >= record.length() + 3 || len == record.length())){
                    memcpy(data + now + 3, record.c_str(), record.length());
                    *(short*)(data + now + 1) = (short)record.length();
                    *(data + now) = 1;
                    if(len >= record.length() + 3) {
                        int next = now + 3 + (int)record.length();
                        *(data + next) = 0;
                        *(short *) (data + next + 1) = len - (short) record.length() - (short) 3;
                    }
                    data_page->markDirty();
                    data_page->releaseBuf(data);
                    return new Location(data_page->getID(), now);
                }else{
                    now += 3 + len;
                }
            }
            data_page->releaseBuf(data);
            return NULL;
        }
    }
    Location RecordManager::insert(const std::string &table_name, const std::string &record, bool fixed) {
        Location ret(-1,0);
        auto td = TableManager::getInstance()->getTableDescription(table_name);
        BOOST_ASSERT_MSG(td != nullptr, "RecordManager::insert(), maybe you type the wrong db name.");
        shared_ptr<Pager> ptr = td->getPager();
        shared_ptr<Page> dic_page = ptr->getPage(1);
        char* dic = dic_page->getBuf();
        int pages = ptr->getValidPageCount();
        for(int i = 1, index = 0; i < pages; i++){
            if(index == PAGER_PAGE_SIZE){
                dic_page->releaseBuf(dic);
                dic_page = ptr->getPage(i);
                dic = dic_page->getBuf();
                index = 0;
                continue;
            }
            char state = dic[index];
            if( (fixed && (state & 1) == 0 && (state & 2) == 0) ||
                (!fixed && (state & 1) == 1 && (state & 2) == 0) )
            {
                Location* result = tryInsert(ptr->getPage(i), record, fixed);
                if(result == NULL){
                    dic[index] |= 2;
                    dic_page->markDirty();
                }else{
                    ret = *result;
                    dic_page->releaseBuf(dic);
                    return ret;
                }
            }
            index++;
        }
        dic_page->releaseBuf(dic);
        // store in a new page
        if(pages % (PAGER_PAGE_SIZE + 1) == 1) // if next page is a dictionary page, skip
            pages ++;
        shared_ptr<Page> new_page = ptr->getPage(pages);
        char* new_buf = new_page->getBuf();
        if(!fixed){
            dic_page = ptr->getPage(((pages - 1) / PAGER_PAGE_SIZE) * PAGER_PAGE_SIZE + 1); // find it's dictionary page
            dic = dic_page->getBuf();
            dic[(pages - 1) % PAGER_PAGE_SIZE - 1] = 1; // find the position is dictionary page
            dic_page->markDirty();
            dic_page->releaseBuf(dic);
            *(new_buf) = 0;
            *(short*)(new_buf + 1) = (short)(PAGER_PAGE_SIZE - 3);
        }else{
            *(short*)new_buf = (short)2;
            *(new_buf + 2) = 0;
            *(short*)(new_buf + 3) = (short)(PAGER_PAGE_SIZE - 7);
            *(short*)(new_buf + 5) = (short)0;
            new_page->markDirty();
        }
        new_page->releaseBuf(new_buf);
        ret = *tryInsert(new_page, record, fixed);
        return ret;
    }
    void RecordManager::update(const std::string &table_name, std::function<bool(const std::vector<std::string>&)> &checker,
                std::function<void(std::vector<std::string>&)>& solver){
        BOOST_ASSERT(checker);
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(table_name);
        BOOST_ASSERT_MSG(td != nullptr, "RecordManager::update(), maybe you type the wrong db name.");
        shared_ptr<Pager> ptr = td->getPager();
        std::function<void(std::vector<std::string>&, int, int)> update_func = [&solver, &td, &table_name, &ptr](std::vector<std::string>& vec, int pageID, int now){
            solver(vec);
            bool fixed_res;
            string res = td->embed(vec, fixed_res);
            shared_ptr<Page> dic_page = ptr->getPage(((pageID - 1) / PAGER_PAGE_SIZE) * PAGER_PAGE_SIZE + 1); // find it's dictionary page
            char * dic = dic_page->getBuf();
            bool fixed = (dic[(pageID - 1) % PAGER_PAGE_SIZE - 1] & 1) == 0;
            dic_page->releaseBuf(dic);
            if(fixed && fixed_res){
                shared_ptr<Page> p = ptr->getPage(pageID);
                char * data = p->getBuf();
                memcpy(data + now + 1, res.c_str(), res.length());
                p->markDirty();
                p->releaseBuf(data);
            }else {
                RecordManager::getInstance()->delOneRecord(table_name, pageID, now, fixed);
                RecordManager::getInstance()->insert(table_name, res, fixed_res);
            }
            return;
        };
        select(table_name, checker, update_func);
        return;
    }
    void RecordManager::delOneRecord(const std::string &table_name, int pageID, int now, bool fixed){
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(table_name);
        BOOST_ASSERT_MSG(td != nullptr, "RecordManager::delOneRecord(), maybe you type the wrong db name.");
        shared_ptr<Page> p = td->getPager()->getPage(pageID);
        char * data = p->getBuf();
        if(fixed){
            data[now] = 0;
            *(short*)(data + now + 1) = (short)td->len;
            *(short*)(data + now + 3) = *(short*)data;
            *(short*)data = (short)now;
        }else{
            data[now] = 0;
        }
        p->markDirty();
        p->releaseBuf(data);
    }

    void RecordManager::del(const std::string &table_name, std::function<bool(const std::vector<std::string>&)> &checker,
             std::function<void(const std::vector<std::string>&)>& solver){
        BOOST_ASSERT(checker);
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(table_name);
        shared_ptr<Pager> ptr = td->getPager();
        std::function<void(std::vector<std::string>&, int, int)> del_func = [&solver, &table_name, &ptr](std::vector<std::string>& vec, int pageID, int now){
             if (solver)
               solver(vec);
             shared_ptr<Page> dic_page = ptr->getPage(((pageID - 1) / PAGER_PAGE_SIZE) * PAGER_PAGE_SIZE + 1); // find it's dictionary page
             char * dic = dic_page->getBuf();
             bool fixed = (dic[(pageID - 1) % PAGER_PAGE_SIZE - 1] & 1) == 0;
             RecordManager::getInstance()->delOneRecord(table_name, pageID, now, fixed);
             if((dic[pageID - 2] & 2) == 1)
                dic[pageID -2] ^= 2;
             dic_page->markDirty();
             dic_page->releaseBuf(dic);
             return;
         };
         select(table_name, checker, del_func);
         return;
    }

    /**
     * Traverse all records and call `solver` on the records on which checker returns true.
     * O(N), N = record count.
     *
     * Let's talk about page & record format:
     *
     * Page 0 ??
     *
     * Page 1 is Dictionary page.
     *   This page stores page attributes.
     *   If dic[i] & 1 == true, page i+2 is a fixed-length page(a page contains only fixed-length record).
     *   Otherwize, page i+2 is a variable-length page.
     *
     * Page n (n >= 2)
     *   These pages store records.
     *   For pages of fixed-length records.
     *      +00 uint16  firstFreeRecordOffset   // Offset in page of the first free record slot
     *      +02 Record  records[M]              // Data.
     *
     * Record
     *  Fixed-length record
     *      +00 uint8 attribute                 //
     *
     * @param table_name
     * @param checker: Required, check if the record should be selected.
     * @param solver: Optional, solver is called for each selected records.
     */
    void RecordManager::select(const std::string &table_name, std::function<bool(const std::vector<std::string>&)> &checker,
             std::function<void(std::vector<std::string>&, int, int)> & solver) {
        BOOST_ASSERT(checker);
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(table_name);
        BOOST_ASSERT_MSG(td != nullptr, "RecordManager::select(), maybe you type the wrong db name.");
        shared_ptr<Pager> ptr = td->getPager();
        int pages = ptr->getValidPageCount();
        shared_ptr<Page> dic_page = ptr->getPage(1);
        char * dic = dic_page->getBuf();
        for(int i = 2, index = 0;i < pages;i++){
            if(index == PAGER_PAGE_SIZE){
                dic_page->releaseBuf(dic);
                dic_page = ptr->getPage(i);
                dic = dic_page->getBuf();
                index = 0;
                continue;
            }
            shared_ptr<Page> p = ptr->getPage(i);
            bool fixed = (dic[index] & 1) == 0;
            char * data = p->getBuf();
            int now = fixed? 2 : 0;
            while(now < (int)PAGER_PAGE_SIZE){
                if(data[now] == 0)//free
                    now += *(short*)(data + now + 1) + (fixed ? 5 : 3);
                else{//used
                    int this_loc = now;
                    now += fixed ? 1: 3;
                    vector<string> parsed_data = td->read(data, PAGER_PAGE_SIZE, now, fixed);
                    if(checker(parsed_data) && solver)
                        solver(parsed_data, i, this_loc);
                }
            }
            p->releaseBuf(data);
            index ++;
        }
        dic_page->releaseBuf(dic);
    }

    std::string RecordManager::getRecord(const std::string &table_name, Location loc) const {
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(table_name);
        BOOST_ASSERT_MSG(td != nullptr, "RecordManager::select(), maybe you type the wrong db name.");
        shared_ptr<Pager> ptr = td->getPager();
        BOOST_ASSERT(ptr->getValidPageCount() > (unsigned)loc.pageNumber);
        BOOST_ASSERT(loc.pageNumber > 0);
        shared_ptr<Page> dic_page = ptr->getPage(((loc.pageNumber - 1) / PAGER_PAGE_SIZE) * PAGER_PAGE_SIZE + 1);
        char * dic = dic_page->getBuf();
        shared_ptr<Page> p = ptr->getPage((unsigned)loc.pageNumber);
        // FIXME: `pages` may be greater than PAGER_PAGE_SIZE, which will cause a segmentation fault!
        //BOOST_ASSERT(loc.pageNumber - 2 < (int)PAGER_PAGE_SIZE);
        bool fixed = (dic[(loc.pageNumber - 1) % PAGER_PAGE_SIZE - 1] & 1) == 0;
        dic_page->releaseBuf(dic);
        char * data = p->getBuf();
        int now = loc.loc;
        BOOST_ASSERT_MSG(data[now] != 0, "Record should not be a free page");//free
        // TODO: currently, we don't support fixed length record.
        BOOST_ASSERT(!fixed);
        uint16_t length = *(uint16_t*)(data + loc.loc + 1);
        BOOST_ASSERT(loc.loc + 3 + length < (int)PAGER_PAGE_SIZE);
        auto ret = string(data + loc.loc + 3, length);
        p->releaseBuf(data);
        return ret;
    }

    void RecordManager::updateRecordNoResize(const std::string &table_name, Location loc,
                                             std::function<bool(std::string &record)> callback) const {

        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(table_name);
        BOOST_ASSERT_MSG(td != nullptr, "RecordManager::select(), maybe you type the wrong db name.");
        shared_ptr<Pager> ptr = td->getPager();
        BOOST_ASSERT(ptr->getValidPageCount() > (unsigned)loc.pageNumber);
        BOOST_ASSERT(loc.pageNumber > 0);
        shared_ptr<Page> dic_page = ptr->getPage(((loc.pageNumber - 1) / PAGER_PAGE_SIZE) * PAGER_PAGE_SIZE + 1);
        char * dic = dic_page->getBuf();
        shared_ptr<Page> p = ptr->getPage((unsigned)loc.pageNumber);
        // FIXME: `pages` may be greater than PAGER_PAGE_SIZE, which will cause a segmentation fault!
        //BOOST_ASSERT(loc.pageNumber - 2 < (int)PAGER_PAGE_SIZE);
        bool fixed = (dic[(loc.pageNumber - 1) % PAGER_PAGE_SIZE - 1] & 1) == 0;
        dic_page->releaseBuf(dic);
        char * data = p->getBuf();
        int now = loc.loc;
        BOOST_ASSERT_MSG(data[now] != 0, "Record should not be a free page");//free
        // TODO: currently, we don't support fixed length record.
        BOOST_ASSERT(!fixed);
        uint16_t length = *(uint16_t*)(data + loc.loc + 1);
        BOOST_ASSERT(loc.loc + 3 + length < (int)PAGER_PAGE_SIZE);
        auto record = string(data + loc.loc + 3, length);
        if (callback(record)) {
            BOOST_ASSERT_MSG(record.size() == length, "This function does not support resize!");
            memcpy(data + loc.loc + 3, record.c_str(), length);
            p->markDirty();
        }
        p->releaseBuf(data);
    }

}

