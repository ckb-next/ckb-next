#include "dpi.h"
#include "usb.h"

void cmd_dpi(usbdevice* kb, usbmode* mode, int dummy, const char* stages, const char* values){
    (void)kb;
    (void)dummy;

    int disable = 0;
    ushort x, y;
    // Try to scan X,Y values
    if(sscanf(values, "%hu,%hu", &x, &y) != 2){
        // If that doesn't work, scan single number
        if(sscanf(values, "%hu", &x) == 1)
            y = x;
        else if(!strncmp(values, "off", 3))
            // If the right side says "off", disable the level(s)
            disable = 1;
        else
            // Otherwise, quit
            return;
    }
    if((x == 0 || y == 0) && !disable)
        return;
    // Scan the left side for stage numbers (comma-separated)
    int left = strlen(stages);
    int position = 0, field = 0;
    char stagename[3];
    while(position < left && sscanf(stages + position, "%2[^,]%n", stagename, &field) == 1){
        uchar stagenum;
        if(sscanf(stagename, "%hhu", &stagenum) && stagenum < DPI_COUNT){
            // Set DPI for this stage
            if(disable){
                mode->dpi.enabled &= ~(1 << stagenum);
                mode->dpi.x[stagenum] = 0;
                mode->dpi.y[stagenum] = 0;
            } else {
                mode->dpi.enabled |= 1 << stagenum;
                mode->dpi.x[stagenum] = x;
                mode->dpi.y[stagenum] = y;
            }
        }
        if(stages[position += field] == ',')
            position++;
    }
}

void cmd_dpisel(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* stage){
    (void)kb;
    (void)dummy1;
    (void)dummy2;

    uchar stagenum;
    if(sscanf(stage, "%hhu", &stagenum) != 1)
        return;
    if(stagenum > DPI_COUNT)
        return;
    mode->dpi.current = stagenum;
}

void cmd_lift(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* height){
    (void)kb;
    (void)dummy1;
    (void)dummy2;

    uchar heightnum;
    if(sscanf(height, "%hhu", &heightnum) != 1)
        return;
    if(heightnum > LIFT_MAX || heightnum < LIFT_MIN)
        return;
    mode->dpi.lift = heightnum;
}

void cmd_snap(usbdevice* kb, usbmode* mode, int dummy1, int dummy2, const char* enable){
    (void)kb;
    (void)dummy1;
    (void)dummy2;

    if(!strcmp(enable, "on"))
        mode->dpi.snap = 1;
    if(!strcmp(enable, "off"))
        mode->dpi.snap = 0;
}

char* printdpi(const dpiset* dpi, const usbdevice* kb){
    (void)kb;

    // Print all DPI settings
    const int BUFFER_LEN = 100;
    char* buffer = malloc(BUFFER_LEN);
    int length = 0;
    for(int i = 0; i < DPI_COUNT; i++){
        // Print the stage number
        int newlen = 0;
        snprintf(buffer + length, BUFFER_LEN - length, length == 0 ? "%d%n" : " %d%n", i, &newlen);
        length += newlen;
        // Print the DPI settings
        if(!(dpi->enabled & (1 << i)))
            snprintf(buffer + length, BUFFER_LEN - length, ":off%n", &newlen);
        else
            snprintf(buffer + length, BUFFER_LEN - length, ":%u,%u%n", dpi->x[i], dpi->y[i], &newlen);
        length += newlen;
    }
    return buffer;
}

