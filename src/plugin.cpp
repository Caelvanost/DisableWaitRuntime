#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <algorithm>
#include <iterator>
#include <spdlog/sinks/basic_file_sink.h>

namespace logger = SKSE::log;

namespace DisableWait
{
    constexpr std::string_view kWaitEvent = "Wait";
    constexpr std::string_view kConsumedWaitEvent = "DisableWaitRuntime_Consumed";

    class InputFilter final : public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        static InputFilter* GetSingleton()
        {
            static InputFilter singleton;
            return std::addressof(singleton);
        }

        RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent* const* eventPtr,
            RE::BSTEventSource<RE::InputEvent*>*) override
        {
            if (!eventPtr) {
                return RE::BSEventNotifyControl::kContinue;
            }

            // Do not alter menu input. RaceMenu reuses Skyrim's vanilla Wait
            // mapping for actions such as Choose Texture, so the Wait user event
            // must remain available while a pausing UI menu is active.
            if (auto* ui = RE::UI::GetSingleton(); ui && ui->GameIsPaused()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            for (auto* event = *eventPtr; event; event = event->next) {
                if (event->eventType != RE::INPUT_EVENT_TYPE::kButton) {
                    continue;
                }

                auto* button = event->AsButtonEvent();
                if (!button) {
                    continue;
                }

                const auto& userEvent = button->QUserEvent();
                if (userEvent == kWaitEvent) {
                    // Neutralize only this transient gameplay input event. The
                    // ControlMap entry itself is left untouched, so UI mods can
                    // still query the Wait binding and the physical key remains
                    // available to hotkey mods that inspect the key code directly.
                    button->userEvent = kConsumedWaitEvent.data();
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }
    };

    void RegisterInputFilter()
    {
        auto* input = RE::BSInputDeviceManager::GetSingleton();
        if (!input) {
            logger::error("BSInputDeviceManager singleton is unavailable");
            return;
        }

        auto* filter = InputFilter::GetSingleton();
        input->AddEventSink(filter);

        // AddEventSink appends sinks. Skyrim's native PlayerControls/MenuControls
        // are already registered by kInputLoaded, so move our filter to the front
        // while preserving the relative order of every other sink. This ensures
        // the transient Wait event is neutralized before native handlers see it.
        auto* source = static_cast<RE::BSTEventSource<RE::InputEvent*>*>(input);
        {
            RE::BSSpinLockGuard lock(source->lock);
            auto it = std::find(source->sinks.begin(), source->sinks.end(), filter);
            if (it != source->sinks.end() && it != source->sinks.begin()) {
                std::rotate(source->sinks.begin(), it, std::next(it));
            }
        }

        logger::info("Wait input filter registered at highest input-sink priority");
    }

    void MessageHandler(SKSE::MessagingInterface::Message* message)
    {
        if (!message) {
            return;
        }

        if (message->type == SKSE::MessagingInterface::kInputLoaded) {
            RegisterInputFilter();
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse);

    auto path = logger::log_directory();
    if (path) {
        *path /= "DisableWaitRuntime.log";
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
        auto log = std::make_shared<spdlog::logger>(
            "global log",
            std::move(sink)
        );
        log->set_level(spdlog::level::info);
        log->flush_on(spdlog::level::info);
        spdlog::set_default_logger(std::move(log));
    }

    logger::info("Disable Wait Runtime v{} loading", DISABLE_WAIT_RUNTIME_VERSION);

    auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(DisableWait::MessageHandler)) {
        logger::critical("Failed to register SKSE messaging listener");
        return false;
    }

    return true;
}
