#Transformations example

file(
 GLOB_RECURSE PBRIMPL_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE PBRIMPL_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this final's asset folder to bin when it is built
if (EMSCRIPTEN)
	add_custom_target(copyAssetsF ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/assets/
	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../assets/)
else()
	add_custom_target(copyAssetsF ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_CURRENT_SOURCE_DIR}/assets/
	${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)
endif()

if (EMSCRIPTEN)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sMIN_WEBGL_VERSION=2 -sNO_DISABLE_EXCEPTION_CATCHING -s ASSERTIONS=1 -s SAFE_HEAP=1")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}\
	--preload-file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../assets/bamboo_color.jpg\
	--preload-file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../assets/bamboo_height.jpg\
	--preload-file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../assets/defaultLit.frag\
	--preload-file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../assets/defaultLit.vert\
	--preload-file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../assets/emissive.frag\
	--preload-file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../assets/emissive.vert\
	--preload-file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../assets/rock_color.jpg\
	--preload-file ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../assets/rock_height.jpg")
else()
	install(FILES ${PBRIMPL_INC} DESTINATION include/pbrImpl)
endif()

add_executable(pbrImpl ${PBRIMPL_SRC} ${PBRIMPL_INC} ${ASSIGNMENT7_ASSETS})

if(EMSCRIPTEN)
	if (BUILD_HTML)
		set_target_properties(pbrImpl PROPERTIES SUFFIX ".html")
	endif()
	
	target_compile_definitions(pbrImpl PUBLIC -DEMSCRIPTEN=1)
    set_target_properties(pbrImpl PROPERTIES LINK_FLAGS "-s USE_GLFW=3 -s USE_WEBGL2=1 -s FULL_ES3=1")
endif()

target_link_libraries(pbrImpl PUBLIC core IMGUI)
target_include_directories(pbrImpl PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when final is built
add_dependencies(pbrImpl copyAssetsF)