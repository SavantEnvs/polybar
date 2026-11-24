# Included (deferred) by mayhem/hook.cmake at the end of polybar's top-level CMakeLists.txt.
set(_mf_dir "${CMAKE_CURRENT_LIST_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/mayhem-fuzz")
get_include_dirs(includes_dir)

if(MAYHEM_MODE STREQUAL "fuzz")
  set(MAYHEM_HARNESSES config_parser tags_parser tags_dispatch ipc_decoder actions value_parsers label builder)
  separate_arguments(_fuzz_engine UNIX_COMMAND "${MAYHEM_FUZZ_ENGINE}")
  foreach(h ${MAYHEM_HARNESSES})
    add_executable(fuzz_${h} ${_mf_dir}/${h}.cpp ${MAYHEM_LSAN_OBJ})
    target_include_directories(fuzz_${h} PRIVATE ${includes_dir})
    target_link_libraries(fuzz_${h} poly ${_fuzz_engine})
    add_executable(fuzz_${h}-standalone ${_mf_dir}/${h}.cpp ${MAYHEM_LSAN_OBJ} ${MAYHEM_STANDALONE_OBJ})
    target_include_directories(fuzz_${h}-standalone PRIVATE ${includes_dir})
    target_link_libraries(fuzz_${h}-standalone poly)
  endforeach()
elseif(MAYHEM_MODE STREQUAL "test")
  find_package(GTest REQUIRED)
  set(MAYHEM_UNIT_TESTS
    utils/actions utils/action_router utils/color utils/math utils/scope utils/string utils/units
    components/builder components/command_line components/config_parser
    drawtypes/label drawtypes/ramp drawtypes/iconset drawtypes/layouticonset
    ipc/decoder ipc/encoder ipc/util
    tags/parser tags/dispatch tags/action_context)
  foreach(t ${MAYHEM_UNIT_TESTS})
    string(REPLACE "/" "_" n ${t})
    add_executable(unit_test.${n} ${CMAKE_SOURCE_DIR}/tests/unit_tests/${t}.cpp)
    target_include_directories(unit_test.${n} PRIVATE ${includes_dir} ${CMAKE_SOURCE_DIR}/tests)
    target_link_libraries(unit_test.${n} poly GTest::gmock_main)
  endforeach()
endif()
