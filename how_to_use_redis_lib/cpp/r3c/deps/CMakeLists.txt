# Writed by yijian (eyjian@qq.com)

set (HIREDIS hiredis-0.14.0)

exec_program(
    tar
    ${CMAKE_CURRENT_SOURCE_DIR}
    ARGS
    xzf ${HIREDIS}.tar.gz
    RETURN_VALUE errcode
)
if (errcode)
    return ()
endif ()

exec_program(
    make
    ${CMAKE_CURRENT_SOURCE_DIR}/${HIREDIS}
    ARGS
    install PREFIX=${CMAKE_INSTALL_PREFIX}
    RETURN_VALUE errcode
)
if (errcode)
    return ()
endif ()
