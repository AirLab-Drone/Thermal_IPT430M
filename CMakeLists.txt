cmake_minimum_required(VERSION 3.8)

Project(thermal_IPT430M)


# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()


find_package(OpenCV REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)



include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${rclcpp_INCLUDE_DIRS}) 
include_directories(${std_msgs_INCLUDE_DIRS})  
include_directories(${sensor_msgs_INCLUDE_DIRS})  
include_directories(${cv_bridge_INCLUDE_DIRS})
include_directories(${image_transport_INCLUDE_DIRS})

include_directories(include)
link_directories(lib)



file(GLOB LIBRARIES_IN_FOLDER "lib/*.so")


foreach(lib ${LIBRARIES_IN_FOLDER})
    message(STATUS "Found library: ${lib}")
endforeach()


add_executable(test_demo src/test_demo.cpp)
target_link_libraries(test_demo 
    ${LIBRARIES_IN_FOLDER}
    ${OpenCV_LIBS}
)



# /* ------------------------------ In ros2 node ------------------------------ */
add_executable(test_thermal_node src/IPT430M_ros2_node.cpp)
target_link_libraries(test_thermal_node 
    # "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    # "$<INSTALL_INTERFACE:include>"
    ${OpenCV_LIBS}
    ${rclcpp_LIBRARIES}
    ${std_msgs_LIBRARIES}
    ${sensor_msgs_LIBRARIES}
    # ${cv_bridge_LIBRARIES}  # 這樣不行
    cv_bridge::cv_bridge      # 這樣會成功==
    image_transport::image_transport
    ${LIBRARIES_IN_FOLDER}
    pthread
    m
    udev
)


