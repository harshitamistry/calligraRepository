include_directories(${KOMAIN_INCLUDES})
set(crashhandler_SRCS main.cpp mainwindow.cpp)
kde4_add_ui_files(crashhandler_SRCS wdg_crash_reporter.ui)
kde4_add_executable(crashhandler ${crashhandler_SRCS})
target_link_libraries(crashhandler komain)
install(TARGETS crashhandler ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES KikiNurse_sm.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
