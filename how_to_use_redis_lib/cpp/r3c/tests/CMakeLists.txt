# Writed by yijian (eyjian@gmail.com)

# r3c_cmd
add_executable(
    r3c_cmd
    r3c_cmd.cpp
)
target_link_libraries(
    r3c_cmd
    libr3c.a
    libhiredis.a
)

# r3c_test
add_executable(
    r3c_test
    r3c_test.cpp
)
target_link_libraries(
    r3c_test
    libr3c.a
    libhiredis.a
)

# r3c_robust
add_executable(
    r3c_robust
    r3c_robust.cpp
)
target_link_libraries(
    r3c_robust
    libr3c.a
    libhiredis.a
)

# r3c_stress
add_executable(
    r3c_stress
    r3c_stress.cpp
)
target_link_libraries(
    r3c_stress
    libr3c.a
    libhiredis.a
)

# r3c_stream
add_executable(
    r3c_stream
    r3c_stream.cpp
)
target_link_libraries(
    r3c_stream
    libr3c.a
    libhiredis.a
)

# redis_command_extension
add_library(
    redis_command_extension
    SHARED
    redis_command_extension.cpp
)
target_link_libraries(
    redis_command_extension
    libr3c.a
    libhiredis.a
)
