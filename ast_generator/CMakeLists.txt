set(sources
    ast_generator.cpp
    ast_generator.h
)

add_library(ast_generator STATIC ${sources})

target_link_libraries(ast_generator Qt5::Core)
target_include_directories(ast_generator PUBLIC .)
target_compile_definitions(ast_generator PRIVATE "QT_DISABLE_DEPRECATED_BEFORE=0x040900")