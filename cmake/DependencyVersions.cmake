include_guard(GLOBAL)

# This file is the sole toolchain/dependency compatibility baseline for local
# development, CI, dependency acquisition, and release builds. Runtime/product
# dependencies remain exact where identity matters; development tools use the
# minimum compatible version required by the current scaffold.
set(PLAYER_WINDOWS_MIN_BUILD "10.0.19045" CACHE INTERNAL "Minimum supported Windows build")
set(PLAYER_WINDOWS_PRIMARY_BUILD "10.0.26100" CACHE INTERNAL "Primary Windows validation build family")
set(PLAYER_WINDOWS_SDK_VERSION "10.0.26100.0" CACHE INTERNAL "Minimum compatible Windows SDK include/lib version")
set(PLAYER_WINDOWS_SDK_RELEASE "10.0.26100.8876" CACHE INTERNAL "Reference Windows SDK servicing release")
set(PLAYER_VISUAL_STUDIO_VERSION "17.14.37" CACHE INTERNAL "Reference Visual Studio 2022 release in the required 17.14 family")
set(PLAYER_VISUAL_STUDIO_BUILD "17.14.37516.0" CACHE INTERNAL "Reference Visual Studio installation build")
set(PLAYER_MSVC_TOOLSET_VERSION "14.44" CACHE INTERNAL "Required MSVC v143 toolset family")
set(PLAYER_MSVC_COMPILER_VERSION "19.44" CACHE INTERNAL "Required cl.exe compiler version family")
set(PLAYER_QT_VERSION "6.8.3" CACHE INTERNAL "Required Qt MSVC 2022 x64 patch version")
set(PLAYER_CMAKE_VERSION "3.30.5" CACHE INTERNAL "Minimum compatible CMake version")
set(PLAYER_NINJA_VERSION "1.12.1" CACHE INTERNAL "Minimum compatible Ninja version")
set(PLAYER_MPV_VERSION "0.41.0" CACHE INTERNAL "Required mpv/libmpv release version")
set(PLAYER_MPV_TAG "v0.41.0" CACHE INTERNAL "Required signed mpv release tag")
set(PLAYER_MPV_COMMIT "41f6a645068483470267271e1d09966ca3b9f413" CACHE INTERNAL "Required mpv release commit")
set(PLAYER_FFMPEG_VERSION "8.0.3" CACHE INTERNAL "Required FFmpeg release version for the libmpv build")
set(PLAYER_CXX_STANDARD "20" CACHE INTERNAL "Required C++ language standard")
