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

        dic_page->releaseBuf(dic);
        return ret;
    }

    void RecodeManager::update(const std::string &table_name, std::function<bool(std::vector<std::string>)> &c,
                               std::function<void(std::vector<std::string>, char *)>) {

    }

    void RecodeManager::del(const std::string &table_name, std::function<bool(std::vector<std::string>)> &c,
                            std::function<void(std::vector<std::string>, char *)>) {

    }

    void RecodeManager::select(const std::string &table_name, std::function<bool(std::vector<std::string>)> &c,
                               std::function<void(std::vector<std::string>, char *)>) {
        shared_ptr<TableDescription> td = TableManager::getInstance()->getTableDescription(table_name);
        shared_ptr<Pager> ptr = td->getPager();
        int pages = FileUtils::filePages(ptr->getFD());
        for(int i = 2;i < pages;i++){
            shared_ptr<Page> p = ptr->getPage(i);

        }
    }
}

