#include "device.h"
#include "devnode.h"
#include "input.h"
#include "notify.h"
#include "usb.h"
#include <stdio.h>
#include <stdlib.h>
#include <libusbk.h>
#include <windows.h>

#ifdef OS_WINDOWS

static int kbdevaddress[DEV_MAX] = {[0 ... DEV_MAX-1] = -1};
static int kbbusnumber[DEV_MAX] = {[0 ... DEV_MAX-1] = -1};
#define HAS_ALL_HANDLES(kb) ((kb)->ifcount && (kb)->handlecount == (kb)->ifcount)
static int HotKRunning = 1;

typedef struct _DESCRIPTOR_ITERATOR
{
    LONG	Remaining;
    union
    {
        PUCHAR							Bytes;
        PUSB_COMMON_DESCRIPTOR			Common;
        PUSB_CONFIGURATION_DESCRIPTOR	Config;
        PUSB_INTERFACE_DESCRIPTOR		Interface;
        PUSB_ENDPOINT_DESCRIPTOR		Endpoint;
    } Ptr;
} DESCRIPTOR_ITERATOR, *PDESCRIPTOR_ITERATOR;

KUSB_DRIVER_API kUSB;

BOOL InitDescriptorIterator(PDESCRIPTOR_ITERATOR descIterator, BYTE* configDescriptor, DWORD lengthTransferred)
{
    memset(descIterator, 0, sizeof(*descIterator));
    descIterator->Ptr.Bytes		= configDescriptor;
    descIterator->Remaining		= descIterator->Ptr.Config->wTotalLength;

    if (lengthTransferred > sizeof(USB_CONFIGURATION_DESCRIPTOR) && lengthTransferred >= descIterator->Ptr.Config->wTotalLength)
    {
        if ( descIterator->Ptr.Config->wTotalLength >= sizeof(USB_CONFIGURATION_DESCRIPTOR) + sizeof(USB_INTERFACE_DESCRIPTOR))
            return TRUE;
    }

    SetLastError(ERROR_BAD_LENGTH);
    descIterator->Remaining = 0;
    return FALSE;
}

int os_usbsend(usbdevice* kb, const uchar* out_msg, int is_recv, const char* file, int line) {
    BOOL success = 1;
    UINT len = 0;
    if (kb->fwversion >= 0x120 || IS_V2_OVERRIDE(kb)){
        // If we need to read a response, lock the interrupt mutex
        if(is_recv)
            if(pthread_mutex_lock(intmutex(kb)))
                ckb_fatal("Error locking interrupt mutex in os_usbsend()\n");
        success = kUSB.WritePipe(kb->handle[kb->ifcount - 1], kb->ifcount, (uchar*)out_msg, MSG_SIZE, &len, NULL);
    } else {
        // Note, Ctrl Transfers require an index, not an endpoint, which is why kb->epcount - 1 works
        /*struct usbdevfs_ctrltransfer transfer = { 0x21, 0x09, 0x0200, kb->epcount - 1, MSG_SIZE, 5000, (void*)out_msg };
        res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, &transfer);*/
    }

    if (!success){
        int ioctlerrno = GetLastError();
        ckb_err_fn(" %s, res = 0x%x\n", file, line, len ? strerror(ioctlerrno) : "No data written", len);
        /*if(res == -1 && ioctlerrno == ETIMEDOUT){
            if(is_recv)
                pthread_mutex_unlock(intmutex(kb));
            return -1;
        } else {
            if(is_recv)
                pthread_mutex_unlock(intmutex(kb));
            return 0;
        }*/
    } else if (len != MSG_SIZE)
        ckb_warn_fn("Wrote %d bytes (expected %d)\n", file, line, len, MSG_SIZE);
#ifdef DEBUG_USB_SEND
    print_urb_buffer("Sent:", out_msg, MSG_SIZE, file, line, __func__, INDEX_OF(kb, keyboard));
#endif

    return len;
}

