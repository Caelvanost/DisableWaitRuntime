#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <spdlog/sinks/basic_file_sink.h>

namespace logger = SKSE::log;

namespace DisableWait
{
    constexpr std::string_view kWaitEvent = "Wait";

    void UnbindWait()
    {
        auto* controlMap = RE::ControlMap::GetSingleton();
        if (!controlMap) {
            logger::error("ControlMap singleton is unavailable");
            return;
        }

        const auto gameplayIndex =
            static_cast<std::size_t>(RE::UserEvents::INPUT_CONTEXT_ID::kGameplay);

        auto* gameplay = controlMap->controlMap[gameplayIndex];
        if (!gameplay) {
            logger::error("Gameplay input context is unavailable");
            return;
        }

        std::size_t changed = 0;

        const auto deviceCount = RE::ControlMap::InputContext::GetNumDeviceMappings();
        for (std::size_t device = 0; device < deviceCount; ++device) {
            auto& mappings = gameplay->deviceMappings[device];

            for (auto& mapping : mappings) {
                if (mapping.eventID.c_str() && kWaitEvent == mapping.eventID.c_str()) {
                    const auto invalidKey = static_cast<std::uint16_t>(RE::ControlMap::kInvalid);
                    if (mapping.inputKey != invalidKey || mapping.modifier != 0) {
                        logger::info(
                            "Unbinding Wait on device {} (old key = 0x{:X})",
                            device,
                            mapping.inputKey);

                        mapping.inputKey = invalidKey;
                        mapping.modifier = 0;
                        ++changed;
                    }
                }
            }
        }

        if (changed > 0) {
            logger::info("Wait unbound from {} gameplay mapping(s)", changed);
        }
    }

    class MenuWatcher final : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
    {
    public:
        static MenuWatcher* GetSingleton()
        {
            static MenuWatcher singleton;
            return std::addressof(singleton);
        }

        RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent* event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
        {
            if (!event || event->opening) {
                return RE::BSEventNotifyControl::kContinue;
            }

            // The controls menu can rebuild ControlMap mappings. Queue the
            // unbind for the next SKSE task so Skyrim has finished applying
            // any remapping before we remove Wait again.
            if (auto* tasks = SKSE::GetTaskInterface()) {
                tasks->AddTask([]() {
                    UnbindWait();
                });
            } else {
                UnbindWait();
            }

            return RE::BSEventNotifyControl::kContinue;
        }
    };

    void RegisterMenuWatcher()
    {
        auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            logger::error("UI singleton is unavailable; runtime remap protection disabled");
            return;
        }

        ui->AddEventSink<RE::MenuOpenCloseEvent>(MenuWatcher::GetSingleton());
        logger::info("Menu remap watcher registered");
    }

    void MessageHandler(SKSE::MessagingInterface::Message* message)
    {
        if (!message) {
            return;
        }

        switch (message->type) {
        case SKSE::MessagingInterface::kDataLoaded:
            RegisterMenuWatcher();
            UnbindWait();
            break;

        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kNewGame:
            UnbindWait();
            break;

        default:
            break;
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
