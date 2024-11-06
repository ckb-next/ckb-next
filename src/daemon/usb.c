#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"
#include "keymap_patch.h"
#include <ckbnextconfig.h>
#include "legacykb_proto.h"

// Values taken from the official website
// Mice not in the list default to 12000 in the GUI
const dpi_list mouse_dpi_list[] = {
    { P_M55_RGB_PRO, 12400 },
    { P_M65, 8200 },
    { P_M65_PRO, 12000 },
    { P_M65_RGB_ELITE, 18000 },
    { P_M95, 8200 },
    { P_GLAIVE, 16000 },
    { P_SABRE_O, 6400 },
    { P_SABRE_L, 8200 },
    { P_SABRE_N, 10000 },
    { P_SCIMITAR, 12000 },
    { P_SCIMITAR_PRO, 16000 },
    { P_SCIMITAR_ELITE, 18000 },
    { P_SABRE_O2, 6400 },
    { P_HARPOON, 6000 },
    { P_HARPOON_PRO, 12000 },
    { P_KATAR_PRO_XT, 18000 },
    { P_KATAR, 8000 },
    { P_KATAR_PRO, 12400 },
    { P_IRONCLAW, 18000 },
    { P_NIGHTSWORD, 18000},
    { P_IRONCLAW_W_U, 18000 },
    { P_HARPOON_WL_U, 10000 },
    { P_DARK_CORE_RGB_PRO, 18000 },
    { P_DARK_CORE_RGB_PRO_SE, 18000 },
    { P_GLAIVE_PRO, 16000 },
    { 0, 0 }, // Keep last and do not remove
};

// WARNING: macOS doesn't support idVendor != 0x1b1c at the moment
const device_desc models[] = {
    // Keyboards
    { V_CORSAIR, P_K55, },
    { V_CORSAIR, P_K55_PRO, },
    { V_CORSAIR, P_K55_PRO_XT, },
    { V_CORSAIR, P_K60_PRO_RGB, },
    { V_CORSAIR, P_K60_PRO_RGB_LP, },
    { V_CORSAIR, P_K60_PRO_RGB_SE, },
    { V_CORSAIR, P_K60_PRO_MONO, },
    { V_CORSAIR, P_K60_PRO_TKL, },
    { V_CORSAIR, P_K63_NRGB, },
    { V_CORSAIR, P_K63_NRGB_WL, },
    { V_CORSAIR, P_K63_NRGB_WL2, },
    { V_CORSAIR, P_K63_NRGB_WL3, },
    { V_CORSAIR, P_K63_NRGB_WL4, },
    { V_CORSAIR, P_K65, },
    { V_CORSAIR, P_K65_LEGACY, },
    { V_CORSAIR, P_K65_LUX, },
    { V_CORSAIR, P_K65_RFIRE, },
    { V_CORSAIR, P_K66, },
    { V_CORSAIR, P_K68, },
    { V_CORSAIR, P_K68_NRGB, },
    { V_CORSAIR, P_K70, },
    { V_CORSAIR, P_K70_LEGACY, },
    { V_CORSAIR, P_K70_LUX, },
    { V_CORSAIR, P_K70_LUX_NRGB, },
    { V_CORSAIR, P_K70_RFIRE, },
    { V_CORSAIR, P_K70_RFIRE_NRGB, },
    { V_CORSAIR, P_K70_MK2, },
    { V_CORSAIR, P_K70_MK2SE, },
    { V_CORSAIR, P_K70_MK2LP, },
    { V_CORSAIR, P_K70_TKL, },
    { V_CORSAIR, P_K70_PRO, },
    { V_CORSAIR, P_K90_LEGACY, },
    { V_CORSAIR, P_K95, },
    { V_CORSAIR, P_K95_LEGACY, },
    { V_CORSAIR, P_K95_PLATINUM, },
    { V_CORSAIR, P_STRAFE, },
    { V_CORSAIR, P_STRAFE_NRGB, },
    { V_CORSAIR, P_STRAFE_NRGB_2, },
    { V_CORSAIR, P_STRAFE_MK2, },
    { V_CORSAIR, P_K95_PLATINUM_XT, },
    { V_CORSAIR, P_K57_D, },
    { V_CORSAIR, P_K57_U, },
    { V_CORSAIR, P_K100_OPTICAL, },
    { V_CORSAIR, P_K100_MECHANICAL, },
    { V_CORSAIR, P_K100_OPTICAL_VARIANT, },
    { V_CORSAIR, P_K65_MINI, },
    // Mice
    { V_CORSAIR, P_M55_RGB_PRO, },
    { V_CORSAIR, P_M65, },
    { V_CORSAIR, P_M65_PRO, },
    { V_CORSAIR, P_M65_RGB_ELITE, },
    { V_CORSAIR, P_M95, },
    { V_CORSAIR, P_GLAIVE, },
    { V_CORSAIR, P_SABRE_O, },
    { V_CORSAIR, P_SABRE_L, },
    { V_CORSAIR, P_SABRE_N, },
    { V_CORSAIR, P_SCIMITAR, },
    { V_CORSAIR, P_SCIMITAR_PRO, },
    { V_CORSAIR, P_SCIMITAR_ELITE, },
    { V_CORSAIR, P_SABRE_O2, },
    { V_CORSAIR, P_HARPOON, },
    { V_CORSAIR, P_HARPOON_PRO, },
    { V_CORSAIR, P_KATAR, },
    { V_CORSAIR, P_KATAR_PRO, },
    { V_CORSAIR, P_KATAR_PRO_XT, },
    { V_CORSAIR, P_IRONCLAW, },
    { V_CORSAIR, P_NIGHTSWORD, },
    { V_CORSAIR, P_DARK_CORE, },
    { V_CORSAIR, P_DARK_CORE_SE, },
    { V_CORSAIR, P_DARK_CORE_WL, },
    { V_CORSAIR, P_DARK_CORE_SE_WL, },
    { V_CORSAIR, P_DARK_CORE_RGB_PRO, },
    { V_CORSAIR, P_DARK_CORE_RGB_PRO_WL, },
    { V_CORSAIR, P_DARK_CORE_RGB_PRO_SE, },
    { V_CORSAIR, P_DARK_CORE_RGB_PRO_SE_WL, },
    { V_CORSAIR, P_IRONCLAW_W_U, },
    { V_CORSAIR, P_IRONCLAW_W_D, },
    { V_CORSAIR, P_HARPOON_WL_U, },
    { V_CORSAIR, P_HARPOON_WL_D, },
    { V_CORSAIR, P_GLAIVE_PRO, },
    // Mousepads
    { V_CORSAIR, P_POLARIS, },
    // Headset stands
    { V_CORSAIR, P_ST100, },
    // Misc
    { V_CORSAIR, P_GENERIC_BRAGI_DONGLE, },
};

