add_subdirectory(sqapi)

include(ixwebsocket.cmake)

add_library(json INTERFACE)
target_include_directories(json INTERFACE 
	"json/include/"
)

set(OPENSSL_DEFINITIONS "-DOPENSSL_API_COMPAT=30000")

target_compile_definitions(${G2OWS_MODULE}
	PRIVATE
		SCRAT_EXPORT
)

if(WIN32)
	target_link_libraries(${G2OWS_MODULE}
		PRIVATE 
			sqapi
			ixwebsocket
			json
			${OPENSSL_LIBRARIES}
			crypt32.lib
			ws2_32.lib
	)
else()
	target_link_libraries(${G2OWS_MODULE}
	PRIVATE 
		sqapi
		ixwebsocket
		json
		${OPENSSL_LIBRARIES}
	)
endif()