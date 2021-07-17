add_library(lexertk INTERFACE)
add_library(lexertk::lexertk ALIAS lexertk)

set(headers
        include/lexertk/detail.hpp
        include/lexertk/generator.hpp
        include/lexertk/generator.ipp
        include/lexertk/helper.hpp
        include/lexertk/lexertk.hpp
        include/lexertk/token.hpp
        include/lexertk/token.ipp
        )

foreach (header_file ${headers})
    list(APPEND absolute_header_files
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${header_file}>
            $<INSTALL_INTERFACE:${header_file}>
            )
endforeach ()

target_sources(lexertk
        PUBLIC ${absolute_header_files}
        )

set_target_properties(lexertk PROPERTIES PUBLIC_HEADER "${absolute_header_files}")

target_link_libraries(lexertk INTERFACE fmt::fmt)

target_include_directories(lexertk INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>)


install(TARGETS lexertk
        EXPORT ${CMAKE_PROJECT_NAME}Targets
        ARCHIVE
        COMPONENT bin
        PUBLIC_HEADER
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lexertk
        COMPONENT dev
        )
