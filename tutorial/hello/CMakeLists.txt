

project ("hello")

######################################
#  hello Information
######################################
message(" ===================== ${PROJECT_NAME} Information ======================")
message(CMAKE_PREFIX_PATH         	  =  ${CMAKE_PREFIX_PATH})
message(CMAKE_SOURCE_DIR         	    =  ${CMAKE_SOURCE_DIR})
message(PROJECT_SOURCE_DIR            =  ${PROJECT_SOURCE_DIR})
message(CMAKE_INCLUDE_PATH            =  ${CMAKE_INCLUDE_PATH})
message(CMAKE_LIBRARY_PATH            =  ${CMAKE_LIBRARY_PATH})
message(PROJECT_SOURCE_DIR            =  ${PROJECT_SOURCE_DIR})
message(CMAKE_BUILD_TYPE              =  ${CMAKE_BUILD_TYPE})
message(CMAKE_CURRENT_LIST_DIR			=  ${CMAKE_CURRENT_LIST_DIR})
message(al_framework						     =  ${al_framework})



add_executable( ${PROJECT_NAME} 
	"hello.cpp" 
)

target_include_directories( ${PROJECT_NAME}
	PUBLIC 
		${CMAKE_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/include
)

#target_link_libraries( ${PROJECT_NAME}
#	PUBLIC 
#		al_framework
#)
