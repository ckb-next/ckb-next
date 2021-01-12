// Code based on testvmulti.c
// May need to be rewritten properly at some point

#include "command.h"
#include "device.h"
#include "input.h"

#ifdef OS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "input_windows_vmulti.h"
#include <hidsdi.h>
#include <setupapi.h>

static BYTE keyCodes[KBD_KEY_CODES];
static BYTE shiftKeyFlags;
static BYTE mouseButtons;
static BYTE wheelPosition;

static int add_to_keys(int scancode)
{
    // Find an empty position
    int pos = -1;
    for(int i = 0; i < KBD_KEY_CODES; i++)
    {
        if(!keyCodes[i]){
            pos = i;
            break;
        }
    }

    // Return if the list is full
    if(pos < 0)
        return 1;

    // Check if the scancode is already added by another device
    for(int i = pos; i < KBD_KEY_CODES; i++)
        if(keyCodes[i] == scancode)
            return 0;

    // Finally, if all went well, add the key
    keyCodes[pos] = scancode;
    return 0;
}

static void remove_from_keys(int scancode)
{
    // Search for a key in the array and remove it
    for(int i = 0; i < KBD_KEY_CODES; i++)
        if(keyCodes[i] == scancode)
            keyCodes[i] = 0;
}

static void add_to_field(BYTE* field, int scancode)
{
    *field |= (BYTE)scancode;
}

static void remove_from_field(BYTE* field, int scancode)
{
    *field &= ~(BYTE)scancode;
}

BOOLEAN
CheckIfOurDevice(
    HANDLE file,
    USAGE myUsagePage,
    USAGE myUsage)
{
    PHIDP_PREPARSED_DATA Ppd = NULL; // The opaque parser info describing this device
    HIDD_ATTRIBUTES                 Attributes; // The Attributes of this hid device.
    HIDP_CAPS                       Caps; // The Capabilities of this hid device.
    BOOLEAN                         result = FALSE;

    if (!HidD_GetPreparsedData (file, &Ppd))
    {
        printf("Error: HidD_GetPreparsedData failed \n");
        goto cleanup;
    }

    if (!HidD_GetAttributes(file, &Attributes))
    {
        printf("Error: HidD_GetAttributes failed \n");
        goto cleanup;
    }
    printf("Ven: %d Dev: %d, \n", Attributes.VendorID, Attributes.ProductID);
    if (Attributes.VendorID == VMULTI_VID && Attributes.ProductID == VMULTI_PID)
    {
        if (!HidP_GetCaps (Ppd, &Caps))
        {
            printf("Error: HidP_GetCaps failed \n");
            goto cleanup;
        }
        printf("UsagePage %d Usage %d\n", Caps.UsagePage, Caps.Usage);
        if ((Caps.UsagePage == myUsagePage) && (Caps.Usage == myUsage))
        {
            printf("Success: Found my device.. \n");
            result = TRUE;
        }
    }

cleanup:

    if (Ppd != NULL)
    {
        HidD_FreePreparsedData (Ppd);
    }

    return result;
}


HANDLE
OpenDeviceInterface (
    HDEVINFO hardwareDeviceInfo,
    PSP_DEVICE_INTERFACE_DATA deviceInterfaceData,
    USAGE myUsagePage,
    USAGE myUsage
    )
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;

    DWORD        predictedLength = 0;
    DWORD        requiredLength = 0;
    HANDLE       file = INVALID_HANDLE_VALUE;

    SetupDiGetDeviceInterfaceDetail(
                            hardwareDeviceInfo,
                            deviceInterfaceData,
                            NULL, // probing so no output buffer yet
                            0, // probing so output buffer length of zero
                            &requiredLength,
                            NULL
                            ); // not interested in the specific dev-node

    predictedLength = requiredLength;

    deviceInterfaceDetailData =
         (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc (predictedLength);

    if (!deviceInterfaceDetailData)
    {
        printf("Error: OpenDeviceInterface: malloc failed\n");
        goto cleanup;
    }

    deviceInterfaceDetailData->cbSize =
                    sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

    if (!SetupDiGetDeviceInterfaceDetail(
                            hardwareDeviceInfo,
                            deviceInterfaceData,
                            deviceInterfaceDetailData,
                            predictedLength,
                            &requiredLength,
                            NULL))
    {
        printf("Error: SetupDiGetInterfaceDeviceDetail failed\n");
        free (deviceInterfaceDetailData);
        goto cleanup;
    }

    file = CreateFile ( deviceInterfaceDetailData->DevicePath,
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_READ,
                            NULL, // no SECURITY_ATTRIBUTES structure
                            OPEN_EXISTING, // No special create flags
                            0, // No special attributes
                            NULL); // No template file

    if (INVALID_HANDLE_VALUE == file) {
        printf("Error: CreateFile failed: %ld\n", GetLastError());
        goto cleanup;
    }

    if (CheckIfOurDevice(file, myUsagePage, myUsage)){

        goto cleanup;

    }

    CloseHandle(file);

    file = INVALID_HANDLE_VALUE;

cleanup:

    free (deviceInterfaceDetailData);

    return file;

}

