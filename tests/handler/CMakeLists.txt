include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    ${CMAKE_BINARY_DIR}/tests/common
    ${TP_QT5_INCLUDE_DIRS}
    ${GSETTINGS_QT_INCLUDE_DIRS}
    )

generate_telepathy_test(HandlerTest SOURCES HandlerTest.cpp handlercontroller.cpp approver.cpp)
