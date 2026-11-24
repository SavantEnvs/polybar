# Injected via -DCMAKE_PROJECT_polybar_INCLUDE=<this file> by mayhem/build.sh.
# Defers including mayhem/fuzz/targets.cmake until upstream's top-level CMakeLists.txt has defined
# the `poly` library, so the harnesses and the unit tests link it without editing any upstream file.
# (DEFER evaluates its arguments when the call runs, so the path is expanded here via EVAL.)
cmake_language(EVAL CODE "cmake_language(DEFER CALL include [[${CMAKE_CURRENT_LIST_DIR}/fuzz/targets.cmake]])")
