
add_subdirectory(util)
add_subdirectory(websocket)

target_sources(${G2OWS_MODULE} PRIVATE
    "sqmain.cpp"
)