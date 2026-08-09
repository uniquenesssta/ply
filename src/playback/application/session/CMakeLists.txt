add_subdirectory(backend)

target_sources(
    player_playback_application
    PRIVATE
        media_generation_gate.cpp
        playback_session.cpp
        playback_session_thread.cpp
        playback_shutdown.cpp
    PUBLIC
        media_generation_gate.h
        playback_session.h
        playback_session_thread.h
        playback_shutdown.h
)
