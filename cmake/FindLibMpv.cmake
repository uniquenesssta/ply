include_guard(GLOBAL)

include(FindPackageHandleStandardArgs)

if(NOT DEFINED PLAYER_LIBMPV_ROOT OR "${PLAYER_LIBMPV_ROOT}" STREQUAL "")
    message(FATAL_ERROR "PLAYER_LIBMPV_ROOT must be defined before FindLibMpv.cmake is evaluated.")
endif()

if(IS_ABSOLUTE "${PLAYER_LIBMPV_ROOT}" OR NOT "${PLAYER_LIBMPV_ROOT}" MATCHES "^\\.\\./")
    message(FATAL_ERROR
        "PLAYER_LIBMPV_ROOT must remain repository-parent-relative and begin with ../; received '${PLAYER_LIBMPV_ROOT}'."
    )
endif()

set(_player_libmpv_root "${CMAKE_SOURCE_DIR}/${PLAYER_LIBMPV_ROOT}")

find_path(LibMpv_INCLUDE_DIR NAMES mpv/client.h PATHS "${_player_libmpv_root}/include" NO_DEFAULT_PATH)
find_library(
    LibMpv_IMPORT_LIBRARY
    NAMES mpv libmpv mpv-2 libmpv-2
    PATHS "${_player_libmpv_root}/lib" "${_player_libmpv_root}"
    NO_DEFAULT_PATH
)
find_file(
    LibMpv_RUNTIME_LIBRARY
    NAMES libmpv-2.dll mpv-2.dll
    PATHS "${_player_libmpv_root}/bin" "${_player_libmpv_root}"
    NO_DEFAULT_PATH
)
find_file(LibMpv_MANIFEST_FILE NAMES dependency-manifest.json PATHS "${_player_libmpv_root}" NO_DEFAULT_PATH)

if(LibMpv_MANIFEST_FILE)
    file(READ "${LibMpv_MANIFEST_FILE}" _player_libmpv_manifest_json)

    function(_player_libmpv_manifest_value output_name)
        string(JSON _value ERROR_VARIABLE _json_error GET "${_player_libmpv_manifest_json}" ${ARGN})
        if(NOT _json_error STREQUAL "NOTFOUND")
            string(JOIN "." _manifest_key ${ARGN})
            message(FATAL_ERROR
                "libmpv dependency manifest is missing or has an invalid '${_manifest_key}' value: ${_json_error}"
            )
        endif()
        set(${output_name} "${_value}" PARENT_SCOPE)
    endfunction()

    _player_libmpv_manifest_value(LibMpv_VERSION mpv version)
    _player_libmpv_manifest_value(_player_libmpv_tag mpv tag)
    _player_libmpv_manifest_value(_player_libmpv_commit mpv commit)
    _player_libmpv_manifest_value(_player_ffmpeg_version ffmpeg version)
    _player_libmpv_manifest_value(_player_ffmpeg_commit ffmpeg commit)
    _player_libmpv_manifest_value(_player_libplacebo_version libplacebo version)
    _player_libmpv_manifest_value(_player_libplacebo_commit libplacebo commit)
    _player_libmpv_manifest_value(_player_libass_version libass version)
    _player_libmpv_manifest_value(_player_libass_commit libass commit)
    _player_libmpv_manifest_value(_player_freetype_version freetype version)
    _player_libmpv_manifest_value(_player_freetype_commit freetype commit)
    _player_libmpv_manifest_value(_player_fribidi_version fribidi version)
    _player_libmpv_manifest_value(_player_fribidi_commit fribidi commit)
    _player_libmpv_manifest_value(_player_harfbuzz_version harfbuzz version)
    _player_libmpv_manifest_value(_player_harfbuzz_commit harfbuzz commit)

    foreach(_identity_check IN ITEMS
        "LibMpv_VERSION;${PLAYER_MPV_VERSION};mpv.version"
        "_player_libmpv_tag;${PLAYER_MPV_TAG};mpv.tag"
        "_player_libmpv_commit;${PLAYER_MPV_COMMIT};mpv.commit"
        "_player_ffmpeg_version;${PLAYER_FFMPEG_VERSION};ffmpeg.version"
        "_player_ffmpeg_commit;${PLAYER_FFMPEG_COMMIT};ffmpeg.commit"
        "_player_libplacebo_version;${PLAYER_LIBPLACEBO_VERSION};libplacebo.version"
        "_player_libplacebo_commit;${PLAYER_LIBPLACEBO_COMMIT};libplacebo.commit"
        "_player_libass_version;${PLAYER_LIBASS_VERSION};libass.version"
        "_player_libass_commit;${PLAYER_LIBASS_COMMIT};libass.commit"
        "_player_freetype_version;${PLAYER_FREETYPE_VERSION};freetype.version"
        "_player_freetype_commit;${PLAYER_FREETYPE_COMMIT};freetype.commit"
        "_player_fribidi_version;${PLAYER_FRIBIDI_VERSION};fribidi.version"
        "_player_fribidi_commit;${PLAYER_FRIBIDI_COMMIT};fribidi.commit"
        "_player_harfbuzz_version;${PLAYER_HARFBUZZ_VERSION};harfbuzz.version"
        "_player_harfbuzz_commit;${PLAYER_HARFBUZZ_COMMIT};harfbuzz.commit"
    )
        list(GET _identity_check 0 _actual_variable)
        list(GET _identity_check 1 _expected_value)
        list(GET _identity_check 2 _identity_name)
        if(NOT "${${_actual_variable}}" STREQUAL "${_expected_value}")
            message(FATAL_ERROR
                "libmpv manifest ${_identity_name} mismatch. Expected '${_expected_value}', found '${${_actual_variable}}'."
            )
        endif()
    endforeach()

    string(JSON _player_ffmpeg_policy_length ERROR_VARIABLE _player_ffmpeg_policy_error
        LENGTH "${_player_libmpv_manifest_json}" buildPolicy ffmpeg)
    if(NOT _player_ffmpeg_policy_error STREQUAL "NOTFOUND")
        message(FATAL_ERROR
            "libmpv dependency manifest is missing or has an invalid 'buildPolicy.ffmpeg' array: ${_player_ffmpeg_policy_error}"
        )
    endif()

    set(_player_ffmpeg_schannel_enabled FALSE)
    if(_player_ffmpeg_policy_length GREATER 0)
        math(EXPR _player_ffmpeg_policy_last "${_player_ffmpeg_policy_length} - 1")
        foreach(_player_ffmpeg_policy_index RANGE 0 ${_player_ffmpeg_policy_last})
            string(JSON _player_ffmpeg_policy_entry GET
                "${_player_libmpv_manifest_json}" buildPolicy ffmpeg ${_player_ffmpeg_policy_index})
            if(_player_ffmpeg_policy_entry STREQUAL "enable-schannel")
                set(_player_ffmpeg_schannel_enabled TRUE)
                break()
            endif()
        endforeach()
    endif()

    if(NOT _player_ffmpeg_schannel_enabled)
        message(FATAL_ERROR
            "libmpv FFmpeg build policy does not enable Windows Schannel TLS. Rebuild the audited package before using HTTP/HTTPS media."
        )
    endif()