const size_t N_MODELS = sizeof(models) / sizeof(device_desc);

device_desc ignored_devices[DEV_MAX] = {0};

/// brief .
///
/// \brief reset_stop is boolean: Reset stopper for when the program shuts down.
///
/// Is set only by \a quit() to true (1)
/// to inform several usb_* functions to end their loops and tries.
_Atomic int reset_stop = 0;

/// brief .
///
/// \brief features_mask Mask of features to exclude from all devices
///
/// That bit mask ist set to enable all (-1).
/// When interpreting the input parameters, some of these bits can be cleared.
/// \n At the moment binding, notifying and mouse-acceleration can be disabled via command line.
/// \n Have a look at \a main() in main.c for details.
int features_mask = -1;

// Enable experimental support for not fully supported devices
int enable_experimental = 0;

/// brief .
///
/// vendor_str returns "corsair" if the given \a vendor argument is equal to \a V_CORSAIR \c (0x1bc)
/// else it returns ""
///
/// \attention There is also a string defined V_CORSAIR_STR, which returns the device number as string in hex "1b1c".
const char* vendor_str(ushort vendor){
    if(vendor == V_CORSAIR)
        return "corsair";
    return "";
}

/// brief .
///
/// At present, various models and their properties are known from corsair products.
/// Some models differ in principle (mice and keyboards),
/// others differ in the way they function (for example, RGB and non RGB), but they are very similar.
///
/// Here, only the first point is taken into consideration and we return a unified model string.
/// If the model is not known with its number, \a product_str returns an empty string.
///
/// The model numbers and corresponding strings wwith the numbers in hex-string are defined in \c usb.h
///
/// At present, this function is used to initialize \c kb->name and to give information in debug strings.
///
/// \attention The combinations below have to fit to the combinations in the macros mentioned above.
/// So if you add a device with a new number, change both.
///
/// \todo There are macros defined in usb.h to detect all the combinations below.
/// the only difference is the parameter:
/// The macros need the \a kb*,
/// product_str() needs the \a product \a ID
///
const char* product_str(ushort product){
    if(product == P_K100_OPTICAL || product == P_K100_MECHANICAL || product == P_K100_OPTICAL_VARIANT)
        return "k100";
    if(product == P_K95_LEGACY)
        return "k95l";
    if(product == P_K95)
        return "k95";
    if(product == P_K95_PLATINUM || product == P_K95_PLATINUM_XT)
        return "k95p";
    if(product == P_K70 || product == P_K70_LEGACY || product == P_K70_LUX || product == P_K70_LUX_NRGB || product == P_K70_RFIRE || product == P_K70_RFIRE_NRGB)
        return "k70";
    if(product == P_K70_MK2 || product == P_K70_MK2SE || product == P_K70_MK2LP)
        return "k70mk2";
    if(product == P_K70_TKL)
        return "k70tkl";
    if(product == P_K70_PRO)
        return "k70pro";
    if(product == P_K68 || product == P_K68_NRGB)
        return "k68";
    if(product == P_K65 || product == P_K65_LEGACY || product == P_K65_LUX || product == P_K65_RFIRE)
        return "k65";
    if(product == P_K65_MINI)
        return "k65_mini";
    if(product == P_K66)
        return "k66";
    if(product == P_K63_NRGB)
        return "k63";
    if(product == P_K63_NRGB_WL || product == P_K63_NRGB_WL2 || product == P_K63_NRGB_WL3 || product == P_K63_NRGB_WL4)
        return "k63_wireless";
    if(product == P_K60_PRO_RGB || product == P_K60_PRO_RGB_LP || product == P_K60_PRO_RGB_SE || product == P_K60_PRO_MONO)
        return "k60";
    if(product == P_K60_PRO_TKL)
        return "k60_tkl";
    if(product == P_K57_U || product == P_K57_D || product == P_K55_PRO_XT)
        return "k57_wireless";
    if(product == P_K55)
        return "k55";
    if(product == P_K55_PRO)
        return "k55pro";
    if(product == P_STRAFE || product == P_STRAFE_NRGB || product == P_STRAFE_NRGB_2)
        return "strafe";
    if(product == P_STRAFE_MK2)
        return "strafe_mk2";
    if(product == P_M95)
        return "m95";
    if(product == P_M55_RGB_PRO)
        return "m55";
    if(product == P_M65 || product == P_M65_PRO)
        return "m65";
    if(product == P_M65_RGB_ELITE)
        return "m65e";
    if(product == P_SABRE_O || product == P_SABRE_L || product == P_SABRE_N || product == P_SABRE_O2)
        return "sabre";
    if(product == P_SCIMITAR || product == P_SCIMITAR_PRO || product == P_SCIMITAR_ELITE)
        return "scimitar";
    if(product == P_HARPOON || product == P_HARPOON_PRO)
        return "harpoon";
    // Katar PRO XT and Katar PRO works the same, so we just do that
    if(product == P_KATAR_PRO_XT || product == P_KATAR_PRO)
        return "katarproxt";
    if(product == P_GLAIVE)
        return "glaive";
    if(product == P_GLAIVE_PRO)
        return "glaivepro";
    if(product == P_KATAR)
        return "katar";
    if(product == P_IRONCLAW)
        return "ironclaw";
    if(product == P_NIGHTSWORD)
        return "nightsword";
    if(product == P_IRONCLAW_W_U || product == P_IRONCLAW_W_D || product == P_HARPOON_WL_U || product == P_HARPOON_WL_D)
        return "ironclaw_wireless";
    if(product == P_DARK_CORE_RGB_PRO || product == P_DARK_CORE_RGB_PRO_WL || product == P_DARK_CORE_RGB_PRO_SE || product == P_DARK_CORE_RGB_PRO_SE_WL)
        return "dark_core_rgb_pro";
    if(product == P_POLARIS)
        return "polaris";
    if(product == P_DARK_CORE || product == P_DARK_CORE_WL || product == P_DARK_CORE_SE || product == P_DARK_CORE_SE_WL)
        return "darkcore";
    if(product == P_ST100)
        return "st100";
    if(product == P_GENERIC_BRAGI_DONGLE)
        return "bragi_dongle";
    return "";
}

