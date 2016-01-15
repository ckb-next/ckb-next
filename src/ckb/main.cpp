#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QSharedMemory>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

QSharedMemory appShare("ckb");

#ifdef Q_OS_MACX
// App Nap is an OSX feature designed to save power by putting background apps to sleep.
// However, it interferes with ckb's animations, so we want it off
extern "C" void disableAppNap();
#endif

// Shared memory sizes
#define SHMEM_SIZE      128 * 1024
#define SHMEM_SIZE_V015 16

// Scan shared memory for an active PID
static bool pidActive(const QStringList& lines){
    foreach(const QString& line, lines){
        if(line.startsWith("PID ")){
            bool ok;
            pid_t pid;
            // Valid PID found?
            if((pid = line.split(" ")[1].toLong(&ok)) > 0 && ok){
                // kill -0 does nothing to the application, but checks if it's running
                return (kill(pid, 0) == 0 || errno != ESRCH);
            }
        }
    }
    // If the PID wasn't listed in the shmem, assume it is running
    return true;
}

// Check if the application is running. Optionally write something to its shared memory.
static bool isRunning(const char* command){
    // Try to create shared memory (if successful, application was not already running)
    if(!appShare.create(SHMEM_SIZE) || !appShare.lock()){
        // Lock existing shared memory
        if(!appShare.attach() || !appShare.lock())
            return true;
        bool running = false;
        if(appShare.size() == SHMEM_SIZE_V015){
            // Old shmem - no PID listed so assume the process is running, and print the command directly to the buffer
            if(command){
                void* data = appShare.data();
                snprintf((char*)data, SHMEM_SIZE_V015, "%s", command);
            }
            running = true;
        } else {
            // New shmem. Scan the contents of the shared memory for a PID
            QStringList lines = QString((const char*)appShare.constData()).split("\n");
            if(pidActive(lines)){
                running = true;
                // Append the command
                if(command){
                    lines.append(QString("Option ") + command);
                    QByteArray newMem = lines.join("\n").left(SHMEM_SIZE).toLatin1();
                    // Copy the NUL byte as well as the string
                    memcpy(appShare.data(), newMem.constData(), newMem.length() + 1);
                }
            }
        }
        if(running){
            appShare.unlock();
            return true;
        }
    }
    // Not already running. Initialize the shared memory with our PID
    snprintf((char*)appShare.data(), appShare.size(), "PID %ld", (long)getpid());
    appShare.unlock();
    return false;
}

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("ckb");

    // Although the daemon runs as root, the GUI needn't and shouldn't be, as it has the potential to corrupt settings data.
    if(getuid() == 0){
        printf("The ckb GUI should not be run as root.\n");
        return 0;
    }

    // Seed the RNG for UsbIds
    qsrand(QDateTime::currentMSecsSinceEpoch());
#ifdef Q_OS_MACX
    disableAppNap();
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 3, 0)
    // Enable HiDPI support
    qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // If launched with --version, print version info and then quit
    if(qApp->arguments().contains("--version")){
        printf("ckb %s\n", CKB_VERSION_STR);
        return 0;
    }

    // Kill existing app when launched with --close
    if(qApp->arguments().contains("--close")){
        if(isRunning("Close"))
            printf("Asking existing instance to close.\n");
        else
            printf("ckb is not running.\n");
        return 0;
    }

    // Launch in background if requested, or if re-launching a previous session
    bool background = qApp->arguments().contains("--background") || qApp->arguments().contains("-session") || qApp->arguments().contains("--session");
    if(isRunning(background ? 0 : "Open")){
        printf("ckb is already running. Exiting.\n");
        return 0;
    }
    MainWindow w;
    if(!background)
        w.show();

    return a.exec();
}
