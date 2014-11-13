#include "includes.h"
#include "keyboard.h"

const key* keymap_system = 0;

const key* getkeymap(const char* name){
    if(!strcmp(name, "us"))
        return keymap_us;
    if(!strcmp(name, "uk"))
        return keymap_uk;
    return 0;
}
