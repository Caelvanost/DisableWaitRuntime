#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <chrono>
#include <spdlog/sinks/basic_file_sink.h>

namespace logger = SKSE::log;

namespace DisableWait
{
    constexpr std::string_view kWaitEvent = "Wait";
    constexpr auto kSuppressionWindow = std::chrono::milliseconds(500);

    std::chrono::steady_clock::time_point g_blockSleepWaitUntil{};

    bool ShouldBlockSleepWaitMenu()
    {
        const auto now = std::chrono::steady_clock::now();
        if (now <= g_blockSleepWaitUntil) {
            g_blockSleepWaitUntil = {};
            return true;
        }

        g_blockSleepWaitUntil = {};
        return false;
    }

    class InputWatcher final : public RE::BSTEventSink<RE::InputEvent*>
    {
    public:
        static InputWatcher* GetSingleton()
        {
            static InputWatcher singleton;
            return std::addressof(singleton);
        }

        RE::BSEventNotifyControl ProcessEvent(
            RE::InputEvent* const* eventPtr,
            RE::BSTEventSource<RE::InputEvent*>*) override
        {
            if (!eventPtr) {
                return RE::BSEventNotifyControl::kContinue;
            }

            auto* ui = RE::UI::GetSingleton();
            if (ui && ui->GameIsPaused()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            for (auto* event = *eventPtr; event; event = event->next) {
                if (event->eventType != RE::INPUT_EVENT_TYPE::kButton) {
                    continue;
                }

                auto* button = event->AsButtonEvent();
                if (!button || !button->IsDown() || button->HeldDuration() != 0.0f) {
                    continue;
                }

                const auto userEvent = button->QUserEvent();
                if (userEvent.c_str() && kWaitEvent == userEvent.c_str()) {
                    // Keep the vanilla Wait mapping intact so UI mods such as
                    // RaceMenu can still query and reuse it. We only arm a short
                    // suppression window for the Sleep/Wait menu opened by this
                    // gameplay input.
                    g_blockSleepWaitUntil = std::chrono::steady_clock::now() + kSuppressionWindow;
                    break;
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }
    };

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
            if (!event || !event->opening) {
                return RE::BSEventNotifyControl::kContinue;
            }

            const bool isSleepWait =
                event->menuName.c_str() && RE::SleepWaitMenu::MENU_NAME == event->menuName.c_str();

            if (!isSleepWait || !ShouldBlockSleepWaitMenu()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            // Do not inject a UI message while Skyrim is still dispatching the
            // menu-open event. Queue the hide for the next SKSE task instead.
            const auto menuName = event->menuName;
            if (auto* tasks = SKSE::GetTaskInterface()) {
                tasks->AddTask([menuName]() {
                    if (auto* queue = RE::UIMessageQueue::GetSingleton()) {
                        queue->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
                    }
                });
            } else if (auto* queue = RE::UIMessageQueue::GetSingleton()) {
                queue->AddMessage(menuName, RE::UI_MESSAGE_TYPE::kHide, nullptr);
            }

            logger::info("Suppressed gameplay Wait menu while preserving the Wait key mapping");
            return RE::BSEventNotifyControl::kContinue;
        }
    };

    void RegisterInputWatcher()
    {
        auto* input = RE::BSInputDeviceManager::GetSingleton();
        if (!input) {
            logger::error("BSInputDeviceManager singleton is unavailable");
            return;
        }

        input->AddEventSink(InputWatcher::GetSingleton());
        logger::info("Wait input watcher registered");
    }

    void RegisterMenuWatcher()
    {
        auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            logger::error("UI singleton is unavailable");
            return;
        }

        ui->AddEventSink<RE::MenuOpenCloseEvent>(MenuWatcher::GetSingleton());
        logger::info("Sleep/Wait menu watcher registered");
    }

    void MessageHandler(SKSE::MessagingInterface::Message* message)
    {
        if (!message) {
            return;
        }

        switch (message->type) {
        case SKSE::MessagingInterface::kInputLoaded:
            RegisterInputWatcher();
            break;

        case SKSE::MessagingInterface::kDataLoaded:
            RegisterMenuWatcher();
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
