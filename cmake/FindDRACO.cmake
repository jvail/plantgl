# Include Directory
find_path(DRACO_INCLUDE_DIR "draco" PATHS $ENV{PATH} $ENV{CONDA_PREFIX}/include)

# Library Directory
find_library(DRACO_LIBRARY NAMES "draco" "draco.a" "libdraco.a" PATHS $ENV{PATH} $ENV{CONDA_PREFIX}/lib $ENV{CONDA_PREFIX}/lib64)

if (DRACO_INCLUDE_DIR AND DRACO_LIBRARY)
    set(DRACO_FOUND ON)
    set(DRACO_INCLUDE_DIRS ${DRACO_INCLUDE_DIR})
    set(DRACO_LIBRARIES ${DRACO_LIBRARY})
    message(STATUS "Found DRACO: ${DRACO_LIBRARY}")
else()
    set(DRACO_FOUND OFF)

    if (DRACO_FIND_REQUIRED)
        message(SEND_ERROR "Unable to find DRACO library.")
    endif()
endif()

if (DRACO_FOUND)
    include_directories(${DRACO_INCLUDE_DIRS})
endif()
