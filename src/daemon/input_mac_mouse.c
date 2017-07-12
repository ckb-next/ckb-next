#include "includes.h"

#ifdef OS_MAC

// Most of this code shamelessly stolen from:
// http://www.opensource.apple.com/source/IOHIDFamily/IOHIDFamily-606.1.7/IOHIDSystem/IOHIPointing.cpp
// http://www.opensource.apple.com/source/IOHIDFamily/IOHIDFamily-606.1.7/IOHIDSystem/IOFixed64.h + IOFixed64.cpp
// http://www.opensource.apple.com/source/IOKitUser/IOKitUser-1050.1.21/hidsystem.subproj/IOEventStatusAPI.c
// Why they thought that mouse acceleration belongs in a low-level input driver is beyond me...

// Original license:
/*
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2010 Apple Computer, Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

// The code has been modified for ckb. This file is distributed under the same license.
// So if anyone actually wants this ungodly mess, those are your terms.

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct timespec last_setup_poll;
static int has_setup = 0;

// The following functions are modified from IOHIDCopyCFTypeParameter, designed to get the mouse info properly
// (it's stored in several different locations; a naïve search will fail)

static kern_return_t IOServiceCopyCF(io_registry_entry_t service, CFStringRef key, CFTypeRef * parameter){
    kern_return_t	kr = KERN_SUCCESS;
    CFDictionaryRef	paramDict;
    CFTypeRef		tempParameter = NULL;

    if( (paramDict = IORegistryEntryCreateCFProperty( service, CFSTR(kIOHIDParametersKey), kCFAllocatorDefault, kNilOptions)))
    {
        if ( (tempParameter = CFDictionaryGetValue( paramDict, key)) )
            CFRetain(tempParameter);

        CFRelease(paramDict);
    }

    if ( !tempParameter )
        tempParameter = IORegistryEntryCreateCFProperty( service, key, kCFAllocatorDefault, kNilOptions);

    if ( !tempParameter )
        kr = kIOReturnBadArgument;

    *parameter = tempParameter;

    return( kr );
}

static kern_return_t IOServiceCopyCFRecursive(io_registry_entry_t service, CFStringRef key, CFTypeRef* parameter){
    // Iterate child registries
    kern_return_t res;
    io_iterator_t child_iter;
    if((res = IORegistryEntryCreateIterator(service, kIOServicePlane, kIORegistryIterateRecursively, &child_iter)) != KERN_SUCCESS)
        return kIOReturnBadArgument;

    io_registry_entry_t child_service;
    while((child_service = IOIteratorNext(child_iter)) != 0){
        io_string_t path;
        IORegistryEntryGetPath(child_service, kIOServicePlane, path);
        res = IOServiceCopyCF(child_service, key, parameter);
        IOObjectRelease(child_service);
        // If the child has it, return success
        if(res == KERN_SUCCESS)
            break;
    }
    IOObjectRelease(child_iter);
    // Return found or not found, depending on above outcome
    return res;
}

// Get a handle for an IOService
static io_service_t GetService(mach_port_t master, const char* name){
    kern_return_t res;
    io_iterator_t iter;
    if((res = IOServiceGetMatchingServices(master, IOServiceMatching(name), &iter)) != KERN_SUCCESS)
        return 0;
    io_service_t service = IOIteratorNext(iter);
    IOObjectRelease(iter);
    return service;
}

// v Usable function
static kern_return_t IOPointingCopyCFTypeParameter(CFStringRef key, CFTypeRef * parameter){
    // Open master port if not done yet
    static mach_port_t master = 0;
    kern_return_t res;
    if(!master && (res = IOMasterPort(bootstrap_port, &master)) != KERN_SUCCESS){
        master = 0;
        return kIOReturnError;
    }
    // Open IOHIPointing and IOHIDSystem if not done yet
    static io_service_t hidsystem = 0, hipointing = 0;
    if(!hidsystem)
        hidsystem = GetService(master, kIOHIDSystemClass);
    if(!hipointing)
        hipointing = GetService(master, kIOHIPointingClass);

    // Find the parameter
    CFTypeRef tempParameter = NULL;
    // Try IOHIPointing first
    if(IOServiceCopyCF(hipointing, key, &tempParameter) == KERN_SUCCESS)
        *parameter = tempParameter;
    // Failing that, try IOHIDSystem
    else if(IOServiceCopyCF(hidsystem, key, &tempParameter) == KERN_SUCCESS)
        *parameter = tempParameter;
    // Try recursive searches
    else if(IOServiceCopyCFRecursive(hipointing, key, &tempParameter) == KERN_SUCCESS)
        *parameter = tempParameter;
    else if(IOServiceCopyCFRecursive(hidsystem, key, &tempParameter) == KERN_SUCCESS)
        *parameter = tempParameter;
    else {
        // Not found
        *parameter = 0;
        return kIOReturnBadArgument;
    }
    return KERN_SUCCESS;
}

// IOFixed64 emulation (this was originally a C++ class; it has been converted to C)

typedef struct {
    SInt64 value;
} IOFixed64;

#define sc2f(sc)    ((sc) * 65536LL)
#define f2sc(f)     ((f).value / 65536LL)
static SInt32 as32(IOFixed64 f) { SInt64 res = f2sc(f); if(res > INT_MAX) return INT_MAX; if(res < INT_MIN) return INT_MIN; return (SInt32)res; }
static bool f_gt_sc(IOFixed64 lhs, SInt64 rhs) { return lhs.value > sc2f(rhs); }
static bool f_lt(IOFixed64 lhs, IOFixed64 rhs) { return lhs.value < rhs.value; }
static bool f_gt(IOFixed64 lhs, IOFixed64 rhs) { return lhs.value > rhs.value; }
static IOFixed64 f_add(IOFixed64 lhs, IOFixed64 rhs) { IOFixed64 r = { lhs.value + rhs.value }; return r; }
static IOFixed64 f_sub(IOFixed64 lhs, IOFixed64 rhs) { IOFixed64 r = { lhs.value - rhs.value }; return r; }
static IOFixed64 f_div(IOFixed64 lhs, IOFixed64 rhs) { IOFixed64 r = { lhs.value * 65536LL / rhs.value }; return r; }
static IOFixed64 f_mul(IOFixed64 lhs, IOFixed64 rhs) { IOFixed64 r = { (lhs.value * rhs.value) / 65536LL }; return r; }
static IOFixed64 f_mul_sc(IOFixed64 lhs, SInt64 rhs) { IOFixed64 r = { lhs.value * rhs }; return r; }

static IOFixed64 exponent(const IOFixed64 original, const UInt8 power)
{
    IOFixed64 result = {0};
    if (power) {
        int i;
        result = original;
        for (i = 1; i < power; i++) {
            result = f_mul(result, original);
        }
    }
    return result;
}

static UInt32 llsqrt(UInt64 x)
{
    UInt64 rem = 0;
    UInt64 root = 0;
    int i;

    for (i = 0; i < 32; i++) {
        root <<= 1;
        rem = ((rem << 2) + (x >> 62));
        x <<= 2;

        root++;

        if (root <= rem) {
            rem -=  root;
            root++;
        } else {
            root--;
        }
    }

    return(UInt32)(root >> 1);
}

UInt16 lsqrt(UInt32 x)
{
    UInt32 rem = 0;
    UInt32 root = 0;
    int i;

    for (i = 0; i < 16; i++) {
        root <<= 1;
        rem = ((rem << 2) + (x >> 30));
        x <<= 2;

        root++;

        if (root <= rem) {
            rem -=  root;
            root++;
        } else {
            root--;
        }
    }

    return(UInt16)(root >> 1);
}

static IOFixed64 IOQuarticFunction( const IOFixed64 x, const IOFixed64 *gains )
{
    // Computes hyper-cubic polynomial with 0-intercept: f(x) = m1*x + m2^2 * x^2 + m3^3 * x^3 + m4^4 * x^4
    IOFixed64 function_at_x = f_add(f_mul(x, gains[0]), exponent(f_mul(x, gains[1]), 2));

    // -- Because of IOFixed overhead, don't bother computing higher expansions unless their gain coefficients are non-zero:
    if( gains[2].value != 0LL )
        function_at_x = f_add(function_at_x, exponent(f_mul(x, gains[2]), 3));

    if( gains[3].value != 0LL )
        function_at_x = f_add(function_at_x, exponent(f_mul(x, gains[3]), 4));

    return function_at_x;
}

static IOFixed64 IOQuarticDerivative( const IOFixed64 x, const IOFixed64 *gains )
{
    // For hyper-cubic polynomial with 0-intercept: f(x) = m1*x + m2^2 * x^2 + m3^3 * x^3 + m4^4 * x^4
    // This function evaluates the derivative: f'(x) = m1 + 2 * x * m2^2 + 3 * x^2 * m3^3 + 4 * x^3 * m4^4
    IOFixed64 derivative_at_x = f_add(gains[0], f_mul_sc(f_mul(x, exponent(gains[1], 2)), 2LL));

    // -- Because of IOFixed overhead, don't bother computing higher expansions unless their gain coefficients are non-zero:
    if( gains[2].value != 0LL )
        derivative_at_x = f_add(derivative_at_x, f_mul_sc(f_mul(exponent(x, 2), exponent(gains[2], 3)), 3LL));

    if( gains[3].value != 0LL )
        derivative_at_x = f_add(derivative_at_x, f_mul_sc(f_mul(exponent(x, 3), exponent(gains[3], 4)), 4LL));

    return derivative_at_x;
}

static inline IOFixed IOFixedMultiply(IOFixed a, IOFixed b)
{
    return (IOFixed)((((SInt64) a) * ((SInt64) b)) >> 16);
}

static inline IOFixed IOFixedDivide(IOFixed a, IOFixed b)
{
    return (IOFixed)((((SInt64) a) << 16) / ((SInt64) b));
}

static IOFixed64 OSObjectToIOFixed64(CFNumberRef object){
    IOFixed64 result = {0};
    if(object && CFGetTypeID(object) == CFNumberGetTypeID())
        CFNumberGetValue(object, kCFNumberIntType, &result.value);
    return result;
}

// Constants

#define MAX_DEVICE_THRESHOLD        0x7fffffff

#define FRAME_RATE		(67 << 16)
#define SCREEN_RESOLUTION	(96 << 16)

#define kIOFixedOne                     0x10000ULL
#define SCROLL_DEFAULT_RESOLUTION       (9 * kIOFixedOne)
#define SCROLL_CONSUME_RESOLUTION       (100 * kIOFixedOne)
#define SCROLL_CONSUME_COUNT_MULTIPLIER 3
#define SCROLL_EVENT_THRESHOLD_MS_LL    150ULL
#define SCROLL_EVENT_THRESHOLD_MS       (SCROLL_EVENT_THRESHOLD_MS_LL * kIOFixedOne)
#define SCROLL_CLEAR_THRESHOLD_MS_LL    500ULL

#define SCROLL_MULTIPLIER_RANGE         0x00018000
#define SCROLL_MULTIPLIER_A             0x00000002 /*IOFixedDivide(SCROLL_MULTIPLIER_RANGE,SCROLL_EVENT_THRESHOLD_MS*2)*/
#define SCROLL_MULTIPLIER_B             0x000003bb /*IOFixedDivide(SCROLL_MULTIPLIER_RANGE*3,(SCROLL_EVENT_THRESHOLD_MS^2)*2)*/
#define SCROLL_MULTIPLIER_C             0x00018041


