cmake_minimum_required(VERSION 3.20)

project(ChatServer LANGUAGES CXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

if(MSVC)
    add_compile_options(/FS /utf-8)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    AsioIOServicePool.cpp
    ChatGrpcClient.cpp
    ChatServiceImpl.cpp
    ConfigMgr.cpp
    CServer.cpp
    CSession.cpp
    LogicSystem.cpp
    MysqlDao.cpp
    MysqlMgr.cpp
    RedisMgr.cpp
    StatusGrpcClient.cpp
    UserMgr.cpp
    main.cpp
    message.grpc.pb.cc
    message.pb.cc
)

set(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_EXAMPLE OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_INSTALL OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_CMAKE_PACKAGE OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_PKGCONFIG_SUPPORT OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_OBJECT_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(D:/Jsoncpp ${CMAKE_BINARY_DIR}/jsoncpp-build)

set(gRPC_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_CODEGEN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_CSHARP_EXT OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_CPP_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_CSHARP_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_NODE_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_PHP_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_PYTHON_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_BUILD_GRPC_RUBY_PLUGIN OFF CACHE BOOL "" FORCE)
set(gRPC_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(D:/cppsoft/grpc ${CMAKE_BINARY_DIR}/grpc-build)

set(REDIS_ROOT "D:/cppsoft/windows-redis" CACHE PATH "Windows Redis/hiredis root directory")
find_path(REDIS_INCLUDE_DIR
    NAMES hiredis/hiredis.h hiredis.h
    PATHS "${REDIS_ROOT}" "${REDIS_ROOT}/include" "${REDIS_ROOT}/deps/hiredis"
    NO_DEFAULT_PATH
)
find_library(REDIS_LIBRARY
    NAMES hiredis redis
    PATHS "${REDIS_ROOT}" "${REDIS_ROOT}/lib" "${REDIS_ROOT}/libs" "${REDIS_ROOT}/x64" "${REDIS_ROOT}/x64/Debug" "${REDIS_ROOT}/x64/Release" "${REDIS_ROOT}/msvs/hiredis/x64/Debug" "${REDIS_ROOT}/msvs/hiredis/x64/Release"
    NO_DEFAULT_PATH
)
find_library(REDIS_INTEROP_LIBRARY
    NAMES Win32_Interop
    PATHS "${REDIS_ROOT}/src/Win32_Interop/x64/Debug" "${REDIS_ROOT}/src/Win32_Interop/x64/Release"
    NO_DEFAULT_PATH
)

set(MYSQL_CONCPP_ROOT "D:/cppsoft/mysql-connector-c++-8.3.0-winx64" CACHE PATH "MySQL Connector/C++ root directory")
find_path(MYSQL_CONCPP_INCLUDE_DIR
    NAMES jdbc/mysql_driver.h mysqlx/xdevapi.h
    PATHS "${MYSQL_CONCPP_ROOT}/include"
    NO_DEFAULT_PATH
)
find_library(MYSQL_CONCPP_JDBC_LIBRARY
    NAMES mysqlcppconn
    PATHS "${MYSQL_CONCPP_ROOT}/lib64/vs14/debug" "${MYSQL_CONCPP_ROOT}/lib64/vs14"
    NO_DEFAULT_PATH
)
find_library(MYSQL_CONCPP_XDEVAPI_LIBRARY
    NAMES mysqlcppconn8
    PATHS "${MYSQL_CONCPP_ROOT}/lib64/vs14/debug" "${MYSQL_CONCPP_ROOT}/lib64/vs14"
    NO_DEFAULT_PATH
)

add_executable(ChatServer ${SOURCES})

add_custom_command(TARGET ChatServer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/config.ini"
        "$<TARGET_FILE_DIR:ChatServer>/config.ini"
)

target_compile_features(ChatServer PRIVATE cxx_std_17)

target_compile_definitions(ChatServer PRIVATE
    _WIN32_WINNT=0x0601
)

target_include_directories(ChatServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    D:/boost_1_90_0
)

if(MYSQL_CONCPP_INCLUDE_DIR)
    target_include_directories(ChatServer PRIVATE
        ${MYSQL_CONCPP_INCLUDE_DIR}
        ${MYSQL_CONCPP_INCLUDE_DIR}/jdbc
    )
endif()

if(REDIS_INCLUDE_DIR)
    target_include_directories(ChatServer PRIVATE ${REDIS_INCLUDE_DIR})
endif()

target_link_libraries(ChatServer PRIVATE
    jsoncpp_static
    grpc++
    libprotobuf
)

if(MYSQL_CONCPP_JDBC_LIBRARY)
    target_link_libraries(ChatServer PRIVATE ${MYSQL_CONCPP_JDBC_LIBRARY})
endif()

if(MYSQL_CONCPP_XDEVAPI_LIBRARY)
    target_link_libraries(ChatServer PRIVATE ${MYSQL_CONCPP_XDEVAPI_LIBRARY})
endif()

if(REDIS_LIBRARY)
    target_link_libraries(ChatServer PRIVATE ${REDIS_LIBRARY})
endif()

if(REDIS_INTEROP_LIBRARY)
    target_link_libraries(ChatServer PRIVATE ${REDIS_INTEROP_LIBRARY})
endif()

if(EXISTS "${MYSQL_CONCPP_ROOT}/lib64/debug")
    add_custom_command(TARGET ChatServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MYSQL_CONCPP_ROOT}/lib64/debug"
            "$<TARGET_FILE_DIR:ChatServer>"
    )
endif()

foreach(MYSQL_CONCPP_DLL libcrypto-3-x64.dll libssl-3-x64.dll)
    if(EXISTS "${MYSQL_CONCPP_ROOT}/lib64/${MYSQL_CONCPP_DLL}")
        add_custom_command(TARGET ChatServer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MYSQL_CONCPP_ROOT}/lib64/${MYSQL_CONCPP_DLL}"
                "$<TARGET_FILE_DIR:ChatServer>"
        )
    endif()
endforeach()
