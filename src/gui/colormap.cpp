#include "colormap.h"

void ColorMap::clear()
{
  _colors.clear();
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
    // Copy key names
    foreach(const Key& key, newKeys)
    {
        _colors.insert({key.name, {}});
    }
}

QRgb* ColorMap::colorForName(const char* name){
  auto iterator = _colors.find(name);
  return iterator == _colors.end() ? nullptr : &iterator->second;
}

const QRgb* ColorMap::colorForName(const char* name) const {
  return const_cast<ColorMap*>(this)->colorForName(name);
}
