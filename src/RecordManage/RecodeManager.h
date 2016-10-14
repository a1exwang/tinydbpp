//
// Created by chord on 16/10/14.
//

#ifndef TINYDBPP_RECODEMANAGER_H
#define TINYDBPP_RECODEMANAGER_H


#include <cstdlib>

class RecodeManager {
    static RecodeManager* ins = NULL;
    RecodeManager(){}
public:
    static RecodeManager* getInstance(){
        if(!ins) return ins = new RecodeManager();
        else return ins;
    }
};


#endif //TINYDBPP_RECODEMANAGER_H
