

set(kritatoolpolygon_PART_SRCS
    tool_polygon.cc
    kis_tool_polygon.cc
    )


kde4_add_plugin(kritatoolpolygon ${kritatoolpolygon_PART_SRCS})



target_link_libraries(kritatoolpolygon kritaui)

install(TARGETS kritatoolpolygon  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_polygon_cursor.png  	tool_polygon.png dark_tool_polygon.png light_tool_polygon.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolpolygon.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

