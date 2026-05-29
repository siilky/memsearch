# Generic build definition for the memsearch library.
# Project-specific settings (table-opener-common, RTTI/optimisation policy,
# linker map/debug options) are layered on by the consuming project after
# add_subdirectory() — see the superproject's cmake/memsearch.cmake.

add_library(memsearch STATIC
    memory.cpp
    memsearch.cpp
    pch.cpp
)

# Precompiled header
target_precompile_headers(memsearch PRIVATE pch.h)
set_source_files_properties(pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

# Parse our own (possibly UTF-8) sources correctly
target_compile_options(memsearch PRIVATE /utf-8)
