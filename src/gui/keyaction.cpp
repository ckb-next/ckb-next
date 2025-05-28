#include <QDebug>
#include "keyaction.h"
#include "kb.h"
#include "kbanim.h"
#include "kbprofile.h"
#include "monotonicclock.h"
#include <QUrl>
#include <cstring>
#include <cstdio>
#ifdef USE_XCB
#include "mainwindow.h"
#include <QWindow>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QX11Info>
#endif
#endif

KeyAction::Type KeyAction::type() const {
    if(_value.isEmpty())
        return UNBOUND;
    if(_value.at(0) == '$' || _value == "lghtpgm")
        return SPECIAL;
    return NORMAL;
}

KeyAction::KeyAction(const QString &action, QObject *parent)
    : QObject(parent), _value(action), preProgram(nullptr), relProgram(nullptr), sniperValue(0)
{
}

KeyAction::KeyAction(QObject *parent)
    : QObject(parent), _value(""), preProgram(nullptr), relProgram(nullptr), sniperValue(0)
{
}

KeyAction::~KeyAction(){
    // Clean up processes
    if(preProgram){
        preProgram->kill();
        delete preProgram;
    }
    if(relProgram){
        relProgram->kill();
        delete relProgram;
    }
}

QString KeyAction::defaultAction(const QString& key, KeyMap::Model model){
    // G1-G18 are unbound by default
    if(key.length() >= 2 && key[0] == 'g'
		&& ((key.length() == 2 && key[1] >= '0' && key[1] <= '9')
		|| (key.length() == 3 && key[1] == '1' && key[2] >= '0' && key[2] <= '8')))
        return "";
    // So are thumbgrid buttons
    if(key.startsWith("thumb"))
        return "";
    // TODO: default action for MR
    if(key == "mr")
        return "";
    // M1-M3 switch modes
    if(key == "m1")
        return "$mode:0";
    if(key == "m2")
        return "$mode:1";
    if(key == "m3")
        return ((model == KeyMap::K70MK2) ? "" : "$mode:2");
    // Brightness and Win Lock are their own functions
    if(key == "light")
        return "$light:2";
    if(key == "lock")
        return "$lock:0";
    // DPI buttons
    if(key == "dpiup"){
        if(model == KeyMap::M55 || model == KeyMap::HARPOON || model == KeyMap::GLAIVE || model == KeyMap::IRONCLAW)
            return "$dpi:-4";
        return "$dpi:-2";
    }
    if(key == "dpidn")
        return "$dpi:-1";
    if(key == "sniper")
        return "$dpi:0";
    if(key == "profswitch")
        return "$mode:-3";
    if(key == "profdn")
        return "$mode:-4";

#ifdef Q_OS_MACOS
    // macOS has no forwards and backwards, so we bind them to macros that simulate that action
    if(key == "mouse4")
        return "$macro:+lwin,+lbrace,-lbrace,-lwin:⌘[::key +lwin\n=268957\nkey +lbrace\n=113103\nkey -lbrace\n=208898\nkey -lwin";
    if(key == "mouse5")
        return "$macro:+lwin,+rbrace,-rbrace,-lwin:⌘]::key +lwin\n=268957\nkey +rbrace\n=113103\nkey -rbrace\n=208898\nkey -lwin";
#endif

    // Everything else is a standard keypress
    return key;
}

