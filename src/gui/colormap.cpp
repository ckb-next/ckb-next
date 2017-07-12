#include "colormap.h"

ColorMap::ColorMap() :
    _keyNames(0), _colors(0), _count(0), _mapCount(0)
{
}

ColorMap::~ColorMap(){
    deAlloc();
}

ColorMap::ColorMap(const ColorMap& rhs) :
    _keyNames(0), _colors(0), _count(0), _mapCount(0)
{
    *this = rhs;
}

const ColorMap& ColorMap::operator=(const ColorMap& rhs){
    alloc(rhs._count);
    // Copy key names and colors
    // (Note: it's safe to copy the const char*'s because key names are constants and will never move or be deleted)
    memcpy(_keyNames, rhs._keyNames, sizeof(const char*) * _count);
    memcpy(_colors, rhs._colors, sizeof(QRgb) * _count);
    return rhs;
}

void ColorMap::alloc(int newCount){
    if(newCount > _mapCount){
        // ^ map never shrinks, only expands
        deAlloc();
        _mapCount = newCount;
        _keyNames = new const char*[_mapCount];
        _colors = new QRgb[_mapCount];
    }
    _count = newCount;
    clear();
}

void ColorMap::deAlloc(){
    _count = _mapCount = 0;
    if(_keyNames) delete[] _keyNames;
    if(_colors) delete[] _colors;
}

void ColorMap::clear(){
    memset(_colors, 0, _count * sizeof(QRgb));
}

static int qs_strcmp(const void* lhs, const void* rhs){
    return strcmp(*(const char**)lhs, *(const char**)rhs);
}

void ColorMap::init(const KeyMap& map){
    QList<Key> newKeys = map.positions();
    // There's no point including keys that don't have LEDs, so remove them now
    QMutableListIterator<Key> i(newKeys);
    while(i.hasNext()){
        Key key = i.next();
        if(!key.hasLed)
            i.remove();
    }
    // Now that we know how many keys we'll have, check memory
    alloc(newKeys.count());
    // Copy key names
    int keyPos = 0;
    foreach(const Key& key, newKeys)
        _keyNames[keyPos++] = key.name;     // as above, it's safe to copy these since the strings are constants
    // Sort names for quick access
    qsort(_keyNames, _count, sizeof(const char*), qs_strcmp);
}

QRgb* ColorMap::colorForName(const char* name){
    if(!_keyNames)
        return 0;
    const char** namePtr = (const char**)bsearch(&name, _keyNames, _count, sizeof(const char*), qs_strcmp);
    if(!namePtr)
        return 0;
    ptrdiff_t position = namePtr - _keyNames;
    return _colors + position;
}

const QRgb* ColorMap::colorForName(const char* name) const {
    if(!_keyNames)
        return 0;
    const char** namePtr = (const char**)bsearch(&name, _keyNames, _count, sizeof(const char*), qs_strcmp);
    if(!namePtr)
        return 0;
    ptrdiff_t position = namePtr - _keyNames;
    return _colors + position;
}