#define SCROLL_WHEEL_TO_PIXEL_SCALE     0x000a0000/* IOFixedDivide(SCREEN_RESOLUTION, SCROLL_DEFAULT_RESOLUTION) */
#define SCROLL_PIXEL_TO_WHEEL_SCALE     0x0000199a/* IOFixedDivide(SCREEN_RESOLUTION, SCROLL_DEFAULT_RESOLUTION) */
#define SCROLL_TIME_DELTA_COUNT		8

#define CONVERT_SCROLL_FIXED_TO_FRACTION(fixed, fraction)   \
{                                                           \
    if( fixed >= 0)                                     \
    fraction = fixed & 0xffff;                      \
    else                                                \
    fraction = fixed | 0xffff0000;                  \
    }

#define CONVERT_SCROLL_FIXED_TO_INTEGER(fixedAxis, integer) \
{                                                           \
    SInt32 tempInt = 0;                                 \
    if((fixedAxis < 0) && (fixedAxis & 0xffff))         \
    tempInt = (fixedAxis >> 16) + 1;                \
    else                                                \
    tempInt = (fixedAxis >> 16);                    \
    integer = tempInt;                                  \
    }

#define CONVERT_SCROLL_FIXED_TO_COARSE(fixedAxis, coarse)   \
{                                                           \
    SInt32 tempCoarse = 0;                              \
    CONVERT_SCROLL_FIXED_TO_INTEGER(fixedAxis, tempCoarse)  \
    if (!tempCoarse && (fixedAxis & 0xffff))            \
    tempCoarse = (fixedAxis < 0) ? -1 : 1;          \
    coarse = tempCoarse;                                \
    }

enum {
    kAccelTypeGlobal = -1,
    kAccelTypeY = 0, //delta axis 1
    kAccelTypeX = 1, //delta axis 2
    kAccelTypeZ = 2  //delta axis 3
};

// Structures

typedef struct
{
    IOFixed64   deviceMickysDivider;
    IOFixed64   cursorSpeedMultiplier;
    IOFixed64   accelIndex;
    IOFixed64   gain[4];
    IOFixed64   tangent[2];
} IOHIPointing__PAParameters;

typedef struct
{
    int         firstTangent;
    IOFixed64   m0; // m1 == m0
    IOFixed64   b0; // no b1
    IOFixed64   y0;
    IOFixed64   y1;
    IOFixed64   m_root;
    IOFixed64   b_root;
} IOHIPointing__PASecondaryParameters;

struct CursorDeviceSegment {
    SInt32	devUnits;
    SInt32	slope;
    SInt32	intercept;
};
typedef struct CursorDeviceSegment CursorDeviceSegment;

struct ScaleDataState
{
    UInt8           deltaIndex;
    IOFixed         deltaTime[SCROLL_TIME_DELTA_COUNT];
    IOFixed         deltaAxis[SCROLL_TIME_DELTA_COUNT];
    IOFixed         fraction;
};
typedef struct ScaleDataState ScaleDataState;
struct ScaleConsumeState
{
    UInt32      consumeCount;
    IOFixed     consumeAccum;
};
typedef struct ScaleConsumeState ScaleConsumeState;
struct ScrollAxisAccelInfo
{
    struct timespec     lastEventTime;
    void *              scaleSegments;
    IOItemCount         scaleSegCount;
    ScaleDataState      state;
    ScaleConsumeState   consumeState;
    IOHIPointing__PAParameters          primaryParametrics;
    IOHIPointing__PASecondaryParameters secondaryParametrics;
    SInt32              lastValue;
    UInt32              consumeClearThreshold;
    UInt32              consumeCountThreshold;
    bool                isHighResScroll;
    bool                isParametric;
};
typedef struct ScrollAxisAccelInfo ScrollAxisAccelInfo;
struct ScrollAccelInfo
{
    ScrollAxisAccelInfo axis[3];

