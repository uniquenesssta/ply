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
set(PLAYER_FFMPEG_REF "n8.0.3" CACHE INTERNAL "Required FFmpeg source ref")
set(PLAYER_FFMPEG_COMMIT "8ae0b34901ba60a802f183ee75a250a9fc3e09a5" CACHE INTERNAL "Required FFmpeg release commit")
set(PLAYER_LIBPLACEBO_VERSION "7.351.0" CACHE INTERNAL "Required libplacebo release version")
set(PLAYER_LIBPLACEBO_REF "v7.351.0" CACHE INTERNAL "Required libplacebo source ref")
set(PLAYER_LIBPLACEBO_COMMIT "3188549fba13bbdf3a5a98de2a38c2e71f04e21e" CACHE INTERNAL "Required libplacebo release commit")
set(PLAYER_LIBASS_VERSION "0.17.4" CACHE INTERNAL "Required libass release version")
set(PLAYER_LIBASS_REF "0.17.4" CACHE INTERNAL "Required libass source ref")
set(PLAYER_LIBASS_COMMIT "bbb3c7f1570a4a021e52683f3fbdf74fe492ae84" CACHE INTERNAL "Required libass release commit")
set(PLAYER_FREETYPE_VERSION "2.13.3" CACHE INTERNAL "Required FreeType release version")
set(PLAYER_FREETYPE_REF "VER-2-13-3" CACHE INTERNAL "Required FreeType source ref")
set(PLAYER_FREETYPE_COMMIT "42608f77f20749dd6ddc9e0536788eaad70ea4b5" CACHE INTERNAL "Required FreeType release commit")
set(PLAYER_FRIBIDI_VERSION "1.0.16" CACHE INTERNAL "Required FriBidi release version")
set(PLAYER_FRIBIDI_REF "v1.0.16" CACHE INTERNAL "Required FriBidi source ref")
set(PLAYER_FRIBIDI_COMMIT "68162babff4f39c4e2dc164a5e825af93bda9983" CACHE INTERNAL "Required FriBidi release commit")
set(PLAYER_HARFBUZZ_VERSION "10.2.0" CACHE INTERNAL "Required HarfBuzz release version")
set(PLAYER_HARFBUZZ_REF "10.2.0" CACHE INTERNAL "Required HarfBuzz source ref")
set(PLAYER_HARFBUZZ_COMMIT "7b27c8edd46c674e01dd226fa9e1aa7549f5c436" CACHE INTERNAL "Required HarfBuzz release commit")

set(PLAYER_CXX_STANDARD "20" CACHE INTERNAL "Required C++ language standard")