/// brief .
///
/// \brief get_vtable returns the correct vtable pointer
/// \param vendor short usb vendor ID
/// \param product short usb product ID
/// \return Depending on the type and model, the corresponding vtable pointer is returned (see below)
///
/// At present, we have three different vtables:
/// - \c vtable_mouse is used for all mouse types. This may be wrong with some newer mice?
/// - \c vtable_keyboard is used for all RGB Keyboards.
/// - \c vtable_keyboard_nonrgb for all the rest.
///
/// \todo Is the last point really a good decision and always correct?
///
static const devcmd* get_vtable(usbdevice* kb){
    ushort vendor = kb->vendor;
    ushort product = kb->product;
    if(kb->protocol == PROTO_BRAGI) {
        if(IS_DONGLE(kb))
            return &vtable_bragi_dongle;
        else if(IS_MOUSE(vendor, product))
            return &vtable_bragi_mouse;
        else
            return &vtable_bragi_keyboard;
    } else if(IS_MOUSE(vendor, product)) {
        if(IS_LEGACY(vendor, product))
            return &vtable_mouse_legacy;
        else if(IS_WIRELESS(vendor, product))
            return &vtable_mouse_wireless;
        else
            return &vtable_mouse;
    } else if(IS_MOUSEPAD(vendor, product) || product == P_ST100) {
        return &vtable_mousepad;
    } else {
        if(IS_LEGACY(vendor, product))
            return &vtable_keyboard_legacy;
        else if(IS_WIRELESS(vendor, product))
            return &vtable_keyboard_wireless;
        else
            return &vtable_keyboard;
    }
}