QString KeyAction::friendlyName(const KeyMap& map) const {
    if(_value.isEmpty())
        return tr("Unbound");
    QStringList parts = _value.split(":");
    QString prefix = parts[0];
    if(parts.length() < 2){
        KeyMap::Layout layout = map.layout();
        QString name = KeyMap::friendlyName(_value, layout);
        if(name.isEmpty())
            return tr("(Unknown)");
        return name;
    }
    int suffix = parts[1].toInt();
    if(prefix == "$mode"){
        switch(suffix){
        case MODE_PREV:
        case MODE_PREV_WRAP:
            return tr("Switch to previous mode");
        case MODE_NEXT:
        case MODE_NEXT_WRAP:
            return tr("Switch to next mode");
        default:
            return tr("Switch to mode %1").arg(suffix + 1);
        }
    } else if(prefix == "$dpi"){
        // Split off custom parameters (if any)
        int level = parts[1].split("+")[0].toInt();
        switch(level){
        case DPI_CYCLE_UP:
            return tr("DPI cycle up");
        case DPI_CYCLE_DOWN:
            return tr("DPI cycle down");
        case DPI_UP:
            return tr("DPI up");
        case DPI_DOWN:
            return tr("DPI down");
        case DPI_SNIPER:
            return tr("Sniper");
        case DPI_CUSTOM:{
            QPoint xy;
            dpiInfo(xy);
            return tr("DPI: %1, %2").arg(xy.x()).arg(xy.y());
        }
        default:
            return tr("DPI stage %1").arg(level);
        }
    } else if(prefix == "$light"){
        switch(suffix){
        case LIGHT_UP:
        case LIGHT_UP_WRAP:
            return tr("Brightness up");
        case LIGHT_DOWN:
        case LIGHT_DOWN_WRAP:
            return tr("Brightness down");
        }
    } else if(prefix == "$lock"){
        switch(suffix){
        case LOCK_TOGGLE:
            return tr("Toggle Windows lock");
        case LOCK_ON:
            return tr("Windows lock on");
        case LOCK_OFF:
            return tr("Windows lock off");
        }
    } else if(prefix == "$anim"){
        return tr("Start animation");
    } else if(prefix == "$program"){
        return tr("Launch program");
    } else if(prefix == "$macro"){
        return tr("Send G-key macro");
    }
    return tr("(Unknown)");
}

QString KeyAction::modeAction(int mode){
    return QString("$mode:%1").arg(mode);
}

QString KeyAction::dpiAction(int level, int customX, int customY){
    QString action = QString("$dpi:%1").arg(level);
    if(level == DPI_CUSTOM)
        action += QString("+%1+%2").arg(customX).arg(customY);
    return action;
}

QString KeyAction::lightAction(int type){
    return QString("$light:%1").arg(type);
}

QString KeyAction::lockAction(int type){
    return QString("$lock:%1").arg(type);
}

QString KeyAction::programAction(const QString& onPress, const QString& onRelease, int stop){
    // URL-encode the commands and place them in the string (":" and "+" are both replaced, so they won't interfere)
    return "$program:" + QString::fromUtf8(QUrl::toPercentEncoding(onPress.trimmed())) + "+" + QString::fromUtf8(QUrl::toPercentEncoding(onRelease.trimmed())) + QString("+%1").arg(stop);
}

const static int ANIM_ONCE = 0x1, ANIM_KRSTOP = 0x2;

QString KeyAction::animAction(const QUuid& guid, bool onlyOnce, bool stopOnRelease){
    int flags = (onlyOnce ? ANIM_ONCE : 0) | (stopOnRelease ? ANIM_KRSTOP : 0);
    return "$anim:" + guid.toString() + QString("+%1").arg(flags);
}

QString KeyAction::specialInfo(int& parameter) const {
    QStringList list = _value.split(":");
    if(list.length() < 2){
        parameter = INT_MIN;
        return "";
    }
    parameter = list[1].toInt();
    return list[0].replace("$", "");
}

int KeyAction::programInfo(QString& onPress, QString& onRelease) const {
    if(!isProgram())
        return 0;
    QString param = _value.mid(9);
    QStringList programs = param.split("+");
    if(programs.length() != 3)
        return 0;
    onPress = QUrl::fromPercentEncoding(programs[0].toUtf8());
    onRelease = QUrl::fromPercentEncoding(programs[1].toUtf8());
    return programs[2].toInt();
}

int KeyAction::dpiInfo(QPoint& custom) const {
    if(!isDPI())
        return 0;
    QString param = _value.mid(5);
    QStringList lxy = param.split("+");
    int level = lxy[0].toInt();
    if(level == DPI_CUSTOM){
        if(lxy.length() != 3)
            return 0;
        custom = QPoint(lxy[1].toInt(), lxy[2].toInt());
    }
    return level;
}

