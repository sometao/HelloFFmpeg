

project("muxing")

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
#  muxing Information
######################################
message(" ===================== ${PROJECT_NAME}  Information ======================")
message(CMAKE_PREFIX_PATH         	  =  ${CMAKE_PREFIX_PATH})
message(CMAKE_SOURCE_DIR         	    =  ${CMAKE_SOURCE_DIR})
message(PROJECT_SOURCE_DIR            =  ${PROJECT_SOURCE_DIR})
message(CMAKE_INCLUDE_PATH            =  ${CMAKE_INCLUDE_PATH})
message(CMAKE_LIBRARY_PATH            =  ${CMAKE_LIBRARY_PATH})
message(PROJECT_SOURCE_DIR            =  ${PROJECT_SOURCE_DIR})
message(CMAKE_BUILD_TYPE              =  ${CMAKE_BUILD_TYPE})
message(CMAKE_CURRENT_LIST_DIR				=  ${CMAKE_CURRENT_LIST_DIR})
message(FFMPEG_INCLUDE_DIR			    	=  ${FFMPEG_INCLUDE_DIR})
message(AVCODEC_LIBRARY			    	=  ${AVCODEC_LIBRARY})
message(AVFORMAT_LIBRARY			    	=  ${AVFORMAT_LIBRARY})
message(AVUTIL_LIBRARY			    	=  ${AVUTIL_LIBRARY})
message(AVDEVICE_LIBRARY			    	=  ${AVDEVICE_LIBRARY})
message(AVFILTER_LIBRARY			    	=  ${AVFILTER_LIBRARY})
message(POSTPROC_LIBRARY			    	=  ${POSTPROC_LIBRARY})
message(SWRESAMPLE_LIBRARY			    	=  ${SWRESAMPLE_LIBRARY})
message(SWSCALE_LIBRARY			    	=  ${SWSCALE_LIBRARY})
message(CMAKE_CXX_FLAGS			    	=  ${CMAKE_CXX_FLAGS})





add_executable( ${PROJECT_NAME} 
	"muxing.cpp" 
)

target_include_directories( ${PROJECT_NAME} 
	PUBLIC 
		${CMAKE_SOURCE_DIR}/include
		${CMAKE_CURRENT_LIST_DIR}/include
		${FFMPEG_INCLUDE_DIR}
)

target_link_libraries( ${PROJECT_NAME} 
	PUBLIC 
		${AVCODEC_LIBRARY}
		${AVFORMAT_LIBRARY}
		${AVUTIL_LIBRARY}
		${AVDEVICE_LIBRARY}
		${AVFILTER_LIBRARY}
		${POSTPROC_LIBRARY}
		${SWRESAMPLE_LIBRARY}
		${SWSCALE_LIBRARY}
)
