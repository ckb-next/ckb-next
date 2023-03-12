#ifndef __APPLE__

#ifdef USE_LIBPULSE
#include <pulse/context.h>
#include <pulse/introspect.h>
#include <pulse/subscribe.h>
#include <pulse/mainloop.h>
#include <QDebug>
#include "monotonicclock.h"
#endif
#include "media.h"

#ifdef USE_LIBPULSE
static muteState lastKnownSink = UNKNOWN;
static muteState lastKnownSource = UNKNOWN;

static pa_context* paContext = nullptr; // Context for communicating with Pulse Audio.
static pa_mainloop* mainLoop = nullptr;
static qint64 reconnectTime = 0; // Time (in MSecs since epoch) to attempt reconnect.
static QString defaultSink; // Name of sink being checked if muted.
static QString defaultSource; // Name of source being checked if muted.

static bool CheckPAOperation(pa_operation* operation){
    if(operation == nullptr)
        return false;

    pa_operation_unref(operation);
    return true;
}

static void SinkCallback(pa_context* context, const pa_sink_info* info, int eol, void* data){
    if(info == nullptr || info->name != defaultSink)
        return;

    lastKnownSink = info->mute ? MUTED : UNMUTED;
}

static void SourceCallback(pa_context* context, const pa_source_info* info, int eol, void* data){
    if(info == nullptr || info->name != defaultSource)
        return;

    lastKnownSource = info->mute ? MUTED : UNMUTED;
}

static void ServerCallback(pa_context* context, const pa_server_info* info, void* data){
    // Keep track of the default source and sink. Only these are checked. If
    // the user changes this, SourceCallback or SinkCallback will be called afterwards
    // automatically. This will then check if the new default source and sink are muted.
    defaultSink = info->default_sink_name;
    defaultSource = info->default_source_name;
}

static void SubscribeCallback(pa_context* context, pa_subscription_event_type_t type, uint32_t index, void* data){
    const pa_subscription_event_type_t eventFacility = static_cast<pa_subscription_event_type_t>(type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK);
    if(eventFacility == PA_SUBSCRIPTION_EVENT_SINK){
        // A sink was added or changed in some way. Get new info to see if it
        // was muted.
        if(!CheckPAOperation(pa_context_get_sink_info_by_index(context, index, SinkCallback, nullptr)))
            qWarning("getMuteState(): pa_context_get_sink_info_by_index() error");
    }
    else if (eventFacility == PA_SUBSCRIPTION_EVENT_SOURCE) {
        if (!CheckPAOperation(pa_context_get_source_info_by_index(context, index, SourceCallback, nullptr)))
            qWarning("getMuteState(): pa_context_get_source_info_by_index() error");
    }
    else if(eventFacility == PA_SUBSCRIPTION_EVENT_SERVER){
        // Server settings were modified. Get new info to see if default sink
        // was changed.
        if(!CheckPAOperation(pa_context_get_server_info(context, ServerCallback, nullptr)))
            qWarning("getMuteState(): pa_context_get_server_info() error");
    }
}

static void ContextStateCallback(pa_context* context, void* data){
    pa_context_state_t state = pa_context_get_state(context);
    if(state == PA_CONTEXT_READY){
        pa_context_set_subscribe_callback(context, SubscribeCallback, nullptr);
        if(!CheckPAOperation(pa_context_subscribe(context,
                                                  static_cast<pa_subscription_mask_t>(PA_SUBSCRIPTION_MASK_SINK |
                                                                                      PA_SUBSCRIPTION_MASK_SOURCE |
                                                                                      PA_SUBSCRIPTION_MASK_SERVER),
                                                  nullptr,
                                                  nullptr)))
            qWarning("getMuteState(): pa_context_subscribe() error");

        // Find initial server state.
        if(!CheckPAOperation(pa_context_get_server_info(context, ServerCallback, nullptr)))
            qWarning("getMuteState(): pa_context_get_server_info() error");

        // Find initial sink mute state.
        if(!CheckPAOperation(pa_context_get_sink_info_list(context, SinkCallback, nullptr)))
            qWarning("getMuteState(): pa_context_get_sink_info_list() error");

        // Find initial source mute state.
        if(!CheckPAOperation(pa_context_get_source_info_list(context, SourceCallback, nullptr)))
            qWarning("getMuteState(): pa_context_get_source_info_list() error");

    }
    else if(state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED){
        qDebug() << "Disconnected from PulseAudio" << state;
        // Either we could not connect to the server or the on going connection
        // was dropped.
        pa_context_unref(paContext);
        paContext = nullptr;

        //Try to reconnect again shortly.
        reconnectTime = MonotonicClock::msecs() + 10000;
        lastKnownSink = lastKnownSource = UNKNOWN;
    }
}

bool isMuteDeviceSupported() {
    return true;
}

static inline muteState mute_state(const muteDevice muteDev) {
    return (muteDev == SINK) ? lastKnownSink : lastKnownSource;
}

muteState getMuteState(const muteDevice muteDev){
    // Setup main loop communicating with Pulse Audio. All
    // communication is done asynchronously.
    if(mainLoop == nullptr){
        mainLoop = pa_mainloop_new();
        if(mainLoop == nullptr)
            return UNKNOWN;
    }

    // Connect to the local Pulse Audio server. It's usually running but a
    // reconnect is attempted periodically whenever the connection fails or is
    // terminated.
    if(paContext == nullptr && MonotonicClock::msecs() >= reconnectTime){
        qDebug() << "Creating PulseAudio context";
        pa_mainloop_api* api = pa_mainloop_get_api(mainLoop);
        if(api == nullptr)
            return UNKNOWN;

        paContext = pa_context_new(api, "ckb-next Mute Indicator");
        if(paContext == nullptr)
            return UNKNOWN;
        pa_context_set_state_callback(paContext, &ContextStateCallback, nullptr);
        pa_context_connect(paContext, nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr);
    }

    pa_mainloop_iterate(mainLoop, 0, nullptr);
    return mute_state(muteDev);
}
void deinitAudioSubsystem() {
    if(paContext)
        pa_context_disconnect(paContext); // This will call the callback which will unref the context

    if(mainLoop){
        pa_mainloop_free(mainLoop);
        mainLoop = nullptr;
    }
}
#else
bool isMuteDeviceSupported() {
    return false;
}
muteState getMuteState(const muteDevice muteDev){
    return UNKNOWN;
}
void deinitAudioSubsystem() {}
#endif

#endif
