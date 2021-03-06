add_subdirectory( tests )
add_subdirectory( pics )

include_directories(${KOTEXT_INCLUDES} ${KOODF_INCLUDES} ${PIGMENT_INCLUDES})
include_directories(${CMAKE_SOURCE_DIR}/libs/widgetutils)
if (LIBATTICA_FOUND)
    include_directories(${LIBATTICA_INCLUDE_DIR})
endif (LIBATTICA_FOUND)

set(kowidgets_LIB_SRCS
    KoTagToolButton.cpp
    KoTagChooserWidget.cpp
    KoTagFilterWidget.cpp
    KoResourceTaggingManager.cpp
    KoResourceItemChooserContextMenu.cpp
    KoAspectButton.cpp
    KoCsvImportDialog.cpp
    KoPageLayoutDialog.cpp
    KoPageLayoutWidget.cpp
    KoPagePreviewWidget.cpp
    KoPositionSelector.cpp
    KoSliderCombo.cpp
    KoColorPopupButton.cpp
    KoUnitDoubleSpinBox.cpp
    KoZoomAction.cpp
    KoZoomController.cpp
    KoZoomInput.cpp
    KoZoomHandler.cpp
    KoZoomMode.cpp
    KoDpi.cpp

    KoFileDialog.cpp
    KoColorPatch.cpp
    KoColorPopupAction.cpp
    KoColorSetWidget.cpp
    KoColorSlider.cpp
    KoDualColorButton.cpp
    KoEditColorSetDialog.cpp
    KoTriangleColorSelector.cpp
    KoUniColorChooser.cpp
    KoUniColorDialog.cpp
    KoXYColorSelector.cpp
    KoResourcePopupAction.cpp
    KoStrokeConfigWidget.cpp
    KoFillConfigWidget.cpp
    KoOpacityConfigWidget.cpp
    KoShadowConfigWidget.cpp
    KoIconToolTip.cpp
    KoResourceItemChooser.cpp
    KoResourceItemChooserSync.cpp
    KoResourceSelector.cpp
    KoResourceModel.cpp
    KoResourceItemDelegate.cpp
    KoResourceItemView.cpp
    KoResourceTagStore.cpp
    KoItemToolTip.cpp
    KoCheckerBoardPainter.cpp
    KoResourceServerAdapter.cpp
    KoResourceServerProvider.cpp
    KoLineStyleSelector.cpp
    KoLineStyleItemDelegate.cpp
    KoLineStyleModel.cpp
    KoMarkerModel.cpp
    KoMarkerItemDelegate.cpp
    KoMarkerSelector.cpp
    KoDockWidgetTitleBar.cpp
    KoDockWidgetTitleBarButton.cpp
    KoViewItemContextBar.cpp
    KoContextBarButton.cpp
    KoResourceFiltering.cpp

    KoResourceModelBase.cpp
)

kde4_add_ui_files( kowidgets_LIB_SRCS
    KoCsvImportDialog.ui
    KoPageLayoutWidget.ui
    KoEditColorSet.ui
    KoShadowConfigWidget.ui
)


kde4_add_library(kowidgets SHARED ${kowidgets_LIB_SRCS})

if(GHNS)
    target_link_libraries(kowidgets kotext pigmentcms kowidgetutils ${KDE4_KNEWSTUFF3_LIBS})
else(GHNS)
    target_link_libraries(kowidgets kotext pigmentcms kowidgetutils)
endif (GHNS)

target_link_libraries(kowidgets LINK_INTERFACE_LIBRARIES kotext pigmentcms kowidgetutils ${KDE4_KDEUI_LIBS})

set_target_properties(kowidgets PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)

install(TARGETS kowidgets  ${INSTALL_TARGETS_DEFAULT_ARGS})

install( FILES
    KoResourceItemChooserContextMenu.h
    KoResourceTaggingManager.h
    KoGenericRegistryModel.h
    KoPageLayoutDialog.h
    KoPageLayoutWidget.h
    KoPagePreviewWidget.h
    KoPositionSelector.h
    kowidgets_export.h
    KoZoomAction.h
    KoZoomController.h
    KoZoomInput.h
    KoDpi.h
    KoZoomHandler.h
    KoZoomMode.h

    KoFileDialog.h
    KoColorPatch.h
    KoStrokeConfigWidget.h
    KoFillConfigWidget.h
    KoOpacityConfigWidget.h
    KoShadowConfigWidget.h
    KoColorPopupAction.h
    KoColorSetWidget.h
    KoColorSlider.h
    KoDualColorButton.h
    KoEditColorSetDialog.h
    KoTriangleColorSelector.h
    KoUniColorChooser.h
    KoUniColorDialog.h
    KoXYColorSelector.h
    KoResourceItemChooser.h
    KoResourceSelector.h
    KoResourceServer.h
    KoResourceServerAdapter.h
    KoResourceServerObserver.h
    KoResourceServerProvider.h
    KoResourceTagStore.h
    KoLineStyleSelector.h
    KoDockWidgetTitleBar.h
    KoDockWidgetTitleBarButton.h

    KoResourceModelBase.h

    DESTINATION
    ${INCLUDE_INSTALL_DIR}/calligra
COMPONENT Devel)

set(filedialogtester_SRCS
    KoFileDialogTester.cpp
    main.cpp
)
kde4_add_ui_files(filedialogtester_SRCS
    KoFileDialogTester.ui
)

kde4_add_executable(filedialogtester ${filedialogtester_SRCS})
target_link_libraries(filedialogtester kowidgets)