    IOFixed             rateMultiplier;
    UInt32              zoom:1;
};
typedef struct ScrollAccelInfo ScrollAccelInfo;

// Static variables

static IOHIPointing__PAParameters* _paraAccelParams = 0;
static IOHIPointing__PASecondaryParameters* _paraAccelSecondaryParams = 0;

static IOFixed _scrollFixedDeltaAxis1 = 0, _scrollFixedDeltaAxis2 = 0, _scrollFixedDeltaAxis3 = 0;
static SInt32 _scrollPointDeltaAxis1 = 0, _scrollPointDeltaAxis2 = 0, _scrollPointDeltaAxis3 = 0;

static void* _scaleSegments = 0;
static IOItemCount _scaleSegCount = 0;
static IOFixed _acceleration = -1, _fractX = 0, _fractY = 0;

static ScrollAccelInfo _scrollWheelInfo;
static ScrollAccelInfo _scrollPointerInfo;

// Misc parameters

static IOFixed resolution()
{
    CFNumberRef number;
    IOPointingCopyCFTypeParameter(CFSTR(kIOHIDPointerResolutionKey), (CFTypeRef*)&number);
    IOFixed result = 100 << 16;

    if (number && CFGetTypeID(number) == CFNumberGetTypeID())
        CFNumberGetValue(number, kCFNumberIntType, &result);
    if(number) CFRelease(number);

    return result;
}

static IOFixed	scrollReportRate()
{
    IOFixed     result = FRAME_RATE;
    CFNumberRef number;
    IOPointingCopyCFTypeParameter(CFSTR(kIOHIDScrollReportRateKey), (CFTypeRef*)&number);

    if (number && CFGetTypeID(number) == CFNumberGetTypeID())
        CFNumberGetValue(number, kCFNumberIntType, &result);
    if(number) CFRelease(number);

    if (result == 0)
        result = FRAME_RATE;

    return result;
}

static IOFixed	scrollResolutionForType(SInt32 type)
{
    IOFixed     res         = 0;
    CFNumberRef 	number      = NULL;
    CFStringRef key         = NULL;

    switch ( type ) {
    case kAccelTypeY:
        key = CFSTR(kIOHIDScrollResolutionYKey);
        break;
    case kAccelTypeX:
        key = CFSTR(kIOHIDScrollResolutionXKey);
        break;
    case kAccelTypeZ:
        key = CFSTR(kIOHIDScrollResolutionZKey);
        break;
    default:
        key = CFSTR(kIOHIDScrollResolutionKey);
        break;

    }
    IOPointingCopyCFTypeParameter(key, (CFTypeRef*)&number);
    if(number && CFGetTypeID(number) == CFNumberGetTypeID())
        CFNumberGetValue(number, kCFNumberIntType, &res);
    else {
        if(number) CFRelease(number);
        IOPointingCopyCFTypeParameter(CFSTR(kIOHIDScrollResolutionKey), (CFTypeRef*)&number);
        if(number && CFGetTypeID(number) == CFNumberGetTypeID())
            CFNumberGetValue(number, kCFNumberIntType, &res);
    }
    if(number) CFRelease(number);

    return res;
}

// Parametric acceleration

static bool
PACurvesFillParamsFromDict(CFDictionaryRef parameters,
                           const IOFixed64 devScale,
                           const IOFixed64 crsrScale,
                           IOHIPointing__PAParameters *outParams)
{
    require(parameters, exit_early);
    require(CFGetTypeID(parameters) == CFDictionaryGetTypeID(), exit_early);

    outParams->deviceMickysDivider = devScale;
    outParams->cursorSpeedMultiplier = crsrScale;

    outParams->accelIndex = OSObjectToIOFixed64(CFDictionaryGetValue(parameters, CFSTR(kHIDAccelIndexKey)));

    outParams->gain[0] = OSObjectToIOFixed64(CFDictionaryGetValue(parameters, CFSTR(kHIDAccelGainLinearKey)));
    outParams->gain[1] = OSObjectToIOFixed64(CFDictionaryGetValue(parameters, CFSTR(kHIDAccelGainParabolicKey)));
    outParams->gain[2] = OSObjectToIOFixed64(CFDictionaryGetValue(parameters, CFSTR(kHIDAccelGainCubicKey)));
    outParams->gain[3] = OSObjectToIOFixed64(CFDictionaryGetValue(parameters, CFSTR(kHIDAccelGainQuarticKey)));

    outParams->tangent[0] = OSObjectToIOFixed64(CFDictionaryGetValue(parameters, CFSTR(kHIDAccelTangentSpeedLinearKey)));
    outParams->tangent[1] = OSObjectToIOFixed64(CFDictionaryGetValue(parameters, CFSTR(kHIDAccelTangentSpeedParabolicRootKey)));

    return ((outParams->gain[0].value != 0LL) ||
            (outParams->gain[1].value != 0LL) ||
            (outParams->gain[2].value != 0LL) ||
            (outParams->gain[3].value != 0LL));

exit_early:
    return false;
}

static bool
PACurvesSetupAccelParams (CFArrayRef parametricCurves,
                          IOFixed64 desired,
                          IOFixed64 devScale,
                          IOFixed64 crsrScale,
                          IOHIPointing__PAParameters *primaryParams,
                          IOHIPointing__PASecondaryParameters *secondaryParams)
{
    bool                    success = false;
    CFDictionaryRef         dict = NULL;

    IOHIPointing__PAParameters high_curve_params;
    IOHIPointing__PAParameters low_curve_params;

    require(parametricCurves, exit_early);
    require(f_gt_sc(crsrScale, 0LL), exit_early);
    require(f_gt_sc(devScale, 0LL), exit_early);
    require(f_gt_sc(desired, 0LL), exit_early);

    CFIndex itrCount = CFArrayGetCount(parametricCurves);
    CFIndex itr = 0;

    while (!success) {
        itr = 0;
        dict = (CFDictionaryRef)CFArrayGetValueAtIndex(parametricCurves, itr++);
        require(PACurvesFillParamsFromDict(dict, devScale, crsrScale, &low_curve_params),
                exit_early);

        while (!success && (NULL != dict)) {
            if (!PACurvesFillParamsFromDict(dict, devScale, crsrScale, &high_curve_params)) {
                break;
            }
            if (desired.value <= high_curve_params.accelIndex.value) {
                success = true;
            }
            else {
                low_curve_params = high_curve_params;
            }
            if(itr == itrCount)
                dict = NULL;
            else
                dict = (CFDictionaryRef)CFArrayGetValueAtIndex(parametricCurves, itr++);
        }

        require(success, exit_early);
    };

    if ( high_curve_params.accelIndex.value > low_curve_params.accelIndex.value ) {
        IOFixed64   ratio = f_div(f_sub(desired, low_curve_params.accelIndex), f_sub(high_curve_params.accelIndex, low_curve_params.accelIndex));
        int         index;

        primaryParams->deviceMickysDivider   = high_curve_params.deviceMickysDivider;
        primaryParams->cursorSpeedMultiplier = high_curve_params.cursorSpeedMultiplier;
        primaryParams->accelIndex            = desired;

        for (index = 0; index < 4; index++) {
            primaryParams->gain[index] = f_add(low_curve_params.gain[index], f_mul(f_sub(high_curve_params.gain[index], low_curve_params.gain[index]), ratio));
            if (primaryParams->gain[index].value < 0LL)
                primaryParams->gain[index].value = 0;
        }
        for (index = 0; index < 2; index++) {
            primaryParams->tangent[index] = f_add(low_curve_params.tangent[index], f_mul(f_sub(high_curve_params.tangent[index], low_curve_params.tangent[index]), ratio));
            if (primaryParams->tangent[index].value < 0LL)
                primaryParams->tangent[index].value = 0;
        }
    }
    else {
        *primaryParams = high_curve_params;
    }

    success = ((primaryParams->gain[0].value != 0LL) ||
            (primaryParams->gain[1].value != 0LL) ||
            (primaryParams->gain[2].value != 0LL) ||
            (primaryParams->gain[3].value != 0LL));

    // calculate secondary values
    bzero(secondaryParams, sizeof(secondaryParams));
    if ((primaryParams->tangent[1].value > 0LL) && (primaryParams->tangent[1].value < primaryParams->tangent[0].value))
        secondaryParams->firstTangent = 1;

    if (secondaryParams->firstTangent == 0) {
        secondaryParams->y0 = IOQuarticFunction(primaryParams->tangent[0], primaryParams->gain);
        secondaryParams->m0 = IOQuarticDerivative(primaryParams->tangent[0], primaryParams->gain);
        secondaryParams->b0 = f_sub(secondaryParams->y0, f_mul(secondaryParams->m0, primaryParams->tangent[0]));
        secondaryParams->y1 = f_add(f_mul(secondaryParams->m0, primaryParams->tangent[1]), secondaryParams->b0);
    }
    else {
        secondaryParams->y1 = IOQuarticFunction( primaryParams->tangent[1], primaryParams->gain );
        secondaryParams->m0 = IOQuarticDerivative( primaryParams->tangent[1], primaryParams->gain );
    }

    secondaryParams->m_root = f_mul_sc(f_mul(secondaryParams->m0, secondaryParams->y1), 2LL);
    secondaryParams->b_root = f_sub(exponent(secondaryParams->y1, 2), f_mul(secondaryParams->m_root, primaryParams->tangent[1]));

exit_early:

    return success;
}

