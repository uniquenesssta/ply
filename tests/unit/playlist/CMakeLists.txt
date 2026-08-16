add_subdirectory(domain)
add_subdirectory(application)
add_subdirectory(presentation)

# The application_container test target compiles ApplicationContainer directly
# in tests/CMakeLists.txt. R7-06 adds Playlist ownership to that composition, so
# mirror the production playlist link dependencies for this existing regression.
target_link_libraries(
    application_container_tests
    PRIVATE
        player_playlist_application
        player_playlist_presentation
)
