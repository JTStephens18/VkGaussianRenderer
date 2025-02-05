# CMakeList.txt : CMake project for VulkanRenderer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(NAME VulkanRenderer)
project(${NAME} VERSION 0.1.0)

# Try to grab vulkan files installed on system
# Alternatively, try to find the package
if (DEFINED VULKAN_SDK_PATH)
	set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
	set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
	set(Vulkan_FOUND "True")
else()
	find_package(Vulkan REQUIRED)
	set(Vulkan_FOUND "True")
	message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()

if (NOT Vulkan_FOUND)
	message (FATAL_ERROR "Could not find Vulkan library!")
else()
	message (STATUS "Using Vulkan lib at: ${Vulkan_LIBRARIES}")
endif()


# Set GLFW_PATH in .env.cmake to target specific glfw
if (DEFINED GLFW_PATH)
	message (STATUS "Using GLFW path specified in .env")
	set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
	if (MSVC)
		set(GLFW_LIB "${GLFW_PATH}/lib-vc2022")
	endif()
else()
	find_package(glfw3 CONFIG REQUIRED)
	set(GLFW_LIB glfw)
	message(STATUS "Found GLFW")
endif()

if (NOT GLFW_LIB)
    message(FATAL_ERROR "Could not find glfw library!")
else()
    message(STATUS "Using glfw lib at: ${GLFW_LIB}")
endif()

#find_package(imgui CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Add source to this project's executable.
#add_executable (${PROJECT_NAME} "VulkanRenderer.cpp" "VulkanRenderer.h")

add_executable (${PROJECT_NAME} ${SOURCES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET VulkanRenderer PROPERTY CXX_STANDARD 20)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

if (WIN32)
	message(STATUS "Creating build for Windows")
  target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
    #${GLM_PATH}
    )
 
  target_link_directories(${PROJECT_NAME} PUBLIC
    ${Vulkan_LIBRARIES}
    ${GLFW_LIB}
  )

  target_link_libraries(${PROJECT_NAME} glfw Vulkan::Vulkan)
elseif(UNIX)
	message(STATUS "Creating build for UNIX")
	    target_include_directories(${PROJECT_NAME} PUBLIC
      ${PROJECT_SOURCE_DIR}/src
    )
    target_link_libraries(${PROJECT_NAME} glfw 
	tinyobjloader::tinyobjloader Vulkan::Vulkan)
endif()

############## Build SHADERS #######################
 
# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator HINTS 
  ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} 
  /usr/bin 
  /usr/local/bin 
  ${VULKAN_SDK_PATH}/Bin
  ${VULKAN_SDK_PATH}/Bin32
  $ENV{VULKAN_SDK}/Bin/ 
  $ENV{VULKAN_SDK}/Bin32/
)
 
# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${PROJECT_SOURCE_DIR}/shaders/*.frag"
  "${PROJECT_SOURCE_DIR}/shaders/*.vert"
)
 
foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)
 
add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)


#include_directories(C:/VulkanSDK/1.4.304.0/Include)

#target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

# TODO: Add tests and install targets if needed.