static IOFixed64
PACurvesGetAccelerationMultiplier(const IOFixed64 device_speed_mickeys,
                                  const IOHIPointing__PAParameters *params,
                                  const IOHIPointing__PASecondaryParameters *secondaryParams)
{
    IOFixed64 result = {0};

    if ((device_speed_mickeys.value > result.value) && (params->deviceMickysDivider.value != result.value)) {
        IOFixed64 standardized_speed = f_div(device_speed_mickeys, params->deviceMickysDivider);
        IOFixed64 accelerated_speed;
        if ((params->tangent[secondaryParams->firstTangent].value != 0LL) && (standardized_speed.value <= params->tangent[secondaryParams->firstTangent].value)) {
            accelerated_speed = IOQuarticFunction(standardized_speed, params->gain);
        }
        else {
            if ((secondaryParams->firstTangent == 0) && (params->tangent[1].value != 0LL) && (standardized_speed.value <= params->tangent[1].value)) {
                accelerated_speed = f_add(f_mul(secondaryParams->m0, standardized_speed), secondaryParams->b0);
            }
            else {
                accelerated_speed.value = sc2f(llsqrt(f2sc(f_add(f_mul(secondaryParams->m_root, standardized_speed), secondaryParams->b_root))));
            }
        }
        IOFixed64 accelerated_pixels = f_mul(accelerated_speed, params->cursorSpeedMultiplier);
        result = f_div(accelerated_pixels, device_speed_mickeys);
    }
    else {
        result.value = 1;
    }

    return result;
}

// Classic acceleration

static CFDataRef copyAccelerationTable()
{
    static const UInt8 accl[] = {
        0x00, 0x00, 0x80, 0x00,
        0x40, 0x32, 0x30, 0x30, 0x00, 0x02, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x09, 0x00, 0x00, 0x71, 0x3B, 0x00, 0x00,
        0x60, 0x00, 0x00, 0x04, 0x4E, 0xC5, 0x00, 0x10,
        0x80, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x5F,
        0x00, 0x00, 0x00, 0x16, 0xEC, 0x4F, 0x00, 0x8B,
        0x00, 0x00, 0x00, 0x1D, 0x3B, 0x14, 0x00, 0x94,
        0x80, 0x00, 0x00, 0x22, 0x76, 0x27, 0x00, 0x96,
        0x00, 0x00, 0x00, 0x24, 0x62, 0x76, 0x00, 0x96,
        0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x96,
        0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x96,
        0x00, 0x00
    };

    CFDataRef data;
    IOPointingCopyCFTypeParameter(CFSTR(kIOHIDPointerAccelerationTableKey), (CFTypeRef*)&data);
    if(data && CFGetTypeID(data) != CFDataGetTypeID()){
        CFRelease(data);
        data = 0;
    }
    if (!data)
        data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, accl, sizeof(accl), kCFAllocatorNull);

    return( data );
}

static CFDataRef copyScrollAccelerationTableForType(SInt32 type)
{
    CFDataRef    data    = NULL;
    CFStringRef key     = NULL;

    switch ( type ) {
    case kAccelTypeY:
        key = CFSTR(kIOHIDScrollAccelerationTableYKey);
        break;
    case kAccelTypeX:
        key = CFSTR(kIOHIDScrollAccelerationTableXKey);
        break;
    case kAccelTypeZ:
        key = CFSTR(kIOHIDScrollAccelerationTableZKey);
        break;
    }

    if ( key )
        IOPointingCopyCFTypeParameter(key, (CFTypeRef*)&data);

    if ( !data || CFGetTypeID(data) != CFDataGetTypeID()) {
        if(data) CFRelease(data);
        IOPointingCopyCFTypeParameter(CFSTR(kIOHIDScrollAccelerationTableKey), (CFTypeRef*)&data);
    }

    if ( !data || CFGetTypeID(data) != CFDataGetTypeID()) {
        if(data) CFRelease(data);
        data = copyAccelerationTable();
    }

    return( data );
}

/*
 Routine:    Interpolate
 This routine interpolates to find a point on the line [x1,y1] [x2,y2] which
 is intersected by the line [x3,y3] [x3,y"].  The resulting y' is calculated
 by interpolating between y3 and y", towards the higher acceleration curve.
*/
static SInt32 Interpolate(  SInt32 x1, SInt32 y1,
                            SInt32 x2, SInt32 y2,
                            SInt32 x3, SInt32 y3,
                            SInt32 scale, Boolean lower )
{

    SInt32 slope;
    SInt32 intercept;
    SInt32 resultY;

    slope = (x2 == x1) ? 0 : IOFixedDivide( y2 - y1, x2 - x1 );
    intercept = y1 - IOFixedMultiply( slope, x1 );
    resultY = intercept + IOFixedMultiply( slope, x3 );
    if( lower)
        resultY = y3 - IOFixedMultiply( scale, y3 - resultY );
    else
        resultY = resultY + IOFixedMultiply( scale, y3 - resultY );

    return( resultY );
}

