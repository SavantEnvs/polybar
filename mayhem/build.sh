#!/usr/bin/env bash
#
# mayhem/build.sh — build polybar's libpoly (sanitized) + the fuzz harnesses in mayhem/fuzz/, and
# upstream's gtest unit suite (normal flags) for mayhem/test.sh.
#
# Upstream's CMake is used unmodified: mayhem/hook.cmake is injected with
# -DCMAKE_PROJECT_polybar_INCLUDE and defers an add_subdirectory(mayhem/fuzz) to the end of the
# top-level CMakeLists.txt, where the `poly` static library exists. Optional modules that need a
# running daemon or extra libraries (alsa, curl, i3, mpd, network, pulseaudio, xkb, xrm, xcursor)
# are off. lib/xpp is a git submodule (mayhem.yml checks out submodules).
set -euo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

cd "$SRC"
[ -f lib/xpp/CMakeLists.txt ] || { echo "lib/xpp submodule is missing (git submodule update --init)" >&2; exit 1; }

common_opts=(
  -DBUILD_DOC=OFF -DBUILD_CONFIG=OFF -DBUILD_SHELL=OFF -DBUILD_TESTS=OFF
  -DBUILD_POLYBAR=OFF -DBUILD_POLYBAR_MSG=ON
  -DENABLE_ALSA=OFF -DENABLE_CURL=OFF -DENABLE_I3=OFF -DENABLE_MPD=OFF -DENABLE_NETWORK=OFF
  -DENABLE_PULSEAUDIO=OFF -DWITH_XKB=OFF -DWITH_XRM=OFF -DWITH_XCURSOR=OFF
  -DCMAKE_PROJECT_polybar_INCLUDE="$SRC/mayhem/hook.cmake"
)

# 1) Sanitized + coverage-instrumented libpoly and the harnesses. Build type "Mayhem" has no
#    per-type flags, so no -DNDEBUG: ipc::decoder's state-machine asserts stay live.
mkdir -p mayhem/bin
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o mayhem/bin/standalone_main.o
# LeakSanitizer off for every fuzz + standalone binary (leaks are not the bug class we fuzz for).
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c mayhem/lsan_off.cc -o mayhem/bin/lsan_off.o
cmake -S . -B mayhem/build-fuzz "${common_opts[@]}" -DCMAKE_BUILD_TYPE=Mayhem \
  -DMAYHEM_MODE=fuzz -DMAYHEM_FUZZ_ENGINE="$LIB_FUZZING_ENGINE" \
  -DMAYHEM_STANDALONE_OBJ="$SRC/mayhem/bin/standalone_main.o" -DMAYHEM_LSAN_OBJ="$SRC/mayhem/bin/lsan_off.o" \
  -DCMAKE_CXX_FLAGS="$SANITIZER_FLAGS -fsanitize=fuzzer-no-link $DEBUG_FLAGS" \
  -DCMAKE_EXE_LINKER_FLAGS="$SANITIZER_FLAGS"
cmake --build mayhem/build-fuzz -j"$MAYHEM_JOBS"
for h in config_parser tags_parser tags_dispatch ipc_decoder actions value_parsers label builder; do
  cp -f "mayhem/build-fuzz/mayhem-fuzz/fuzz_$h" "mayhem/build-fuzz/mayhem-fuzz/fuzz_$h-standalone" mayhem/bin/
done

# 2) Upstream's unit tests (tests/unit_tests) with the project's normal flags, linked against apt's
#    googletest instead of the configure-time download.
cmake -S . -B mayhem/build-test "${common_opts[@]}" -DCMAKE_BUILD_TYPE=Release -DMAYHEM_MODE=test \
  -DCMAKE_CXX_FLAGS="$COVERAGE_FLAGS" -DCMAKE_EXE_LINKER_FLAGS="$COVERAGE_FLAGS"
cmake --build mayhem/build-test -j"$MAYHEM_JOBS"
