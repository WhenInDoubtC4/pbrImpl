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
add_custom_target(copyAssetsF ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${PBRIMPL_INC} DESTINATION include/pbrImpl)
add_executable(pbrImpl ${PBRIMPL_SRC} ${PBRIMPL_INC} ${ASSIGNMENT7_ASSETS})
target_link_libraries(pbrImpl PUBLIC core IMGUI)
target_include_directories(pbrImpl PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when final is built
add_dependencies(pbrImpl copyAssetsF)