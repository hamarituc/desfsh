cmake_minimum_required(VERSION 4.0)

project(
  desfsh
  VERSION 1.1.2
  HOMEPAGE_URL https://github.com/hamarituc/desfsh
  LANGUAGES C
)

find_package(PkgConfig REQUIRED)

find_package(Lua REQUIRED)
find_package(OpenSSL COMPONENTS Crypto REQUIRED)
find_package(ZLIB REQUIRED)

pkg_check_modules(LIBNFC libnfc REQUIRED IMPORTED_TARGET)
pkg_check_modules(LIBFREEFARE libfreefare REQUIRED IMPORTED_TARGET)
pkg_check_modules(LIBREADLINE readline REQUIRED IMPORTED_TARGET)

if(Lua_FOUND AND NOT TARGET Lua::Lua)
  add_library(Lua::Lua INTERFACE IMPORTED)
  set_target_properties(
    Lua::Lua
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${LUA_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES "${LUA_LIBRARIES}"
  )
endif()

add_executable(desfsh)

target_sources(
  desfsh
  PRIVATE
    buffer.c
    cmd_app.c
    cmd_data.c
    cmd_picc.c
    cmd_sec.c
    crc.c
    crypto.c
    debug.c
    desflua.c
    desfsh.c
    fn.c
    help.c
    hexdump.c
    key.c
    shell.c
    show.c
)

target_link_libraries(
  desfsh
  PRIVATE
    Lua::Lua
    OpenSSL::Crypto
    ZLIB::ZLIB
    PkgConfig::LIBNFC
    PkgConfig::LIBFREEFARE
    PkgConfig::LIBREADLINE
)

install(TARGETS desfsh)
