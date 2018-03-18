#ifndef __APPLE__

#include <QDateTime>
#include <QMutex>
#ifdef USE_LIBPULSE
#include <pulse/context.h>
#include <pulse/introspect.h>
#include <pulse/subscribe.h>
#include <pulse/thread-mainloop.h>
#endif
#include "media.h"

static muteState lastKnown = UNKNOWN;

#ifdef USE_LIBPULSE
static pa_context* paContext = nullptr; //Context for communicating with Pulse Audio.
static qint64 reconnectTime = 0; //Time (in MSecs since epoch) to attempt reconnect.
static QString defaultSink; //Name of sink being checked if muted.
static QMutex mutex;
static int failCount = 0;

static bool CheckPAOperation(pa_operation* operation){
    if(operation == nullptr)
        return false;

    pa_operation_unref(operation);
    return true;
}

static void SinkCallback(pa_context* context, const pa_sink_info* info, int eol, void* data){
    QMutexLocker locker(&mutex);

    if(info == nullptr || info->name != defaultSink)
        return;

    lastKnown = info->mute ? MUTED : UNMUTED;
}

static void ServerCallback(pa_context* context, const pa_server_info* info, void* data){
    QMutexLocker locker(&mutex);

    //Keep track of the default sink. This is the only one checked if muted. If
    //the user changes this, SinkCallback will be called afterwards
    //automatically. This will then check if the new default sink is muted.
    defaultSink = info->default_sink_name;
}

static void SubscribeCallback(pa_context* context, pa_subscription_event_type_t type, uint32_t index, void* data){
    const pa_subscription_event_type_t eventFacility = static_cast<pa_subscription_event_type_t>(type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK);
    if(eventFacility == PA_SUBSCRIPTION_EVENT_SINK){
        //A sink was added or changed in some way. Get new info to see if it
        //was muted.
        if(!CheckPAOperation(pa_context_get_sink_info_by_index(context, index, SinkCallback, nullptr)))
            qWarning("getMuteState(): pa_context_get_sink_info_by_index() error");
    }
    else if(eventFacility == PA_SUBSCRIPTION_EVENT_SERVER){
        //Server settings were modified. Get new info to see if default sink
        //was changed.
        if(!CheckPAOperation(pa_context_get_server_info(context, ServerCallback, nullptr)))
            qWarning("getMuteState(): pa_context_get_server_info() error");
    }
}

static void ContextStateCallback(pa_context* context, void* data){
    if(context != paContext)
        return;

    pa_context_state_t state = pa_context_get_state(context);
    if(state == PA_CONTEXT_READY){
        failCount = 0;
        pa_context_set_subscribe_callback(context, SubscribeCallback, nullptr);
        if(!CheckPAOperation(pa_context_subscribe(context,
                                                  static_cast<pa_subscription_mask_t>(PA_SUBSCRIPTION_MASK_SINK |
                                                                                      PA_SUBSCRIPTION_MASK_SERVER),
                                                  nullptr,
                                                  nullptr)))
            qWarning("getMuteState(): pa_context_subscribe() error");

        //Find initial default device.
        if(!CheckPAOperation(pa_context_get_server_info(context, ServerCallback, nullptr)))
            qWarning("getMuteState(): pa_context_get_server_info() error");

        //Find initial mute state.
        if(!CheckPAOperation(pa_context_get_sink_info_list(context, SinkCallback, nullptr)))
            qWarning("getMuteState(): pa_context_get_sink_info_list() error");
    }
    else if(state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED){
        //QMutexLocker locker(&mutex); <-- Deadlocks

        //Either we could not connect to the server or the on going connection
        //was dropped.
        if(paContext != nullptr){
            pa_context_unref(paContext);
            paContext = nullptr;

            //Try to reconnect again shortly.
            reconnectTime = QDateTime::currentMSecsSinceEpoch() + 10000;
        }
        failCount++;
    }
}

#endif

muteState getMuteState(){
#ifdef USE_LIBPULSE
    if(failCount < 10){
        static pa_threaded_mainloop* mainLoop = nullptr;

        QMutexLocker locker(&mutex);

        //Setup main loop thread used for communicating with Pulse Audio. All
        //communication is done asynchronously.
        if(mainLoop == nullptr){
            mainLoop = pa_threaded_mainloop_new();
            if(mainLoop == nullptr)
                return lastKnown;

            if(pa_threaded_mainloop_start(mainLoop) != 0){
                pa_threaded_mainloop_free(mainLoop);
                mainLoop = nullptr;
                return lastKnown;
            }
        }

        //Connect to the local Pulse Audio server. It's usually running but a
        //reconnect is attempted periodically whenever the connection fails or is
        //terminated.
        if(paContext == nullptr && QDateTime::currentMSecsSinceEpoch() >= reconnectTime){
            pa_threaded_mainloop_lock(mainLoop);
            {
                pa_mainloop_api* api = pa_threaded_mainloop_get_api(mainLoop);
                Q_ASSERT(api != nullptr);

                paContext = pa_context_new(api, "QPulse");
                Q_ASSERT(paContext != nullptr);
                pa_context_set_state_callback(paContext, &ContextStateCallback, nullptr);
                pa_context_connect(paContext, nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr);
            }
            pa_threaded_mainloop_unlock(mainLoop);
        }
    }
#endif

    return lastKnown;
}

#endif
