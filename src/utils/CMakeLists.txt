SET(LIBNAME utils)

file(GLOB SOURCE_FILES *.cpp)
file(GLOB HEADER_FILES *.h)

add_library(${LIBNAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} RatioLookup.h)