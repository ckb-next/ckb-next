#if !defined(_VMULTI_COMMON_H_) && defined(__WIN32)
#define _VMULTI_COMMON_H_

//
//These are the device attributes returned by vmulti in response
// to IOCTL_HID_GET_DEVICE_ATTRIBUTES.
//

#define VMULTI_PID              0xBA1C
#define VMULTI_VID              0x001F
#define VMULTI_VERSION          0x0001

//
// These are the report ids
//

#define REPORTID_MTOUCH         0x01
#define REPORTID_FEATURE        0x02
#define REPORTID_MOUSE          0x03
#define REPORTID_RELATIVE_MOUSE 0x04
#define REPORTID_DIGI           0x05
#define REPORTID_JOYSTICK       0x06
#define REPORTID_KEYBOARD       0x07
#define REPORTID_MESSAGE        0x10
#define REPORTID_CONTROL        0x40

//
// Control defined report size
//

#define CONTROL_REPORT_SIZE      0x41

//
// Report header
//

#pragma pack(1)
typedef struct _VMULTI_CONTROL_REPORT_HEADER
{

    BYTE        ReportID;

    BYTE        ReportLength;

} VMultiControlReportHeader;
#pragma pack()

//
// Keyboard specific report infomation
//

#define KBD_LCONTROL_BIT     1
#define KBD_LSHIFT_BIT       2
#define KBD_LALT_BIT         4
#define KBD_LGUI_BIT         8
#define KBD_RCONTROL_BIT     16
#define KBD_RSHIFT_BIT       32
#define KBD_RALT_BIT         64
#define KBD_RGUI_BIT         128

#define KBD_KEY_CODES        6

#pragma pack(1)
typedef struct _VMULTI_KEYBOARD_REPORT
{

    BYTE      ReportID;

    // Left Control, Left Shift, Left Alt, Left GUI
    // Right Control, Right Shift, Right Alt, Right GUI
    BYTE      ShiftKeyFlags;

    BYTE      Reserved;

    // See http://www.usb.org/developers/devclass_docs/Hut1_11.pdf
    // for a list of key codes
    BYTE      KeyCodes[KBD_KEY_CODES];

} VMultiKeyboardReport;

typedef struct _VMULTI_KEYBOARD_OUTPUT_REPORT
{
    // Num Lock, Caps Lock, Scroll Lock, Compose, Kana
    BYTE      LedFlags;
} VMultiKeyboardOutputReport;

#pragma pack()

//
// Joystick specific report infomation
//

#pragma pack(1)
typedef struct _VMULTI_JOYSTICK_REPORT
{

    BYTE      ReportID;

    BYTE      Throttle;

    BYTE      XValue;

    BYTE      YValue;

    BYTE      Hat;

    BYTE      RXValue;

    BYTE      RYValue;

    USHORT    Buttons;

} VMultiJoystickReport;
#pragma pack()

//
// Digitizer specific report infomation
//

#define DIGI_TIPSWITCH_BIT    1
#define DIGI_IN_RANGE_BIT     2

#define DIGI_MIN_COORDINATE   0x0000
#define DIGI_MAX_COORDINATE   0x7FFF

#pragma pack(1)
typedef struct _VMULTI_DIGI_REPORT
{

    BYTE      ReportID;

    BYTE      Status;

    USHORT    XValue;

    USHORT    YValue;

} VMultiDigiReport;
#pragma pack()

//
// Mouse specific report information
//

#define MOUSE_BUTTON_1     0x01
#define MOUSE_BUTTON_2     0x02
#define MOUSE_BUTTON_3     0x04

#define MOUSE_MIN_COORDINATE   0x0000
#define MOUSE_MAX_COORDINATE   0x7FFF

#define MIN_WHEEL_POS   -127
#define MAX_WHEEL_POS    127

#pragma pack(1)
typedef struct _VMULTI_MOUSE_REPORT
{

    BYTE        ReportID;

    BYTE        Button;

    USHORT      XValue;

    USHORT      YValue;

    BYTE        WheelPosition;

} VMultiMouseReport;
#pragma pack()

//
// Relative mouse specific report information
//

#define RELATIVE_MOUSE_MIN_COORDINATE   -127
#define RELATIVE_MOUSE_MAX_COORDINATE   127

#pragma pack(1)
typedef struct _VMULTI_RELATIVE_MOUSE_REPORT
{

    BYTE        ReportID;

    BYTE        Button;

    BYTE        XValue;

    BYTE        YValue;

    BYTE        WheelPosition;

} VMultiRelativeMouseReport;
#pragma pack()

//
// Multitouch specific report information
//

#define MULTI_TIPSWITCH_BIT    1
#define MULTI_IN_RANGE_BIT     2
#define MULTI_CONFIDENCE_BIT   4

#define MULTI_MIN_COORDINATE   0x0000
#define MULTI_MAX_COORDINATE   0x7FFF

#define MULTI_MAX_COUNT        20

#pragma pack(1)
typedef struct
{

    BYTE      Status;

    BYTE      ContactID;

    USHORT    XValue;

    USHORT    YValue;

    USHORT    Width;

    USHORT    Height;

}
TOUCH, *PTOUCH;

typedef struct _VMULTI_MULTITOUCH_REPORT
{

    BYTE      ReportID;

    TOUCH     Touch[2];

    BYTE      ActualCount;

} VMultiMultiTouchReport;
#pragma pack()

//
// Feature report infomation
//

#define DEVICE_MODE_MOUSE        0x00
#define DEVICE_MODE_SINGLE_INPUT 0x01
#define DEVICE_MODE_MULTI_INPUT  0x02

#pragma pack(1)
typedef struct _VMULTI_FEATURE_REPORT
{

    BYTE      ReportID;

    BYTE      DeviceMode;

    BYTE      DeviceIdentifier;

} VMultiFeatureReport;

typedef struct _VMULTI_MAXCOUNT_REPORT
{

    BYTE         ReportID;

    BYTE         MaximumCount;

} VMultiMaxCountReport;
#pragma pack()

//
// Message specific report information
//

#define MESSAGE_SIZE 0x20

#pragma pack(1)
typedef struct _VMULTI_MESSAGE_REPORT
{

    BYTE        ReportID;

    char        Message[MESSAGE_SIZE];

} VMultiMessageReport;
#pragma pack()

#endif
