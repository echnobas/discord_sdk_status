#include <iostream>
#include <chrono>
#include <memory>
#include <stdint.h>
#include <thread>
#include <csignal>

#include "discord_sdk/discord.h"

using namespace std::literals::chrono_literals;


const std::uint64_t D_CLIENT_ID = 824708283024801834;
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
        std::cerr << "Failed initialising discord::Core | " << static_cast<int>(result) << std::endl;
        std::exit(1);
    }

    state.core->SetLogHook(D_LOG_LEVEL, [](discord::LogLevel log_level, const char *message) {
        std::cerr << "discord_sdk (" << static_cast<uint32_t>(log_level) << ") # " << message << std::endl;
    });

    state.core->UserManager().OnCurrentUserUpdate.Connect([&state]() {
        state.core->UserManager().GetCurrentUser(&state.user);
        std::cerr << "Logged in as " << state.user.GetUsername() << "#" << state.user.GetDiscriminator() << std::endl;
    });

    discord::ActivityParty party {};

    discord::Activity activity {};
    activity.SetState("writing rust");
    activity.GetAssets().SetLargeImage("rust2");
    activity.GetParty().SetId("test");
    activity.GetParty().GetSize().SetCurrentSize(69);
    activity.GetParty().GetSize().SetMaxSize(420);
    activity.GetTimestamps().SetStart(0);
    activity.GetTimestamps().SetEnd(9223372036854775807);


    state.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
        std::cerr << ((result == discord::Result::Ok) ? ("discord_sdk set status successfully") : ("discord_sdk failed to set status")) << std::endl;
    });


    std::signal(SIGINT, [](int) { interrupted = true; });

    do {
        state.core->RunCallbacks();
        std::this_thread::sleep_for(20ms);
    } while (!interrupted);
    std::cerr << "Quitting.\n";
}