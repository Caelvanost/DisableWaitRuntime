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
                    // Preserve Skyrim's Wait mapping so UI mods such as RaceMenu
                    // can still query and reuse the bound key. We only mark the
                    // next Sleep/Wait menu opening caused by this gameplay input
                    // for suppression.
                    g_blockSleepWaitUntil = std::chrono::steady_clock::now() + kSuppressionWindow;
                    logger::debug("Native Wait input detected; arming Sleep/Wait menu suppression");
                    break;
                }
            }

            return RE::BSEventNotifyControl::kContinue;
        }
    };

    struct UIMessageQueueHook
    {
        static void thunk(
            RE::UIMessageQueue* queue,
            const RE::BSFixedString& menuName,
            RE::UI_MESSAGE_TYPE type,
            RE::IUIMessageData* data)
        {
            const bool isSleepWait = menuName == RE::SleepWaitMenu::MENU_NAME;
            const bool isOpening =
                type == RE::UI_MESSAGE_TYPE::kShow ||
                type == RE::UI_MESSAGE_TYPE::kReshow;

            if (isSleepWait && isOpening && ShouldBlockSleepWaitMenu()) {
                logger::info("Blocked native Wait menu opening while preserving the Wait key mapping");
                return;
            }

            func(queue, menuName, type, data);
        }

        static void Install()
        {
            REL::Relocation<std::uintptr_t> target{ RE::Offset::UIMessageQueue::AddMessage };
            func = target.write_branch<5>(thunk);
            logger::info("UIMessageQueue hook installed");
        }

        inline static REL::Relocation<decltype(thunk)> func;
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

    void MessageHandler(SKSE::MessagingInterface::Message* message)
    {
        if (!message) {
            return;
        }

        if (message->type == SKSE::MessagingInterface::kInputLoaded) {
            RegisterInputWatcher();
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

    SKSE::AllocTrampoline(64);
    DisableWait::UIMessageQueueHook::Install();

    auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(DisableWait::MessageHandler)) {
        logger::critical("Failed to register SKSE messaging listener");
        return false;
    }

    return true;
}
