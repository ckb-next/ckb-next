#include "led.h"
#include "usb.h"

enum HWANIM_MODE {
    HWANIM_COLOURSHIFT          = 0x00,
    HWANIM_COLOURPULSE          = 0x01,
    HWANIM_RAINBOW              = 0x03,
    HWANIM_WAVE                 = 0x04,
    HWANIM_VISOR                = 0x05,
    HWANIM_RAIN                 = 0x06,
    HWANIM_STATIC               = 0x07,
    HWANIM_TYPELIGHTNING_KEY    = 0x08,
    HWANIM_TYPELIGHTNING_RIPPLE = 0x09,
    HWANIM_NONE                 = 0xff
};

enum HWANIM_ZONE_BIT {
    HWANIM_ZONEBIT_FRONT = 0,
    HWANIM_ZONEBIT_THUMB = 1,
    HWANIM_ZONEBIT_REAR  = 2
};

enum HWANIM_DIRECTION {
    HWANIM_DIRECTION_NONE  = 0,
    HWANIM_DIRECTION_LEFT  = 1,
    HWANIM_DIRECTION_RIGHT = 2
};

enum HWANIM_SPEED {
    HWANIM_SPEED_NONE   = 0,
    HWANIM_SPEED_LOW    = 1,
    HWANIM_SPEED_MEDIUM = 2,
    HWANIM_SPEED_HIGH   = 3
};

const short DARK_CORE_ZONES[3] = {LED_MOUSE + 1, LED_MOUSE + 5, LED_MOUSE + 3};

int updatergb_wireless(usbdevice* kb, lighting* lastlight, lighting* newlight) {
    if(IS_K63_WL(kb) || (IS_DARK_CORE(kb) && HAS_FEATURES(kb, FEAT_DONGLE)))
        return 0;
    for(int i = 0; i < 3; i++)
    {
        uchar r[2] = {newlight->r[DARK_CORE_ZONES[i]]};
        uchar g[2] = {newlight->g[DARK_CORE_ZONES[i]]};
        uchar b[2] = {newlight->b[DARK_CORE_ZONES[i]]};
        apply_hwanim(kb, i, HWANIM_STATIC, HWANIM_SPEED_LOW, HWANIM_DIRECTION_NONE, r, g, b);
    }

    memcpy(lastlight, newlight, sizeof(lighting));
    return 0;
}

void cmd_hwanim(usbdevice* kb, usbmode* dummy1, int dummy2, int keyindex, const char* code) {
    // Unified format for the code string:
    //  - DARK CORE Format: animation mode:speed:random:colour:colour2
    //  - K63 WL    Format: animation mode:speed:dir

    uchar anim = 0xff, speed = 2, rand_or_dir = 0;
    uchar r[2] = {}, g[2] = {}, b[2] = {};
    sscanf(code, "%hhx:%hhu:%hhu:%02hhx%02hhx%02hhx:%02hhx%02hhx%02hhx", &anim, &speed, &rand_or_dir, r, g, b, r+1, g+1, b+1);

    apply_hwanim(kb, kb->keymap[keyindex].led, anim, speed, rand_or_dir, r, g, b);
}

void apply_hwanim(usbdevice* kb, short zone, uchar anim, uchar speed, uchar rand_or_dir, uchar r[2], uchar g[2], uchar b[2]) {
    // Packet to change the devices' animation
    uchar anim_pkt[MSG_SIZE] = {
        CMD_SET, 0xaa
    };

    // Create pointer to anim_pkt which starts at index 4
    uchar* anim_ptr = anim_pkt + 4;

    if(IS_DARK_CORE(kb)){
        switch(speed){
        case HWANIM_SPEED_LOW:
        case HWANIM_SPEED_MEDIUM:
        case HWANIM_SPEED_HIGH:
            break;
        default:
            return;
        }

        switch(zone){
        case HWANIM_ZONEBIT_FRONT:
        case HWANIM_ZONEBIT_THUMB:
        case HWANIM_ZONEBIT_REAR:
            break;
        default:
            return;
        }

        switch(anim){
        case HWANIM_COLOURSHIFT:
        case HWANIM_COLOURPULSE:
            // Deal with the random byte's weirdness
            if (!rand_or_dir) {
                rand_or_dir = 3;
            }
            break;
        case HWANIM_RAINBOW:
        case HWANIM_STATIC:
            // Speed not supported
            speed = 0;
        case HWANIM_NONE:
            break;
        default:
            return;
        }
    } else if(IS_K63_WL(kb)) {
        switch(anim) {
        case HWANIM_COLOURSHIFT:
        case HWANIM_COLOURPULSE:
        case HWANIM_RAINBOW:
        case HWANIM_VISOR:
        case HWANIM_RAIN:
        case HWANIM_TYPELIGHTNING_KEY:
        case HWANIM_TYPELIGHTNING_RIPPLE:
            switch (speed) {
            case HWANIM_SPEED_LOW:
            case HWANIM_SPEED_MEDIUM:
            case HWANIM_SPEED_HIGH:
                break;
            default:
                return;
            }
            // Direction not supported
            rand_or_dir = HWANIM_DIRECTION_NONE;
            break;
        case HWANIM_STATIC:
        case HWANIM_NONE:
            // Direction and speed not supported
            rand_or_dir = HWANIM_DIRECTION_NONE;
            speed = HWANIM_SPEED_NONE;
            break;
        case HWANIM_WAVE:
            switch (rand_or_dir) {
            case HWANIM_DIRECTION_LEFT:
            case HWANIM_DIRECTION_RIGHT:
                break;
            default:
                return;
            }
            break;
        default:
            return;
        }

        // We do this because iCUE does it
        uchar k63wl_pkt[MSG_SIZE] = {
            CMD_SET, 0xaa, 0, 0, 0, 0, 0, 100
        };
        if(!usbsend(kb, k63wl_pkt, 1))
            return;
    }

    // K63WL is shifted one byte to the left since it lacks the zone byte
    if(IS_DARK_CORE(kb))
        *anim_ptr++ = 1 << zone;       // Bitmask.
    *anim_ptr++ = anim;                // Command (static colour).
    *anim_ptr++ = speed;               // Speed (1-3)
    *anim_ptr++ = rand_or_dir;         // Random/Alternating byte (Dark Core) Direction byte on K63WL
    *anim_ptr++ = 100;                 // Opacity (100%).
    if(IS_DARK_CORE(kb)){
        *anim_ptr++ = r[0];
        *anim_ptr++ = g[0];
        *anim_ptr++ = b[0];
        // Second color for effects like alternating color shift
        *anim_ptr++ = r[1];
        *anim_ptr++ = g[1];
        *anim_ptr++ = b[1];
        *anim_ptr++ = 5 - zone;         // Layer byte
    }

    //No testing for success since the function returns after this anyway
    usbsend(kb, anim_pkt, 1);
}