// USB device main loop
static void* devmain(usbdevice* kb){
    /// \attention dmutex should still be locked when this is called
    const int kbfifo = kb->infifo - 1;
    readlines_ctx* linectx = calloc(1, sizeof(readlines_ctx));

    while(1){
        queued_mutex_unlock(dmutex(kb));
        // Read from cmd FIFO
        int ret = readline_fifo(kbfifo, linectx);
        wait_until_suspend_processed();
        queued_mutex_lock(dmutex(kb));

        // End thread when the handle is removed
        if(kb->status == DEV_STATUS_DISCONNECTING || kb->status == DEV_STATUS_DISCONNECTED)
            break;

        if(ret == 0) {
            // EOF
            break;
        } else if(ret < 0) {
            // Retry
            continue;
        } else {
            if(readcmd(kb, linectx->buf)){
                // USB transfer failed or command requested disconnect; destroy device
                closeusb(kb);
                goto cleanup;
            }
        }
    }
cleanup:
    queued_mutex_unlock(dmutex(kb));
    free(linectx);
    return 0;
}

void fill_usbdevice_protocol(usbdevice* kb){
    if(USES_BRAGI(kb->vendor, kb->product))
        kb->protocol = PROTO_BRAGI;

    if(USES_BRAGI_JUMBO(kb->vendor, kb->product))
        kb->out_ep_packet_size = BRAGI_JUMBO_SIZE;
    else
        kb->out_ep_packet_size = MSG_SIZE;
}

/// brief .
///
/// \brief _setupusb A horrible function for setting up an usb device
/// \param context As _setupusb() is called as a new thread, the kb* is transferred as void*
/// \return a ptread_t* 0, here casted as void*. Retval is always null
///
/// The basic structure of the function is somewhat habituated.
/// It is more like an assembler routine than a structured program.
/// This is not really bad, but just getting used to.
///
/// After every action, which can be practically fault-prone,
/// the routine goes into the same error handling:
/// It goes via goto to one of two exit labels.
/// The difference is whether or not an unlock has to be performed on the imutex variable.


///
/// In both cases, closeusb() is called, then an unlock is performed on the dmutex.
///
/// The only case where this error handling is not performed is the correct return of the call to devmain().
/// Here simply the return value of devmain() is passed to the caller.
///
/// In either case, the routine terminates with a void* 0
/// because either devmain() has returned constant null or the routine itself returns zero.
///
/// The basic idea of this routine is the following:
///

