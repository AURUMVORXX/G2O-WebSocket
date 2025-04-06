add_subdirectory(sqapi)
add_subdirectory(ixwebsocket)

add_library(json INTERFACE)
target_include_directories(json INTERFACE 
	"json/include/"
)

target_compile_definitions(${G2OWS_MODULE}
	PRIVATE
		SCRAT_EXPORT
)

target_link_libraries(${G2OWS_MODULE}
	PRIVATE 
		sqapi
		ixwebsocket
		json
)