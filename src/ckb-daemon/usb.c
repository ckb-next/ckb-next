#include "command.h"
#include "device.h"
#include "devnode.h"
#include "firmware.h"
#include "input.h"
#include "led.h"
#include "notify.h"
#include "profile.h"
#include "usb.h"

/// brief .
///
/// \brief usbmutex is a never referenced mutex!
///
/// \todo We should have a look why this mutex is never used.
///
pthread_mutex_t usbmutex = PTHREAD_MUTEX_INITIALIZER;

/// brief .
///
/// \brief reset_stop is boolean: Reset stopper for when the program shuts down.
///
/// Is set only by \a quit() to true (1)
/// to inform several usb_* functions to end their loops and tries.
volatile int reset_stop = 0;

/// brief .
///
/// \brief features_mask Mask of features to exclude from all devices
///
/// That bit mask ist set to enable all (-1).
/// When interpreting the input parameters, some of these bits can be cleared.
/// \n At the moment binding, notifying and mouse-acceleration can be disabled via command line.
/// \n Have a look at \a main() in main.c for details.
int features_mask = -1;

/// brief .
///
/// vendor_str returns "corsair" if the given \a vendor argument is equal to \a V_CORSAIR \c (0x1bc)
/// else it returns ""
///
/// \attention There is also a string defined V_CORSAIR_STR, which returns the device number as string in hex "1b1c".
const char* vendor_str(short vendor){
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
const char* product_str(short product){
    if(product == P_K95 || product == P_K95_NRGB || product == P_K95_PLATINUM)
        return "k95";
    if(product == P_K70 || product == P_K70_NRGB || product == P_K70_LUX || product == P_K70_LUX_NRGB || product == P_K70_RFIRE || product == P_K70_RFIRE_NRGB)
        return "k70";
    if(product == P_K65 || product == P_K65_NRGB || product == P_K65_LUX || product == P_K65_RFIRE)
        return "k65";
    if(product == P_K63_NRGB)
        return "k63";
    if(product == P_STRAFE || product == P_STRAFE_NRGB)
        return "strafe";
    if(product == P_M65 || product == P_M65_PRO)
        return "m65";
    if(product == P_SABRE_O || product == P_SABRE_L || product == P_SABRE_N || product == P_SABRE_O2)
        return "sabre";
    if(product == P_SCIMITAR || product == P_SCIMITAR_PRO)
        return "scimitar";
    if(product == P_HARPOON)
        return "harpoon";
    if(product == P_GLAIVE)
        return "glaive";
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
static const devcmd* get_vtable(short vendor, short product){
    return IS_MOUSE(vendor, product) ? &vtable_mouse : IS_RGB(vendor, product) ? &vtable_keyboard : &vtable_keyboard_nonrgb;
}

// USB device main loop
/// brief .
///
/// \brief devmain is called by _setupusb
/// \param kb the pointer to the device. Even if it has the name kb, it is valid also for a mouse (the whole driver seems to be implemented first for a keyboard).
/// \return always a nullptr
///
/// # Synchronization
/// The syncing via mutexes is interesting:
/// 1. \a imutex (the Input mutex)\n
/// This one is locked in \c setupusb().
/// That function does only two things: Locking the mutex and trying to start a thread at \c _setupusb().
/// _setupusb() unlocks \a imutex  after getting some buffers and initalizing internal structures from the indicators
/// (this function often gets problems with error messages like "unable to read indicators" or "Timeout bla blubb").
/// \warning have a look at \a updateindicators() later.
/// \warning if creating the thread is not successful, the \a imutex remains blocked. Have a look at setupusb() later.
///
/// 2. \a dmutex (the Device mutex)\n
/// This one is very interesting, because it is handled in devmain().
/// It seems that it is locked only in \a _ledthread(), which is a thread created in \a os_setupindicators().
/// os_setupindicators() again is called in \a _setupusb() long before calling devmain().
/// So this mutex is locked when we start the function as the old comment says.\n
/// Before reading from the FIFO and direct afterwards an unlock..lock sequence is implemented here.
/// Even if only the function readlines() should be surrounded by the unlock..lock,
/// the variable definition of the line pointer is also included here. Not nice, but does not bother either.
/// Probably the Unlock..lock is needed so that now another process can change the control structure \a linectx while we wait in readlines().
/// \todo Hope to find the need for dmutex usage later.
/// \n Should this function be declared as pthread_t* function, because of the defintion of pthread-create? But void* works also...
///
static void* devmain(usbdevice* kb){
    /// \attention dmutex should still be locked when this is called
    int kbfifo = kb->infifo - 1;
    ///
    /// First a \a readlines_ctx buffer structure is initialized by \c readlines_ctx_init().
    readlines_ctx linectx;
    readlines_ctx_init(&linectx);
    ///
    /// After some setup functions, beginning in _setupusb() which has called devmain(),
    /// we read the command input-Fifo designated to that device in an endless loop.
    /// This loop has two possible exits (plus reaction to signals, not mentioned here).
    while(1){
        ///
        /// If the reading via readlines() is successful (we might have read multiple lines),
        /// the interpretation is done by readcmd() iff the connection to the device is still available
        /// (checked via IS_CONNECTED(kb)).
        /// This is true if the kb-structure has a handle and an event pointer both != Null).
        /// If not, the loop is left (the first exit point).
        pthread_mutex_unlock(dmutex(kb));
        // Read from FIFO
        const char* line;
        int lines = readlines(kbfifo, linectx, &line);
        pthread_mutex_lock(dmutex(kb));
        // End thread when the handle is removed
        if(!IS_CONNECTED(kb))
            break;
        ///
        /// if nothing is in the line buffer (some magic interrupt?),
        /// continue in the endless while without any reaction.
        if(lines){
            /// \todo readcmd() gets a \b line, not \b lines. Have a look on that later.
            /// \n Is the condition IS_CONNECTED valid? What functions change the condititon for the macro?
            if(readcmd(kb, line)){
                ///
                /// If interpretation and communication with the usb device got errors,
                /// they are signalled by readcmd() (non zero retcode).
                /// In this case the usb device is closed via closeusb()
                /// and the endless loop is left (the second exit point).
                // USB transfer failed; destroy device
                closeusb(kb);
                break;
            }
        }
    }
    pthread_mutex_unlock(dmutex(kb));
    ///
    /// After leaving the endless loop the readlines-ctx structure and its buffers are freed by readlines_ctx_free().
    readlines_ctx_free(linectx);
    return 0;
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
    // Set standard fields
    short vendor = kb->vendor, product = kb->product;
    const devcmd* vt = kb->vtable = get_vtable(vendor, product);
    kb->features = (IS_RGB(vendor, product) ? FEAT_STD_RGB : FEAT_STD_NRGB) & features_mask;
    if(IS_MOUSE(vendor, product)) kb->features |= FEAT_ADJRATE;
    if(IS_MONOCHROME(vendor, product)) kb->features |= FEAT_MONOCHROME;
    kb->usbdelay = USB_DELAY_DEFAULT;

    // Perform OS-specific setup
    ///
    /// - A fixed 100ms wait is the start.
    /// <b>Although the DELAY_LONG macro is given a parameter, it is ignored. Occasionally refactor it.</b>
    DELAY_LONG(kb);

    ///
    /// - The first relevant point is the operating system-specific opening of the interface in os_setupusb().
    /// As a result, some parameters should be set in kb (name, serial, fwversion, epcount = number of usb endpoints),
    /// and all endpoints should be claimed with usbclaim().
    /// Claiming is the only point where os_setupusb() can produce an error (-1, otherwise 0).
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
    if(pthread_create(&kb->inputthread, 0, os_inputmain, kb))
        goto fail;
    pthread_detach(kb->inputthread);
    ///
    /// - The same happens with os_setupindicators(),
    /// which initially initializes all LED variables in kb to off and then starts the _ledthread() thread
    /// with kb as parameter and then detaches it.
    /// Here again only the generation of the thread can fail.
    if(os_setupindicators(kb))
        goto fail;

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
    /// - Not completely understandable is why now via the vtable the function updateindicators() is called.
    /// But this actually happens in the just started thread _ledthread().
    /// Either the initialization is wrong und must done here with force or the overview is lost, what happens when...\n
    /// Regardless: For a mouse nothing happens here, for a keyboard updateindicators_kb() is called via the entry in kb->vtable.
    /// The first parameter is kb again, the second is constant 1 (means force = true).
    /// This causes the LED status to be sent after a 5ms delay via os_sendindicators()
    /// (ioctl with a \c usbdevfs_ctrltransfer).
    /// \n The notification is sent to all currently open notification channels then.
    /// \n Setupindicators() and with it updateindicators_kb() can fail.
    vt->updateindicators(kb, 1);
    ///
    /// - From this point - if an error is detected - the error label is addressed by goto statement,
    /// which first performs an unlock on the imutex.
    /// This is interesting because the next statement is exactly this: An unlock on the imutex.
    pthread_mutex_unlock(imutex(kb));
    ///
    /// - Via vtable the \a kb->start() function is called next.
    /// This is the same for a mouse and an RGB keyboard: start_dev(),
    /// for a non RGB keyboard it is start_kb_nrgb().
    /// \n First parameter is as always kb, second is 0 (makeactive = false).
    ///   - In start_kb_nrgb() set the keyboard into a so-called software mode (NK95_HWOFF)
    /// via ioctl with \c usbdevfs_ctrltransfer in function _nk95cmd(),
    /// which will in turn is called via macro nk95cmd() via start_kb_nrgb().
    /// \n Then two dummy values (active and pollrate) are set in the kb structure and ready.
    ///   - start_dev() does a bit more - because this function is for both mouse and keyboard.
    /// start_dev() calls - after setting an extended timeout parameter - _start_dev(). Both are located in device.c.
    ///   - First, _start_dev() attempts to determine the firmware version of the device,
    /// but only if two conditions are met:
    /// hwload-mode is not null (then hw-loading is disabled)
    /// and the device has the FEAT_HWLOAD feature.
    /// Then the firmware and the poll rate are fetched via getfwversion().
    /// \n If hwload_mode is set to "load only once" (==1), then the HWLOAD feature is masked, so that no further reading can take place.
    ///   - Now check if device needs a firmware update. If so, set it up and leave the function without error.
    ///   - Else load the hardware profile from device if the hw-pointer is not set and hw-loading is possible and allowed.
    /// \n Return error if mode == 2 (load always) and loading got an error.
    /// Else mask the HWLOAD feature, because hwload must be 1 and the error couold be a repeated hw-reading.
    /// \n <b>Puh, that is real Horror code. It seems to be not faulty, but completely unreadable.</b>
    ///
    ///   - Finally, the second parameter of _startdev() is used to check whether the device is to be activated.
    /// Depending on the parameter, the active or the idle-member in the correspondig vtable is called.
    /// These are device-dependent again:
    /// Device | active | idle
    /// ------ | ------ | ----
    /// RGB Keyboard | cmd_active_kb() means: start the device with a lot of kb-specific initializers (software controlled mode) | cmd_idle_kb() set the device with a lot of kb-specific initializers into the hardware controlled mode)
    /// non RGB Keyboard | cmd_io_none() means: Do nothing | cmd_io_none() means: Do nothing
    /// Mouse | cmd_active_mouse() similar to cmd_active_kb() | cmd_idle_mouse similar to cmd_idle_kb()
    ///
    /// - If either \a start() succeeded or the next following usb_tryreset(), it goes on, otherwise again a hard abort occurs.
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
    ckb_info("Setup finished for %s%d\n", devpath, index);
    updateconnected();
    ///
    /// devmain()'s return value is returned by _setupusb() when we terminate.
    return devmain(kb);
    ///
    /// - The remaining code lines are the two exit labels as described above
    fail:
    pthread_mutex_unlock(imutex(kb));
    fail_noinput:
    closeusb(kb);
    pthread_mutex_unlock(dmutex(kb));
    return 0;
}

/// \brief .
///
/// Set up a USB device after its handle is open. Spawns a new thread _setupusb() with standard parameter kb.
/// dmutex must be locked prior to calling this function. The function will unlock it when finished.
/// In kb->thread the thread id is mentioned, because closeusb() needs this info for joining that thread again.
///
void setupusb(usbdevice* kb){
    pthread_mutex_lock(imutex(kb));
    if(pthread_create(&kb->thread, 0, _setupusb, kb))
        ckb_err("Failed to create USB thread\n");
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
    if(!HAS_FEATURES(kb, FEAT_RGB)){
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
    DELAY_LONG(kb);
    int res = os_resetusb(kb, file, line);
    if(res)
        return res;
    DELAY_LONG(kb);
    // Re-initialize the device
    if(kb->vtable->start(kb, kb->active) != 0)
        return -1;
    if(kb->vtable->updatergb(kb, 1) != 0)
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
    ckb_info("Attempting reset...\n");
    while(1){
        int res = resetusb(kb);
        if(!res){
            ckb_info("Reset success\n");
            return 0;
        }
        if(res == -2 || reset_stop)
            break;
    }
    ckb_err("Reset failed. Disconnecting.\n");
    return -1;
}

///
/// hwload_mode is defined in device.c
///
extern int hwload_mode;

/// \brief .
///
/// \todo A lot of different conditions are combined in this code. Don't think, it is good in every combination...
///
/// The main task of _usbsend () is to transfer the complete logical message from the buffer beginning with \a messages to <b>count * MSG_SIZE</b>.
/// \n According to usb 2.0 specification, a USB transmits a maximum of 64 byte user data packets.
/// For the transmission of longer messages we need a segmentation.
/// And that is exactly what happens here.
///
/// The message is given one by one to os_usbsend() in MSG_SIZE (= 64) byte large bites.
/// \attention This means that the buffer given as argument must be n * MSG_SIZE Byte long.
///
/// An essential constant parameter which is relevant for os_usbsend() only is is_recv = 0, which means sending.
///
/// Now it gets a little complicated again:
/// - If os_usbsend() returns 0, only zero bytes could be sent in one of the packets,
/// or it was an error (-1 from the systemcall), but not a timeout.
/// How many Bytes were sent in total from earlier calls does not seem to matter,
/// _usbsend() returns a total of 0.
/// - Returns os_usbsend() -1, first check if \b reset_stop is set globally
/// or (incomprehensible) hwload_mode is not set to "always".
/// In either case, _usbsend() returns 0,
/// otherwise it is assumed to be a temporary transfer error and it simply retransmits the physical packet after a long delay.
/// - If the return value of os_usbsend() was neither 0 nor -1,
/// it specifies the numer of bytes transferred.
/// \n Here is an information hiding conflict with os_usbsend() (at least in the Linux version):
/// \n If os_usbsend() can not transfer the entire packet,
/// errors are thrown and the number of bytes sent is returned.
/// _usbsend() interprets this as well
/// and remembers the total number of bytes transferred in the local variable \b total_sent.
/// Subsequently, however, transmission is continued with the next complete MSG_SIZE block
/// and not with the first of the possibly missing bytes.
/// \todo Check whether this is the same in the macOS variant. It is not dramatic, but if errors occur, it can certainly irritate the devices completely if they receive incomplete data streams. Do we have errors with the messages "Wrote YY bytes (expected 64)" in the system logs? If not, we do not need to look any further.
///
/// When the last packet is transferred,
/// _usbsend() returns the effectively counted set of bytes (from \b total_sent).
/// This at least gives the caller the opportunity to check whether something has been lost in the middle.
///
/// A bit strange is the structure of the program:
/// Handling the \b count MSG_SIZE blocks to be transferred is done
/// in the outer for (...) loop.
/// Repeating the transfer with a treatable error is managed by the inner while(1) loop.
/// \n This must be considered when reading the code;
/// The "break" on successful block transfer leaves the inner while, not the for (...).
///
int _usbsend(usbdevice* kb, const uchar* messages, int count, const char* file, int line){
    int total_sent = 0;
    for(int i = 0; i < count; i++){
        // Send each message via the OS function
        while(1){
            pthread_mutex_lock(mmutex(kb)); ///< Synchonization between macro and color information
            DELAY_SHORT(kb);
            int res = os_usbsend(kb, messages + i * MSG_SIZE, 0, file, line);
            pthread_mutex_unlock(mmutex(kb));
            if(res == 0)
                return 0;
            else if(res != -1){
                total_sent += res;
                break;
            }
            // Stop immediately if the program is shutting down or hardware load is set to tryonce
            if(reset_stop || hwload_mode != 2)
                return 0;
            // Retry as long as the result is temporary failure
            DELAY_LONG(kb);
        }
    }
    return total_sent;
}

/// \brief .
///
/// To fully understand this, you need to know about usb:
/// All control is at the usb host (the CPU).
/// If the device wants to communicate something to the host,
/// it must wait for the host to ask.
/// The usb protocol defines the cycles and periods in which actions are to be taken.
///
/// So in order to receive a data packet from the device,
/// the host must first send a send request.
/// \n This is done by _usbrecv() in the first block
/// by sending the MSG_SIZE large data block from \b out_msg via os_usbsend() as it is a machine depending implementation.
/// The usb target device is as always determined over kb.
///
/// For os_usbsend() to know that it is a receive request,
/// the \b is_recv parameter is set to true (1).
/// With this, os_usbsend () generates a control package for the hardware, not a data packet.
///
/// If sending of the control package is not successful,
/// a maximum of 5 times the transmission is repeated (including the first attempt).
/// If a non-cancelable error is signaled or the drive is stopped via reset_stop,
/// _usbrecv() immediately returns 0.
///
/// After this, the function waits for the requested response from the device using os_usbrecv ().
///
/// os_usbrecv() returns 0, -1 or something else.
/// \n Zero signals a serious error which is not treatable and _usbrecv() also returns 0.
/// \n -1 means that it is a treatable error - a timeout for example -
/// and therefore the next transfer attempt is started after a long pause (DELAY_LONG)
/// if not reset_stop or the wrong hwload_mode require a termination with a return value of 0.
///
/// After 5 attempts, _usbrecv () returns and returns 0 as well as an error message.
///
/// When data is received, the number of received bytes is returned.
/// This should always be MSG_SIZE,
/// but os_usbrecv() can also return less.
/// It should not be more,
/// because then there would be an unhandled buffer overflow,
/// but it could be less.
/// This would be signaled in os_usbrecv () with a message.
///
/// The buffers behind \b out_msg and \b in_msg are MSG_SIZE at least (currently 64 Bytes).
/// More is ok but useless, less brings unpredictable behavior.
///
int _usbrecv(usbdevice* kb, const uchar* out_msg, uchar* in_msg, const char* file, int line){
    // Try a maximum of 5 times
    for (int try = 0; try < 5; try++) {
        // Send the output message
        pthread_mutex_lock(mmutex(kb)); ///< Synchonization between macro and color information
        DELAY_SHORT(kb);
        int res = os_usbsend(kb, out_msg, 1, file, line);
        pthread_mutex_unlock(mmutex(kb));
        if (res == 0)
            return 0;
        else if (res == -1) {
            // Retry on temporary failure
            if (reset_stop)
                return 0;
            DELAY_LONG(kb);
            continue;
        }
        // Wait for the response
        DELAY_MEDIUM(kb);
        res = os_usbrecv(kb, in_msg, file, line);
        if(res == 0)
            return 0;
        else if(res != -1)
            return res;
        if(reset_stop || hwload_mode != 2)
            return 0;
        DELAY_LONG(kb);
    }
    // Give up
    ckb_err_fn("Too many send/recv failures. Dropping.\n", file, line);
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
    pthread_mutex_lock(imutex(kb));
    if(kb->handle){
        int index = INDEX_OF(kb, keyboard);
        ckb_info("Disconnecting %s%d\n", devpath, index);
        os_inputclose(kb);
        updateconnected();
        // Close USB device
        os_closeusb(kb);
    } else
        updateconnected();
    rmdevpath(kb);

    // Wait for thread to close
    pthread_mutex_unlock(imutex(kb));
    pthread_mutex_unlock(dmutex(kb));
    pthread_join(kb->thread, 0);
    pthread_mutex_lock(dmutex(kb));

    // Delete the profile and the control path
    if(!kb->vtable)
        return 0;
    kb->vtable->freeprofile(kb);
    memset(kb, 0, sizeof(usbdevice));
    return 0;
}
