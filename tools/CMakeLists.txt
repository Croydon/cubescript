if(BUILD_REPL_TOOL)
    if(REPL_USE_LINENOISE)
        set(CUBESCRIPT_EXECUTABLE_OPTIONALS "linenoise.cc")
        set(CUBESCRIPT_EXTRA_CXX_FLAGS ${CUBESCRIPT_EXTRA_CXX_FLAGS} "-DCS_REPL_USE_LINENOISE")
    else()
        set(CUBESCRIPT_EXECUTABLE_OPTIONALS "")
    endif()

    add_executable(cubescript_repl
        repl.cc
        ${CUBESCRIPT_EXECUTABLE_OPTIONALS}
    )

    target_include_directories(cubescript_repl BEFORE PUBLIC "../subprojects/libostd/" "../include/")

    target_compile_options(cubescript_repl PUBLIC ${CUBESCRIPT_EXTRA_CXX_FLAGS})
    target_link_libraries(cubescript_repl PRIVATE ${LIBOSTD_LIBRARY} cubescript)

    install(TARGETS cubescript_repl
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )
endif()
