target_include_directories(${G2OWS_MODULE} PRIVATE
    "./"
)

add_subdirectory(server)
add_subdirectory(client)

target_sources(${G2OWS_MODULE} PRIVATE
    "sqmain.cpp"
    "utf8_check.cpp"
)