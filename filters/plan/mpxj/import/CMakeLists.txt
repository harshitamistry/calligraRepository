
include_directories(
    ${CMAKE_BINARY_DIR}/filters/plan/mpxj/planconvert
    ${CMAKE_BINARY_DIR}/filters/plan/mpxj/import
    ${KOMAIN_INCLUDES}
    ${JNI_INCLUDE_DIRS}
)

set(mpxj2plan_PART_SRCS
   mpxjimport.cpp
)


kde4_add_plugin(calligra_filter_mpxj2plan ${mpxj2plan_PART_SRCS})

target_link_libraries(calligra_filter_mpxj2plan komain)
# Note: no need to link to ${JNI_LIBRARIES} currently, JNI not used,
# cmp. "NOTE:" in mpxjimport.cpp

install(TARGETS calligra_filter_mpxj2plan  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES
    calligra_filter_mpp2plan.desktop
    calligra_filter_mpx2plan.desktop
    calligra_filter_planner2plan.desktop
  DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES calligra_planner_mpp.xml DESTINATION ${XDG_MIME_INSTALL_DIR})
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
