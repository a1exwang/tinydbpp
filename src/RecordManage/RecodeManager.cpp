//
// Created by chord on 16/10/14.
//

#include <Pager.h>
#include <Page.h>
#include <FileUtils.h>
#include "RecodeManager.h"
#include "TableManager.h"

using namespace std;
namespace tinydbpp {
    Location* RecodeManager::tryInsert(shared_ptr<Page> data_page, const std::string &record, bool fixed){
        char* data = data_page->getBuf();
        if(fixed){
            short& free_loc = *(short*)data;
            if(free_loc == 0) {
                data_page->releaseBuf(data);
                return NULL;
            }
            short len = *(short*)(data + free_loc + 1);
            if(len < record.length()) {
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
            strcpy(data + this_loc + 1, record.c_str());
            *(data + this_loc) = 1;
            data_page->markDirty();
            data_page->releaseBuf(data);
            return new Location(data_page->getID(), this_loc);
        }else{
            int now = 0;
            while(now + 3 < PAGER_PAGE_SIZE){
                short len = *(short*)(data + now + 1);
                if(*(data + now) == 0 && (len >= record.length() + 3 || len == record.length())){
                    strcpy(data + now + 3, record.c_str());
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
    Location RecodeManager::insert(const std::string &table_name, const std::string &record, bool fixed) {
        Location ret(-1,0);
        shared_ptr<Pager> ptr = TableManager::getInstance()->getTableDescription(table_name)->getPager();
        shared_ptr<Page> dic_page = ptr->getPage(1);
        char* dic = dic_page->getBuf();
        int pages = FileUtils::filePages(ptr->getFD());
        for(int i = 2; i < pages; i++){
            char state = dic[i - 2];
            if( (fixed && (state & 1) == 0 && (state & 2) == 0) ||
                (!fixed && (state & 1) == 1 && (state & 2) == 0) )
            {
                Location* result = tryInsert(ptr->getPage(i), record, fixed);
                if(result == NULL){
                    dic[i - 2] |= 2;
                    dic_page->markDirty();
                }else{
                    ret = *result;
                    break;
                }
            }
        }
        //TODO new page
        shared_ptr<Page> new_page = nullptr;
        char* new_buf = new_page->getBuf();
        if(!fixed){
            dic[pages - 1] = 1;
            dic_page->markDirty();
            *(new_buf) = 0;
            *(short*)(new_buf + 1) = (short)(PAGER_PAGE_SIZE - 3);
        }else{
            *(short*)new_buf = (short)2;
            *(new_buf + 2) = 0;
            *(short*)(new_buf + 3) = (short)(PAGER_PAGE_SIZE - 7);
            *(short*)(new_buf + 5) = (short)0;
        }
        new_page->releaseBuf(new_buf);
        ret = *tryInsert(new_page, record, fixed);
        dic_page->releaseBuf(dic);
        return ret;
    }
    void RecodeManager::update(const std::string &table_name, std::function<bool(const std::vector<std::string>&)> &checker,
                std::function<void(std::vector<std::string>&)>& solver){
        auto rm = this;
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(table_name);
        shared_ptr<Pager> ptr = td->getPager();
        shared_ptr<Page> dic_page = ptr->getPage(1);
        char * dic = dic_page->getBuf();
        std::function<void(std::vector<std::string>&, int, int)> update_func = [&solver, &rm, &td, &table_name, &ptr, &dic_page, &dic](std::vector<std::string>& vec, int pageID, int now){
            solver(vec);
            bool fixed_res;
            string res = td->embed(vec, fixed_res);
            bool fixed = (dic[pageID - 2] & 1) == 0;
            if(fixed && fixed_res){
                shared_ptr<Page> p = ptr->getPage(pageID);
                char * data = p->getBuf();
                strcpy(data + now + 1, res.c_str());
                p->markDirty();
                p->releaseBuf(data);
                dic_page->markDirty();
            }else {
                  rm->delOneRecord(table_name, pageID, now, fixed);
                  rm->insert(table_name, res, fixed_res);
            }
            return;
        };
        select(table_name, checker, update_func);
        dic_page->releaseBuf(dic);
        return;
    }
    void delOneRecord(const std::string &table_name, int pageID, int now, bool fixed){
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(table_name);
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

    void RecodeManager::del(const std::string &table_name, std::function<bool(const std::vector<std::string>&)> &checker,
             std::function<void(const std::vector<std::string>&)>& solver){
        auto rm = this;
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(table_name);
        shared_ptr<Pager> ptr = td->getPager();
        shared_ptr<Page> dic_page = ptr->getPage(1);
        char * dic = dic_page->getBuf();
        auto del_func = [&solver, &rm, &table_name, &ptr, &dic_page, &dic](std::vector<std::string>& vec, int pageID, int now){
             solver(vec);
             bool fixed = (dic[pageID - 2] & 1) == 0;
             rm->delOneRecord(table_name, pageID, now, fixed);
             if((dic[pageID - 2] & 2) == 1)
                dic[pageID -2] ^= 2;
             dic_page->markDirty();
             return;
         };
         dic_page->releaseBuf(dic);
         return;
    }

    void RecodeManager::select(const std::string &table_name, std::function<bool(const std::vector<std::string>&)> &checker,
             std::function<void(std::vector<std::string>&, int, int)> & solver) {
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(table_name);
        shared_ptr<Pager> ptr = td->getPager();
        int pages = FileUtils::filePages(ptr->getFD());
        shared_ptr<Page> dic_page = ptr->getPage(1);
        char * dic = dic_page->getBuf();
        for(int i = 2;i < pages;i++){
            shared_ptr<Page> p = ptr->getPage(i);
            bool fixed = (dic[i - 2] & 1) == 0;
            char * data = p->getBuf();
            int now = fixed? 2 : 0;
            while(now < PAGER_PAGE_SIZE){
                if(data[now] == 0)//free
                    now += *(short*)(data + now + 1) + fixed ? 5 : 3;
                else{//used
                    int this_loc = now;
                    now += fixed ? 3: 1;
                    vector<string> parsed_data = td->read(data, PAGER_PAGE_SIZE, now);
                    if(checker(parsed_data))
                        solver(parsed_data, i, this_loc);
                }
            }
            p->releaseBuf(data);
        }
    }
}