QUuid KeyAction::animInfo(bool &onlyOnce, bool &stopOnRelease) const {
    if(!isAnim())
        return QUuid();
    QString param = _value.mid(6);
    QStringList split = param.split("+");
    if(split.length() < 2)
        return QUuid();
    QUuid id(split[0]);
    int flags = split[1].toInt();
    onlyOnce = !!(flags & ANIM_ONCE);
    stopOnRelease = !!(flags & ANIM_KRSTOP);
    return id;
}

QString KeyAction::driverName() const {
    if(isSpecial())
        return "";
    return _value;
}

void KeyAction::keyEvent(KbBind* bind, bool down){
    // No need to respond to standard actions
    if(!isSpecial())
        return;
    QStringList parts = _value.split(":");
    if(parts.length() < 2)
        return;
    QString prefix = parts[0];
    int suffix = parts[1].toInt();
    if(prefix == "$mode"){
        if(!down)
            return;
        // Change mode
        Kb* device = bind->devParent();
        KbProfile* currentProfile = device->currentProfile();
        int mode = currentProfile->indexOf(currentProfile->currentMode());
        int modeCount = currentProfile->modeCount();
        switch(suffix){
        case MODE_PREV_WRAP:
            mode--;
            if(mode < 0)
                mode = modeCount - 1;
            break;
        case MODE_NEXT_WRAP:
            mode++;
            if(mode >= modeCount)
                mode = 0;
            break;
        case MODE_PREV:
            mode--;
            break;
        case MODE_NEXT:
            mode++;
            break;
        default:
            // Absolute
            mode = suffix;
            break;
        }
        if(mode < 0 || mode >= modeCount)
            return;
        device->setCurrentMode(currentProfile->modes()[mode]);
    } else if(prefix == "$dpi"){
        KbPerf* perf = bind->perf();
        int level = parts[1].split("+")[0].toInt();
        switch(level){
        case DPI_CYCLE_UP:
            if(!down)
                return;
            perf->dpiCycleUp();
            break;
        case DPI_CYCLE_DOWN:
            if(!down)
                return;
            perf->dpiCycleDown();
            break;
        case DPI_UP:
            if(!down)
                return;
            perf->dpiUp();
            break;
        case DPI_DOWN:
            if(!down)
                return;
            perf->dpiDown();
            break;
        case DPI_SNIPER:
            if(down)
                sniperValue = perf->pushSniper();
            else {
                perf->popDpi(sniperValue);
                sniperValue = 0;
            }
            break;
        case DPI_CUSTOM:{
            QPoint xy;
            dpiInfo(xy);
            if(xy.x() <= 0 || xy.y() <= 0)
                break;
            if(down)
                sniperValue = perf->pushDpi(xy, false);
            else {
                perf->popDpi(sniperValue);
                sniperValue = 0;
            }
            break;
        }
        default:
            if(level < 1 || level >= KbPerf::DPI_COUNT
                    || !down)
                return;
            perf->baseDpiIdx(level);
            break;
        }
    } else if(prefix == "$light"){
        if(!down)
            return;
        // Change brightness
        KbLight* light = bind->light();
        int dim = light->dimming();
        switch(suffix){
        case LIGHT_UP:
            if(dim > 0)
                dim--;
            break;
        case LIGHT_DOWN:
            if(dim < KbLight::MAX_DIM)
                dim++;
            break;
        case LIGHT_UP_WRAP:
            dim--;
            if(dim < 0)
                dim = KbLight::MAX_DIM;
            break;
        case LIGHT_DOWN_WRAP:
            dim++;
            if(dim > KbLight::MAX_DIM)
                dim = 0;
            break;
        }
        light->dimming(dim);
    } else if(prefix == "$lock"){
        if(!down)
            return;
        // Change win lock
        switch(suffix){
        case LOCK_TOGGLE:
            bind->winLock(!bind->winLock());
            break;
        case LOCK_ON:
            bind->winLock(true);
            break;
        case LOCK_OFF:
            bind->winLock(false);
            break;
        }
    } else if(prefix == "$anim"){
        // Start animation
        bool onlyOnce = false, stopOnRelease = false;
        QUuid id = animInfo(onlyOnce, stopOnRelease);
        KbAnim* anim = bind->light()->findAnim(id);
        if(!anim)
            return;
        if(down){
            if(!onlyOnce || !anim->isActive())
                // If "only once" is enabled, don't start the animation when it's already running
                anim->trigger(MonotonicClock::msecs(), true);
        } else if(stopOnRelease){
            // Key released - stop animation
            anim->stop();
        }
    } else if(prefix == "$program"){
        // Launch program
        QString onPress, onRelease;
        int stop = programInfo(onPress, onRelease);
        // Stop running programs based on setting
        QProcess* process = nullptr;
        if(down){
            if(stop & PROGRAM_PR_KPSTOP){
                process = preProgram;
                if(process)
                    process->kill();
                process = nullptr;
            }
            if(stop & PROGRAM_RE_KPSTOP)
                process = relProgram;
        } else {
            if(stop & PROGRAM_PR_KRSTOP)
                process = preProgram;
        }
        if(process)
            process->kill();
        // Launch new process if requested
        QString& program = down ? onPress : onRelease;
        if(program.isEmpty())
            return;
        // Check if the program is running already. If so, don't start it again.
        process = down ? preProgram : relProgram;
        if(process){
            if(process->state() == QProcess::NotRunning)
                delete process;
            else
                return;
        }

        // Adjust the selected display.
        const QString& newDisplay = getDisplayForScreenCursor();

        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        if(!newDisplay.isEmpty())
            env.insert(QLatin1String("DISPLAY"), newDisplay);

        // Start the program. Wrap it around sh to parse arguments.
        if((down && (stop & PROGRAM_PR_MULTI))
                || (!down && (stop & PROGRAM_RE_MULTI))){
            // Multiple instances allowed? Start detached process
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            QProcess proc;
            proc.setProgram("sh");
            proc.setArguments(QStringList() << "-c" << program);
            proc.setProcessEnvironment(env);
            proc.startDetached();
#else
            if(!newDisplay.isEmpty())
                program.prepend(QString("DISPLAY=%1 ").arg(newDisplay));
            QProcess::startDetached("sh", QStringList() << "-c" << program);
#endif
        } else {
            process = new QProcess(this);
            process->setProcessEnvironment(env);
            process->start("sh", QStringList() << "-c" << program);
            if(down)
                preProgram = process;
            else
                relProgram = process;
        }
    }
}

