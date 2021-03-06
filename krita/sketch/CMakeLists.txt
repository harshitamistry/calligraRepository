if (WIN32)
    option(ENABLE_SKETCH_STARTUP_MAGIC "Enable Sketch's windows startup magic" ON)

    if (ENABLE_SKETCH_STARTUP_MAGIC)
        add_definitions(-DSKETCH_ENABLE_STARTUP_MAGIC)
    endif (ENABLE_SKETCH_STARTUP_MAGIC)
endif (WIN32)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/..
                    ${CMAKE_CURRENT_SOURCE_DIR}/models
                    ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(draganddrop)

set(kritasketchlib_SRCS
    DocumentManager.cpp
    DocumentListModel.cpp
    KisSelectionExtras.cpp
    KisSketchView.cpp
    KisSketchPart.cpp
    RecentFileManager.cpp
    ProgressProxy.cpp
    PropertyContainer.cpp
    Settings.cpp
    SketchDeclarativeView.cpp
    SketchInputContext.cpp
    VirtualKeyboardController.cpp
    Theme.cpp
    QmlGlobalEngine.cpp
)

set(kritasketch_SRCS
    main.cpp
    MainWindow.cpp
)

set(kritasketchplugin_SRCS
    kritasketchplugin.cpp
    ColorSelectorItem.cpp
    CurveEditorItem.cpp
    Constants.cpp
    ToolManager.cpp
    SimpleTouchArea.cpp
    NetworkAccessManager.cpp
    ImageBuilder.cpp
    KritaNamespace.cpp
    MouseTracker.cpp
    PanelConfiguration.cpp
    PropertyContainer.cpp
)

set(kritasketch_kritaintegrationmodels_SRCS
    models/TemplatesModel.cpp
    models/KeyboardModel.cpp
    models/ColorImageProvider.cpp
    models/CompositeOpModel.cpp
    models/FiltersCategoryModel.cpp
    models/FiltersModel.cpp
    models/LayerCompositeDetails.cpp
    models/LayerModel.cpp
    models/LayerThumbProvider.cpp
    models/PaletteColorsModel.cpp
    models/PaletteModel.cpp
    models/PresetImageProvider.cpp
    models/PresetModel.cpp
    models/RecentImageImageProvider.cpp
    models/RecentImagesModel.cpp
    models/MultiFeedRSSModel.cpp
    models/FileSystemModel.cpp
    models/ColorModelModel.cpp
    models/ColorDepthModel.cpp
    models/ColorProfileModel.cpp
)


set(KRITA_COLORSELECTORNG_PART_SRCS
../plugins/extensions/dockers/advancedcolorselector/kis_my_paint_shade_selector.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_patches.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_history.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_selector.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_common_colors.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_common_colors_recalculation_runner.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_minimal_shade_selector.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_selector_base_proxy.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_shade_selector_line.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_shade_selector_line_editor.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_shade_selector_line_combo_box.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_shade_selector_line_combo_box_popup.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_shade_selector_lines_settings.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_selector_container.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_selector_base.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_selector_component.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_selector_ring.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_selector_triangle.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_selector_simple.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_selector_wheel.cpp
../plugins/extensions/dockers/advancedcolorselector/kis_color_selector_combo_box.cpp
)


if (WIN32)
    set(kritasketch_SRCS ${kritasketch_SRCS} KritaSketchWin.rc)
else (WIN32)
    kde4_add_app_icon(kritasketch_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/kritasketch.png")
endif (WIN32)

# Krita Sketch library with shared things needed in both plugin and application
# =============================================================================

kde4_add_library(kritasketchlib SHARED ${kritasketchlib_SRCS})
set_target_properties(kritasketchlib
    PROPERTIES VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION} DEFINE_SYMBOL MAKE_KRITA_SKETCH_LIB
)


target_link_libraries(kritasketchlib
    ${QT_QTDECLARATIVE_LIBRARY}
    ${QT_QTGUI_LIBRARIES}
    ${QT_QTCORE_LIBRARIES}
    ${QT_LIBRARIES}
    komain
    kritaui
    kowidgets
    ${KDE4_KFILE_LIBRARY}
    ${GLEW_LIBRARIES}
)

include(GNUInstallDirs)

#install(TARGETS   kritasketchlib DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS kritasketchlib ${INSTALL_TARGETS_DEFAULT_ARGS})


# Krita Sketch application
# =============================================================================

kde4_add_executable(kritasketch ${kritasketch_SRCS})

target_link_libraries(kritasketch
    ${QT_QTDECLARATIVE_LIBRARY}
    ${QT_QTGUI_LIBRARIES}
    ${QT_QTCORE_LIBRARIES}
    ${QT_LIBRARIES}
    ${GLEW_LIBRARIES}
    kritaui
    ${GLEW_LIBRARIES}
    kritasketchlib
)

install(TARGETS   kritasketch ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS  kritasketch.desktop               DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES     kritasketchrc kritasketchpanelsrc DESTINATION ${CONFIG_INSTALL_DIR} )
install(FILES     kritasketch.qml                   DESTINATION ${DATA_INSTALL_DIR}/kritasketch)
install(FILES     kritasketch.rc                    DESTINATION ${DATA_INSTALL_DIR}/kritasketch)

# Sketch Qt Quick components
# =============================================================================

automoc4_add_library(kritasketchplugin MODULE ${kritasketchplugin_SRCS} ${kritasketch_kritaintegrationmodels_SRCS} ${KRITA_COLORSELECTORNG_PART_SRCS})

target_link_libraries(kritasketchplugin
    ${QT_QTDECLARATIVE_LIBRARY}
    ${QT_QTGUI_LIBRARIES}
    ${QT_QTCORE_LIBRARIES}
    ${QT_LIBRARIES}
    komain
    kritaui
    kowidgets
    ${KDE4_KFILE_LIBRARY}
    ${GLEW_LIBRARIES}
    kritasketchlib
)

install(TARGETS   kritasketchplugin DESTINATION ${CMAKE_INSTALL_LIBDIR}/calligra/imports/org/krita/sketch)
install(FILES
                  qmldir
                  qml/HelpPage.qml
                  qml/KritaSketchBase.qml
                  qml/MainPage.qml
                  qml/OpenImagePage.qml
                  qml/SaveImagePage.qml
                  qml/SettingsPage.qml
                  qml/WelcomePage.qml
                  qml/CustomImagePage.qml
                                    DESTINATION ${CMAKE_INSTALL_LIBDIR}/calligra/imports/org/krita/sketch)
install(DIRECTORY qml/components    DESTINATION ${CMAKE_INSTALL_LIBDIR}/calligra/imports/org/krita/sketch PATTERN "*~" EXCLUDE)
install(DIRECTORY qml/images        DESTINATION ${CMAKE_INSTALL_LIBDIR}/calligra/imports/org/krita/sketch PATTERN "*~" EXCLUDE)
install(DIRECTORY qml/panels        DESTINATION ${CMAKE_INSTALL_LIBDIR}/calligra/imports/org/krita/sketch PATTERN "*~" EXCLUDE)

install(DIRECTORY themes DESTINATION ${DATA_INSTALL_DIR}/kritasketch)

add_subdirectory(icon)