int updatedpi(usbdevice* kb, int force){
    if(!kb->active)
        return 0;
    dpiset* lastdpi = &kb->profile->lastdpi;
    dpiset* newdpi = &kb->profile->currentmode->dpi;
    // Don't do anything if the settings haven't changed
    if(!force && !lastdpi->forceupdate && !newdpi->forceupdate
            && !memcmp(lastdpi, newdpi, sizeof(dpiset)))
        return 0;
    lastdpi->forceupdate = newdpi->forceupdate = 0;

    if (newdpi->current != lastdpi->current) {
        // Before we switch the current DPI stage, make sure the stage we are
        // switching to is both enabled and configured to the correct DPI.

        // Enable the stage if necessary.
        if ((lastdpi->enabled & 1 << newdpi->current) == 0) {
            uchar newenabled;
            // If the new enabled flags contain both the current and previous
            // stages, use it.
            if (newdpi->enabled & 1 << newdpi->current &&
                newdpi->enabled & 1 << lastdpi->current) {
                newenabled = newdpi->enabled;
            } else {
                // Otherwise just enable the new stage. We'll write the actual
                // requested flags after switching stages.
                newenabled = lastdpi->enabled | 1 << newdpi->current;
            }
            uchar data_pkt[MSG_SIZE] = { 0x07, 0x13, 0x05, 0, newenabled };
            if(!usbsend(kb, data_pkt, 1))
                return -2;
            // Cache the flags we wrote.
            lastdpi->enabled = newenabled;
        }
        // Set the DPI for the new stage if necessary.
        if (newdpi->x[newdpi->current] != lastdpi->x[newdpi->current] ||
            newdpi->y[newdpi->current] != lastdpi->y[newdpi->current]) {
            uchar data_pkt[MSG_SIZE] = { 0x07, 0x13, 0xd0, 0 };
            data_pkt[2] |= newdpi->current;
            data_pkt[5] = newdpi->x[newdpi->current] & 0xFF;
            data_pkt[6] = (newdpi->x[newdpi->current] >> 8) & 0xFF;
            data_pkt[7] = newdpi->y[newdpi->current] & 0xFF;
            data_pkt[8] = (newdpi->y[newdpi->current] >> 8) & 0xFF;
            if(!usbsend(kb, data_pkt, 1))
                return -1;
            // Set these values in the cache so we don't rewrite them.
            lastdpi->x[newdpi->current] = newdpi->x[newdpi->current];
            lastdpi->y[newdpi->current] = newdpi->y[newdpi->current];
        }
        // Set current DPI stage.
        uchar data_pkt[MSG_SIZE] = { 0x07, 0x13, 0x02, 0, newdpi->current };
        if(!usbsend(kb, data_pkt, 1))
            return -2;
    }
    
    // Send X/Y DPIs. We've changed to the new stage already so these can be set
    // safely.
    for(int i = 0; i < DPI_COUNT; i++){
        if (newdpi->x[i] == lastdpi->x[i] && newdpi->y[i] == lastdpi->y[i])
            continue;
        uchar data_pkt[MSG_SIZE] = { 0x07, 0x13, 0xd0, 0 };
        data_pkt[2] |= i;
        data_pkt[5] = newdpi->x[i] & 0xFF;
        data_pkt[6] = (newdpi->x[i] >> 8) & 0xFF;
        data_pkt[7] = newdpi->y[i] & 0xFF;
        data_pkt[8] = (newdpi->y[i] >> 8) & 0xFF;
        if(!usbsend(kb, data_pkt, 1))
            return -1;
    }

    // Send settings
    if (newdpi->enabled != lastdpi->enabled) {
        uchar data_pkt[MSG_SIZE] = { 0x07, 0x13, 0x05, 0, newdpi->enabled };
        if(!usbsend(kb, data_pkt, 1))
            return -2;
    }
    if (newdpi->lift != lastdpi->lift) {
        uchar data_pkt[MSG_SIZE] = { 0x07, 0x13, 0x03, 0, newdpi->lift };
        if(!usbsend(kb, data_pkt, 1))
            return -2;
    }
    if (newdpi->snap != lastdpi->snap) {
        uchar data_pkt[MSG_SIZE] = { 0x07, 0x13, 0x04, 0, newdpi->snap, 0x05 };
        if(!usbsend(kb, data_pkt, 1))
            return -2;
    }

    // Finished
    memcpy(lastdpi, newdpi, sizeof(dpiset));
    return 0;
}

