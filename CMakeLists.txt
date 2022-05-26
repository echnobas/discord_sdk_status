cmake_minimum_required(VERSION 3.2)
project(discord_rpc_client)

add_subdirectory(discord_sdk)

add_executable(discord_rpc_client main.cpp)
target_link_libraries(discord_rpc_client discord_sdk)