endif()

find_package_handle_standard_args(
    LibMpv
    REQUIRED_VARS LibMpv_INCLUDE_DIR LibMpv_IMPORT_LIBRARY LibMpv_RUNTIME_LIBRARY LibMpv_MANIFEST_FILE
    VERSION_VAR LibMpv_VERSION
)

if(LibMpv_FOUND AND NOT TARGET LibMpv::LibMpv)
    file(GLOB _player_libmpv_runtime_files
        LIST_DIRECTORIES FALSE
        "${_player_libmpv_root}/*.dll"
        "${_player_libmpv_root}/bin/*.dll"
    )
    list(REMOVE_DUPLICATES _player_libmpv_runtime_files)

    if(_player_libmpv_runtime_files STREQUAL "")
        message(FATAL_ERROR
            "The fixed libmpv package contains no runtime DLLs under '${PLAYER_LIBMPV_ROOT}' or '${PLAYER_LIBMPV_ROOT}/bin'."
        )
    endif()

    add_library(LibMpv::LibMpv SHARED IMPORTED GLOBAL)
    set_target_properties(
        LibMpv::LibMpv
        PROPERTIES
            IMPORTED_IMPLIB "${LibMpv_IMPORT_LIBRARY}"
            IMPORTED_LOCATION "${LibMpv_RUNTIME_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${LibMpv_INCLUDE_DIR}"
            PLAYER_MANIFEST_FILE "${LibMpv_MANIFEST_FILE}"
            PLAYER_RUNTIME_FILES "${_player_libmpv_runtime_files}"
    )
endif()

function(player_stage_libmpv_runtime target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "player_stage_libmpv_runtime target does not exist: ${target_name}")
    endif()
    if(NOT TARGET LibMpv::LibMpv)
        message(FATAL_ERROR "LibMpv::LibMpv must exist before staging runtime files.")
    endif()

    get_target_property(_runtime_files LibMpv::LibMpv PLAYER_RUNTIME_FILES)
    get_target_property(_manifest_file LibMpv::LibMpv PLAYER_MANIFEST_FILE)
    if(NOT _runtime_files OR NOT _manifest_file)
        message(FATAL_ERROR "LibMpv::LibMpv is missing runtime or manifest staging metadata.")
    endif()

    add_custom_command(
        TARGET ${target_name}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_runtime_files} "$<TARGET_FILE_DIR:${target_name}>"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${target_name}>/dependencies/libmpv"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_manifest_file}"
            "$<TARGET_FILE_DIR:${target_name}>/dependencies/libmpv/dependency-manifest.json"
        VERBATIM
    )
endfunction()

mark_as_advanced(LibMpv_INCLUDE_DIR LibMpv_IMPORT_LIBRARY LibMpv_RUNTIME_LIBRARY LibMpv_MANIFEST_FILE)
