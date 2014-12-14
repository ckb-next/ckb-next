#include "includes.h"
#include "keyboard.h"

const key* keymap_system = 0;

const key* getkeymap(const char* name){
    if(!strcmp(name, "de"))
        return keymap_de;
    if(!strcmp(name, "fr"))
        return keymap_fr;
    if(!strcmp(name, "gb"))
        return keymap_gb;
    if(!strcmp(name, "se"))
        return keymap_se;
    if(!strcmp(name, "us"))
        return keymap_us;
    return 0;
}

const char* getmapname(const key* layout){
    if(layout == keymap_de)
        return "de";
    if(layout == keymap_fr)
        return "fr";
    if(layout == keymap_gb)
        return "gb";
    if(layout == keymap_se)
        return "se";
    if(layout == keymap_us)
        return "us";
    return "";
}