static bool SetupAcceleration (CFDataRef data, IOFixed desired, IOFixed devScale, IOFixed crsrScale, void ** scaleSegments, IOItemCount * scaleSegCount) {
    const UInt16 *	lowTable = 0;
    const UInt16 *	highTable;

    SInt32	x1, y1, x2, y2, x3, y3;
    SInt32	prevX1, prevY1;
    SInt32	upperX, upperY;
    SInt32	lowerX, lowerY;
    SInt32	lowAccl = 0, lowPoints = 0;
    SInt32	highAccl, highPoints;
    SInt32	scale;
    UInt32	count;
    Boolean	lower;

    SInt32	scaledX1, scaledY1;
    SInt32	scaledX2, scaledY2;

    CursorDeviceSegment *	segments;
    CursorDeviceSegment *	segment;
    SInt32			segCount;

    if( !data || !devScale || !crsrScale)
        return false;

    if( desired < (IOFixed) 0) {
        // disabling mouse scaling
        if(*scaleSegments && *scaleSegCount)
            free( *scaleSegments);
        *scaleSegments = NULL;
        *scaleSegCount = 0;
        return false;
    }

    highTable = (const UInt16 *)CFDataGetBytePtr(data);

    scaledX1 = scaledY1 = 0;

    scale = OSReadBigInt32((volatile void *)highTable, 0);
    highTable += 4;

    // normalize table's default (scale) to 0.5
    if( desired > 0x8000) {
        desired = IOFixedMultiply( desired - 0x8000,
                                   0x10000 - scale );
        desired <<= 1;
        desired += scale;
    } else {
        desired = IOFixedMultiply( desired, scale );
        desired <<= 1;
    }

    count = OSReadBigInt16((volatile void *)(highTable++), 0);
    scale = (1 << 16);

    // find curves bracketing the desired value
    do {
        highAccl = OSReadBigInt32((volatile void *)highTable, 0);
        highTable += 2;
        highPoints = OSReadBigInt16((volatile void *)(highTable++), 0);

        if( desired <= highAccl)
            break;

        if( 0 == --count) {
            // this much over the highest table
            scale = (highAccl) ? IOFixedDivide( desired, highAccl ) : 0;
            lowTable	= 0;
            break;
        }

        lowTable	= highTable;
        lowAccl		= highAccl;
        lowPoints	= highPoints;
        highTable	+= lowPoints * 4;

    } while( true );

    // scale between the two
    if( lowTable) {
        scale = (highAccl == lowAccl) ? 0 :
                                        IOFixedDivide((desired - lowAccl), (highAccl - lowAccl));

    }

    // or take all the high one
    else {
        lowTable	= highTable;
        lowAccl		= highAccl;
        lowPoints	= 0;
    }

    if( lowPoints > highPoints)
        segCount = lowPoints;
    else
        segCount = highPoints;
    segCount *= 2;
    segments = calloc( sizeof(CursorDeviceSegment), segCount );
    assert( segments );
    segment = segments;

    x1 = prevX1 = y1 = prevY1 = 0;

    lowerX = OSReadBigInt32((volatile void *)lowTable, 0);
    lowTable += 2;
    lowerY = OSReadBigInt32((volatile void *)lowTable, 0);
    lowTable += 2;
    upperX = OSReadBigInt32((volatile void *)highTable, 0);
    highTable += 2;
    upperY = OSReadBigInt32((volatile void *)highTable, 0);
    highTable += 2;

    do {
        // consume next point from first X
        lower = (lowPoints && (!highPoints || (lowerX <= upperX)));

        if( lower) {
            /* highline */
            x2 = upperX;
            y2 = upperY;
            x3 = lowerX;
            y3 = lowerY;
            if( lowPoints && (--lowPoints)) {
                lowerX = OSReadBigInt32((volatile void *)lowTable, 0);
                lowTable += 2;
                lowerY = OSReadBigInt32((volatile void *)lowTable, 0);
                lowTable += 2;
            }
        } else  {
            /* lowline */
            x2 = lowerX;
            y2 = lowerY;
            x3 = upperX;
            y3 = upperY;
            if( highPoints && (--highPoints)) {
                upperX = OSReadBigInt32((volatile void *)highTable, 0);
                highTable += 2;
                upperY = OSReadBigInt32((volatile void *)highTable, 0);
                highTable += 2;
            }
        }
        {
            // convert to line segment
            assert( segment < (segments + segCount) );

            scaledX2 = IOFixedMultiply( devScale, /* newX */ x3 );
            scaledY2 = IOFixedMultiply( crsrScale,
                                        /* newY */    Interpolate( x1, y1, x2, y2, x3, y3,
                                                                   scale, lower ) );
            if( lowPoints || highPoints)
                segment->devUnits = scaledX2;
            else
                segment->devUnits = MAX_DEVICE_THRESHOLD;

            segment->slope = ((scaledX2 == scaledX1)) ? 0 :
                                                        IOFixedDivide((scaledY2 - scaledY1), (scaledX2 - scaledX1));

            segment->intercept = scaledY2
                    - IOFixedMultiply( segment->slope, scaledX2 );

            scaledX1 = scaledX2;
            scaledY1 = scaledY2;
            segment++;
        }

        // continue on from last point
        if( lowPoints && highPoints) {
            if( lowerX > upperX) {
                prevX1 = x1;
                prevY1 = y1;
            } else {
                /* swaplines */
                prevX1 = x1;
                prevY1 = y1;
                x1 = x3;
                y1 = y3;
            }
        } else {
            x2 = x1;
            y2 = y1;
            x1 = prevX1;
            y1 = prevY1;
            prevX1 = x2;
            prevY1 = y2;
        }

    } while( lowPoints || highPoints );

    if( *scaleSegCount && *scaleSegments)
        free( *scaleSegments);
    *scaleSegCount = segCount;
    *scaleSegments = (void *) segments;

    return true;
}

// Putting it together

static void setupForAcceleration(IOFixed desired){
    CFArrayRef      parametricAccelerationCurves;
    IOPointingCopyCFTypeParameter(CFSTR(kHIDTrackingAccelParametricCurvesKey), (CFTypeRef*)&parametricAccelerationCurves);
    IOFixed         devScale    = IOFixedDivide( resolution(), FRAME_RATE );
    IOFixed         crsrScale   = IOFixedDivide( SCREEN_RESOLUTION, FRAME_RATE );
    bool            useParametric = false;

    if (!parametricAccelerationCurves || CFGetTypeID(parametricAccelerationCurves) != CFArrayGetTypeID()) {
        if(parametricAccelerationCurves) CFRelease(parametricAccelerationCurves);
        IOPointingCopyCFTypeParameter(CFSTR(kHIDAccelParametricCurvesKey), (CFTypeRef*)&parametricAccelerationCurves);
    }
    // Try to set up the parametric acceleration data
    if (parametricAccelerationCurves && CFGetTypeID(parametricAccelerationCurves) == CFArrayGetTypeID()) {
        if ( !_paraAccelParams )
        {
            _paraAccelParams = (IOHIPointing__PAParameters*)malloc(sizeof(IOHIPointing__PAParameters));
        }
        if ( !_paraAccelSecondaryParams )
        {
            _paraAccelSecondaryParams = (IOHIPointing__PASecondaryParameters*)malloc(sizeof(IOHIPointing__PASecondaryParameters));
        }

        if (_paraAccelParams && _paraAccelSecondaryParams) {
            IOFixed64 desired64 = {desired};
            IOFixed64 devScale64 = {devScale};
            IOFixed64 crsrScale64 = {crsrScale};

            useParametric = PACurvesSetupAccelParams(parametricAccelerationCurves,
                                                     desired64,
                                                     devScale64,
                                                     crsrScale64,
                                                     _paraAccelParams,
                                                     _paraAccelSecondaryParams);
        }
    }
    if(parametricAccelerationCurves) CFRelease(parametricAccelerationCurves);

    // If that fails, fall back to classic acceleration
    if (!useParametric) {
        CFDataRef  table         = copyAccelerationTable();

        if (_paraAccelParams)
            free(_paraAccelParams);
        if (_paraAccelSecondaryParams)
            free(_paraAccelSecondaryParams);
        _paraAccelParams = NULL;
        _paraAccelSecondaryParams = NULL;

        if (SetupAcceleration (table, desired, devScale, crsrScale, &_scaleSegments, &_scaleSegCount))
        {
            _acceleration = desired;
            _fractX = _fractY = 0;
        }
        if(table) CFRelease(table);
    }
}