static void* _setupusb(void* context){
    /// \n First some initialization of kb standard structured and local vars is done.
    /// - \b kb is set to the pointer given from start environment
    /// - local vars \b vendor and \b product are set to the values from the corresponding fields of kb
    /// - local var \b vt \b and the \b kb->vtable are both set to the retval of \a get_vtable()
    /// - \b kb->features are set depending on the type of hardware connected:
    ///   - set either to standard non rgb (all common flags like binding, notify, FW, hardware-loading etc)
    ///     or in case of RGB-device set to standard + RGB, pollrate-change and fw-update
    ///   - exclude all features which are disabled via feature_mask (set by daemon CLI parameters)
    ///   - if it is a mouse, add adjust-rate
    ///   - if it is a monochrome device, set the flag for RGB-protocol, but single color
    /// - the standard delay time is initialized in kb->usbdelay
    ///
    usbdevice* kb = context;
    queued_mutex_lock(dmutex(kb));
    fill_usbdevice_protocol(kb);
    queued_mutex_lock(imutex(kb));
    // Set standard fields
    ushort vendor = kb->vendor, product = kb->product;
    const devcmd* vt = get_vtable(kb);
    memcpy(&kb->vtable, vt, sizeof(devcmd));
    vt = &kb->vtable;

    if(!(IS_DONGLE(kb) && kb->protocol == PROTO_BRAGI))
        kb->features = (IS_LEGACY(vendor, product) ? FEAT_STD_LEGACY : FEAT_STD_RGB) & features_mask;
    if(SUPPORTS_ADJRATE(kb))
        kb->features |= FEAT_ADJRATE;
    if(IS_MONOCHROME(vendor, product))
        kb->features |= FEAT_MONOCHROME;
    if(IS_DONGLE(kb))
        kb->features |= (FEAT_DONGLE | FEAT_FWVERSION);
    if(IS_WIRELESS_DEV(kb)){
        kb->features |= FEAT_WIRELESS;
        if((kb->protocol == PROTO_BRAGI && !IS_DONGLE(kb)) || kb->protocol != PROTO_BRAGI)
            kb->features |= FEAT_BATTERY;
    }
    // Disable FWUPDATE for all bragi devices
    if(kb->protocol == PROTO_BRAGI)
        kb->features &= ~FEAT_FWUPDATE;

    // Check if the device needs a patched keymap, and if so patch it.
    patchkeys(kb);

    // Perform OS-specific setup
    DELAY_100MS();

    ///
    /// - The first relevant point is the operating system-specific opening of the interface in os_setupusb().
    /// As a result, some parameters should be set in kb (name, serial, fwversion, epcount = number of usb endpoints),
    /// and all endpoints should be claimed with usbclaim().
    /// Claiming is the only point where os_setupusb() can produce an error (-1, otherwise 0).
    if(!kb->parent)
        if(os_setupusb(kb))
            goto fail;

    ///
    /// - The following two statements deal with possible errors when setting the kb values in the current routine:
    /// If the version or the name was not read correctly, they are set to default values:
    ///   - serial is set to "<vendor>: <product> -NoID"
    ///   - the name is set to "<vendor> <product>".
    // Make up a device name and serial if they weren't assigned
    if(!kb->serial[0])
        snprintf(kb->serial, SERIAL_LEN, "%04x:%04x-NoID", kb->vendor, kb->product);
    if(!kb->name[0])
        snprintf(kb->name, KB_NAME_LEN, "%s %s", vendor_str(kb->vendor), product_str(kb->product));

    // Ask the protocol handler to set the endpoints required for the main input thread
    kb->vtable.fill_input_eps(kb);

    // Set up an input device for key events
    ///
    /// - Then the user level input subsystem is activated via os_openinput().
    /// There are two file descriptors, one for the mouse and one for the keyboard.
    /// <b>As mentioned in structures.h, not the just opened FD numbers are stored under kb->uinput_kb or kb->uinput_mouse, but the values increased by 1!</b>
    /// The reason is, if the open fails or not open has been done until now,
    /// that struct member is set to 0, not to -1 or other negative value.
    /// So all usage of this kb->handle must be something like \c "kb->handle - 1", as you can find it in the code.
    if(os_inputopen(kb))
        goto fail;
    ///
    /// - The next action is to create a separate thread, which gets as parameter kb and starts with os_inputmain().
    /// The thread is immediately detached so that it can return its resource completely independently if it should terminate.
    if(!kb->parent){
        if(pthread_create(&kb->inputthread, 0, os_inputmain, kb))
            goto fail;

#ifndef OS_MAC
        // name this thread externally if not on mac,
        // on mac `pthread_setname_np` is only naming the current thread
        char inputthread_name[THREAD_NAME_MAX] = "ckbX input";
        inputthread_name[3] = INDEX_OF(kb, keyboard) + '0';
        pthread_setname_np(kb->inputthread, inputthread_name);
#endif // OS_MAC
        pthread_detach(kb->inputthread);

        ///
        /// - The same happens with os_setupindicators(),
        /// which initially initializes all LED variables in kb to off and then starts the _ledthread() thread
        /// with kb as parameter and then detaches it.
        /// Here again only the generation of the thread can fail.
        if(!(IS_MOUSE_DEV(kb) || IS_MOUSEPAD_DEV(kb))) {
            if(os_setupindicators(kb))
                goto fail;
        }
    }

    // Set up device
    ///
    /// - Via an entry in the vable (allocprofile, identical for all three vtable types),
    /// allocprofile() is called in profile.c.
    /// With a valid parameter kb, a usbprofile structure is allocated and stored as a kb->profile.
    /// Then initmode() is called for each of the initializable modes (MODE_COUNT, currently 6).
    /// This procedure creates the memory space for the mode information,
    /// initializes the range to 0, and then sets the light.forceupdate and dpi.forceupdate to true.
    /// This forces an update later in the initialization of the device.
    /// \n The first mode is set as the current mode and two force flags are set
    /// (this seems to be mode-intersecting flags for light and update).
    /// \warning There is no error handling for the allocprofile() and initmode() procedures.
    /// However, since they allocate storage areas, the subsequent assignments and initializations can run in a SEGV.
    vt->allocprofile(kb);
    ///
    /// - From this point - if an error is detected - the error label is addressed by goto statement,
    /// which first performs an unlock on the imutex.
    /// This is interesting because the next statement is exactly this: An unlock on the imutex.
    queued_mutex_unlock(imutex(kb));

    if(vt->start(kb, 0) && usb_tryreset(kb))
        goto fail_noinput;
    ///
    /// - Next, go to mkdevpath(). After securing the EUID (effective UID) especially for macOS, work starts really in _mkdevpath().
    /// Create - no matter how many devices were registered -
    /// either the ckb0/ files \b version, \b pid and \b connected
    /// or the \b cmd command fifo, the first notification fifo \b notify0, \b model and \b serial as well as the \b features of the device and the \b pollrate.
    // Make /dev path
    if(mkdevpath(kb))
        goto fail_noinput;
    ///
    /// - If all this is done and no error has occurred,
    /// a debug info is printed ("Setup finished for ckbx")
    /// updateconnected() writes the new device into the text file under ckb0/
    /// and devmain() is called.
    // Finished. Enter main loop
    int index = INDEX_OF(kb, keyboard);
    ckb_info("Setup finished for %s%d", devpath, index);
    kb->status = DEV_STATUS_CONNECTED;
    updateconnected(kb);

    ///
    /// devmain()'s return value is returned by _setupusb() when we terminate.
    return devmain(kb);
    ///
    /// - The remaining code lines are the two exit labels as described above
    fail:
    queued_mutex_unlock(imutex(kb));
    fail_noinput:
    closeusb(kb);
    queued_mutex_unlock(dmutex(kb));
    return 0;
}

