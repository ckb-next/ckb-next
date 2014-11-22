#include "kblightwidget.h"
#include "ui_kblightwidget.h"
#include <cmath>

extern int framerate;

// Keyboard LED positions, measured roughly in 16th inches. Most keys are 3/4" apart.
struct keypos {
    const char* name;
    int x, y;
};

keypos positions_us[] = {
    {"mr", 38, 0}, {"m1", 50, 0}, {"m2", 62, 0}, {"m3", 74, 0}, {"light", 222, 0}, {"lock", 234, 0}, {"mute", 273, 0},
    {"g1", 0, 14}, {"g2", 11, 14}, {"g3", 22, 14}, {"esc", 38, 14}, {"f1", 58, 14}, {"f2", 70, 14}, {"f3", 82, 14}, {"f4", 94, 14}, {"f5", 114, 14}, {"f6", 126, 14}, {"f7", 138, 14}, {"f8", 150, 14}, {"f9", 170, 14}, {"f10", 182, 14}, {"f11", 194, 14}, {"f12", 206, 14}, {"prtscn", 222, 14}, {"scroll", 234, 14}, {"pause", 246, 14}, {"stop", 262, 14}, {"prev", 273, 14}, {"play", 285, 14}, {"next", 296, 14},
    {"g4", 0, 25}, {"g5", 11, 25}, {"g6", 22, 25}, {"grave", 38, 27}, {"1", 50, 27}, {"2", 62, 27}, {"3", 74, 27}, {"4", 86, 27}, {"5", 98, 27}, {"6", 110, 27}, {"7", 122, 27}, {"8", 134, 27}, {"9", 146, 27}, {"0", 158, 27}, {"minus", 170, 27}, {"equal", 182, 27}, {"bspace", 200, 27}, {"ins", 222, 27}, {"home", 234, 27}, {"pgup", 246, 27}, {"numlock", 261, 27}, {"numslash", 273, 27}, {"numstar", 285, 27}, {"numminus", 297, 27},
    {"g7", 0, 39}, {"g8", 11, 39}, {"g9", 22, 39}, {"tab", 42, 39}, {"q", 56, 39}, {"w", 68, 39}, {"e", 80, 39}, {"r", 92, 39}, {"t", 104, 39}, {"y", 116, 39}, {"u", 128, 39}, {"i", 140, 39}, {"o", 152, 39}, {"p", 164, 39}, {"lbrace", 178, 39}, {"rbrace", 190, 39}, {"bslash", 202, 39}, {"del", 222, 39}, {"end", 234, 39}, {"pgdn", 246, 39}, {"num7", 261, 39}, {"num8", 273, 39}, {"num9", 285, 39}, {"numplus", 297, 33},
    {"g10", 0, 50}, {"g11", 11, 50}, {"g12", 22, 50}, {"caps", 44, 51}, {"a", 59, 51}, {"s", 71, 51}, {"d", 83, 51}, {"f", 95, 51}, {"g", 107, 51}, {"h", 119, 51}, {"j", 131, 51}, {"k", 143, 51}, {"l", 155, 51}, {"colon", 167, 51}, {"quote", 179, 51}, {"enter", 198, 51}, {"num4", 273, 51}, {"num5", 273, 51}, {"num6", 285, 51},
    {"g13", 0, 64}, {"g14", 11, 64}, {"g15", 22, 64}, {"lshift", 46, 63}, {"z", 65, 63}, {"x", 77, 63}, {"c", 89, 63}, {"v", 101, 63}, {"b", 113, 63}, {"n", 125, 63}, {"m", 137, 63}, {"comma", 149, 63}, {"dot", 161, 63}, {"slash", 173, 63}, {"rshift", 195, 63}, {"up", 234, 63}, {"num1", 261, 63}, {"num2", 273, 63}, {"num3", 285, 63}, {"numenter", 297, 69},
    {"g16", 0, 75}, {"g17", 11, 75}, {"g18", 22, 75}, {"lctrl", 42, 75}, {"lwin", 56, 75}, {"lalt", 70, 75}, {"space", 122, 75}, {"ralt", 164, 75}, {"rwin", 178, 75}, {"rmenu", 190, 75}, {"rctrl", 202, 75}, {"left", 222, 75}, {"down", 234, 75}, {"right", 246, 75}, {"num0", 267, 75}, {"numdot", 285, 75},
};

