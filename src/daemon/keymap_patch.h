// Keymap patches for specific devices
typedef struct {
    size_t idx; // Index of key to patch in keymap
    const char* name;
    short led;
    short scan;
} keypatch;

// Collection of keypatches
typedef struct {
    ushort vendor;
    ushort product;
    const keypatch* patch;
    size_t patchlen;
} keypatches;

///
/// \brief patchkeys Used to patch the keymaps when necessary
/// \param kb THE usbdevice*
void patchkeys(usbdevice* kb);
