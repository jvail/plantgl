# Include Directory
find_path(EIGEN_INCLUDE_DIR_1 "Core" PATHS $ENV{PATH} $ENV{CONDA_PREFIX}/include)

if (EIGEN_INCLUDE_DIR_1)
    set(EIGEN_INCLUDE_DIR ${EIGEN_INCLUDE_DIR_1})
else()
    find_path(EIGEN_INCLUDE_DIR_2 "eigen3/Eigen/Core" PATHS $ENV{PATH} $ENV{CONDA_PREFIX}/include)
    
    if (EIGEN_INCLUDE_DIR_2)
        set(EIGEN_INCLUDE_DIR "${EIGEN_INCLUDE_DIR_2}/eigen3")
    endif()
endif()

if (EIGEN_INCLUDE_DIR)
    set(EIGEN_FOUND ON)
    set(EIGEN_INCLUDE_DIRS ${EIGEN_INCLUDE_DIR})
    
    # EIGEN found
    message(STATUS "Found Eigen: ${EIGEN_INCLUDE_DIR}/Eigen")
else()
    set(EIGEN_FOUND OFF)
    
    if (Eigen_FIND_REQUIRED)
        # EIGEN not found
        message(SEND_ERROR "Unable to find Eigen library.")
    endif()
endif()

if (EIGEN_FOUND)
    # Build with Eigen
    define_cpp_macro(PGL_WITH_EIGEN 1)
    
    include_directories(${EIGEN_INCLUDE_DIRS})
    
elseif (NOT Eigen_FIND_REQUIRED)
    message(STATUS "Building without Eigen - Library not found.")
endif()
