# Find the FAISS library
#
# The following variables are set if FAISS is found:
#  FAISS_FOUND        - True when FAISS is found
#  FAISS_INCLUDE_DIRS - The directory where FAISS headers are located
#  FAISS_LIBRARIES    - FAISS libraries to link against

find_path(FAISS_INCLUDE_DIR
    NAMES faiss/Index.h
    PATHS
        ${FAISS_ROOT}/include
        /usr/include
        /usr/local/include
)

find_library(FAISS_LIBRARY
    NAMES faiss
    PATHS
        ${FAISS_ROOT}/lib
        /usr/lib
        /usr/local/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FAISS
    REQUIRED_VARS
        FAISS_LIBRARY
        FAISS_INCLUDE_DIR
)

if(FAISS_FOUND)
    set(FAISS_INCLUDE_DIRS ${FAISS_INCLUDE_DIR})
    set(FAISS_LIBRARIES ${FAISS_LIBRARY})
endif()