int savedpi(usbdevice* kb, dpiset* dpi, lighting* light){
    // Send X/Y DPIs
    for(int i = 0; i < DPI_COUNT; i++){
        uchar data_pkt[MSG_SIZE] = { 0x07, 0x13, 0xd0, 1 };
        data_pkt[2] |= i;
        data_pkt[5] = dpi->x[i] & 0xFF;
        data_pkt[6] = (dpi->x[i] >> 8) & 0xFF;
        data_pkt[7] = dpi->y[i] & 0xFF;
        data_pkt[8] = (dpi->y[i] >> 8) & 0xFF;
        // Save the RGB value for this setting too
        data_pkt[9] = light->r[LED_MOUSE + N_MOUSE_ZONES + i];
        data_pkt[10] = light->g[LED_MOUSE + N_MOUSE_ZONES + i];
        data_pkt[11] = light->b[LED_MOUSE + N_MOUSE_ZONES + i];
        if(!usbsend(kb, data_pkt, 1))
            return -1;
    }

    // Send settings
    uchar data_pkt[4][MSG_SIZE] = {
        { 0x07, 0x13, 0x05, 1, dpi->enabled },
        { 0x07, 0x13, 0x02, 1, dpi->current },
        { 0x07, 0x13, 0x03, 1, dpi->lift },
        { 0x07, 0x13, 0x04, 1, dpi->snap, 0x05 }
    };
    if(!usbsend(kb, data_pkt[0], 4))
        return -2;
    // Finished
    return 0;
}

int loaddpi(usbdevice* kb, dpiset* dpi, lighting* light){
    // Ask for settings
    uchar data_pkt[4][MSG_SIZE] = {
        { 0x0e, 0x13, 0x05, 1, },
        { 0x0e, 0x13, 0x02, 1, },
        { 0x0e, 0x13, 0x03, 1, },
        { 0x0e, 0x13, 0x04, 1, }
    };
    uchar in_pkt[4][MSG_SIZE];
    for(int i = 0; i < 4; i++){
        if(!usbrecv(kb, data_pkt[i], in_pkt[i]))
            return -2;
        if(memcmp(in_pkt[i], data_pkt[i], 4)){
            ckb_err("Bad input header\n");
            return -3;
        }
    }
    // Copy data from device
    dpi->enabled = in_pkt[0][4];
    dpi->enabled &= (1 << DPI_COUNT) - 1;
    dpi->current = in_pkt[1][4];
    if(dpi->current >= DPI_COUNT)
        dpi->current = 0;
    dpi->lift = in_pkt[2][4];
    if(dpi->lift < LIFT_MIN || dpi->lift > LIFT_MAX)
        dpi->lift = LIFT_MIN;
    dpi->snap = !!in_pkt[3][4];

    // Get X/Y DPIs
    for(int i = 0; i < DPI_COUNT; i++){
        uchar data_pkt[MSG_SIZE] = { 0x0e, 0x13, 0xd0, 1 };
        uchar in_pkt[MSG_SIZE];
        data_pkt[2] |= i;
        if(!usbrecv(kb, data_pkt, in_pkt))
            return -2;
        if(memcmp(in_pkt, data_pkt, 4)){
            ckb_err("Bad input header\n");
            return -3;
        }
        // Copy to profile
        dpi->x[i] = (in_pkt[6] << 8) | in_pkt[5];
        dpi->y[i] = (in_pkt[8] << 8) | in_pkt[7];
        light->r[LED_MOUSE + N_MOUSE_ZONES + i] = in_pkt[9];
        light->g[LED_MOUSE + N_MOUSE_ZONES + i] = in_pkt[10];
        light->b[LED_MOUSE + N_MOUSE_ZONES + i] = in_pkt[11];
    }
    // Finished. Set SW DPI light to the current hardware level
    light->r[LED_MOUSE + 2] = light->r[LED_MOUSE + N_MOUSE_ZONES + dpi->current];
    light->g[LED_MOUSE + 2] = light->g[LED_MOUSE + N_MOUSE_ZONES + dpi->current];
    light->b[LED_MOUSE + 2] = light->b[LED_MOUSE + N_MOUSE_ZONES + dpi->current];
    return 0;
}
