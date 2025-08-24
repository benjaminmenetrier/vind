# Install scripts
list( APPEND tool_files
    compare.py
    cpplint.py
)

foreach(FILENAME IN LISTS tool_files)
    set(_tool ${PROJECT_NAME}_${FILENAME})
    #Install in build-tree
    execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${PROJECT_SOURCE_DIR}/tools/${FILENAME}
        ${CMAKE_BINARY_DIR}/bin/${_tool} )
    #Install in install-tree
    install(PROGRAMS ${FILENAME} RENAME ${_tool} DESTINATION bin )
endforeach()
