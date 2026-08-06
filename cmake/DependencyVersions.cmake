include_guard(GLOBAL)

# This file is the sole version baseline for build-time dependencies.
set(PLAYER_QT_MIN_VERSION "6.8" CACHE STRING "Minimum supported Qt 6 version")
set(PLAYER_CXX_STANDARD "20" CACHE STRING "Required C++ language standard")

# libmpv is intentionally not resolved in the scaffold commit.
# Its exact version and distribution source will be pinned during R0-03/R2-01.