HANDLE SearchMatchingHwID (USAGE myUsagePage, USAGE myUsage){
    HDEVINFO                  hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA  deviceInterfaceData;
    GUID                      hidguid;
    int                       i;

    HidD_GetHidGuid(&hidguid);

    hardwareDeviceInfo =
            SetupDiGetClassDevs ((LPGUID)&hidguid,
                                            NULL,
                                            NULL, // Define no
                                            (DIGCF_PRESENT |
                                            DIGCF_INTERFACEDEVICE));

    if (INVALID_HANDLE_VALUE == hardwareDeviceInfo)
    {
        printf("SetupDiGetClassDevs failed: %lx\n", GetLastError());
        return INVALID_HANDLE_VALUE;
    }

    deviceInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

    //
    // Enumerate devices of this interface class
    //

    printf("\n....looking for our HID device (with UP=0x%x "
                "and Usage=0x%x)\n", myUsagePage, myUsage);

    for (i = 0; SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                            0, // No care about specific PDOs
                            (LPGUID)&hidguid,
                            i, //
                            &deviceInterfaceData);
                            i++)
    {

        //
        // Open the device interface and Check if it is our device
        // by matching the Usage page and Usage from Hid_Caps.
        // If this is our device then send the hid request.
        //

        HANDLE file = OpenDeviceInterface(hardwareDeviceInfo, &deviceInterfaceData, myUsagePage, myUsage);

        if (file != INVALID_HANDLE_VALUE)
        {
            SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
            return file;
        }

        //
        //device was not found so loop around.
        //

    }

    printf("Failure: Could not find our HID device \n");

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);

    return INVALID_HANDLE_VALUE;
}

typedef struct _vmulti_client_t{
    HANDLE hControl;
    HANDLE hMessage;
    BYTE controlReport[CONTROL_REPORT_SIZE];
} vmulti_client_t;

BOOL
HidOutput(
    BOOL useSetOutputReport,
    HANDLE file,
    PCHAR buffer,
    ULONG bufferSize
    )
{
    ULONG bytesWritten;
    if (useSetOutputReport)
    {
        //
        // Send Hid report thru HidD_SetOutputReport API
        //

        if (!HidD_SetOutputReport(file, buffer, bufferSize))
        {
            printf("failed HidD_SetOutputReport %ld\n", GetLastError());
            return FALSE;
        }
    }
    else
    {
        if (!WriteFile(file, buffer, bufferSize, &bytesWritten, NULL))
        {
            printf("failed WriteFile %ld\n", GetLastError());
            return FALSE;
        }
    }

    return TRUE;
}

static vmulti_client_t vmulti;

int os_inputopen(usbdevice* kb){
    if(vmulti.hControl && vmulti.hMessage)
    {
        ckb_info("vmulti already open\n");
        return 0;
    }
    vmulti.hControl = SearchMatchingHwID(0xff00, 0x0001);
    if (vmulti.hControl == INVALID_HANDLE_VALUE || vmulti.hControl == NULL)
        return 1;

    vmulti.hMessage = SearchMatchingHwID(0xff00, 0x0002);
    if (vmulti.hMessage == INVALID_HANDLE_VALUE || vmulti.hMessage == NULL) {
        //vmulti_disconnect(vmulti);
        return FALSE;
    }

    if (!HidD_SetNumInputBuffers(vmulti.hMessage, 10)){
        ckb_fatal("failed HidD_SetNumInputBuffers %ld\n", GetLastError());
        //vmulti_disconnect(vmulti);
        return 1;
    }

    return 0;
}

