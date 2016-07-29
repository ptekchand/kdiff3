

########### kdiff3 KPart ###############
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Parts CoreAddons)

set(kdiff3part_PART_SRCS 
   kdiff3_part.cpp 
   kdiff3.cpp 
   directorymergewindow.cpp 
   merger.cpp 
   pdiff.cpp 
   difftextwindow.cpp 
   diff.cpp 
   optiondialog.cpp 
   mergeresultwindow.cpp 
   fileaccess.cpp 
   gnudiff_analyze.cpp 
   gnudiff_io.cpp 
   gnudiff_xmalloc.cpp 
   common.cpp 
   smalldialogs.cpp 
   progress.cpp )

add_library(kdiff3part MODULE ${kdiff3part_PART_SRCS})

set_target_properties(kdiff3part PROPERTIES DEFINE_SYMBOL KDIFF3_PART)

target_link_libraries(kdiff3part ${KDiff3_LIBRARIES} KF5::Parts KF5::CoreAddons ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kdiff3part DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/parts )

########### kdiff3 executable ###############

set(kdiff3_SRCS 
   main.cpp 
   kdiff3_shell.cpp
   ${kdiff3part_PART_SRCS}
    )

add_executable(kdiff3 ${kdiff3_SRCS})

target_link_libraries(kdiff3  KF5::Parts ${QT_QT3SUPPORT_LIBRARY} ${KDiff3_LIBRARIES} )

install(TARGETS kdiff3 ${INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( FILES  kdiff3part.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES  kdiff3_part.rc DESTINATION ${DATA_INSTALL_DIR}/kdiff3part )
install( FILES  kdiff3_shell.rc DESTINATION ${DATA_INSTALL_DIR}/kdiff3 )
install( PROGRAMS  kdiff3.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )

ecm_install_icons(ICONS
    hicolor/16-apps-kdiff3.png
    hicolor/32-apps-kdiff3.png
    DESTINATION ${ICON_INSTALL_DIR}
    THEME hicolor
)

ecm_install_icons(ICONS
    lowcolor/16-apps-kdiff3.png
    lowcolor/32-apps-kdiff3.png
    DESTINATION ${ICON_INSTALL_DIR}
    THEME locolor
)