static void ScaleAxes (void * scaleSegments, int * axis1p, IOFixed *axis1Fractp, int * axis2p, IOFixed *axis2Fractp)
{
    SInt32			dx, dy;
    SInt32			mag;
    IOFixed			scale;
    CursorDeviceSegment	*	segment;

    if( !scaleSegments)
        return;

    dx = (*axis1p) << 16;
    dy = (*axis2p) << 16;

    // mag is (x^2 + y^2)^0.5 and converted to fixed point
    mag = (lsqrt(*axis1p * *axis1p + *axis2p * *axis2p)) << 16;
    if (mag == 0)
        return;

    // scale
    for(
        segment = (CursorDeviceSegment *) scaleSegments;
        mag > segment->devUnits;
        segment++)	{}

    scale = IOFixedDivide(
                segment->intercept + IOFixedMultiply( mag, segment->slope ),
                mag );

    dx = IOFixedMultiply( dx, scale );
    dy = IOFixedMultiply( dy, scale );

    // add fract parts
    dx += *axis1Fractp;
    dy += *axis2Fractp;

    *axis1p = dx / 65536;
    *axis2p = dy / 65536;

    // get fractional part with sign extend
    if( dx >= 0)
        *axis1Fractp = dx & 0xffff;
    else
        *axis1Fractp = dx | 0xffff0000;
    if( dy >= 0)
        *axis2Fractp = dy & 0xffff;
    else
        *axis2Fractp = dy | 0xffff0000;
}

static void scalePointer(int* dxp, int* dyp)
// Description:	Perform pointer acceleration computations here.
//		Given the resolution, dx, dy, and time, compute the velocity
//		of the pointer over a Manhatten distance in inches/second.
//		Using this velocity, do a lookup in the pointerScaling table
//		to select a scaling factor. Scale dx and dy up as appropriate.
// Preconditions:
// *	_deviceLock should be held on entry
{
    if (_paraAccelParams && _paraAccelSecondaryParams) {
        IOFixed64 deltaX = {sc2f(*dxp)};
        IOFixed64 deltaY = {sc2f(*dyp)};
        IOFixed64 fractX = {_fractX};
        IOFixed64 fractY = {_fractY};
        IOFixed64 mag = {sc2f(llsqrt(f2sc(f_add(f_mul(deltaX, deltaX), f_mul(deltaY, deltaY)))))};

        IOFixed64 mult = PACurvesGetAccelerationMultiplier(mag, _paraAccelParams, _paraAccelSecondaryParams);
        deltaX = f_mul(deltaX, mult);
        deltaY = f_mul(deltaY, mult);
        deltaX = f_add(deltaX, fractX);
        deltaY = f_add(deltaY, fractY);

        *dxp = as32(deltaX);
        *dyp = as32(deltaY);

        _fractX = deltaX.value;
        _fractY = deltaY.value;

        // sign extend fractional part
        if( deltaX.value < 0LL )
            _fractX |= 0xffff0000;
        else
            _fractX &= 0x0000ffff;

        if( deltaY.value < 0LL)
            _fractY |= 0xffff0000;
        else
            _fractY &= 0x0000ffff;
    }
    else {
        ScaleAxes(_scaleSegments, dxp, &_fractX, dyp, &_fractY);
    }
}

static void setupScrollForAcceleration( IOFixed desired ){
    IOFixed     devScale    = 0;
    IOFixed     scrScale    = 0;
    IOFixed     reportRate  = scrollReportRate();
    CFDataRef   accelTable  = NULL;
    UInt32      type        = 0;

    _scrollWheelInfo.rateMultiplier    = IOFixedDivide(reportRate, FRAME_RATE);
    _scrollPointerInfo.rateMultiplier  = IOFixedDivide(reportRate, FRAME_RATE);

    if (desired < 0) {
    }
    else {

        CFArrayRef parametricAccelerationCurves;
        IOPointingCopyCFTypeParameter(CFSTR(kHIDScrollAccelParametricCurvesKey), (CFTypeRef*)&parametricAccelerationCurves);

        for ( type=kAccelTypeY; type<=kAccelTypeZ; type++) {
            IOFixed     res = scrollResolutionForType(type);
            // Zero scroll resolution says you don't want acceleration.
            if ( res ) {
                _scrollWheelInfo.axis[type].isHighResScroll    = res > (SCROLL_DEFAULT_RESOLUTION * 2);
                _scrollPointerInfo.axis[type].isHighResScroll  = _scrollWheelInfo.axis[type].isHighResScroll;

                _scrollWheelInfo.axis[type].consumeClearThreshold = (IOFixedDivide(res, SCROLL_CONSUME_RESOLUTION) >> 16) * 2;
                _scrollPointerInfo.axis[type].consumeClearThreshold = _scrollWheelInfo.axis[type].consumeClearThreshold;

                _scrollWheelInfo.axis[type].consumeCountThreshold = _scrollWheelInfo.axis[type].consumeClearThreshold * SCROLL_CONSUME_COUNT_MULTIPLIER;
                _scrollPointerInfo.axis[type].consumeCountThreshold = _scrollPointerInfo.axis[type].consumeClearThreshold * SCROLL_CONSUME_COUNT_MULTIPLIER;

                bzero(&(_scrollWheelInfo.axis[type].state), sizeof(ScaleDataState));
                bzero(&(_scrollWheelInfo.axis[type].consumeState), sizeof(ScaleConsumeState));
                bzero(&(_scrollPointerInfo.axis[type].state), sizeof(ScaleDataState));
                bzero(&(_scrollPointerInfo.axis[type].consumeState), sizeof(ScaleConsumeState));

                clock_gettime(CLOCK_MONOTONIC, &(_scrollWheelInfo.axis[type].lastEventTime));
                _scrollPointerInfo.axis[type].lastEventTime = _scrollWheelInfo.axis[type].lastEventTime;

                if (parametricAccelerationCurves && CFGetTypeID(parametricAccelerationCurves) == CFArrayGetTypeID() && reportRate) {
                    IOFixed64 desired64 = { desired };
                    IOFixed64 devScale64 = { res };
                    IOFixed64 scrScale64 = { SCREEN_RESOLUTION };

                    devScale64 = f_div(devScale64, *(IOFixed64[]){{ reportRate }});

                    scrScale64 = f_div(scrScale64, *(IOFixed64[]){{ FRAME_RATE }});

                    _scrollWheelInfo.axis[type].isParametric =
                            PACurvesSetupAccelParams(parametricAccelerationCurves,
                                                     desired64,
                                                     devScale64,
                                                     scrScale64,
                                                     &_scrollWheelInfo.axis[type].primaryParametrics,
                                                     &_scrollWheelInfo.axis[type].secondaryParametrics);
                }

                if (!_scrollWheelInfo.axis[type].isParametric) {
                    accelTable = copyScrollAccelerationTableForType(type);

                    // Setup pixel scroll wheel acceleration table
                    devScale = IOFixedDivide( res, reportRate );
                    scrScale = IOFixedDivide( SCREEN_RESOLUTION, FRAME_RATE );

                    SetupAcceleration (accelTable, desired, devScale, scrScale, &(_scrollWheelInfo.axis[type].scaleSegments), &(_scrollWheelInfo.axis[type].scaleSegCount));

                    // Grab the pointer resolution
                    res = resolution();
                    reportRate = FRAME_RATE;

                    // Setup pixel pointer drag/scroll acceleration table
                    devScale = IOFixedDivide( res, reportRate );
                    scrScale = IOFixedDivide( SCREEN_RESOLUTION, FRAME_RATE );

                    SetupAcceleration (accelTable, desired, devScale, scrScale, &(_scrollPointerInfo.axis[type].scaleSegments), &(_scrollPointerInfo.axis[type].scaleSegCount));

                    if (accelTable)
                        CFRelease(accelTable);
                }
            }
        }
        if(parametricAccelerationCurves) CFRelease(parametricAccelerationCurves);
    }
}

