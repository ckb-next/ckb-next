#include "kbwidget.h"
#include "ui_kbwidget.h"
#include <cmath>
#include <fcntl.h>

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

void KbWidget::frameUpdate(){
    readInput();
    static int prevBG = -1, prevAnim = -1;
    static float prevLight = 0.f;
    static int frame = 0;
    int bg = bgColor.rgba() & 0xFFFFFF;
    int fg = fgColor.rgba() & 0xFFFFFF;
    int rate = ui->framerateBox->currentIndex() + 1;
    keypos* positions = (ui->layoutBox->currentIndex() == 1) ? positions_uk : positions_us;
    int N_POSITIONS = (positions == positions_uk) ? N_POSITIONS_UK : N_POSITIONS_US;
    if(ui->brightnessBox->currentIndex() == 3)
        return;
    float light = (3 - ui->brightnessBox->currentIndex()) / 3.f;
    if(!(frame++ % rate)){
        QFile cmd;
        getCmd(cmd);
        int anim = ui->animBox->currentIndex();
        switch(anim){
        case 0:
            // No animation
            if(bg != prevBG || prevAnim != anim || light != prevLight){
                float r = (bg >> 16) & 0xFF;
                float g = (bg >> 8) & 0xFF;
                float b = bg & 0xFF;
                r *= light;
                g *= light;
                b *= light;
                cmd.write(QString().sprintf("rgb on %02x%02x%02x\n", (int)r, (int)g, (int)b).toLatin1());
            }
            break;
        case 1: {
            // Wave
            float size = WIDTH + 36.f;
            static float wavepos = -36.f;
            if(prevAnim != anim)
                wavepos = -36.f;
            cmd.write("rgb on");
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
            wavepos += (size + 36.f) / 2.f / 60.f * rate;
            if(wavepos >= size)
                wavepos = -36.f;
            break;
        }
        case 2: {
            // Ripple
            float size = sqrt(WIDTH*WIDTH/2. + HEIGHT*HEIGHT/2.);
            float cx = WIDTH / 2.f, cy = HEIGHT / 2.f;
            static float ringpos = -36.f;
            if(prevAnim != anim)
                ringpos = -36.f;
            cmd.write("rgb on");
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
            ringpos += (size + 36.f) / 60.f * rate;
            if(ringpos >= size)
                ringpos = -36.f;
            break;
        }
        }
        prevAnim = anim;
        prevBG = bg;
        prevLight = light;
        cmd.close();
    }
}

void KbWidget::readInput(){
    QFile notify;
    int fd = open(notifypath.toLatin1().constData(), O_RDONLY | O_NONBLOCK);
    if(notify.open(fd, QIODevice::ReadOnly, QFileDevice::AutoCloseHandle)){
        QString line;
        while((line = notify.readLine()) != ""){
            QStringList components = line.trimmed().split(" ");
            if(components[0] == "layout"){
                if(components[1] == "uk")
                    ui->layoutBox->setCurrentIndex(1);
                else if(components[1] == "us")
                    ui->layoutBox->setCurrentIndex(0);
            } else if(components[0] == "key"){
                if(components[1] == "+light"){
                    int index = ui->brightnessBox->currentIndex() - 1;
                    if(index < 0)
                        index = ui->brightnessBox->count() - 1;
                    ui->brightnessBox->setCurrentIndex(index);
                }
            } else if(components[0] == "mode"){
                if(components[2] == "rgb"){
                    if(components[3] == "on"){
                        if(ui->brightnessBox->currentIndex() == 3)
                            ui->brightnessBox->setCurrentIndex(0);
                        continue;
                    } else if(components[3] == "off"){
                        ui->brightnessBox->setCurrentIndex(3);
                        continue;
                    }
                    bool ok;
                    int rgb = components[3].toInt(&ok, 16);
                    rgb |= 0xFF000000;
                    if(ok){
                        bgColor = QColor((QRgb)rgb);
                        ui->bgButton->color = bgColor;
                        ui->bgButton->updateImage();
                    }
                }
            }
        }
        notify.close();
    }
}

KbWidget::KbWidget(QWidget *parent, const QString &path) :
    QWidget(parent), devpath(path), cmdpath(path + "/cmd"), notifyNumber(0),
    ui(new Ui::KbWidget)
{
    ui->setupUi(this);

    QFile mpath(path + "/model"), spath(path + "/serial"), fpath(path + "/fwversion");
    if(mpath.open(QIODevice::ReadOnly)){
        model = mpath.read(100);
        model = model.remove("Corsair").remove("Gaming Keyboard").remove("Keyboard").trimmed();
        mpath.close();
    }
    if(spath.open(QIODevice::ReadOnly)){
        serial = spath.read(100);
        serial = serial.trimmed();
        spath.close();
        ui->serialLabel->setText(serial);
    }
    if(fpath.open(QIODevice::ReadOnly)){
        firmware = fpath.read(100);
        firmware = firmware.trimmed();
        fpath.close();
        ui->fwLabel->setText(firmware);
    }
    // Find an available notification node
    for(int i = 1; i < 10; i++){
        QString notify = QString(path + "/notify%1").arg(i);
        if(!QFile::exists(notify)){
            notifyNumber = i;
            notifypath = notify;
            break;
        }
    }
    QFile cmd;
    getCmd(cmd);
    if(notifyNumber > 0)
        cmd.write(QString("notifyon %1 ").arg(notifyNumber).toLatin1());
    cmd.write("notify light ");
    cmd.write("get :rgbon ");
    cmd.write("get :rgb ");
    cmd.write("get :layout\n");
    cmd.close();

    ui->bgButton->color = bgColor = QColor(255, 0, 0);
    ui->bgButton->updateImage();
    fgColor = QColor(255, 255, 255);
    connect(ui->bgButton, SIGNAL(colorChanged(QColor)), this, SLOT(changeBG(QColor)));
    connect(ui->fgButton, SIGNAL(colorChanged(QColor)), this, SLOT(changeFG(QColor)));
}

KbWidget::~KbWidget(){
    if(notifyNumber > 0){
        QFile cmd;
        getCmd(cmd);
        int bg = ui->bgButton->color.rgba() & 0xFFFFFF;
        cmd.write(QString().sprintf("rgb %06x ", bg).toLatin1());
        cmd.write(QString("notifyoff %1\n").arg(notifyNumber).toLatin1());
        cmd.close();
    }
    delete ui;
}

void KbWidget::getCmd(QFile& file){
    int fd = open(cmdpath.toLatin1().constData(), O_WRONLY | O_NONBLOCK);
    if(!file.open(fd, QIODevice::WriteOnly, QFileDevice::AutoCloseHandle))
        cmdpath = notifypath = devpath = "";
}

void KbWidget::changeFG(QColor newColor){
    fgColor = newColor;
}

void KbWidget::changeBG(QColor newColor){
    bgColor = newColor;
}

void KbWidget::on_layoutBox_currentIndexChanged(int index){
    QFile cmd;
    getCmd(cmd);
    if(index == 1)
        cmd.write("layout uk\n");
    else
        cmd.write("layout us\n");
    cmd.close();
}

void KbWidget::on_brightnessBox_currentIndexChanged(int index){
    if(index == 3){
        QFile cmd;
        getCmd(cmd);
        cmd.write("rgb off\n");
        cmd.close();
    }
}
