cmake_minimum_required(VERSION 3.15)

get_filename_component(BLEND_SOURCE_DIR_REALPATH "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(BLEND_BINARY_DIR_REALPATH "${CMAKE_BINARY_DIR}" REALPATH)
if(BLEND_SOURCE_DIR_REALPATH STREQUAL BLEND_BINARY_DIR_REALPATH)
  message(FATAL_ERROR "In-source builds are not allowed. Create a separate build directory and run CMake from there.")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigUser.cmake")
  include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigUser.cmake")
endif()
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigDefault.cmake")
set(BLEND_USER_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

project(
  ${BLEND_PACKAGE_NAME}
  VERSION ${BLEND_PACKAGE_VERSION}
  DESCRIPTION ${BLEND_PACKAGE_DESCRIPTION_SUMMARY}
  LANGUAGES C
)
set(CMAKE_INSTALL_PREFIX "${BLEND_USER_INSTALL_PREFIX}" CACHE PATH "Install path prefix" FORCE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(CMakePackageConfigHelpers)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(BlendInstallDirs)

option(BLEND_BUILD_TESTS "Build BLEND test executables" ${BLEND_BUILD_TESTS})
option(BLEND_BUILD_SHARED "Build the BLEND shared library" ${BLEND_BUILD_SHARED})
option(BLEND_BUILD_STATIC "Build the BLEND static library" ${BLEND_BUILD_STATIC})
option(BLEND_BUILD_EXAMPLES "Build BLEND example executables" ${BLEND_BUILD_EXAMPLES})
option(BLEND_BUILD_DOCS "Build BLEND documentation targets" ${BLEND_BUILD_DOCS})

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src")
configure_file(
  src/blend_version.h.in
  "${CMAKE_CURRENT_BINARY_DIR}/src/blend_version.h"
  @ONLY
)

set(BLEND_EXPORT_TARGETS)
set(
  BLEND_PUBLIC_HEADERS
  src/blend.h
  src/blend_api.h
  src/blend_boundary.h
  src/blend_contribution.h
  src/blend_error.h
  src/blend_polygon.h
  src/blend_report.h
  src/blend_utils.h
  src/blend_window.h
  "${CMAKE_CURRENT_BINARY_DIR}/src/blend_version.h"
)
set(
  BLEND_SOURCES
  src/blend_api.c
  src/blend_boundary.c
  src/blend_contribution.c
  src/blend_error.c
  src/blend_polygon.c
  src/blend_report.c
  src/blend_utils.c
  src/blend_window.c
)
set(
  BLEND_MODULE_SOURCES
  src/blend_docs.c
  src/blend_module.c
  src/blend_monotone.c
  src/blend_window1d.c
  src/blend_window2d.c
  src/blend_window3d.c
)

if(BLEND_BUILD_SHARED)
  add_library(blend_shared SHARED ${BLEND_SOURCES})
  add_library(BLEND::blend ALIAS blend_shared)
  set_target_properties(
    blend_shared
    PROPERTIES
      OUTPUT_NAME blend
      EXPORT_NAME blend
      PUBLIC_HEADER "${BLEND_PUBLIC_HEADERS}"
      VERSION ${BLEND_LIB_VERSION}
      SOVERSION ${BLEND_LIB_SOVERSION}
  )
  target_include_directories(
    blend_shared
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
      "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )
  target_link_libraries(blend_shared PUBLIC m)
  list(APPEND BLEND_EXPORT_TARGETS blend_shared)
endif()

if(BLEND_BUILD_STATIC)
  add_library(blend_static STATIC ${BLEND_SOURCES})
  if(NOT TARGET BLEND::blend)
    add_library(BLEND::blend ALIAS blend_static)
  endif()
  add_library(BLEND::blend_static ALIAS blend_static)
  set_target_properties(
    blend_static
    PROPERTIES
      OUTPUT_NAME blend
      EXPORT_NAME blend_static
      PUBLIC_HEADER "${BLEND_PUBLIC_HEADERS}"
  )
  target_include_directories(
    blend_static
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
      "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
  )
  target_link_libraries(blend_static PUBLIC m)
  list(APPEND BLEND_EXPORT_TARGETS blend_static)
endif()

if(NOT BLEND_EXPORT_TARGETS)
  message(FATAL_ERROR "At least one of BLEND_BUILD_SHARED or BLEND_BUILD_STATIC must be TRUE.")
endif()

add_executable(blend src/blend.c ${BLEND_MODULE_SOURCES})
target_include_directories(
  blend
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
if(TARGET blend_static)
  target_link_libraries(blend PRIVATE blend_static)
else()
  target_link_libraries(blend PRIVATE blend_shared)
endif()

if(BLEND_BUILD_TESTS)
  enable_testing()

  add_executable(blend_api_test tests/blend_api_test.c)
  if(TARGET blend_static)
    target_link_libraries(blend_api_test PRIVATE blend_static)
  else()
    target_link_libraries(blend_api_test PRIVATE blend_shared)
  endif()
  add_test(NAME blend_api_test COMMAND $<TARGET_FILE:blend_api_test>)
  add_executable(blend_header_test tests/blend_header_test.c)
  target_include_directories(
    blend_header_test
    PRIVATE
      "${CMAKE_CURRENT_BINARY_DIR}/src"
      "${CMAKE_CURRENT_SOURCE_DIR}/src"
  )
  add_test(NAME blend_header_test COMMAND $<TARGET_FILE:blend_header_test>)
  add_test(NAME blend_cli_usage COMMAND blend)
  add_test(NAME blend_cli_help COMMAND blend --help)
  add_test(NAME blend_cli_show_modules COMMAND blend --show-modules)
  add_test(NAME blend_cli_show_windows COMMAND blend --show-windows)
  set_tests_properties(
    blend_cli_show_windows
    PROPERTIES
      PASS_REGULAR_EXPRESSION "boxcar[\r\n]+cosine[\r\n]+hann[\r\n]+tukey[\r\n]+trapezoid[\r\n]+linear[\r\n]+hamming[\r\n]+blackman[\r\n]+blackmanharris[\r\n]+welch[\r\n]+parzen[\r\n]+gaussian[\r\n]+smoothstep[\r\n]+smootherstep[\r\n]+exponential[\r\n]+sine[\r\n]+bohman[\r\n]+nuttall[\r\n]+kaiser[\r\n]+cauchy[\r\n]+quadratic[\r\n]+cubic[\r\n]+poisson[\r\n]+bartlett[\r\n]+barthann[\r\n]+bartletthann[\r\n]+exactblackman[\r\n]+blackmannuttall[\r\n]+flattop[\r\n]+lanczos[\r\n]+riesz[\r\n]+riemann[\r\n]+fejer[\r\n]+connes[\r\n]+hanningpoisson[\r\n]+kaiserbessel[\r\n]+plancktaper[\r\n]+quartic[\r\n]+quintic[\r\n]+septic[\r\n]+nonic[\r\n]+logistic[\r\n]+tanh[\r\n]+erf[\r\n]+arctan[\r\n]+gompertz[\r\n]+softsign[\r\n]+agnesi[\r\n]+inversequadratic[\r\n]+inversemultiquadric[\r\n]+powerlaw[\r\n]+root[\r\n]+circular[\r\n]+sech[\r\n]+sech2[\r\n]+student[\r\n]+laplace"
  )
  add_test(NAME blend_cli_module_exists COMMAND blend docs =)
  add_test(
    NAME blend_docs_usage_no_error
    COMMAND bash -c "tmp=$(mktemp -d); $<TARGET_FILE:blend> docs > $tmp/out 2> $tmp/err; status=$?; grep -q '^blend docs - Show HTML documentation for a BLEND module' $tmp/out && grep -q '^usage: blend docs' $tmp/out && grep -q '^REQUIRED ARGUMENTS:' $tmp/out && grep -q '^OPTIONAL ARGUMENTS:' $tmp/out && ! grep -q '^BLEND modules:' $tmp/out && test ! -s $tmp/err; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_docs_module_lookup
    COMMAND bash -c "$<TARGET_FILE:blend> docs -Q window2d"
  )
  set_tests_properties(
    blend_docs_module_lookup
    PROPERTIES
      PASS_REGULAR_EXPRESSION "(file://.*/doc/html/modules/window2d.html|https://cascadiaquakes.github.io/blend/modules/window2d.html)"
  )
  add_test(
    NAME blend_docs_option_lookup
    COMMAND bash -c "$<TARGET_FILE:blend> docs -Q window1d -R"
  )
  set_tests_properties(
    blend_docs_option_lookup
    PROPERTIES
      PASS_REGULAR_EXPRESSION "(file://.*/doc/html/modules/window1d.html#r|https://cascadiaquakes.github.io/blend/modules/window1d.html#r)"
  )
  add_test(
    NAME blend_docs_server_lookup
    COMMAND blend docs -Q -S windows
  )
  set_tests_properties(
    blend_docs_server_lookup
    PROPERTIES
      PASS_REGULAR_EXPRESSION "https://cascadiaquakes.github.io/blend/reference/windows.html"
  )
  add_test(
    NAME blend_docs_query_must_be_first
    COMMAND blend docs window1d -Q
  )
  set_tests_properties(
    blend_docs_query_must_be_first
    PROPERTIES
      WILL_FAIL TRUE
  )
  add_test(
    NAME blend_monotone_usage_no_error
    COMMAND bash -c "tmp=$(mktemp -d); $<TARGET_FILE:blend> monotone --help > $tmp/out 2> $tmp/err; status=$?; grep -q '^blend monotone - Check or enforce xy-monotone polygon boundaries' $tmp/out && grep -q '^usage: blend monotone' $tmp/out && test ! -s $tmp/err; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_monotone_file_check
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n2 0\\n2 2\\n0 2\\n' > $tmp/poly.txt; $<TARGET_FILE:blend> monotone $tmp/poly.txt; status=$?; rm -rf $tmp; exit $status"
  )
  set_tests_properties(
    blend_monotone_file_check
    PROPERTIES
      PASS_REGULAR_EXPRESSION "monotone: polygon is xy-monotone"
  )
  add_test(
    NAME blend_monotone_stdin_check
    COMMAND bash -c "printf '0 0\\n3 0\\n3 3\\n2 3\\n2 1\\n1 1\\n1 3\\n0 3\\n' | $<TARGET_FILE:blend> monotone"
  )
  set_tests_properties(
    blend_monotone_stdin_check
    PROPERTIES
      PASS_REGULAR_EXPRESSION "monotone: polygon is not xy-monotone"
  )
  add_test(
    NAME blend_monotone_envelope
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n3 0\\n3 3\\n2 3\\n2 1\\n1 1\\n1 3\\n0 3\\n' > $tmp/poly.txt; $<TARGET_FILE:blend> monotone -Me $tmp/poly.txt > $tmp/out 2> $tmp/err; status=$?; grep -q 'using envelope method' $tmp/err && grep -q 'original number of vertices = 8, final number of vertices = 4' $tmp/err && awk 'END {if (NR != 4) exit 1} ($1==0 && $2==0){a=1} ($1==3 && $2==0){b=1} ($1==3 && $2==3){c=1} ($1==0 && $2==3){d=1} END {exit !(a && b && c && d)}' $tmp/out; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_monotone_best_piecewise_envelope
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n4 0\\n4.5 0.5\\n5 1\\n5 4\\n4.7 4.4\\n4 5\\n1 5\\n0.7 4.7\\n0.3 4.4\\n-1 4\\n-1 1\\n-0.2 0.8\\n-0.8 0.4\\n' > $tmp/poly.txt; awk '{line[NR]=$0} END{for(i=NR;i>0;i--) print line[i]}' $tmp/poly.txt > $tmp/rev.txt; $<TARGET_FILE:blend> monotone -Mb -G256/256 $tmp/poly.txt > $tmp/out 2> $tmp/err; status=$?; $<TARGET_FILE:blend> monotone -Mb -G256/256 $tmp/rev.txt > $tmp/rev_out 2> $tmp/rev_err; rev_status=$?; grep -q 'using best IoU piecewise envelope method' $tmp/err && grep -q 'original number of vertices = 14, final number of vertices' $tmp/err && grep -q 'using best IoU piecewise envelope method' $tmp/rev_err && grep -q 'original number of vertices = 14, final number of vertices' $tmp/rev_err && $<TARGET_FILE:blend> monotone $tmp/out | grep -q 'polygon is xy-monotone' && $<TARGET_FILE:blend> monotone $tmp/rev_out | grep -q 'polygon is xy-monotone'; result=$?; rm -rf $tmp; test $status -eq 0 && test $rev_status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_monotone_timing
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n3 0\\n3 3\\n2 3\\n2 1\\n1 1\\n1 3\\n0 3\\n' > $tmp/poly.txt; $<TARGET_FILE:blend> monotone -Vt -Mb -G16/16 $tmp/poly.txt > $tmp/out 2> $tmp/err; status=$?; grep -q 'blend \\[timing\\]: monotone: read 8 polygon vertices' $tmp/err && grep -q 'blend \\[timing\\]: polygon: evaluating best IoU xy-monotone envelope candidates on 256 grid samples' $tmp/err && grep -q 'blend \\[timing\\]: polygon: selected best IoU xy-monotone envelope' $tmp/err && grep -q 'blend \\[timing\\]: monotone: converted polygon with -Mb' $tmp/err; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_monotone_envelope_already_monotone
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n2 0\\n2 2\\n0 2\\n' > $tmp/poly.txt; $<TARGET_FILE:blend> monotone -Me $tmp/poly.txt > $tmp/out 2> $tmp/err; status=$?; grep -q '^monotone: polygon is already xy-monotone$' $tmp/out && test ! -s $tmp/err && test $(wc -l < $tmp/out) -eq 1; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_monotone_reject_self_intersection
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n4 0\\n1 3\\n3 -1\\n4 4\\n0 4\\n' > $tmp/poly.txt; $<TARGET_FILE:blend> monotone $tmp/poly.txt > $tmp/out 2> $tmp/err; status=$?; grep -q 'polygon edges must not self-intersect' $tmp/err; result=$?; rm -rf $tmp; test $status -ne 0 && exit $result; exit 1"
  )
  add_test(
    NAME blend_example_ex01_monotone
    COMMAND bash -c "tmp=$(mktemp -d); cp ${CMAKE_CURRENT_SOURCE_DIR}/doc/examples/ex01_monotone/south_america.txt ${CMAKE_CURRENT_SOURCE_DIR}/doc/examples/ex01_monotone/ex01_monotone.sh $tmp; cd $tmp; BLEND=$<TARGET_FILE:blend> sh ./ex01_monotone.sh > run.out 2> run.err; status=$?; grep -q 'monotone: polygon is not xy-monotone' ex01_status.txt && grep -q 'using envelope method' run.err && grep -q 'using best IoU piecewise envelope method' run.err && test -s south_america_envelope.txt && test -s south_america_monotone.txt && test -s south_america_monotone_plot.txt; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window1d_usage_no_error
    COMMAND bash -c "tmp=$(mktemp -d); $<TARGET_FILE:blend> window1d > $tmp/out 2> $tmp/err; status=$?; grep -q '^blend window1d - Generate 1-D blending weights' $tmp/out && grep -q '^usage: blend window1d' $tmp/out && grep -q '^REQUIRED ARGUMENTS:' $tmp/out && grep -q '^OPTIONAL ARGUMENTS:' $tmp/out && test ! -s $tmp/err; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window1d_timing_grid
    COMMAND bash -c "tmp=$(mktemp -d); $<TARGET_FILE:blend> window1d -Vt -R0/2 -I1 -Fboxcar -T0/0 > $tmp/out 2> $tmp/err; status=$?; grep -q 'blend \\[timing\\]: window1d: writing 3 grid nodes' $tmp/err && grep -q 'blend \\[timing\\]: window1d: wrote 3 grid nodes' $tmp/err; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(NAME blend_window1d_cosine COMMAND blend window1d -R0/4 -I1 --taper_ratio=0.25 --function=cosine)
  set_tests_properties(
    blend_window1d_cosine
    PROPERTIES
      PASS_REGULAR_EXPRESSION "0 0.345491502813[\r\n]+1 1.000000000000"
  )
  add_test(NAME blend_window1d_asymmetric COMMAND blend window1d -R0/4 -I1 --taper_ratio=0.25/0.0 --function=cosine)
  set_tests_properties(
    blend_window1d_asymmetric
    PROPERTIES
      PASS_REGULAR_EXPRESSION "0 0.345491502813[\r\n]+1 1.000000000000[\r\n]+2 1.000000000000[\r\n]+3 1.000000000000[\r\n]+4 1.000000000000"
  )
  add_test(
    NAME blend_window1d_window_unit_interval
    COMMAND bash -c "$<TARGET_FILE:blend> window1d -R40/60 -I0.5 -Ftrapezoid -T0.2/0.2 | awk '$2 < -1.0e-12 || $2 > 1.0 + 1.0e-12 {exit 1}'"
  )
  add_test(NAME blend_window1d_arbitrary_region COMMAND blend window1d -R10/14 -I1 --taper_ratio=0.25/0.0 --function=cosine)
  set_tests_properties(
    blend_window1d_arbitrary_region
    PROPERTIES
      PASS_REGULAR_EXPRESSION "10 0.345491502813[\r\n]+11 1.000000000000[\r\n]+12 1.000000000000[\r\n]+13 1.000000000000[\r\n]+14 1.000000000000"
  )
  add_test(
    NAME blend_window1d_increment_count
    COMMAND bash -c "$<TARGET_FILE:blend> window1d -R10/14 -I0.1 --taper_ratio=0/0 | awk 'END {printf \"%d %.12g\\n\", NR, $1}'"
  )
  set_tests_properties(
    blend_window1d_increment_count
    PROPERTIES
      PASS_REGULAR_EXPRESSION "41 14"
  )
  add_test(
    NAME blend_window1d_adjust_region
    COMMAND bash -c "tmp=$(mktemp -d); $<TARGET_FILE:blend> window1d -R10/14.05 -I0.1 --taper_ratio=0/0 > $tmp/out 2> $tmp/err; status=$?; awk 'END {printf \"%d %.12g\\n\", NR, $1}' $tmp/out; grep 'adjusted region end' $tmp/err; rm -rf $tmp; exit $status"
  )
  set_tests_properties(
    blend_window1d_adjust_region
    PROPERTIES
      PASS_REGULAR_EXPRESSION "42 14.1[\r\n]+blend \\[warning\\]: window1d: adjusted region end from 14.05 to 14.1 to match increment 0.1"
  )
  add_test(
    NAME blend_window1d_query
    COMMAND bash -c "printf '0.5\\n2\\n5\\n' | $<TARGET_FILE:blend> window1d --region=0/4 --increment=1 --taper_ratio=0.25"
  )
  set_tests_properties(
    blend_window1d_query
    PROPERTIES
      PASS_REGULAR_EXPRESSION "0.5 0.672745751406[\r\n]+2 1.000000000000[\r\n]+5 0.000000000000"
  )
  add_test(
    NAME blend_window1d_blendfile_product
    COMMAND bash -c "tmp=$(mktemp -d); printf '1 4 boxcar 0/0\\n2 5 cosine 0.25/0.25\\n' > $tmp/blend.txt; $<TARGET_FILE:blend> window1d -R0/6 -I1 -B$tmp/blend.txt; status=$?; rm -rf $tmp; exit $status"
  )
  set_tests_properties(
    blend_window1d_blendfile_product
    PROPERTIES
      PASS_REGULAR_EXPRESSION "0 0.000000000000[\r\n]+1 1.000000000000[\r\n]+2 0.500000000000[\r\n]+3 1.000000000000[\r\n]+4 1.000000000000[\r\n]+5 0.500000000000[\r\n]+6 0.000000000000"
  )
  add_test(
    NAME blend_window1d_blendfile_average
    COMMAND bash -c "tmp=$(mktemp -d); printf '1 4 boxcar 0/0\\n2 5 cosine 0.25/0.25\\n' > $tmp/blend.txt; $<TARGET_FILE:blend> window1d -R0/6 -I1 -Ca -B$tmp/blend.txt; status=$?; rm -rf $tmp; exit $status"
  )
  set_tests_properties(
    blend_window1d_blendfile_average
    PROPERTIES
      PASS_REGULAR_EXPRESSION "0 0.000000000000[\r\n]+1 1.000000000000[\r\n]+2 0.750000000000[\r\n]+3 1.000000000000[\r\n]+4 1.000000000000[\r\n]+5 0.500000000000[\r\n]+6 0.000000000000"
  )
  add_test(
    NAME blend_window1d_blendfile_ignores_function_taper
    COMMAND bash -c "tmp=$(mktemp -d); printf '1 4 boxcar 0/0\\n' > $tmp/blend.txt; $<TARGET_FILE:blend> window1d -R0/5 -I1 -Ftrapezoid -T0.4 -B$tmp/blend.txt > $tmp/out 2> $tmp/err; status=$?; grep -q -- '-F/--function is ignored' $tmp/err && grep -q -- '-T/--taper_ratio is ignored' $tmp/err && grep -q '1 1.000000000000' $tmp/out; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window1d_boxcar_ignores_taper
    COMMAND bash -c "tmp=$(mktemp -d); $<TARGET_FILE:blend> window1d -R0/4 -I1 -Fboxcar -T0.2/0.2 > $tmp/out 2> $tmp/err; status=$?; grep -q -- '-T/--taper_ratio is ignored for boxcar windows' $tmp/err && awk '$2 != 1.0 {exit 1}' $tmp/out; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window1d_blendfile_boxcar_ignores_taper
    COMMAND bash -c "tmp=$(mktemp -d); printf '1 4 boxcar 0.2/0.2\\n' > $tmp/blend.txt; $<TARGET_FILE:blend> window1d -R0/5 -I1 -B$tmp/blend.txt > $tmp/out 2> $tmp/err; status=$?; grep -q -- 'taper ratios on blendfile line 1 are ignored for boxcar windows' $tmp/err && grep -q '2 1.000000000000' $tmp/out; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(NAME blend_window1d_reject_output_option COMMAND blend window1d -R0/4 -I1 -Gweights.txt)
  set_tests_properties(blend_window1d_reject_output_option PROPERTIES WILL_FAIL TRUE)
  add_test(NAME blend_window1d_bad_ratio COMMAND blend window1d -R0/4 -I1 --taper_ratio=0.5)
  set_tests_properties(blend_window1d_bad_ratio PROPERTIES WILL_FAIL TRUE)

  add_test(
    NAME blend_window2d_usage_no_error
    COMMAND bash -c "tmp=$(mktemp -d); $<TARGET_FILE:blend> window2d > $tmp/out 2> $tmp/err; status=$?; grep -q '^blend window2d - Generate 2-D blending weights' $tmp/out && grep -q '^usage: blend window2d' $tmp/out && grep -q '^REQUIRED ARGUMENTS:' $tmp/out && grep -q '^OPTIONAL ARGUMENTS:' $tmp/out && test ! -s $tmp/err; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window2d_timing_query
    COMMAND bash -c "tmp=$(mktemp -d); printf '1 1\\n' | $<TARGET_FILE:blend> window2d -Vt -R0/2/0/2 -I1 -Fboxcar/boxcar > $tmp/out 2> $tmp/err; status=$?; grep -q 'blend \\[timing\\]: window2d: evaluated 1 query points' $tmp/err; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window2d_boxcar_grid
    COMMAND bash -c "$<TARGET_FILE:blend> window2d -R0/2/0/2 -I1 -Fboxcar/boxcar -T0/0/0/0 2> /dev/null | awk 'NR != 9 {next} {print} END {if (NR != 9) exit 1}'"
  )
  set_tests_properties(
    blend_window2d_boxcar_grid
    PROPERTIES
      PASS_REGULAR_EXPRESSION "2 2 1.000000000000"
  )
  add_test(
    NAME blend_window2d_query
    COMMAND bash -c "printf '1.5 1.5\\n3 3\\n' | $<TARGET_FILE:blend> window2d -R0/2/0/2 -I1 -Fboxcar/boxcar"
  )
  set_tests_properties(
    blend_window2d_query
    PROPERTIES
      PASS_REGULAR_EXPRESSION "1.5 1.5 1.000000000000[\r\n]+3 3 0.000000000000"
  )
  add_test(
    NAME blend_window2d_blendfile_polygon
    COMMAND bash -c "tmp=$(mktemp -d); printf '1 1\\n3 1\\n3 3\\n1 3\\n' > $tmp/poly.txt; printf '%s boxcar/boxcar 0/0/0/0\\n' $tmp/poly.txt > $tmp/blend.txt; $<TARGET_FILE:blend> window2d -R0/4/0/4 -I1 -B$tmp/blend.txt > $tmp/out; status=$?; awk '($1==0 && $2==0)||($1==1 && $2==1)||($1==2 && $2==2)||($1==4 && $2==4){print}' $tmp/out; rm -rf $tmp; exit $status"
  )
  set_tests_properties(
    blend_window2d_blendfile_polygon
    PROPERTIES
      PASS_REGULAR_EXPRESSION "0 0 0.000000000000[\r\n]+1 1 1.000000000000[\r\n]+2 2 1.000000000000[\r\n]+4 4 0.000000000000"
  )
  add_test(
    NAME blend_window2d_blendfile_query
    COMMAND bash -c "tmp=$(mktemp -d); printf '1 1\\n3 1\\n3 3\\n1 3\\n' > $tmp/poly.txt; printf '%s boxcar/boxcar 0/0/0/0\\n' $tmp/poly.txt > $tmp/blend.txt; printf '2.5 2.5\\n0.5 0.5\\n' | $<TARGET_FILE:blend> window2d -R0/4/0/4 -I1 -B$tmp/blend.txt; status=$?; rm -rf $tmp; exit $status"
  )
  set_tests_properties(
    blend_window2d_blendfile_query
    PROPERTIES
      PASS_REGULAR_EXPRESSION "2.5 2.5 1.000000000000[\r\n]+0.5 0.5 0.000000000000"
  )
  add_test(
    NAME blend_window2d_blendfile_monotone_envelope
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n3 0\\n3 3\\n2 3\\n2 1\\n1 1\\n1 3\\n0 3\\n' > $tmp/poly.txt; printf '%s boxcar/boxcar 0/0/0/0\\n' $tmp/poly.txt > $tmp/blend.txt; printf '1.5 2.5\\n' | $<TARGET_FILE:blend> window2d -R0/3/0/3 -I1 -ME -B$tmp/blend.txt > $tmp/out 2> $tmp/err; status=$?; grep -q 'using its strict xy-monotone envelope from -ME' $tmp/err && grep -q 'original number of vertices = 8, final number of vertices = 4' $tmp/err && grep -q 'modified polygon for .* will not be written; use -N to write it' $tmp/err && grep -q '1.5 2.5 1.000000000000' $tmp/out; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window2d_blendfile_write_monotone_polygon
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n3 0\\n3 3\\n2 3\\n2 1\\n1 1\\n1 3\\n0 3\\n' > $tmp/poly.txt; printf '%s boxcar/boxcar 0/0/0/0\\n' $tmp/poly.txt > $tmp/blend.txt; printf '1.5 2.5\\n' | $<TARGET_FILE:blend> window2d -R0/3/0/3 -I1 -ME -N -B$tmp/blend.txt > $tmp/out 2> $tmp/err; status=$?; test -s $tmp/poly_monotone.txt && grep -q 'wrote modified xy-monotone polygon to' $tmp/err && $<TARGET_FILE:blend> monotone -ME $tmp/poly_monotone.txt | grep -q 'polygon is already strictly xy-monotone'; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window2d_blendfile_best_piecewise_envelope
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n4 0\\n4.5 0.5\\n5 1\\n5 4\\n4.7 4.4\\n4 5\\n1 5\\n0.7 4.7\\n0.3 4.4\\n-1 4\\n-1 1\\n-0.2 0.8\\n-0.8 0.4\\n' > $tmp/poly.txt; printf '%s boxcar/boxcar 0/0/0/0\\n' $tmp/poly.txt > $tmp/blend.txt; printf '4.5 0.5\\n' | $<TARGET_FILE:blend> window2d -R-1/5/0/5 -I1 -MB -B$tmp/blend.txt > $tmp/out 2> $tmp/err; status=$?; grep -q 'using its strict best IoU piecewise xy-monotone envelope from -MB' $tmp/err && grep -q 'original number of vertices = 14, final number of vertices' $tmp/err && grep -q '4.5 0.5 0.750000000000' $tmp/out; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window2d_south_america_boundary
    COMMAND bash -c "tmp=$(mktemp -d); cp ${CMAKE_CURRENT_SOURCE_DIR}/doc/examples/ex03_window2d/south_america.txt $tmp; printf 'south_america.txt cosine/cosine 0.49/0.49/0.49/0.49\\n' > $tmp/blend.txt; cd $tmp; $<TARGET_FILE:blend> window2d -R-85/-30/-60/15 -I0.5 -Bblend.txt -Cp -ME -N > out 2> err; status=$?; test -s out && test -s south_america_monotone.txt && grep -q 'grid-snapped polygon south_america.txt' err && grep -q 'using its strict xy-monotone envelope from -ME' err && ! grep -q 'falling back to -ME' err && ! grep -q 'blend \\[error\\]' err && ! grep -q 'could not assemble boundary' err && ! grep -q 'irregular vertices' err; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window2d_blendfile_reject_nonmonotone
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n3 0\\n3 3\\n2 3\\n2 1\\n1 1\\n1 3\\n0 3\\n' > $tmp/poly.txt; printf '%s boxcar/boxcar 0/0/0/0\\n' $tmp/poly.txt > $tmp/blend.txt; printf '1.5 2.5\\n' | $<TARGET_FILE:blend> window2d -R0/3/0/3 -I1 -B$tmp/blend.txt > $tmp/out 2> $tmp/err; status=$?; grep -q 'use -ME or -MB' $tmp/err; result=$?; rm -rf $tmp; test $status -ne 0 && exit $result; exit 1"
  )
  add_test(
    NAME blend_window2d_blendfile_ignores_function_taper
    COMMAND bash -c "tmp=$(mktemp -d); printf '1 1\\n3 1\\n3 3\\n1 3\\n' > $tmp/poly.txt; printf '%s boxcar/boxcar 0/0/0/0\\n' $tmp/poly.txt > $tmp/blend.txt; $<TARGET_FILE:blend> window2d -R0/4/0/4 -I1 -Ftrapezoid/cosine -T0.2/0.2/0.2/0.2 -B$tmp/blend.txt > $tmp/out 2> $tmp/err; status=$?; grep -q -- '-F/--function is ignored' $tmp/err && grep -q -- '-T/--taper_ratio is ignored' $tmp/err && grep -q '2 2 1.000000000000' $tmp/out; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window2d_boxcar_ignores_taper
    COMMAND bash -c "tmp=$(mktemp -d); $<TARGET_FILE:blend> window2d -R0/2/0/2 -I1 -Fboxcar/boxcar -T0.2/0.2/0.2/0.2 > $tmp/out 2> $tmp/err; status=$?; grep -q -- 'taper ratios are ignored for boxcar window dimensions' $tmp/err && awk '$3 != 1.0 {exit 1}' $tmp/out; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(NAME blend_window2d_bad_ratio COMMAND blend window2d -R0/2/0/2 -I1 --taper_ratio=0.5/0/0/0)
  set_tests_properties(blend_window2d_bad_ratio PROPERTIES WILL_FAIL TRUE)

  add_test(
    NAME blend_window3d_usage_no_error
    COMMAND bash -c "tmp=$(mktemp -d); $<TARGET_FILE:blend> window3d > $tmp/out 2> $tmp/err; status=$?; grep -q '^blend window3d - Generate 3-D blending weights' $tmp/out && grep -q '^usage: blend window3d' $tmp/out && grep -q '^REQUIRED ARGUMENTS:' $tmp/out && grep -q '^OPTIONAL ARGUMENTS:' $tmp/out && test ! -s $tmp/err; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window3d_timing_grid
    COMMAND bash -c "tmp=$(mktemp -d); $<TARGET_FILE:blend> window3d -Vt -R0/1/0/1/0/1 -I1 -Fboxcar/boxcar/boxcar -T0/0/0/0/0/0 > $tmp/out 2> $tmp/err; status=$?; grep -q 'blend \\[timing\\]: window3d: writing 8 grid nodes (2 x 2 x 2)' $tmp/err && grep -q 'blend \\[timing\\]: window3d: wrote 8 grid nodes' $tmp/err; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window3d_boxcar_grid
    COMMAND bash -c "$<TARGET_FILE:blend> window3d -R0/2/0/2/0/2 -I1 -Fboxcar/boxcar/boxcar -T0/0/0/0/0/0 2> /dev/null | awk 'NR != 27 {next} {print} END {if (NR != 27) exit 1}'"
  )
  set_tests_properties(
    blend_window3d_boxcar_grid
    PROPERTIES
      PASS_REGULAR_EXPRESSION "2 2 2 1.000000000000"
  )
  add_test(
    NAME blend_window3d_query
    COMMAND bash -c "printf '1.5 1.5 1.5\\n3 3 3\\n' | $<TARGET_FILE:blend> window3d -R0/2/0/2/0/2 -I1 -Fboxcar/boxcar/boxcar"
  )
  set_tests_properties(
    blend_window3d_query
    PROPERTIES
      PASS_REGULAR_EXPRESSION "1.5 1.5 1.5 1.000000000000[\r\n]+3 3 3 0.000000000000"
  )
  add_test(
    NAME blend_window3d_blendfile_query
    COMMAND bash -c "tmp=$(mktemp -d); printf '1 1\\n3 1\\n3 3\\n1 3\\n' > $tmp/poly.txt; printf '%s 1 3 boxcar/boxcar/boxcar 0/0/0/0/0/0\\n' $tmp/poly.txt > $tmp/blend.txt; printf '2.5 2.5 2.5\\n2.5 2.5 0.5\\n0.5 0.5 2.5\\n' | $<TARGET_FILE:blend> window3d -R0/4/0/4/0/4 -I1 -B$tmp/blend.txt; status=$?; rm -rf $tmp; exit $status"
  )
  set_tests_properties(
    blend_window3d_blendfile_query
    PROPERTIES
      PASS_REGULAR_EXPRESSION "2.5 2.5 2.5 1.000000000000[\r\n]+2.5 2.5 0.5 0.000000000000[\r\n]+0.5 0.5 2.5 0.000000000000"
  )
  add_test(
    NAME blend_window3d_blendfile_best_piecewise_envelope
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n4 0\\n4.5 0.5\\n5 1\\n5 4\\n4.7 4.4\\n4 5\\n1 5\\n0.7 4.7\\n0.3 4.4\\n-1 4\\n-1 1\\n-0.2 0.8\\n-0.8 0.4\\n' > $tmp/poly.txt; printf '%s 0 2 boxcar/boxcar/boxcar 0/0/0/0/0/0\\n' $tmp/poly.txt > $tmp/blend.txt; printf '4.5 0.5 1\\n' | $<TARGET_FILE:blend> window3d -R-1/5/0/5/0/2 -I1 -MB -B$tmp/blend.txt > $tmp/out 2> $tmp/err; status=$?; grep -q 'using its strict best IoU piecewise xy-monotone envelope from -MB' $tmp/err && grep -q 'original number of vertices = 14, final number of vertices' $tmp/err && grep -q 'modified polygon for .* will not be written; use -N to write it' $tmp/err && grep -q '4.5 0.5 1 0.750000000000' $tmp/out; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window3d_south_america_boundary
    COMMAND bash -c "tmp=$(mktemp -d); cp ${CMAKE_CURRENT_SOURCE_DIR}/doc/examples/ex04_window3d/south_america.txt $tmp; printf 'south_america.txt 10 75 cosine/cosine/cosine 0.49/0.49/0.49/0.49/0.49/0.49\\n' > $tmp/blend.txt; cd $tmp; $<TARGET_FILE:blend> window3d -R-85/-30/-60/15/0/80 -I1/1/5 -Bblend.txt -Cp -ME -N > out 2> err; status=$?; test -s out && test -s south_america_monotone.txt && grep -q 'grid-snapped polygon south_america.txt' err && grep -q 'using its strict xy-monotone envelope from -ME' err && ! grep -q 'falling back to -ME' err && ! grep -q 'blend \\[error\\]' err && ! grep -q 'could not assemble boundary' err && ! grep -q 'irregular vertices' err; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window3d_blendfile_write_monotone_polygon
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n3 0\\n3 3\\n2 3\\n2 1\\n1 1\\n1 3\\n0 3\\n' > $tmp/poly.txt; printf '%s 0 2 boxcar/boxcar/boxcar 0/0/0/0/0/0\\n' $tmp/poly.txt > $tmp/blend.txt; printf '1.5 2.5 1\\n' | $<TARGET_FILE:blend> window3d -R0/3/0/3/0/2 -I1 -ME -N -B$tmp/blend.txt > $tmp/out 2> $tmp/err; status=$?; test -s $tmp/poly_monotone.txt && grep -q 'wrote modified xy-monotone polygon to' $tmp/err && $<TARGET_FILE:blend> monotone -ME $tmp/poly_monotone.txt | grep -q 'polygon is already strictly xy-monotone'; result=$?; rm -rf $tmp; test $status -eq 0 && exit $result"
  )
  add_test(
    NAME blend_window3d_blendfile_reject_nonmonotone
    COMMAND bash -c "tmp=$(mktemp -d); printf '0 0\\n3 0\\n3 3\\n2 3\\n2 1\\n1 1\\n1 3\\n0 3\\n' > $tmp/poly.txt; printf '%s 0 2 boxcar/boxcar/boxcar 0/0/0/0/0/0\\n' $tmp/poly.txt > $tmp/blend.txt; printf '1.5 2.5 1\\n' | $<TARGET_FILE:blend> window3d -R0/3/0/3/0/2 -I1 -B$tmp/blend.txt > $tmp/out 2> $tmp/err; status=$?; grep -q 'use -ME or -MB' $tmp/err; result=$?; rm -rf $tmp; test $status -ne 0 && exit $result; exit 1"
  )
  add_test(
    NAME blend_window3d_bad_ratio
    COMMAND blend window3d -R0/2/0/2/0/2 -I1 --taper_ratio=0.5/0/0/0/0/0
  )
  set_tests_properties(blend_window3d_bad_ratio PROPERTIES WILL_FAIL TRUE)

endif()

if(BLEND_BUILD_EXAMPLES)
  foreach(example ex01_api ex02_api ex03_api ex04_api)
    add_executable(${example} doc/examples/${example}/${example}.c)
    if(TARGET blend_static)
      target_link_libraries(${example} PRIVATE blend_static)
    else()
      target_link_libraries(${example} PRIVATE blend_shared)
    endif()
  endforeach()
endif()

set(BLEND_DOC_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/doc/html")
set(BLEND_DOC_LATEX_DIR "${CMAKE_CURRENT_BINARY_DIR}/doc/latex")
set(BLEND_DOC_PDF "${BLEND_DOC_HTML_DIR}/_downloads/blend.pdf")
set(BLEND_DOC_USAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/doc/usage")
set(BLEND_DOC_USAGE_FILES "${BLEND_DOC_USAGE_DIR}/blend.txt")
target_compile_definitions(
  blend
  PRIVATE
    BLEND_DOC_HTML_DIR_BUILD="${BLEND_DOC_HTML_DIR}"
    BLEND_DOC_HTML_DIR_INSTALL="${CMAKE_INSTALL_PREFIX}/${BLEND_INSTALL_DOCDIR}/html"
    BLEND_DOC_SERVER_URL="https://cascadiaquakes.github.io/blend/"
)

add_custom_command(
  OUTPUT "${BLEND_DOC_USAGE_DIR}/blend.txt"
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${BLEND_DOC_USAGE_DIR}"
  COMMAND bash -c "\"$1\" -? > \"$2\"" _ "$<TARGET_FILE:blend>" "${BLEND_DOC_USAGE_DIR}/blend.txt"
  DEPENDS blend
  COMMENT "Generating BLEND command usage"
  VERBATIM
)

foreach(module docs monotone window1d window2d window3d)
  list(APPEND BLEND_DOC_USAGE_FILES "${BLEND_DOC_USAGE_DIR}/${module}.txt")
  add_custom_command(
    OUTPUT "${BLEND_DOC_USAGE_DIR}/${module}.txt"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${BLEND_DOC_USAGE_DIR}"
    COMMAND bash -c "\"$1\" \"$2\" --help > \"$3\"" _ "$<TARGET_FILE:blend>" "${module}" "${BLEND_DOC_USAGE_DIR}/${module}.txt"
    DEPENDS blend
    COMMENT "Generating BLEND ${module} usage"
    VERBATIM
  )
endforeach()

add_custom_target(docs_usage DEPENDS ${BLEND_DOC_USAGE_FILES})

if(BLEND_BUILD_DOCS)
  find_program(SPHINX_BUILD_EXECUTABLE NAMES sphinx-build)
  find_program(LATEXMK_EXECUTABLE NAMES latexmk)

  if(SPHINX_BUILD_EXECUTABLE)
    add_custom_target(
      docs_html
      COMMAND "${CMAKE_COMMAND}" -E env "BLEND_DOC_USAGE_DIR=${BLEND_DOC_USAGE_DIR}"
              "${SPHINX_BUILD_EXECUTABLE}"
              -b html
              -D version=${PROJECT_VERSION}
              -D release=${PROJECT_VERSION}
              "${CMAKE_CURRENT_SOURCE_DIR}/doc/rst/source"
              "${BLEND_DOC_HTML_DIR}"
      COMMENT "Building BLEND HTML documentation"
      VERBATIM
    )
    add_dependencies(docs_html docs_usage)
  else()
    message(WARNING "sphinx-build was not found; documentation targets will not be available.")
  endif()

  if(SPHINX_BUILD_EXECUTABLE AND LATEXMK_EXECUTABLE)
    add_custom_target(
      docs_pdf
      COMMAND "${CMAKE_COMMAND}" -E env "BLEND_DOC_USAGE_DIR=${BLEND_DOC_USAGE_DIR}"
              "${SPHINX_BUILD_EXECUTABLE}"
              -b latex
              -D version=${PROJECT_VERSION}
              -D release=${PROJECT_VERSION}
              "${CMAKE_CURRENT_SOURCE_DIR}/doc/rst/source"
              "${BLEND_DOC_LATEX_DIR}"
      COMMAND "${CMAKE_COMMAND}" -E chdir "${BLEND_DOC_LATEX_DIR}"
              "${LATEXMK_EXECUTABLE}" -pdf -interaction=nonstopmode -halt-on-error blend.tex
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${BLEND_DOC_HTML_DIR}/_downloads"
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${BLEND_DOC_LATEX_DIR}/blend.pdf" "${BLEND_DOC_PDF}"
      COMMENT "Building BLEND PDF documentation"
      VERBATIM
    )
    add_dependencies(docs_pdf docs_html)
  elseif(SPHINX_BUILD_EXECUTABLE)
    message(WARNING "latexmk was not found; docs_pdf target will not be available.")
  endif()

  if(TARGET docs_html AND TARGET docs_pdf)
    add_custom_target(docs DEPENDS docs_html docs_pdf)
  elseif(TARGET docs_html)
    add_custom_target(docs DEPENDS docs_html)
  endif()
endif()

install(
  TARGETS ${BLEND_EXPORT_TARGETS}
  EXPORT blend-targets
  ARCHIVE DESTINATION ${BLEND_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${BLEND_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${BLEND_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${BLEND_INSTALL_INCLUDEDIR}
)

install(
  TARGETS blend
  RUNTIME DESTINATION ${BLEND_INSTALL_BINDIR}
)

install(
  EXPORT blend-targets
  FILE blend-targets.cmake
  NAMESPACE BLEND::
  DESTINATION ${BLEND_INSTALL_CMAKEDIR}
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/blend-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  cmake/blend-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/blend-config.cmake"
  INSTALL_DESTINATION ${BLEND_INSTALL_CMAKEDIR}
  PATH_VARS BLEND_INSTALL_INCLUDEDIR
)

configure_file(
  share/tools/blend_uninstall.sh.in
  "${CMAKE_CURRENT_BINARY_DIR}/blend_uninstall.sh"
  @ONLY
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/blend-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/blend-config-version.cmake"
  DESTINATION ${BLEND_INSTALL_CMAKEDIR}
)

install(
  FILES ChangeLog LICENSE NOTICE
  DESTINATION ${BLEND_INSTALL_DOCDIR}
)

install(
  DIRECTORY doc/examples
  DESTINATION ${BLEND_INSTALL_DOCDIR}
  USE_SOURCE_PERMISSIONS
  FILES_MATCHING
    PATTERN "*.c"
    PATTERN "*.md"
    PATTERN "*.gp"
    PATTERN "*.sh"
    PATTERN "*.blend"
    PATTERN "*.txt"
    PATTERN "*.png"
)

install(
  DIRECTORY doc/rst
  DESTINATION ${BLEND_INSTALL_DOCDIR}
)

install(
  DIRECTORY "${BLEND_DOC_HTML_DIR}/"
  DESTINATION "${BLEND_INSTALL_DOCDIR}/html"
  OPTIONAL
)

install(
  PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/blend_uninstall.sh"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/tools
)
