cmake_minimum_required(VERSION 2.8) 

PROJECT(mftp)

#if (NOT DEFINED ENV{Mplatform})    
#    message(FATAL_ERROR "not defined environment variable:Mplatform") 
#endif()  
 
INCLUDE_DIRECTORIES(include)
INCLUDE_DIRECTORIES("/home/lwh/workspace/platform/include") 
#INCLUDE_DIRECTORIES(${Mplatform}/include) 

link_directories("/home/lwh/workspace/platform/libs/x86/")

ADD_DEFINITIONS(-DLOGGER_COMPILE_LEVEL=ERROR)

AUX_SOURCE_DIRECTORY(src/server SERVER)
AUX_SOURCE_DIRECTORY(src/client CLIENT)

ADD_EXECUTABLE(services ${SERVER}) 
TARGET_LINK_LIBRARIES(services logger)

ADD_EXECUTABLE(client ${CLIENT}) 
TARGET_LINK_LIBRARIES(client logger)
