HEADERS       = ./headers/glwidget.h \
				./headers/Camera.h \
                ./headers/Window.h \
                ./headers/MainWindow.h \
                ./headers/Logo.h \
				./headers/basicglwidget.h \
				./headers/basicwindow.h\
				./headers/model.h \
				./headers/phongglwidget.h \
				./headers/phongwindow.h \
				./headers/definitions.h \
				./headers/texturingglwidget.h \
				./headers/texturingwindow.h \
				./headers/normalmapglwidget.h \
				./headers/normalmapwindow.h \
				./headers/ssowindow.h \
				./headers/ssowidget.h \
				./headers/raytracingwindow.h \
				./headers/sphere.h

SOURCES       = ./sources/glwidget.cpp \
                ./sources/main.cpp \
				./sources/Camera.cpp \
                ./sources/Window.cpp \
                ./sources/MainWindow.cpp \
                ./sources/Logo.cpp \
				./sources/basicglwidget.cpp \
				./sources/basicwindow.cpp \
				./sources/model.cpp \
				./sources/phongglwidget.cpp \
				./sources/phongwindow.cpp \
				./sources/texturingglwidget.cpp \
				./sources/texturingwindow.cpp \
				./sources/normalmapglwidget.cpp \
				./sources/normalmapwindow.cpp \
				./sources/ssowindow.cpp \
				./sources/ssowidget.cpp \
				./sources/raytracingwindow.cpp

QT           += widgets
FORMS		 = ./forms/basicwindow.ui \
			   ./forms/phongwindow.ui \
			   ./forms/texturingwindow.ui \
			   ./forms/normalmapwindow.ui \
			   ./forms/raytracingwindow.ui
			   
CONFIG += console
INCLUDEPATH += ./glm \
			   ./headers \
			   ./sources \
			   ./forms 
# install
target.path = $$[QT_INSTALL_EXAMPLES]/opengl/hellogl2
INSTALLS += target