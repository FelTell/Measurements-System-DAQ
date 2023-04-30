set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TRIPLE arm-none-eabi)
set(COMMON_COMPILER_FLAGS "-mcpu=cortex-m3 -mfloat-abi=soft -mthumb ")

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Linux")
    set(EXE_SUFFIX "")
elseif(${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    set(EXE_SUFFIX ".exe")
endif()

set(CMAKE_C_COMPILER ${TRIPLE}-gcc${EXE_SUFFIX})
set(CMAKE_C_COMPILER_TARGET ${TRIPLE})


set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_FLAGS_INIT ${COMMON_COMPILER_FLAGS})


