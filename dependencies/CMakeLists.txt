add_subdirectory(sqapi)

include(ixwebsocket.cmake)

set(OPENSSL_DEFINITIONS "-DOPENSSL_API_COMPAT=30000")
find_package(OpenSSL REQUIRED)

target_compile_definitions(${G2OWS_MODULE}
	PRIVATE
		SCRAT_EXPORT
)

if(WIN32)
	target_link_libraries(${G2OWS_MODULE}
		PRIVATE 
			sqapi
			ixwebsocket
			OpenSSL::SSL
			OpenSSL::Crypto
			crypt32.lib
			ws2_32.lib
	)
else()
	target_link_libraries(${G2OWS_MODULE}
	PRIVATE 
		sqapi
		ixwebsocket
		OpenSSL::SSL
		OpenSSL::Crypto
	)
endif()