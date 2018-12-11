#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <libusbk.h>
#include <windows.h>

#ifdef OS_WINDOWS

static int kbdevaddress[DEV_MAX] = {[0 ... DEV_MAX-1] = -1};
static int kbbusnumber[DEV_MAX] = {[0 ... DEV_MAX-1] = -1};
#define HAS_ALL_HANDLES(kb) ((kb)->ifcount && (kb)->handlecount == (kb)->ifcount)

KUSB_DRIVER_API kUSB;

int os_usbsend(usbdevice* kb, const uchar* out_msg, int is_recv, const char* file, int line) {
    return 64;
}

int os_usbrecv(usbdevice* kb, uchar* in_msg, const char* file, int line){
    return 64;
}

int _nk95cmd(usbdevice* kb, uchar bRequest, ushort wValue, const char* file, int line){
    return 64;
}

void os_sendindicators(usbdevice* kb) {

}

void* os_inputmain(void* context){
    usbdevice* kb = context;
    return 0;
}

static int usbunclaim(usbdevice* kb, int resetting) {
    return 0;
}

void os_closeusb(usbdevice* kb){

}

static int usbclaim(usbdevice* kb){
    return 0;
}

int os_resetusb(usbdevice* kb, const char* file, int line) {
    return 0;
}

int os_setupusb(usbdevice* kb) {
    return 0;
}

VOID KUSB_API OnHotPlug(
    KHOT_HANDLE Handle,
    KLST_DEVINFO_HANDLE DeviceInfo,
    KLST_SYNC_FLAG NotificationType)
{
    UNREFERENCED_PARAMETER(Handle);

    // Write arrival/removal event notifications to console output as they occur.
    /*printf(
        "\n"
        "[%s] %s (%s) [%s]\n"
        "  InstanceID          : %s\n"
        "  DeviceInterfaceGUID : %s\n"
        "  DevicePath          : %s\n"
        "  \nthing %d\n",
        NotificationType == KLST_SYNC_FLAG_ADDED ? "ARRIVAL" : "REMOVAL",
        DeviceInfo->DeviceDesc,
        DeviceInfo->Mfg,
        DeviceInfo->Service,
        DeviceInfo->DeviceID,
        DeviceInfo->DeviceInterfaceGUID,
        DeviceInfo->DevicePath,
        DeviceInfo->DeviceAddress);*/


}

/*static int parseDevPath(char* in, char* out){
    if(strlen(in) > FILENAME_MAX - 1)
        return 1;

    char* strend = strrchr(in, '&');
    if(strend == NULL)
        return 1;

    // Need to be sure the dest str is zeroed out
    strncpy(out, in, strend - in);
    // Replace the interface number with 0
    out[30] = '0';
    //printf("%s\n", out);
    return 0;
}*/

int usbadd(KLST_DEVINFO_HANDLE deviceInfo, short vendor, short product) {
#ifdef DEBUG
    ckb_info(">>>vendor = 0x%x, product = 0x%x, path = %s, syspath = %s\n", vendor, product, path, syspath);
#endif // DEDBUG

    // Search for a matching address and bus number
    for(int index = 1; index < DEV_MAX; index++){
        if(kbbusnumber[index] == deviceInfo->BusNumber && kbdevaddress[index] == deviceInfo->DeviceAddress)
        {
            ckb_info("We have a match for %d!\n", index);
            usbdevice* kb = keyboard + index;
            if(pthread_mutex_trylock(dmutex(kb)))
                continue;

            ckb_info("Match found at %d\n", index);
            if(HAS_ALL_HANDLES(kb)){
                ckb_err("Tried to add a handle to a device that already has all handles. Expect things to break!\n");
                pthread_mutex_unlock(dmutex(kb));
                return 0;
            }
            kUSB.Init(kb->handle + deviceInfo->Common.MI, deviceInfo);
            kb->handlecount++;
            // Check if we now have all the handles
            if(HAS_ALL_HANDLES(kb)){
                ckb_info("We have all handles now for %d!\n", index);
                // Mutex remains locked
                setupusb(kb);
                return 0;
            }
            pthread_mutex_unlock(dmutex(kb));
            return 0;
        }
    }
    // Find a free USB slot
    for(int index = 1; index < DEV_MAX; index++){
        usbdevice* kb = keyboard + index;
        if(pthread_mutex_trylock(dmutex(kb)))
            continue;

        if(!IS_CONNECTED(kb)){
            ckb_info("Connecting %s at %s%d\n", deviceInfo->DeviceDesc, devpath, index);
            // On success:
            // Set up device
            kb->vendor = vendor;
            kb->product = product;
            kbbusnumber[index] = deviceInfo->BusNumber;
            kbdevaddress[index] = deviceInfo->DeviceAddress;
            kb->handlecount = 1;
            kUSB.Init(kb->handle + 0, deviceInfo);
            // FIXME: We need to probe the device for this
            kb->ifcount = 2;
            pthread_mutex_unlock(dmutex(kb));
            return 0;
        }
        pthread_mutex_unlock(dmutex(kb));
    }
    ckb_err("No free devices\n");
    return -1;
}