int os_usbrecv(usbdevice* kb, uchar* in_msg, const char* file, int line){
    int res = 0;
    if(kb->fwversion >= 0x120 || IS_V2_OVERRIDE(kb)){
        // Wait for 2s
        struct timespec condwait = {0};
        condwait.tv_sec = time(NULL) + 2;
        int condret = pthread_cond_timedwait(intcond(kb), intmutex(kb), &condwait);
        if(condret != 0){
            if(pthread_mutex_unlock(intmutex(kb)))
                ckb_fatal("Error unlocking interrupt mutex in os_usbrecv()\n");
            if(condret == ETIMEDOUT)
                ckb_warn_fn("ckb%d: Timeout while waiting for response\n", file, line, INDEX_OF(kb, keyboard));
            else
                ckb_warn_fn("Interrupt cond error %i\n", file, line, condret);
            return -1;
        }
        memcpy(in_msg, kb->interruptbuf, MSG_SIZE);
        memset(kb->interruptbuf, 0, MSG_SIZE);
        res = MSG_SIZE;
        if(pthread_mutex_unlock(intmutex(kb)))
            ckb_fatal("Error unlocking interrupt mutex in os_usbrecv()\n");
    } /*else {
        struct usbdevfs_ctrltransfer transfer = { 0xa1, 0x01, 0x0300, kb->epcount - 1, MSG_SIZE, 5000, in_msg };
        res = ioctl(kb->handle - 1, USBDEVFS_CONTROL, &transfer);
        if(res <= 0){
            // This is done because ckb_err_fn can set errno itself
            int ioctlerrno = errno;
            ckb_err_fn("%s\n", file, line, res ? strerror(ioctlerrno) : "No data read");
            if(res == -1 && ioctlerrno == ETIMEDOUT)
                return -1;
            else
                return 0;
        } else if(res != MSG_SIZE)
            ckb_warn_fn("Read %d bytes (expected %d)\n", file, line, res, MSG_SIZE);
    }*/

#ifdef DEBUG_USB_RECV
    print_urb_buffer("Recv:", in_msg, MSG_SIZE, file, line, __func__, INDEX_OF(kb, keyboard));
#endif

    return res;
}

int _nk95cmd(usbdevice* kb, uchar bRequest, ushort wValue, const char* file, int line){
    return 64;
}

void os_sendindicators(usbdevice* kb) {

}

void* os_inputmain(void* context){
    usbdevice* kb = context;
    uchar buffer[64];
    unsigned int len;
    BOOL success = 1;
    while(1) {
        success = kUSB.ReadPipe(kb->handle[0], 0x81, buffer, sizeof(buffer), &len, NULL);
        //printf("Success: %d\n", success);
        if(!success)
            break;
        process_input_urb(kb, buffer, len, 0x81);
    }
    return 0;
}

void os_closeusb(usbdevice* kb){

}

int os_resetusb(usbdevice* kb, const char* file, int line) {
    return 0;
}

int os_setupusb(usbdevice* kb) {
    return 0;
}

static int get_UTF8_string_desc(HANDLE* handle, UCHAR iString, char* out, size_t out_len){
    WINUSB_SETUP_PACKET Pkt;
    KUSB_SETUP_PACKET* kPkt = (KUSB_SETUP_PACKET*)&Pkt;
    BYTE buffer[128];
    memset(&buffer, 0, sizeof(buffer));
    memset(&Pkt, 0, sizeof(Pkt));
    kPkt->BmRequest.Dir = BMREQUEST_DIR_DEVICE_TO_HOST;
    kPkt->Request = USB_REQUEST_GET_DESCRIPTOR;
    kPkt->ValueHi = USB_DESCRIPTOR_TYPE_STRING;
    kPkt->ValueLo = iString;
    kPkt->Length = (USHORT)sizeof(buffer);
    UINT lengthTransferred;
    if (!kUSB.ControlTransfer(handle, Pkt, buffer, sizeof(buffer), &lengthTransferred, NULL))
        ckb_err("Something elsest happened %ld\n", GetLastError());

    // Sanity check:
    // 0x01 is bDescriptorType, which should be 0x03 for string descriptor
    if(buffer[0x01] == 0x03) {
        size_t outlen = out_len;
        size_t srclen = buffer[0] - 2;
        u16dec_char((char*)(buffer + 2), out, &srclen, &outlen);
    }
    return 1;
}

