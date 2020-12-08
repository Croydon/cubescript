add_library(cubescript 
    ../include/cubescript/cubescript.hh
    ../include/cubescript/cubescript_conf.hh
    cs_print.hh
    cs_gen.cc
    cs_util.cc
    cs_val.cc
    cs_vm.cc
    cubescript.cc
    lib_list.cc
    lib_math.cc
    lib_str.cc
)

target_include_directories(cubescript BEFORE PUBLIC "../subprojects/libostd/" "../include/")

target_compile_options(cubescript PUBLIC ${CUBESCRIPT_EXTRA_CXX_FLAGS})
target_link_libraries(cubescript PRIVATE ${LIBOSTD_LIBRARY})

install(TARGETS cubescript
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    )
