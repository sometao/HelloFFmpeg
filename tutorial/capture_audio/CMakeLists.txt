



######################################
#  capture_audio Information
######################################
message(" ===================== capture_audio Information ======================")
message(CMAKE_PREFIX_PATH         	  =  ${CMAKE_PREFIX_PATH})
message(CMAKE_SOURCE_DIR         	    =  ${CMAKE_SOURCE_DIR})
message(PROJECT_SOURCE_DIR            =  ${PROJECT_SOURCE_DIR})
message(CMAKE_INCLUDE_PATH            =  ${CMAKE_INCLUDE_PATH})
message(CMAKE_LIBRARY_PATH            =  ${CMAKE_LIBRARY_PATH})
message(PROJECT_SOURCE_DIR            =  ${PROJECT_SOURCE_DIR})
message(CMAKE_BUILD_TYPE              =  ${CMAKE_BUILD_TYPE})
message(CMAKE_CURRENT_LIST_DIR						     =  ${CMAKE_CURRENT_LIST_DIR})
message(al_framework						     =  ${al_framework})



######################################
#  OpenAL lib setting
######################################
#make sure openAL32.dll is in your PATH.

#find_path(OpenAL_INCLUDE_DIR "al.h")
#find_library(OpenAL_LIBRARY "libs/Win64/OpenAL32.lib")


#message(OpenAL_INCLUDE_DIR         	    =  ${OpenAL_INCLUDE_DIR})
#message(OpenAL_LIBRARY         	    =  ${OpenAL_LIBRARY})

#find_library(WINMM_LIBRARY "Winmm.lib")

add_executable( capture_audio 
	"capture_audio.cpp" 
)

target_include_directories( capture_audio
	PUBLIC 
		${PROJECT_SOURCE_DIR}/include
		${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries( capture_audio
	PUBLIC 
		al_framework
)