/// \brief .
///
/// Set up a USB device after its handle is open. Spawns a new thread _setupusb() with standard parameter kb.
/// dmutex must be locked prior to calling this function. The function will unlock it when finished.
/// In kb->thread the thread id is mentioned, because closeusb() needs this info for joining that thread again.
///
void setupusb(usbdevice* kb){

    if(pthread_create(&kb->thread, 0, _setupusb, kb)) {
        ckb_err("Failed to create USB thread");
        return;
    }

#ifndef OS_MAC
    // name this thread externally if not on mac,
    // on mac `pthread_setname_np` is only naming the current thread
    char usbthread_name[THREAD_NAME_MAX] = "ckbX usb";
    usbthread_name[3] = INDEX_OF(kb, keyboard) + '0';
    pthread_setname_np(kb->thread, usbthread_name);
#endif // OS_MAC
    queued_mutex_unlock(dmutex(kb));
}

/// \brief .
///
/// First is checked, whether a firmware-upgrade is indicated for the device.
/// If so, revertusb() returns 0.
/// \todo Why is this useful? Are there problems seen with deactivating a device with older fw-version??? Why isn't this an error indicating reason and we return success (0)?
///
/// Anyway, the following steps are similar to some other procs, dealing with low level usb handling:
/// - If we do not have an RGB device, a simple setting to Hardware-mode (NK95_HWON) is sent to the device via n95cmd().
/// \todo The return value of nk95cmd() is ignored (but sending the ioctl may produce an error and _nk95_cmd will indicate this), instead revertusb() returns success in any case.
///
/// - If we have an RGB device, setactive() is called with second param active = false.
/// That function will have a look on differences between keyboards and mice.
/// \n More precisely setactive() is just a macro to call via the kb->vtable enties either the active() or the idle() function where the vtable points to.
/// setactive() may return error indications. If so, revertusb() returns -1, otherwise 0 in any other case.
///
int revertusb(usbdevice* kb){
    if(NEEDS_FW_UPDATE(kb))
        return 0;

    // FIXME: This should be moved to a device-specific function
    if(IS_LEGACY_DEV(kb)){
        nk95cmd(kb, NK95_HWON);
        return 0;
    }
    if(setactive(kb, 0))
        return -1;
    return 0;
}

/// \brief .
///
/// First reset the device via os_resetusb() after a long delay (it may send something to the host).
/// If this worked (retval == 0), give the device another long delay
/// Then perform the initialization via the device specific start() function entry in kb->vtable
/// and if this is successful also, return the result of the device depenten updatergb() with force=true.
///
int _resetusb(usbdevice* kb, const char* file, int line){
    // Perform a USB reset
    DELAY_100MS();
    int res = os_resetusb(kb, file, line);
    if(res)
        return res;
    DELAY_100MS();
    // Re-initialize the device
    if(kb->vtable.start(kb, kb->active) != 0)
        return -1;
    if(kb->vtable.updatergb(kb, 1) != 0)
        return -1;
    return 0;
}