static void AccelerateScrollAxis(   IOFixed *               axisp,
                                    ScrollAxisAccelInfo *   scaleInfo,
                                    struct timespec*        timeStamp,
                                    IOFixed                 rateMultiplier,
                                    bool                    clear)
{
    IOFixed absAxis             = 0;
    int     avgIndex            = 0;
    IOFixed	avgCount            = 0;
    IOFixed avgAxis             = 0;
    IOFixed	timeDeltaMS         = 0;
    IOFixed	avgTimeDeltaMS      = 0;
    UInt64	currentTimeNSLL     = 0;
    UInt64	lastEventTimeNSLL   = 0;
    UInt64  timeDeltaMSLL       = 0;

    if ( ! (scaleInfo && ( scaleInfo->scaleSegments || scaleInfo->isParametric) ) )
        return;

    absAxis = abs(*axisp);

    if( absAxis == 0 )
        return;

    currentTimeNSLL = timeStamp->tv_nsec + timeStamp->tv_sec * 1000000000;
    lastEventTimeNSLL = scaleInfo->lastEventTime.tv_nsec + scaleInfo->lastEventTime.tv_sec * 1000000000;

    scaleInfo->lastEventTime = *timeStamp;

    timeDeltaMSLL = (currentTimeNSLL - lastEventTimeNSLL) / 1000000;

    // RY: To compensate for non continual motion, we have added a second
    // threshold.  This whill allow a user with a standard scroll wheel
    // to continue with acceleration when lifting the finger within a
    // predetermined time.  We should also clear out the last time deltas
    // if the direction has changed.
    if ((timeDeltaMSLL >= SCROLL_CLEAR_THRESHOLD_MS_LL) || clear) {
        bzero(&(scaleInfo->state), sizeof(ScaleDataState));
        timeDeltaMSLL = SCROLL_CLEAR_THRESHOLD_MS_LL;
    }

    timeDeltaMS = ((UInt32) timeDeltaMSLL) * kIOFixedOne;

    scaleInfo->state.deltaTime[scaleInfo->state.deltaIndex] = timeDeltaMS;
    scaleInfo->state.deltaAxis[scaleInfo->state.deltaIndex] = absAxis;

    // RY: To eliminate jerkyness associated with the scroll acceleration,
    // we scroll based on the average of the last n events.  This has the
    // effect of make acceleration smoother with accel and decel.
    for (int index=0; index < SCROLL_TIME_DELTA_COUNT; index++)
    {
        avgIndex = (scaleInfo->state.deltaIndex + SCROLL_TIME_DELTA_COUNT - index) % SCROLL_TIME_DELTA_COUNT;
        avgAxis         += scaleInfo->state.deltaAxis[avgIndex];
        avgCount ++;

        if ((scaleInfo->state.deltaTime[avgIndex] <= 0) ||
                (scaleInfo->state.deltaTime[avgIndex] >= SCROLL_EVENT_THRESHOLD_MS)) {
            // the previous event was too long before this one. stop looking.
            avgTimeDeltaMS += SCROLL_EVENT_THRESHOLD_MS;
            break;
        }

        avgTimeDeltaMS  += scaleInfo->state.deltaTime[avgIndex];

        if (avgTimeDeltaMS >= (SCROLL_CLEAR_THRESHOLD_MS_LL * kIOFixedOne)) {
            // the previous event was too long ago. stop looking.
            break;
        }
    }

    // Bump the next index
    scaleInfo->state.deltaIndex = (scaleInfo->state.deltaIndex + 1) % SCROLL_TIME_DELTA_COUNT;

    avgAxis         = (avgCount) ? (avgAxis / avgCount) : 0;
    avgTimeDeltaMS  = (avgCount) ? (avgTimeDeltaMS / avgCount) : 0;
    avgTimeDeltaMS  = IOFixedMultiply(avgTimeDeltaMS, rateMultiplier);
    if (avgTimeDeltaMS > SCROLL_EVENT_THRESHOLD_MS) {
        avgTimeDeltaMS = SCROLL_EVENT_THRESHOLD_MS;
    }
    else if (avgTimeDeltaMS < kIOFixedOne) {
        // anything less than 1 ms is not resonable
        avgTimeDeltaMS = kIOFixedOne;
    }

    // RY: Since we want scroll acceleration to work with the
    // time delta and the accel curves, we have come up with
    // this approach:
    //
    // scrollMultiplier = (SCROLL_MULTIPLIER_A * (avgTimeDeltaMS^2)) +
    //                      (SCROLL_MULTIPLIER_B * avgTimeDeltaMS) +
    //                          SCROLL_MULTIPLIER_C
    //
    // scrollMultiplier *= avgDeviceDelta
    //
    // The boost curve follows a quadratic/parabolic curve which
    // results in a smoother boost.
    //
    // The resulting multipler is applied to the average axis
    // magnitude and then compared against the accleration curve.
    //
    // The value acquired from the graph will then be multiplied
    // to the current axis delta.
    IOFixed64           scrollMultiplier;
    IOFixed64           timedDelta = { avgTimeDeltaMS };
    IOFixed64           axisValue = { *axisp };
    IOFixed64           minimumMultiplier = { kIOFixedOne >> 4 };

    scrollMultiplier    = f_mul(f_mul(*(IOFixed64[]){ { SCROLL_MULTIPLIER_A } }, timedDelta), timedDelta);
    scrollMultiplier = f_sub(scrollMultiplier,     f_mul(*(IOFixed64[]){ { SCROLL_MULTIPLIER_B } }, timedDelta));
    scrollMultiplier = f_add(scrollMultiplier,     *(IOFixed64[]){ { SCROLL_MULTIPLIER_C } });
    scrollMultiplier = f_mul(scrollMultiplier,     *(IOFixed64[]){ { rateMultiplier } });
    scrollMultiplier = f_mul(scrollMultiplier,     *(IOFixed64[]){ { avgAxis } });
    if (f_lt(scrollMultiplier, minimumMultiplier)) {
        scrollMultiplier = minimumMultiplier;
    }

    if (scaleInfo->isParametric) {
        scrollMultiplier = PACurvesGetAccelerationMultiplier(scrollMultiplier, &scaleInfo->primaryParametrics, &scaleInfo->secondaryParametrics);
    }
    else {
        CursorDeviceSegment	*segment;

        // scale
        for(segment = (CursorDeviceSegment *) scaleInfo->scaleSegments;
            f_gt(scrollMultiplier, *(IOFixed64[]){ { segment->devUnits } });
            segment++)
        {}

        if (avgCount > 2) {
            // Continuous scrolling in one direction indicates a desire to go faster.
            scrollMultiplier = f_mul(scrollMultiplier, *(IOFixed64[]){ { sc2f((SInt64)lsqrt(avgCount * 16)) } });
            scrollMultiplier = f_div(scrollMultiplier, *(IOFixed64[]){ { sc2f(4) } });
        }

        scrollMultiplier = f_add(*(IOFixed64[]){ { segment->intercept } }, f_div(f_mul(scrollMultiplier, *(IOFixed64[]){ { segment->slope } }), *(IOFixed64[]){ { absAxis } } ));
    }
    axisValue = f_mul(axisValue, scrollMultiplier);
    *axisp = axisValue.value;
}