keypos positions_uk[] = {
    {"mr", 38, 0}, {"m1", 50, 0}, {"m2", 62, 0}, {"m3", 74, 0}, {"light", 222, 0}, {"lock", 234, 0}, {"mute", 273, 0},
    {"g1", 0, 14}, {"g2", 11, 14}, {"g3", 22, 14}, {"esc", 38, 14}, {"f1", 58, 14}, {"f2", 70, 14}, {"f3", 82, 14}, {"f4", 94, 14}, {"f5", 114, 14}, {"f6", 126, 14}, {"f7", 138, 14}, {"f8", 150, 14}, {"f9", 170, 14}, {"f10", 182, 14}, {"f11", 194, 14}, {"f12", 206, 14}, {"prtscn", 222, 14}, {"scroll", 234, 14}, {"pause", 246, 14}, {"stop", 262, 14}, {"prev", 273, 14}, {"play", 285, 14}, {"next", 296, 14},
    {"g4", 0, 25}, {"g5", 11, 25}, {"g6", 22, 25}, {"grave", 38, 27}, {"1", 50, 27}, {"2", 62, 27}, {"3", 74, 27}, {"4", 86, 27}, {"5", 98, 27}, {"6", 110, 27}, {"7", 122, 27}, {"8", 134, 27}, {"9", 146, 27}, {"0", 158, 27}, {"minus", 170, 27}, {"equal", 182, 27}, {"bspace", 200, 27}, {"ins", 222, 27}, {"home", 234, 27}, {"pgup", 246, 27}, {"numlock", 261, 27}, {"numslash", 273, 27}, {"numstar", 285, 27}, {"numminus", 297, 27},
    {"g7", 0, 39}, {"g8", 11, 39}, {"g9", 22, 39}, {"tab", 42, 39}, {"q", 56, 39}, {"w", 68, 39}, {"e", 80, 39}, {"r", 92, 39}, {"t", 104, 39}, {"y", 116, 39}, {"u", 128, 39}, {"i", 140, 39}, {"o", 152, 39}, {"p", 164, 39}, {"lbrace", 178, 39}, {"rbrace", 190, 39}, {"del", 222, 39}, {"end", 234, 39}, {"pgdn", 246, 39}, {"num7", 261, 39}, {"num8", 273, 39}, {"num9", 285, 39}, {"numplus", 297, 33},
    {"g10", 0, 50}, {"g11", 11, 50}, {"g12", 22, 50}, {"caps", 44, 51}, {"a", 59, 51}, {"s", 71, 51}, {"d", 83, 51}, {"f", 95, 51}, {"g", 107, 51}, {"h", 119, 51}, {"j", 131, 51}, {"k", 143, 51}, {"l", 155, 51}, {"colon", 167, 51}, {"quote", 179, 51}, {"hash", 191, 51}, {"enter", 198, 51}, {"num4", 273, 51}, {"num5", 273, 51}, {"num6", 285, 51},
    {"g13", 0, 64}, {"g14", 11, 64}, {"g15", 22, 64}, {"lshift", 46, 63}, {"bslash", 53, 63}, {"z", 65, 63}, {"x", 77, 63}, {"c", 89, 63}, {"v", 101, 63}, {"b", 113, 63}, {"n", 125, 63}, {"m", 137, 63}, {"comma", 149, 63}, {"dot", 161, 63}, {"slash", 173, 63}, {"rshift", 195, 63}, {"up", 234, 63}, {"num1", 261, 63}, {"num2", 273, 63}, {"num3", 285, 63}, {"numenter", 297, 69},
    {"g16", 0, 75}, {"g17", 11, 75}, {"g18", 22, 75}, {"lctrl", 42, 75}, {"lwin", 56, 75}, {"lalt", 70, 75}, {"space", 122, 75}, {"ralt", 164, 75}, {"rwin", 178, 75}, {"rmenu", 190, 75}, {"rctrl", 202, 75}, {"left", 222, 75}, {"down", 234, 75}, {"right", 246, 75}, {"num0", 267, 75}, {"numdot", 285, 75},
};

#define WIDTH 298
#define HEIGHT 76
#define N_POSITIONS_US (sizeof(positions_us)/sizeof(keypos))
#define N_POSITIONS_UK (sizeof(positions_uk)/sizeof(keypos))

