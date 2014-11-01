#define _DEFAULT_SOURCE
#include <features.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N_KEYS 144
FILE* output = 0;

// Keyboard LED positions, measured roughly in 16th inches. Most keys are 3/4" apart.
typedef struct {
    const char* name;
    int x, y;
} keypos;
keypos positions[] = {
    {"mr", 38, 0}, {"m1", 50, 0}, {"m2", 62, 0}, {"m3", 74, 0}, {"light", 222, 0}, {"lock", 234, 0}, {"mute", 273, 0},
    {"g1", 0, 14}, {"g2", 11, 14}, {"g3", 22, 14}, {"esc", 38, 14}, {"f1", 58, 14}, {"f2", 70, 14}, {"f3", 82, 14}, {"f4", 94, 14}, {"f5", 114, 14}, {"f6", 126, 14}, {"f7", 138, 14}, {"f8", 150, 14}, {"f9", 170, 14}, {"f10", 182, 14}, {"f11", 194, 14}, {"f12", 206, 14}, {"prtscn", 222, 14}, {"scroll", 234, 14}, {"pause", 246, 14}, {"stop", 262, 14}, {"prev", 273, 14}, {"play", 285, 14}, {"next", 296, 14},
    {"g4", 0, 25}, {"g5", 11, 25}, {"g6", 22, 25}, {"grave", 38, 27}, {"1", 50, 27}, {"2", 62, 27}, {"3", 74, 27}, {"4", 86, 27}, {"5", 98, 27}, {"6", 110, 27}, {"7", 122, 27}, {"8", 134, 27}, {"9", 146, 27}, {"0", 158, 27}, {"minus", 170, 27}, {"equal", 182, 27}, {"bspace", 200, 27}, {"ins", 222, 27}, {"home", 234, 27}, {"pgup", 246, 27}, {"numlock", 261, 27}, {"numslash", 273, 27}, {"numstar", 285, 27}, {"numminus", 297, 27},
    {"g7", 0, 39}, {"g8", 11, 39}, {"g9", 22, 39}, {"tab", 42, 39}, {"q", 56, 39}, {"w", 68, 39}, {"e", 80, 39}, {"r", 92, 39}, {"t", 104, 39}, {"y", 116, 39}, {"u", 128, 39}, {"i", 140, 39}, {"o", 152, 39}, {"p", 164, 39}, {"lbrace", 178, 39}, {"rbrace", 190, 39}, {"bslash", 202, 39}, {"del", 222, 39}, {"end", 234, 39}, {"pgdn", 246, 39}, {"num7", 261, 39}, {"num8", 273, 39}, {"num9", 285, 39}, {"numplus", 297, 33},
    {"g10", 0, 50}, {"g11", 11, 50}, {"g12", 22, 50}, {"caps", 44, 51}, {"a", 59, 51}, {"s", 71, 51}, {"d", 83, 51}, {"f", 95, 51}, {"g", 107, 51}, {"h", 119, 51}, {"j", 131, 51}, {"k", 143, 51}, {"l", 155, 51}, {"colon", 167, 51}, {"quote", 179, 51}, {"enter", 198, 51}, {"num4", 273, 51}, {"num5", 273, 51}, {"num6", 285, 51},
    {"g13", 0, 64}, {"g14", 11, 64}, {"g15", 22, 64}, {"lshift", 46, 63}, {"z", 65, 63}, {"x", 77, 63}, {"c", 89, 63}, {"v", 101, 63}, {"b", 113, 63}, {"n", 125, 63}, {"m", 137, 63}, {"comma", 149, 63}, {"dot", 161, 63}, {"slash", 173, 63}, {"rshift", 195, 63}, {"up", 234, 63}, {"num1", 261, 63}, {"num2", 273, 63}, {"num3", 285, 63}, {"numenter", 297, 69},
    {"g16", 0, 75}, {"g17", 11, 75}, {"g18", 22, 75}, {"lctrl", 42, 75}, {"lwin", 56, 75}, {"lalt", 70, 75}, {"space", 122, 75}, {"ralt", 164, 75}, {"rwin", 178, 75}, {"rmenu", 190, 75}, {"rctrl", 202, 75}, {"left", 222, 75}, {"down", 234, 75}, {"right", 246, 75}, {"num0", 267, 75}, {"numdot", 285, 75},
};
#define WIDTH 298
#define HEIGHT 76
#define N_POSITIONS (sizeof(positions)/sizeof(keypos))

void mainloop_random(float fr, float fg, float fb, float br, float bg, float bb){
    static float r[N_KEYS], g[N_KEYS], b[N_KEYS];
    static float rspeed[N_KEYS], gspeed[N_KEYS], bspeed[N_KEYS];
    static int firstrun = 1;
    static int frame = 0;
    // On first run, fill all colors randomly
    if(firstrun){
        srand(time(NULL));
        for(int i = 0; i < N_KEYS; i++){
            r[i] = rand() % 256;
            g[i] = rand() % 256;
            b[i] = rand() % 256;
        }
        firstrun = 0;
    }
    // On frame 0, pick a new set of colors to cycle to and choose the appropriate speeds to get there by frame 120
    if(frame == 0){
        for(int i = 0; i < N_KEYS; i++){
            float r2 = rand() % 256;
            float g2 = rand() % 256;
            float b2 = rand() % 256;
            rspeed[i] = (r2 - r[i]) / 120.f;
            gspeed[i] = (g2 - g[i]) / 120.f;
            bspeed[i] = (b2 - b[i]) / 120.f;
        }
    }
    // Update and output the keys
    fprintf(output, "rgb ");
    for(int i = 0; i < N_KEYS; i++){
        r[i] += rspeed[i];
        g[i] += gspeed[i];
        b[i] += bspeed[i];
        fprintf(output, "#%d:%02x%02x%02x,", i, (int)(r[i]), (int)(g[i]), (int)(b[i]));
    }
    fputc('\n', output);
    fflush(output);
    frame = (frame + 1) % 120;
}