/// \brief .
///
/// This function is called if an usb command ran into an error
/// in case of one of the following two situations:
/// - When setting up a new usb device and the start() function got an error (\see _setupusb())
/// - If upgrading to a new firmware gets an error (\see cmd_fwupdate()).
///
/// The previous action which got the error will NOT be re-attempted.
///
/// In an endless loop usb_tryreset() tries to reset the given usb device via the macro resetusb().
/// \n This macro calls _resetusb() with debugging information.
/// \n _resetusb() sends a command via the operating system dependent function os_resetusb()
/// and - if successful - reinitializes the device.
/// os_resetusb() returns -2 to indicate a broken device and all structures should be removed for it.
/// \n In that case, the loop is terminated, an error message is produced and usb_tryreset() returns -1.
///
/// In case resetusb() has success, the endless loop is left via a return 0 (success).
/// \n If the return value from resetusb() is -1, the loop is continued with the next try.
///
/// If the global variable \b reset_stop is set directly when the function is called or after each try,
/// usb_tryreset() stops working and returns -1.
///
/// \todo Why does usb_tryreset() hide the information returned from resetusb()? Isn't it needed by the callers?
///
int usb_tryreset(usbdevice* kb){
    if(reset_stop)
        return -1;
    ckb_info("Attempting reset...");
    while(1){
        int res = resetusb(kb);
        if(!res){
            ckb_info("Reset success");
            return 0;
        }
        if(res == -2 || reset_stop)
            break;
    }
    ckb_err("Reset failed. Disconnecting.");
    return -1;
}

// Wrapper around the vtable write() function for error handling and recovery
int _usbsend(usbdevice* kb, void* messages, size_t msg_len, int count, const char* file, int line){
    int total_sent = 0;
    for(int i = 0; i < count; i++){
        // Send each message via the OS function
        while(1){
            kb->vtable.delay(kb, DELAY_SEND);
            queued_mutex_lock(mmutex(kb)); ///< Synchonization between macro and color information
            int res = kb->vtable.write(kb, messages + i * msg_len, msg_len, 0, file, line);
            queued_mutex_unlock(mmutex(kb));
            if(res == 0)
                return 0;
            else if(res != -1){
                total_sent += res;
                break;
            }
            // Stop immediately if the program is shutting down or hardware load is set to tryonce
            if(reset_stop)
                return 0;
            // Retry as long as the result is temporary failure
            DELAY_100MS();
        }
    }
    return total_sent;
}

// Wrapper around the vtable write() and read() functions for error handling and recovery
int _usbrecv(usbdevice* kb, void* out_msg, size_t msg_len, uchar* in_msg, const char* file, int line){
    // For now assume that msg_len is for both out_msg and in_msg

    // Try a maximum of 5 times
    for (int try = 0; try < 5; try++) {
        // Send the output message
        queued_mutex_lock(mmutex(kb)); ///< Synchonization between macro and color information
        kb->vtable.delay(kb, DELAY_SEND);
        int res = kb->vtable.write(kb, out_msg, msg_len, 1, file, line);
        queued_mutex_unlock(mmutex(kb));
        if (res == 0)
            return 0;
        else if (res == -1) {
            // Retry on temporary failure
            if (reset_stop)
                return 0;
            DELAY_100MS();
            continue;
        }
        // Wait for the response
        kb->vtable.delay(kb, DELAY_RECV);
        res = kb->vtable.read(kb, in_msg, msg_len, 0, file, line);
        if(res == 0)
            return 0;
        else if(res != -1)
            return res;
        if(reset_stop)
            return 0;
        DELAY_100MS();
    }
    // Give up
    ckb_err_fn("Too many send/recv failures. Dropping.", file, line);
    return 0;
}