void os_inputclose(usbdevice* kb){

}

void os_keypress(usbdevice* kb, int scancode, int down){
    if(scancode & SCAN_MOUSE){
       VMultiControlReportHeader* pReport = NULL;
       VMultiRelativeMouseReport* pMouseReport = NULL;

       //
       // Set the report header
       //

       pReport = (VMultiControlReportHeader*)vmulti.controlReport;
       pReport->ReportID = REPORTID_CONTROL;
       pReport->ReportLength = sizeof(VMultiRelativeMouseReport);

       //
       // Set the input report
       //

       pMouseReport = (VMultiRelativeMouseReport*)(vmulti.controlReport + sizeof(VMultiControlReportHeader));
       pMouseReport->ReportID = REPORTID_RELATIVE_MOUSE;
       //pMouseReport->Button = 0;
       pMouseReport->XValue = 0;
       pMouseReport->YValue = 0;
       pMouseReport->Button = 0;
       wheelPosition = 0;
       if((scancode == BTN_WHEELUP || scancode == BTN_WHEELDOWN)) {
            if(down)
                wheelPosition = (scancode == BTN_WHEELUP) ? 1 : -1;
       } else {
           int scan = (scancode & ~SCAN_MOUSE);
           if(down)
               add_to_field(&mouseButtons, scan);
           else
               remove_from_field(&mouseButtons, scan);
           pMouseReport->Button = mouseButtons;
       }
       pMouseReport->WheelPosition = wheelPosition;
       // Send the report
       HidOutput(FALSE, vmulti.hControl, (PCHAR)vmulti.controlReport, CONTROL_REPORT_SIZE);

       return;
   }

    VMultiControlReportHeader* pReport = NULL;
    VMultiKeyboardReport* pKeyboardReport = NULL;

    //
    // Set the report header
    //

    pReport = (VMultiControlReportHeader*)vmulti.controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(VMultiKeyboardReport);

    //
    // Set the input report
    //

    pKeyboardReport = (VMultiKeyboardReport*)(vmulti.controlReport + sizeof(VMultiControlReportHeader));
    pKeyboardReport->ReportID = REPORTID_KEYBOARD;

    //TODO rework this
    if(IS_MODIFIER(scancode))
    {
        scancode -= 0x100;
        if(down)
            add_to_field(&shiftKeyFlags, scancode);
        else
            remove_from_field(&shiftKeyFlags, scancode);
    }
    else
    {
        if(down)
            add_to_keys(scancode);
        else
            remove_from_keys(scancode);
    }
    memcpy(pKeyboardReport->KeyCodes, keyCodes, KBD_KEY_CODES);
    pKeyboardReport->ShiftKeyFlags = shiftKeyFlags;

    // Send the report
    // TODO fix unused ret
    HidOutput(FALSE, vmulti.hControl, (PCHAR)vmulti.controlReport, CONTROL_REPORT_SIZE);
}

void os_mousemove(usbdevice* kb, int x, int y){
    VMultiControlReportHeader* pReport = NULL;
    VMultiRelativeMouseReport* pMouseReport = NULL;

    //
    // Set the report header
    //

    pReport = (VMultiControlReportHeader*)vmulti.controlReport;
    pReport->ReportID = REPORTID_CONTROL;
    pReport->ReportLength = sizeof(VMultiRelativeMouseReport);

    //
    // Set the input report
    //

    pMouseReport = (VMultiRelativeMouseReport*)(vmulti.controlReport + sizeof(VMultiControlReportHeader));
    pMouseReport->ReportID = REPORTID_RELATIVE_MOUSE;
    pMouseReport->Button = mouseButtons;
    pMouseReport->XValue = x;
    pMouseReport->YValue = y;
    pMouseReport->WheelPosition = wheelPosition;

    // Send the report
    // TODO fix unused ret
    HidOutput(FALSE, vmulti.hControl, (PCHAR)vmulti.controlReport, CONTROL_REPORT_SIZE);
}

int os_setupindicators(usbdevice* kb){
    // Initialize LEDs to all off
    kb->hw_ileds = kb->hw_ileds_old = kb->ileds = 0;
    return 0;
}

#endif