void mainloop_wave(float fr, float fg, float fb, float br, float bg, float bb){
    float size = WIDTH + 36.f;
    static float wavepos = -36.f;
    fprintf(output, "rgb ");
    for(int i = 0; i < N_POSITIONS; i++){
        float r = br;
        float g = bg;
        float b = bb;
        float distance = fabs(positions[i].x - wavepos);
        if(distance <= 36.f){
            r = r * distance / 36.f + fr * (1.f - distance / 36.f);
            g = g * distance / 36.f + fg * (1.f - distance / 36.f);
            b = b * distance / 36.f + fb * (1.f - distance / 36.f);
        }
        fprintf(output, "%s:%02x%02x%02x", positions[i].name, (int)r, (int)g, (int)b);
    }
    fputc('\n', output);
    fflush(output);
    wavepos += (size + 36.f) / 2.f / 60.f;
    if(wavepos >= size)
        wavepos = -36.f;
}

void mainloop_ripple(float fr, float fg, float fb, float br, float bg, float bb){
    float size = sqrt(WIDTH*WIDTH/2. + HEIGHT*HEIGHT/2.);
    float cx = WIDTH / 2.f, cy = HEIGHT / 2.f;
    static float ringpos = -36.f;
    fprintf(output, "rgb ");
    for(int i = 0; i < N_POSITIONS; i++){
        float r = br;
        float g = bg;
        float b = bb;
        float distance = fabs(sqrt(pow(positions[i].x - cx, 2.) + pow(positions[i].y - cy, 2.)) - ringpos);
        if(distance <= 36.f){
            r = r * distance / 36.f + fr * (1.f - distance / 36.f);
            g = g * distance / 36.f + fg * (1.f - distance / 36.f);
            b = b * distance / 36.f + fb * (1.f - distance / 36.f);
        }
        fprintf(output, "%s:%02x%02x%02x", positions[i].name, (int)r, (int)g, (int)b);
    }
    fputc('\n', output);
    fflush(output);
    ringpos += (size + 36.f) / 60.f;
    if(ringpos >= size)
        ringpos = -36.f;
}

void mainloop_gradient(float fr, float fg, float fb, float br, float bg, float bb){
    static float grad = 1.f;
    int r = fr * grad + br * (1.f - grad);
    int g = fg * grad + bg * (1.f - grad);
    int b = fb * grad + bb * (1.f - grad);
    fprintf(output, "rgb %02x%02x%02x\n", r, g, b);
    fflush(output);
    if(grad == 0.f)
        exit(0);
    grad -= 1.f / 2.f / 60.f;
    if(grad < 0.f)
        grad = 0.f;
}

void mainloop_solid(float fr, float fg, float fb, float br, float bg, float bb){
    fprintf(output, "rgb %02x%02x%02x\n", (int)fr, (int)fg, (int)fb);
    fflush(output);
    exit(0);
}

int readcolor(const char* clr){
    if(!strcmp(clr, "red"))
        return 0xff0000;
    if(!strcmp(clr, "green"))
        return 0x00ff00;
    if(!strcmp(clr, "blue"))
        return 0x0000ff;
    if(!strcmp(clr, "cyan"))
        return 0x00ffff;
    if(!strcmp(clr, "magenta"))
        return 0xff00ff;
    if(!strcmp(clr, "yellow"))
        return 0xffff00;
    if(!strcmp(clr, "white"))
        return 0xffffff;
    if(!strcmp(clr, "black"))
        return 0x000000;
    int rgb;
    if(sscanf(clr, "%6x", &rgb) == 1)
        return rgb;
    return 0;
}

int main(int argc, char** argv){
    if(argc < 2){
        printf("Usage: ckb (solid | gradient | ripple | wave | random) [foreground] [background]\n");
        exit(0);
    }
    void (*mainloop)(float,float,float,float,float,float);
    if(!strcmp(argv[1], "solid"))
        mainloop = mainloop_solid;
    else if(!strcmp(argv[1], "gradient"))
        mainloop = mainloop_gradient;
    else if(!strcmp(argv[1], "ripple"))
        mainloop = mainloop_ripple;
    else if(!strcmp(argv[1], "wave"))
        mainloop = mainloop_wave;
    else if(!strcmp(argv[1], "random"))
        mainloop = mainloop_random;
    else {
        printf("Usage: ckb (solid | gradient | ripple | wave | random) [foreground] [background]\n");
        exit(0);
    }

    output = fopen("/dev/input/ckb1/led", "w");
    int foreground = 0xffffff, background = 0xffffff;
    if(argc >= 3)
        foreground = background = readcolor(argv[2]);
    if(argc >= 4)
        background = readcolor(argv[3]);
    float fr = (foreground >> 16) & 0xff, fg = (foreground >> 8) & 0xff, fb = foreground & 0xff;
    float br = (background >> 16) & 0xff, bg = (background >> 8) & 0xff, bb = background & 0xff;
    while(1){
        mainloop(fr, fg, fb, br, bg, bb);
        usleep(16667);
    }
    return 0;
}