/// \brief .
///
/// An imutex lock ensures first of all, that no communication is currently running from the viewpoint of the driver to the user input device
/// (ie the virtual driver with which characters or mouse movements are sent from the daemon to the operating system as inputs).
///
/// If the \b kb has an acceptable value != 0,
/// the index of the device is looked for
/// and with this index os_inputclose() is called.
/// After this no more characters can be sent to the operating system.
///
/// Then the connection to the usb device is capped by os_closeusb().
/// \todo What is not yet comprehensible is the call to updateconnected() BEFORE os_closeusb().
/// Should that be in the other sequence?
/// Or is updateconnected() not displaying the connected usb devices,
/// but the representation which uinput devices are loaded?
/// Questions about questions ...
///
/// If there is no valid \b handle, only updateconnected() is called.
/// We are probably trying to disconnect a connection under construction.
/// Not clear.
///
/// The cmd pipe as well as all open notify pipes are deleted via rmdevpath ().
/// \n This means that nothing can happen to the input path -
/// so the device-specific imutex is unlocked again and remains unlocked.
///
/// Also the dmutex is unlocked now, but only to join the thread,
/// which was originally taken under \b kb->thread
/// (which started with _setupusb()) with pthread_join() again.
/// Because of the closed devices that thread would have to quit sometime
/// \see the hack note with rmdevpath())
///
/// As soon as the thread is caught, the dmutex is locked again, which is what I do not understand yet: What other thread can do usb communication now?
/// \n If the vtabel exists for the given kb
/// (why not? It seems to have race conditions here!!),
/// via the vtable the actually device-specific, but still everywhere identical freeprofile() is called.
/// This frees areas that are no longer needed.
/// Then the \b usbdevice structure in its array is set to zero completely.
///
/// Error handling is rather unusual in closeusb();
/// Everything works
/// (no matter what the called functions return),
/// and closeusb() always returns zero (success).
///
int closeusb(usbdevice* kb){
    kb->status = DEV_STATUS_DISCONNECTING;
    const int index = INDEX_OF(kb, keyboard);
    ckb_info("Disconnecting %s%d", devpath, index);
    updateconnected(kb);

    // If the device has children, disconnect them first
    pthread_mutex_lock(cmutex(kb));
    for(int i = 0; i < MAX_CHILDREN; i++){
        if(!kb->children[i])
            continue;
        queued_mutex_lock(dmutex(kb->children[i]));
        closeusb(kb->children[i]);
        queued_mutex_unlock(dmutex(kb->children[i]));
        kb->children[i] = NULL;
    }

    queued_mutex_lock(imutex(kb));
    os_inputclose(kb);

    // Close USB device
    os_closeusb(kb);
    rmdevpath(kb);

    // Wait for threads to close
    if(kb->pollthread || kb->thread){
        queued_mutex_unlock(imutex(kb));
        queued_mutex_unlock(dmutex(kb));

        // Shut down the device polling thread
        if(kb->pollthread){
            // Unlock dmutex to allow pollthread to quit
            pthread_kill(*kb->pollthread, SIGUSR2);
            pthread_join(*kb->pollthread, NULL);
            free(kb->pollthread);
            kb->pollthread = NULL;
        }

        if(kb->thread){
            if(pthread_equal(pthread_self(), kb->thread)){
#ifdef DEBUG_MUTEX
                ckb_info("Attempted to pthread_join() self. Detaching instead.");
#endif
                int detachres = pthread_detach(kb->thread);
                if(detachres)
                    ckb_err("pthread_detach() returned %s (%d)", strerror(detachres), detachres);
            } else {
#ifdef DEBUG_MUTEX
                ckb_info("Joining thread 0x%lx for ckb%d by thread 0x%lx", kb->thread, INDEX_OF(kb, keyboard), pthread_self());
#endif
                int joinres = pthread_join(kb->thread, NULL);
                if(joinres)
                    ckb_err("pthread_join() returned %s (%d)", strerror(joinres), joinres);
            }
        }
        queued_mutex_lock(dmutex(kb));
        queued_mutex_lock(imutex(kb));
    }

    // Delete the profile
    if(kb->vtable.freeprofile)
        kb->vtable.freeprofile(kb);
    // This implicitly sets the status to STATUS_DISCONNECTED
    memset(kb, 0, sizeof(usbdevice));
    pthread_mutex_unlock(cmutex(kb));
    queued_mutex_unlock(imutex(kb));
    return 0;
}

/// Formats and writes the current urb buffer to the console
void print_urb_buffer(const char* prefix, const unsigned char* buffer, int actual_length, const char* file, int line, const char* function, int devnum, const uchar ep){
    char* converted = malloc(actual_length * 3 + 1);
    for(int i = 0; i < actual_length; i++)
        sprintf(converted + i * 3, "%02x ", buffer[i]);
    char ep_str[6] = {0};
    if(ep)
        snprintf(ep_str, sizeof(ep_str), " (%02x)", ep);
    if(line == 0)
        ckb_info("ckb%d %s%s %s", devnum, prefix, ep_str, converted);
    else
        ckb_info("ckb%d %s (via %s:%d) %s%s %s", devnum, function, file, line, prefix, ep_str, converted);
    free(converted);
}

#define TRY_WITH_RESET_NOFREE(action, ret)       \
    ret = 0;                                     \
    while(action){                               \
        if((ret = usb_tryreset(kb)))             \
            break;                               \
    }                                            \
    if(ret){                                     \
        ckb_err(#action " failed after resume"); \
        queued_mutex_unlock(dmutex(kb));         \
        continue;                                \
    }

void reactivate_devices(){
    ckb_info("System has woken from sleep");
    for(int i = 1; i < DEV_MAX; i++){
        usbdevice* kb = keyboard + i;
        queued_mutex_lock(dmutex(kb));
        if(kb->status == DEV_STATUS_CONNECTED && !NEEDS_FW_UPDATE(kb) && kb->active){
            int ret;
            // If the device was active, mark it as disabled and re-enable it
            kb->active = 0;
            const devcmd* vt = &kb->vtable;
            TRY_WITH_RESET_NOFREE(vt->active(kb, 0, 0, 0, 0), ret);
            TRY_WITH_RESET_NOFREE(vt->updatergb(kb, 1), ret);
            TRY_WITH_RESET_NOFREE(vt->updatedpi(kb, 1), ret);
        }
        queued_mutex_unlock(dmutex(kb));
    }
}
