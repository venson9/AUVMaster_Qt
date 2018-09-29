#-------------------------------------------------
#
# Project created by QtCreator 2018-06-10T00:50:22
#
#-------------------------------------------------

QT       += core gui
QT += serialport
QT += charts

CONFIG += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AUVMaster
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    controlpane.cpp \
    logpane.cpp \
    opencvpane.cpp \
    chartpane.cpp \
    communication.cpp \
    propeller.cpp \
    autocontrol.cpp \
    sepintegralPID.cpp \
    normalPID.cpp \
    switchled.cpp \
    task_cruise.cpp \
    task_light.cpp \
    vision.cpp \
    funcs.cpp \
    task_color.cpp \
    task_test.cpp

HEADERS  += mainwindow.h \
    controlpane.h \
    logpane.h \
    opencvpane.h \
    defines.h \
    chartpane.h \
    communication.h \
    propeller.h \
    autocontrol.h \
    structs.h \
    sepintegralPID.h \
    normalPID.h \
    switchled.h \
    task.hpp \
    task_cruise.h \
    task_light.h \
    vision.h \
    funcs.h \
    task_color.h \
    Digraph.hpp \
    DigraphNode.hpp \
    DigraphArc.hpp \
    task_test.h

FORMS    += mainwindow.ui \
    controlpane.ui

##Opencv2.4.9================================
#INCLUDEPATH += D:\opencv\build\include\

#CONFIG(debug, debug|release): {
#LIBS += -LD:/opencv/build/x64/vc12/lib \
#-lopencv_core249d \
#-lopencv_imgproc249d \
#-lopencv_highgui249d \
#-lopencv_ml249d \
#-lopencv_video249d \
#-lopencv_features2d249d \
#-lopencv_calib3d249d \
#-lopencv_objdetect249d \
#-lopencv_contrib249d \
#-lopencv_legacy249d \
#-lopencv_flann249d
#} else:CONFIG(release, debug|release): {
#LIBS += -LD:/opencv/build/x64/vc12/lib \
#-lopencv_core249 \
#-lopencv_imgproc249 \
#-lopencv_highgui249 \
#-lopencv_ml249 \
#-lopencv_video249 \
#-lopencv_features2d249 \
#-lopencv_calib3d249 \
#-lopencv_objdetect249 \
#-lopencv_contrib249 \
#-lopencv_legacy249 \
#-lopencv_flann249
#}

#OpenCV3.2===========================================
INCLUDEPATH += D:\opencv3.2\mybulid\install\include

LIBS += -LD:\opencv3.2\mybulid\install\x64\vc14\lib \
-lopencv_aruco320 \
-lopencv_bgsegm320 \
-lopencv_bioinspired320 \
-lopencv_calib3d320 \
-lopencv_ccalib320 \
-lopencv_core320 \
-lopencv_datasets320 \
-lopencv_dnn320 \
-lopencv_dpm320 \
-lopencv_face320 \
-lopencv_features2d320 \
-lopencv_flann320 \
-lopencv_fuzzy320 \
-lopencv_highgui320 \
-lopencv_imgcodecs320 \
-lopencv_imgproc320 \
-lopencv_line_descriptor320 \
-lopencv_ml320 \
-lopencv_objdetect320 \
-lopencv_optflow320 \
-lopencv_phase_unwrapping320 \
-lopencv_photo320 \
-lopencv_plot320 \
-lopencv_reg320 \
-lopencv_rgbd320 \
-lopencv_saliency320 \
-lopencv_shape320 \
-lopencv_stereo320 \
-lopencv_stitching320 \
-lopencv_structured_light320 \
-lopencv_superres320 \
-lopencv_surface_matching320 \
-lopencv_text320 \
-lopencv_tracking320 \
-lopencv_video320 \
-lopencv_videoio320 \
-lopencv_videostab320 \
-lopencv_xfeatures2d320 \
-lopencv_ximgproc320 \
-lopencv_xobjdetect320 \
-lopencv_xphoto320

#维视相机
INCLUDEPATH += $$quote(D:\Program Files\Microvision\E_EM\SDK\include)

LIBS += $$quote(D:/Program Files/Microvision/E_EM/SDK/lib/MVCamProptySheet.lib)
LIBS += $$quote(D:/Program Files/Microvision/E_EM/SDK/lib/MVGigE.lib)
LIBS += $$quote(D:/Program Files/Microvision/E_EM/SDK/lib/MVTickDlg.lib)
LIBS += -lgdi32
