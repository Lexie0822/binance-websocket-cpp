# FindWebsocketpp.cmake

find_path(WEBSOCKETPP_INCLUDE_DIR
    NAMES websocketpp/client.hpp
    PATHS ${WEBSOCKETPP_INCLUDE_DIR}
    DOC "The WebSocket++ include directory"
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Websocketpp DEFAULT_MSG WEBSOCKETPP_INCLUDE_DIR)

mark_as_advanced(WEBSOCKETPP_INCLUDE_DIR)

if(WEBSOCKETPP_FOUND AND NOT TARGET websocketpp::websocketpp)
    add_library(websocketpp::websocketpp INTERFACE IMPORTED)
    set_target_properties(websocketpp::websocketpp PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${WEBSOCKETPP_INCLUDE_DIR}"
    )
endif()
