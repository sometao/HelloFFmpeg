



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



######################################
#  OpenAL lib setting
######################################
#make sure openAL32.dll is in your PATH.

find_path(OpenAL_INCLUDE_DIR "al.h")
find_library(OpenAL_LIBRARY "libs/Win64/OpenAL32.lib")


message(OpenAL_INCLUDE_DIR          =  ${OpenAL_INCLUDE_DIR})
message(OpenAL_LIBRARY         	    =  ${OpenAL_LIBRARY})

find_library(WINMM_LIBRARY "Winmm.lib")

add_library(al_framework 
		"src/Framework.cpp"
		"src/aldlist.cpp" 
		"src/CWaves.cpp"
		"src/LoadOAL.cpp"
)

target_include_directories( al_framework
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${OpenAL_INCLUDE_DIR}
)

target_link_libraries( al_framework
	PRIVATE
		${OpenAL_LIBRARY}
		${WINMM_LIBRARY}
		${CMAKE_THREAD_LIBS_INIT}
		${CMAKE_DL_LIBS}
)
