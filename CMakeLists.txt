cmake_minimum_required(VERSION 3.12)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)


project(mail-client LANGUAGE C;CXX)

include_directories(${PROJECT_SOURCE_DIR}/lib/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)

# add email namespace source files
#file(GLOB email_namespace_SRC CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/email/*.h"
#												"${PROJECT_SOURCE_DIR}/email/*.hpp"
#												"${PROJECT_SOURCE_DIR}/email/*.cpp" )

set(EMAIL_SOURCES
	Address.cpp
	Address.h
	Client.cpp
	Client.h
	DateTime.cpp
	DateTime.h
	email.hpp
	Message.cpp
	Message.h)

# add_library(email-module, ${EMAIL_SOURCES})

add_executable(mail-client ${EMAIL_SOURCES})

# link library
if (WIN32)
	target_link_libraries(email-client, libcurl_imp.lib)
elseif (APPLE)

else ()

endif ()
