cmake_minimum_required(VERSION 3.25)
project("Telegram echo bot")

include_directories(third_party/json_parser)

link_libraries(curl m)

set(CMAKE_C_FLAGS "-g")
add_executable(bot main.c third_party/json_parser/json.c)
