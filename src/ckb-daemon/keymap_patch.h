// Keymap patches for specific devices
typedef struct {
    int idx; // Index of key to patch in keymap
    const char* name;
    short led;
    short scan;
} keypatch;

// Collection of keypatches
typedef struct {
    short product;
    keypatch* patch;
    unsigned patchlen;
} keypatches;

///
/// \brief patchkeys Used to patch the keymaps when necessary
/// \param kb THE usbdevice*
void patchkeys(usbdevice* kb);
