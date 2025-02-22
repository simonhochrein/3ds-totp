file(GLOB QUIRC_FILES quirc/*.c)
add_library(quirc ${QUIRC_FILES})
target_include_directories(quirc PUBLIC quirc)

file(GLOB TINY_HMAC_FILES tiny-hmac-c/src/*.c)
add_library(tinyhmac ${TINY_HMAC_FILES})
target_include_directories(tinyhmac PUBLIC tiny-hmac-c/src)