static int usb_add_device(KLST_DEVINFO_HANDLE deviceInfo){
    if(deviceInfo->Common.Vid == V_CORSAIR){
        for(size_t c = 0; c < N_MODELS; c++){
            if(models[c] == deviceInfo->Common.Pid)
                return usbadd(deviceInfo, V_CORSAIR, models[c]);
        }
    }
    return 1;
}

// This function checks if any of the devices match
static BOOL KUSB_API ShowDevicesCB(KLST_HANDLE DeviceList,
                                   KLST_DEVINFO_HANDLE deviceInfo,
                                   PVOID MyContext)
{
    // print some information about the device.
    /*printf("%04X:%04X (%s): %s - %s\n",
           deviceInfo->Common.Vid,
           deviceInfo->Common.Pid,
           deviceInfo->Common.InstanceID,
           deviceInfo->DeviceDesc,
           deviceInfo->Mfg);
    printf("More info %s - %s - %d - %s\n", deviceInfo->ClassGUID, deviceInfo->DevicePath, deviceInfo->DeviceAddress, deviceInfo->DeviceID);
    printf("even More info %d\n", deviceInfo->Common.MI);*/


    usb_add_device(deviceInfo);
    // If this function returns FALSE then enumeration ceases.
    return TRUE;
}

int usbmain(){
    LibK_LoadDriverAPI(&kUSB, 0);
    /*KHOT_HANDLE hotHandle = NULL;
    KHOT_PARAMS hotParams;

    memset(&hotParams, 0, sizeof(hotParams));
    hotParams.OnHotPlug = OnHotPlug;
    hotParams.Flags |= KHOT_FLAG_PLUG_ALL_ON_INIT;

    // A "real world" application should set a specific device interface guid if possible.
    // strcpy(hotParams.PatternMatch.DeviceInterfaceGUID, "{F676DCF6-FDFE-E0A9-FC12-8057DBE8E4B8}");
    strcpy(hotParams.PatternMatch.DeviceInterfaceGUID, "*");

    printf("Initialize a HotK device notification event monitor..\n");
    printf("Looking for 'DeviceInterfaceGUID's matching the pattern '%s'..\n", hotParams.PatternMatch.DeviceInterfaceGUID);

    // Initializes a new HotK handle.
    if (!HotK_Init(&hotHandle, &hotParams))
    {
        int errorCode = GetLastError();
        printf("HotK_Init failed. ErrorCode: %08Xh\n",  errorCode);
    }*/

    // List all devices and connect any existing ones
    KLST_HANDLE deviceList = NULL;
    UINT count = 0;

    /*
    Initialize a new LstK (device list) handle.
    The list is polulated with all usb devices libusbK can access.
    */
    if (!LstK_Init(&deviceList, 0))
    {
        ckb_err("Something happened\n");
    }

    // Get the number of devices contained in the device list.
    LstK_Count(deviceList, &count);
    if (!count)
    {
        printf("No devices connected.\n");

        // Always free the device list if LstK_Init returns TRUE
        LstK_Free(deviceList);

    }
    LstK_Enumerate(deviceList, ShowDevicesCB, NULL);

    // Free the device list
    LstK_Free(deviceList);

    while(1)
        sleep(1);
    return 0;
}

void usbkill(){

}

#endif
