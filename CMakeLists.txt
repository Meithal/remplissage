cmake_minimum_required(VERSION 3.25)
project(fenetrage_remplissage C)

set(CMAKE_C_STANDARD 11)

add_executable(fenetrage_remplissage main.c glad/src/glad.c gui.h gui.c shapes.h shapes.c)

target_include_directories(fenetrage_remplissage PRIVATE glad/include)
target_include_directories(fenetrage_remplissage PRIVATE glfw/include)

add_subdirectory(glfw)
target_link_libraries(fenetrage_remplissage PRIVATE glfw)

#add_subdirectory(glew/build/cmake)

add_executable(nuk_canvas Nuklear/example/canvas.c)
target_link_libraries(nuk_canvas PRIVATE glfw glew)
target_include_directories(nuk_canvas PRIVATE glew/include)

add_executable(nuk_extended Nuklear/example/extended.c)
target_link_libraries(nuk_extended PRIVATE glfw glew)
target_include_directories(nuk_extended PRIVATE glew/include)


#add_executable(nuk_filebrowser Nuklear/example/file_browser.c)
#target_link_libraries(nuk_filebrowser PRIVATE glfw glew)
#target_include_directories(nuk_filebrowser PRIVATE glew/include)

add_executable(nuk_skin Nuklear/example/skinning.c)
target_link_libraries(nuk_skin PRIVATE glfw glew)
target_include_directories(nuk_skin PRIVATE glew/include)

set_target_properties(nuk_skin nuk_extended nuk_canvas
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin)



# Define the source and destination directories for shader files
set(NUK_ICONS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Nuklear/example/icon")
set(NUK_ICONS_BUILD_DIR "${CMAKE_BINARY_DIR}/icon")
set(NUK_IMAGES_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Nuklear/example/images")
set(NUK_IMAGES_BUILD_DIR "${CMAKE_BINARY_DIR}/images")
set(NUK_FONTS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Nuklear/extra_font")
set(NUK_FONTS_BUILD_DIR "${CMAKE_BINARY_DIR}/extra_font")
set(NUK_SKINS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Nuklear/example/skins")
set(NUK_SKINS_BUILD_DIR "${CMAKE_BINARY_DIR}/skins")

# Create a list of shader files
file(GLOB NUK_ICONS_FILES "${NUK_ICONS_SOURCE_DIR}/*")
file(GLOB NUK_IMAGES_FILES "${NUK_IMAGES_SOURCE_DIR}/*")
file(GLOB NUK_FONT_FILES "${NUK_FONTS_SOURCE_DIR}/*")
file(GLOB NUK_SKIN_FILES "${NUK_SKINS_SOURCE_DIR}/*")

# Custom command to copy the shaders only when they are changed
add_custom_command(
        OUTPUT ${NUK_ICONS_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUK_ICONS_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NUK_ICONS_FILES} ${NUK_ICONS_BUILD_DIR}
        DEPENDS ${NUK_ICONS_FILES}
        COMMENT "Copying icon files to the build directory if they have changed"
)
add_custom_command(
        OUTPUT ${NUK_IMAGES_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUK_IMAGES_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NUK_IMAGES_FILES} ${NUK_IMAGES_BUILD_DIR}
        DEPENDS ${NUK_IMAGES_FILES}
        COMMENT "Copying images files to the build directory if they have changed"
)
add_custom_command(
        OUTPUT ${NUK_FONTS_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUK_FONTS_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NUK_FONT_FILES} ${NUK_FONTS_BUILD_DIR}
        DEPENDS ${NUK_FONT_FILES}
        COMMENT "Copying font files to the build directory if they have changed"
)
add_custom_command(
        OUTPUT ${NUK_SKINS_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUK_SKINS_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${NUK_SKIN_FILES} ${NUK_SKINS_BUILD_DIR}
        DEPENDS ${NUK_SKIN_FILES}
        COMMENT "Copying skin files to the build directory if they have changed"
)

# Add a custom target that depends on the shaders to ensure they are copied
add_custom_target(copy_nuk_icons ALL DEPENDS ${NUK_ICONS_BUILD_DIR})
add_custom_target(copy_nuk_images ALL DEPENDS ${NUK_IMAGES_BUILD_DIR})
add_custom_target(copy_nuk_fonts ALL DEPENDS ${NUK_FONTS_BUILD_DIR})
add_custom_target(copy_nuk_skins ALL DEPENDS ${NUK_SKINS_BUILD_DIR})
ADD_DEPENDENCIES ( nuk_extended copy_nuk_icons copy_nuk_images copy_nuk_fonts)
ADD_DEPENDENCIES ( nuk_skin copy_nuk_skins)
