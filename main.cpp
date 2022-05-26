#include <iostream>
#include <chrono>
#include <memory>
#include <signal.h>
#include <thread>
#include <csignal>

using namespace std::literals::chrono_literals;

#include "discord_sdk/discord.h"

const std::uint64_t D_CLIENT_ID = 978697179394895872;
const discord::LogLevel D_LOG_LEVEL = discord::LogLevel::Info;

namespace {
    volatile bool interrupted = false; // Set to true on ctrl+c
}

struct State {
    discord::User user;
    std::unique_ptr<discord::Core> core;
};

int main() {
    State state {};

    discord::Core *core {};
    discord::Result result = discord::Core::Create(D_CLIENT_ID, DiscordCreateFlags_Default, &core);

    state.core.reset(core);
    if (!state.core) {
        std::cerr << "Failed initialising discord::Core | " << static_cast<int>(result) << "\n";
        std::exit(1);
    }

    core->SetLogHook(D_LOG_LEVEL, [](discord::LogLevel log_level, const char *message) {
        std::cerr << "discord_sdk (" << static_cast<uint32_t>(log_level) << ") # " << message << std::endl;
    });

    core->UserManager().OnCurrentUserUpdate.Connect([&state]() {
        state.core->UserManager().GetCurrentUser(&state.user);
        std::cerr << "Logged in as " << state.user.GetUsername() << "#" << state.user.GetDiscriminator() << std::endl;
    });

    discord::Activity activity {};
    activity.SetState("hello romanian");
    activity.GetAssets().SetLargeImage("7130406-hsc00001-7");
    core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        std::cerr << ((result == discord::Result::Ok) ? "discord_sdk set status successfully\n" : "discord_sdk failed to set status\n");
    });

    std::signal(SIGINT, [](int) { interrupted = true; });

    do {
        state.core->RunCallbacks();
        std::this_thread::sleep_for(20ms);
    } while (!interrupted);
    std::cerr << "Quitting.\n";
}