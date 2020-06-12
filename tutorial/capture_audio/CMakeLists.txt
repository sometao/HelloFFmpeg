

project ("capture_audio")


######################################
# ffmpeg lib and include
######################################

find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h)
find_library(AVCODEC_LIBRARY avcodec)
find_library(AVFORMAT_LIBRARY avformat)
find_library(AVUTIL_LIBRARY avutil)
find_library(AVDEVICE_LIBRARY avdevice)
find_library(AVFILTER_LIBRARY avfilter)
find_library(POSTPROC_LIBRARY postproc)
find_library(SWRESAMPLE_LIBRARY swresample)
find_library(SWSCALE_LIBRARY swscale)



######################################
#  OpenAL lib setting
######################################
#make sure openAL32.dll is in your PATH.

find_path(OpenAL_INCLUDE_DIR "al.h")
find_library(OpenAL_LIBRARY "libs/Win64/OpenAL32.lib")


######################################
#  capture_audio Information
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
#message(al_framework						     =  ${al_framework})



add_executable( ${PROJECT_NAME} 
	"src/capture_audio_to_wav.cpp" 
	"src/capture_audio_to_aac.cpp" 
	"main.cpp"
 "src/capture_audio_to_wav2.cpp")

target_include_directories( ${PROJECT_NAME}
	PUBLIC 
		${CMAKE_SOURCE_DIR}/include
		${PROJECT_SOURCE_DIR}/include
		${OpenAL_INCLUDE_DIR}
		${FFMPEG_INCLUDE_DIR}

)

target_link_libraries( ${PROJECT_NAME}
	PUBLIC 
		${OpenAL_LIBRARY}
		${AVCODEC_LIBRARY}
		${AVFORMAT_LIBRARY}
		${AVUTIL_LIBRARY}
		${AVDEVICE_LIBRARY}
		${AVFILTER_LIBRARY}
		${POSTPROC_LIBRARY}
		${SWRESAMPLE_LIBRARY}
		${SWSCALE_LIBRARY}
)