// From main.cpp
extern const char* DISPLAY;

QString KeyAction::getDisplayForScreenCursor(){
    QString displaystr;
#ifdef USE_XCB
    if(QApplication::platformName() != QLatin1String("xcb"))
        return displaystr;
#if QT_VERSION < QT_VERSION_CHECK(6, 0 ,0)
    xcb_connection_t* conn = QX11Info::connection();
#else
    xcb_connection_t* conn = nullptr;
    QNativeInterface::QX11Application* x11Application;
    if((x11Application = qGuiApp->nativeInterface<QNativeInterface::QX11Application>()))
        conn = x11Application->connection();
#endif

    // First, get the window id
    const MainWindow* const mw = MainWindow::mainWindow;
    if(!mw || !mw->windowHandle() || !conn)
        return displaystr;

    xcb_window_t ckb_xcb_id = static_cast<xcb_window_t>(mw->windowHandle()->winId());

    // Check if the cursor is in the same screen as the ckb-next window
    xcb_query_pointer_reply_t* ptr_reply = xcb_query_pointer_reply(conn, xcb_query_pointer(conn, ckb_xcb_id), nullptr);

    if(!ptr_reply->same_screen) {
        // Get the screen ID
        int screen = 0;
        xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(conn));
        for(int i = 0; iter.rem; i++) {
            if(iter.data->root == ptr_reply->root) {
                screen = i;
                break;
            }
            xcb_screen_next(&iter);
        }

        char* host;
        int parsed_display;
        if(xcb_parse_display(DISPLAY, &host, &parsed_display, nullptr)) {
            displaystr = QString("%1:%2.%3").arg(QString::fromUtf8(host)).arg(parsed_display).arg(screen);
            free(host);
        }
    } else if(DISPLAY) {
        displaystr = QString(DISPLAY);
    }

    free(ptr_reply);
#endif
    return displaystr;
}

QString KeyAction::macroAction(const QString& macroDef) {
    return QString ("$macro:%1").arg(macroDef);
}