int usbadd(KLST_DEVINFO_HANDLE deviceInfo, short vendor, short product) {
#ifdef DEBUG
    ckb_info(">>>vendor = 0x%x, product = 0x%x, path = %s, syspath = %s\n", vendor, product, path, syspath);
#endif // DEDBUG

    // Search for a matching address and bus number
    for(int index = 1; index < DEV_MAX; index++){
        if(kbbusnumber[index] == deviceInfo->BusNumber && kbdevaddress[index] == deviceInfo->DeviceAddress){
            usbdevice* kb = keyboard + index;
            if(pthread_mutex_trylock(dmutex(kb)))
                continue;

            ckb_info("Match found for %s at %d\n", deviceInfo->DeviceDesc, index);
            if(HAS_ALL_HANDLES(kb)){
                ckb_err("Tried to add a handle to a device that already has all handles. Expect things to break!\n");
                pthread_mutex_unlock(dmutex(kb));
                return 0;
            }
            kUSB.Init(kb->handle + deviceInfo->Common.MI, deviceInfo);
            kb->handlecount++;
            // Check if we now have all the handles
            if(HAS_ALL_HANDLES(kb)){
                ckb_info("We now have all the handles for ckb%d! Setting up the device...\n", index);
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
            // Set up device
            kb->vendor = vendor;
            kb->product = product;
            kbbusnumber[index] = deviceInfo->BusNumber;
            kbdevaddress[index] = deviceInfo->DeviceAddress;
            kb->handlecount = 1;
            kUSB.Init(kb->handle + 0, deviceInfo);
            // Get descriptor
            BYTE configDescriptorBuffer[4096];
            UINT lengthTransferred;
            WINUSB_SETUP_PACKET Pkt;
            KUSB_SETUP_PACKET* kPkt = (KUSB_SETUP_PACKET*)&Pkt;
            memset(&Pkt, 0, sizeof(Pkt));
            kPkt->BmRequest.Dir = BMREQUEST_DIR_DEVICE_TO_HOST;
            kPkt->Request = USB_REQUEST_GET_DESCRIPTOR;
            kPkt->ValueHi = USB_DESCRIPTOR_TYPE_CONFIGURATION;
            kPkt->ValueLo = 0;
            kPkt->Length = (USHORT)sizeof(configDescriptorBuffer);

            if (!kUSB.ControlTransfer(kb->handle[0], Pkt, configDescriptorBuffer, sizeof(configDescriptorBuffer), &lengthTransferred, NULL))
                ckb_err("Something happened %ld\n", GetLastError());
            // Iterate
            DESCRIPTOR_ITERATOR descIterator;
            if (!InitDescriptorIterator(&descIterator, configDescriptorBuffer, lengthTransferred))
                ckb_err("Something else happened %ld\n", GetLastError());

            ckb_info("ckb%d is supposed to have %d interfaces\n", index, descIterator.Ptr.Config->bNumInterfaces);
            kb->ifcount = (descIterator.Ptr.Config->bNumInterfaces ? descIterator.Ptr.Config->bNumInterfaces : 2);

            // Get device descriptor (for fw version in bcdDevice)
            BYTE devDescrBuf[4096];
            memset(&devDescrBuf, 0, sizeof(devDescrBuf));
            memset(&Pkt, 0, sizeof(Pkt));
            kPkt->BmRequest.Dir = BMREQUEST_DIR_DEVICE_TO_HOST;
            kPkt->Request = USB_REQUEST_GET_DESCRIPTOR;
            kPkt->ValueHi = USB_DESCRIPTOR_TYPE_DEVICE;
            kPkt->Length = (USHORT)sizeof(devDescrBuf);
            if (!kUSB.ControlTransfer(kb->handle[0], Pkt, devDescrBuf, sizeof(devDescrBuf), &lengthTransferred, NULL))
                ckb_err("Something elser happened %ld\n", GetLastError());
            USB_DEVICE_DESCRIPTOR devDescr;
            memcpy(&devDescr, devDescrBuf, sizeof(devDescr));
            kb->fwversion = devDescr.bcdDevice;
            ckb_info("bcdDevice %x\n", kb->fwversion);

            // Get strings (serial/dev name)
            /*devDescr.iSerialNumber;
            devDescr.iManufacturer;
            */

            get_UTF8_string_desc(kb->handle[0], devDescr.iSerialNumber, kb->serial, SERIAL_LEN - 1);
            get_UTF8_string_desc(kb->handle[0], devDescr.iProduct, kb->name, KB_NAME_LEN);

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

void KUSB_API OnHotPlug(KHOT_HANDLE Handle, KLST_DEVINFO_HANDLE deviceInfo, KLST_SYNC_FLAG NotificationType)
{
    UNREFERENCED_PARAMETER(Handle);

    if(NotificationType == KLST_SYNC_FLAG_ADDED)
        usb_add_device(deviceInfo);
    else
        ckb_fatal("FIXME: Device removed. Things will break\n");
}

int usbmain(){
    LibK_LoadDriverAPI(&kUSB, 0);
    KHOT_HANDLE hotHandle = NULL;
    KHOT_PARAMS hotParams;

    memset(&hotParams, 0, sizeof(hotParams));
    hotParams.OnHotPlug = OnHotPlug;
    hotParams.Flags |= KHOT_FLAG_PLUG_ALL_ON_INIT;

    strcpy(hotParams.PatternMatch.DeviceInterfaceGUID, "*");

    if (!HotK_Init(&hotHandle, &hotParams))
    {
        int errorCode = GetLastError();
        ckb_fatal("Error initialising HotK: %08Xh\n",  errorCode);
        return errorCode;
    }

    while(HotKRunning)
        sleep(1);

    if(hotHandle != NULL)
        HotK_Free(hotHandle);

    return 0;
}

void usbkill(){

}

#endif