void KbLightWidget::frameUpdate(QFile& cmd, int modenumber, int layout){
    int bg = bgColor.rgba() & 0xFFFFFF;
    int fg = fgColor.rgba() & 0xFFFFFF;
    keypos* positions = (layout == 1) ? positions_uk : positions_us;
    int N_POSITIONS = (positions == positions_uk) ? N_POSITIONS_UK : N_POSITIONS_US;
    if(forceLight)
        cmd.write(QString().sprintf("mode %d notify mr m1 m2 m3 light lock\n", modenumber).toLatin1());
    if(ui->brightnessBox->currentIndex() == 3){
        if(forceLight)
            cmd.write(QString().sprintf("mode %d switch rgb off\n", modenumber).toLatin1());
        forceLight = false;
        return;
    }
    float light = (3 - ui->brightnessBox->currentIndex()) / 3.f;
    int anim = ui->animBox->currentIndex();
    switch(anim){
    case 0:
        // No animation
        if(forceLight){
            float r = (bg >> 16) & 0xFF;
            float g = (bg >> 8) & 0xFF;
            float b = bg & 0xFF;
            r *= light;
            g *= light;
            b *= light;
            cmd.write(QString().sprintf("mode %d switch rgb on %02x%02x%02x\n", modenumber, (int)r, (int)g, (int)b).toLatin1());
        }
        break;
    case 1: {
        // Wave
        float size = WIDTH + 36.f;
        static float wavepos = -36.f;
        if(forceLight)
            wavepos = -36.f;
        cmd.write(QString().sprintf("mode %d switch rgb on", modenumber).toLatin1());
        for(int i = 0; i < N_POSITIONS; i++){
            float r = (bg >> 16) & 0xFF;
            float g = (bg >> 8) & 0xFF;
            float b = bg & 0xFF;
            float distance = fabs(positions[i].x - wavepos);
            if(distance <= 36.f){
                r = r * distance / 36.f + ((fg >> 16) & 0xFF) * (1.f - distance / 36.f);
                g = g * distance / 36.f + ((fg >> 8) & 0xFF) * (1.f - distance / 36.f);
                b = b * distance / 36.f + (fg & 0xFF) * (1.f - distance / 36.f);
            }
            r *= light;
            g *= light;
            b *= light;
            cmd.write(QString().sprintf(" %s:%02x%02x%02x", positions[i].name, (int)r, (int)g, (int)b).toLatin1());
        }
        cmd.write("\n");
        wavepos += (size + 36.f) / 2.f / (float)framerate;
        if(wavepos >= size)
            wavepos = -36.f;
        break;
    }
    case 2: {
        // Ripple
        float size = sqrt(WIDTH*WIDTH/2. + HEIGHT*HEIGHT/2.);
        float cx = WIDTH / 2.f, cy = HEIGHT / 2.f;
        static float ringpos = -36.f;
        if(forceLight)
            ringpos = -36.f;
        cmd.write(QString().sprintf("mode %d switch rgb on", modenumber).toLatin1());
        for(int i = 0; i < N_POSITIONS; i++){
            float r = (bg >> 16) & 0xFF;
            float g = (bg >> 8) & 0xFF;
            float b = bg & 0xFF;
            float distance = fabs(sqrt(pow(positions[i].x - cx, 2.) + pow(positions[i].y - cy, 2.)) - ringpos);
            if(distance <= 36.f){
                r = r * distance / 36.f + ((fg >> 16) & 0xFF) * (1.f - distance / 36.f);
                g = g * distance / 36.f + ((fg >> 8) & 0xFF) * (1.f - distance / 36.f);
                b = b * distance / 36.f + (fg & 0xFF) * (1.f - distance / 36.f);
            }
            r *= light;
            g *= light;
            b *= light;
            cmd.write(QString().sprintf(" %s:%02x%02x%02x", positions[i].name, (int)r, (int)g, (int)b).toLatin1());
        }
        cmd.write("\n");
        ringpos += (size + 36.f) / (float)framerate;
        if(ringpos >= size)
            ringpos = -36.f;
        break;
    }
    }
    forceLight = false;
}

void KbLightWidget::close(QFile &cmd, int modenumber){
    forceLight = true;
    if(ui->brightnessBox->currentIndex() == 3){
        cmd.write(QString().sprintf("mode %d rgb off\n", modenumber).toLatin1());
        return;
    }
    int bg = ui->bgButton->color.rgba() & 0xFFFFFF;
    cmd.write(QString().sprintf("mode %d rgb %06x\n", modenumber, bg).toLatin1());
}

void KbLightWidget::changeFG(QColor newColor){
    fgColor = newColor;
    forceLight = true;
}

void KbLightWidget::changeBG(QColor newColor){
    bgColor = newColor;
    forceLight = true;
}

void KbLightWidget::on_brightnessBox_currentIndexChanged(int index){
    forceLight = true;
}

void KbLightWidget::on_animBox_currentIndexChanged(int index){
    forceLight = true;
}

KbLightWidget::KbLightWidget(QWidget *parent) :
    QWidget(parent), active(false), forceLight(true),
    ui(new Ui::KbLightWidget)
{
    ui->setupUi(this);

    ui->bgButton->color = bgColor = QColor(255, 0, 0);
    ui->bgButton->updateImage();
    fgColor = QColor(255, 255, 255);
    connect(ui->bgButton, SIGNAL(colorChanged(QColor)), this, SLOT(changeBG(QColor)));
    connect(ui->fgButton, SIGNAL(colorChanged(QColor)), this, SLOT(changeFG(QColor)));
}

KbLightWidget::~KbLightWidget(){
    delete ui;
}