static void scaleWheel(int* deltaAxis1, SInt32* fixedDeltaAxis1, SInt32* pointDeltaAxis1, struct timespec ts){
    int deltaAxis2 = 0, deltaAxis3 = 0;

    bool negative = (*deltaAxis1 < 0);
    _scrollFixedDeltaAxis1 = *deltaAxis1 * SCROLL_DEFAULT_RESOLUTION;
    //_scrollFixedDeltaAxis1 = *deltaAxis1 << 16;
    CONVERT_SCROLL_FIXED_TO_COARSE(IOFixedMultiply(_scrollFixedDeltaAxis1, SCROLL_WHEEL_TO_PIXEL_SCALE), _scrollPointDeltaAxis1);

    bool            directionChange[3]          = {0,0,0};
    bool            typeChange                  = FALSE;
    SInt32*         pDeltaAxis[3]               = {deltaAxis1, &deltaAxis2, &deltaAxis3};
    SInt32*         pScrollFixedDeltaAxis[3]    = {&_scrollFixedDeltaAxis1, &_scrollFixedDeltaAxis2, &_scrollFixedDeltaAxis3};
    IOFixed*        pScrollPointDeltaAxis[3]    = {&_scrollPointDeltaAxis1, &_scrollPointDeltaAxis2, &_scrollPointDeltaAxis3};

    for (UInt32 type=kAccelTypeY; type<=kAccelTypeZ; type++ ) {
        directionChange[type]       = ((_scrollWheelInfo.axis[type].lastValue == 0) ||
                                       ((_scrollWheelInfo.axis[type].lastValue < 0) && (*(pDeltaAxis[type]) > 0)) ||
                                       ((_scrollWheelInfo.axis[type].lastValue > 0) && (*(pDeltaAxis[type]) < 0)));
        _scrollWheelInfo.axis[type].lastValue  = *(pDeltaAxis[type]);

        if ( _scrollWheelInfo.axis[type].scaleSegments || _scrollWheelInfo.axis[type].isParametric ) {

            *(pScrollPointDeltaAxis[type])  = _scrollWheelInfo.axis[type].lastValue << 16;

            AccelerateScrollAxis(pScrollPointDeltaAxis[type],
                                 &(_scrollWheelInfo.axis[type]),
                                 &ts,
                                 _scrollWheelInfo.rateMultiplier,
                                 directionChange[type] || typeChange);

            CONVERT_SCROLL_FIXED_TO_COARSE(pScrollPointDeltaAxis[type][0], pScrollPointDeltaAxis[type][0]);

            // RY: Convert pixel value to points
            *(pScrollFixedDeltaAxis[type]) = *(pScrollPointDeltaAxis[type]) << 16;

            if ( directionChange[type] )
                bzero(&(_scrollWheelInfo.axis[type].consumeState), sizeof(ScaleConsumeState));

            // RY: throttle the tranlation of scroll based on the resolution threshold.
            // This allows us to not generated traditional scroll wheel (line) events
            // for high res devices at really low (fine granularity) speeds.  This
            // prevents a succession of single scroll events that can make scrolling
            // slowly actually seem faster.
            if ( _scrollWheelInfo.axis[type].consumeCountThreshold )
            {
                _scrollWheelInfo.axis[type].consumeState.consumeAccum += *(pScrollFixedDeltaAxis[type]) + ((*(pScrollFixedDeltaAxis[type])) ? _scrollWheelInfo.axis[type].state.fraction : 0);
                _scrollWheelInfo.axis[type].consumeState.consumeCount += abs(_scrollWheelInfo.axis[type].lastValue);

                if (*(pScrollFixedDeltaAxis[type]) &&
                        ((abs(_scrollWheelInfo.axis[type].lastValue) >= (SInt32)_scrollWheelInfo.axis[type].consumeClearThreshold) ||
                         (_scrollWheelInfo.axis[type].consumeState.consumeCount >= _scrollWheelInfo.axis[type].consumeCountThreshold)))
                {
                    *(pScrollFixedDeltaAxis[type]) = _scrollWheelInfo.axis[type].consumeState.consumeAccum;
                    _scrollWheelInfo.axis[type].consumeState.consumeAccum = 0;
                    _scrollWheelInfo.axis[type].consumeState.consumeCount = 0;
                }
                else
                {
                    *(pScrollFixedDeltaAxis[type]) = 0;
                }
            }

            *(pScrollFixedDeltaAxis[type]) = IOFixedMultiply(*(pScrollFixedDeltaAxis[type]), SCROLL_PIXEL_TO_WHEEL_SCALE);

            // RY: Generate fixed point and course scroll deltas.
            CONVERT_SCROLL_FIXED_TO_COARSE(*(pScrollFixedDeltaAxis[type]), *(pDeltaAxis[type]));
        }
    }
    *fixedDeltaAxis1 = _scrollFixedDeltaAxis1;
    *pointDeltaAxis1 = _scrollPointDeltaAxis1;
    // Prevent direction reversing (I'm not sure why this happens...)
    if(negative != (*deltaAxis1 < 0))
        *deltaAxis1 = -*deltaAxis1;
    if(negative != (*fixedDeltaAxis1 < 0))
        *fixedDeltaAxis1 = -*fixedDeltaAxis1;
    if(negative != (*pointDeltaAxis1 < 0))
        *pointDeltaAxis1 = -*pointDeltaAxis1;
}

// Setup utilities

static uint get_desired(CFStringRef type_key, CFStringRef fallback_key){
    int res = 0;
    CFTypeRef number = 0;
    CFTypeRef accelKey = 0;
    if(IOPointingCopyCFTypeParameter(type_key, &accelKey) == kIOReturnSuccess){
        if(CFGetTypeID(accelKey) == CFStringGetTypeID())
            IOPointingCopyCFTypeParameter(accelKey, &number);
        if(accelKey) CFRelease(accelKey);
    }
    if(!number)
        IOPointingCopyCFTypeParameter(fallback_key, (CFTypeRef*)&number);
    if(!number)
        return 0;
    if(CFGetTypeID(number) == CFNumberGetTypeID())
        CFNumberGetValue(number, kCFNumberIntType, &res);
    else if(CFGetTypeID(number) == CFDataGetTypeID())
        CFDataGetBytes(number, CFRangeMake(0, sizeof(int)), (UInt8*)&res);
    CFRelease(number);
    return res;
}

static void do_setup(io_connect_t event, struct timespec now){
    // If it's been a while since the last check (1s) or we haven't set up yet, get the desired acceleration values
    struct timespec last = last_setup_poll;
    timespec_add(&last, 1000000000);
    if(!has_setup || timespec_gt(now, last)){
        static uint desired_mouse = UINT_MAX, desired_wheel = UINT_MAX;
        uint desired = get_desired(CFSTR(kIOHIDPointerAccelerationTypeKey), CFSTR(kIOHIDPointerAccelerationKey));
        // Set up parameters again if the value has changed
        if(desired != desired_mouse || !has_setup)
            setupForAcceleration(desired_mouse = desired);
        desired = get_desired(CFSTR(kIOHIDScrollAccelerationTypeKey), CFSTR(kIOHIDScrollAccelerationKey));
        if(desired != desired_wheel || !has_setup)
            setupScrollForAcceleration(desired_wheel = desired);
        has_setup = 1;
        last_setup_poll = now;
    }
}

// External functions (called from input_mac.c)

void mouse_accel(io_connect_t event, int* x, int* y){
    pthread_mutex_lock(&mutex);
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    do_setup(event, now);
    scalePointer(x, y);
    pthread_mutex_unlock(&mutex);
}

void wheel_accel(io_connect_t event, int* deltaAxis1, SInt32* fixedDeltaAxis1, SInt32* pointDeltaAxis1){
    pthread_mutex_lock(&mutex);
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    do_setup(event, now);
    scaleWheel(deltaAxis1, fixedDeltaAxis1, pointDeltaAxis1, now);
    pthread_mutex_unlock(&mutex);
}

#endif  // OS_MAC
