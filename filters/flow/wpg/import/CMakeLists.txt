include_directories(
    ../../../outputhelper/
    ${LIBODFGEN_INCLUDE_DIRS}
    ${LIBREVENGE_INCLUDE_DIRS}
    ${LIBWPG_INCLUDE_DIRS}
    ${KOMAIN_INCLUDES}
)

set(wpg2odg_PART_SRCS
    WPGImport.cpp
    ../../../outputhelper/StringDocumentHandler.cxx
    ../../../outputhelper/FemtoZip.cxx
    ../../../outputhelper/OutputFileHelper.cxx
)

kde4_add_plugin(calligra_filter_wpg2odg ${wpg2odg_PART_SRCS})

target_link_libraries(calligra_filter_wpg2odg komain ${LIBODFGEN_LIBRARIES} ${LIBREVENGE_LIBRARIES} ${LIBWPG_LIBRARIES})

install(TARGETS calligra_filter_wpg2odg DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES calligra_filter_wpg2odg.desktop DESTINATION ${SERVICES_INSTALL_DIR})

if(SHOULD_BUILD_FILEMANAGER_THUMBNAIL)
    install(FILES flow_wpg_thumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
endif(SHOULD_BUILD_FILEMANAGER_THUMBNAIL)
