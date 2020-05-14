###################################################################################################
# Proyecto 	: HMI.pro - Programa de supervision y control para el regulador de turbina
#
# Autor		: DAM-12-05-2020 - Codificacion inicial
###################################################################################################

QMAKE_CXXFLAGS += -Wno-deprecated-copy

####Modulos QT requeridos
QT += widgets

####Recursos comunes todas las distribuciones
RESOURCES  = HMI.qrc

####Configuraciones comunes todas las distribuciones
INCLUDEPATH += ../includes
TARGET = HMI

CONFIG(debug, debug|release) {
	OBJECTS_DIR	= ./build/temp/HMI/debug
} else {
	OBJECTS_DIR	= ./build/temp/HMI/release
}

##Directorios de trabajo
MOC_DIR 	= ./build/temp/HMI
UI_DIR 		= ./build/temp/HMI
RCC_DIR 	= ./build/temp/HMI

##Descripcion Target
VERSION = 1.0

####Especificos para version Windows
win32 {
	message(HMI.pro - Compilacion para Windows con VS-2017)
	TEMPLATE  = vcapp
	
	#LIBS += -L$$PWD/../Lib

	#CONFIG(debug, debug|release) {
		#LIBS += -lNCReportDebug2 
		#-lQt5Xlsxd
	#} else {
		#LIBS += -lNCReport2 
		#-lQt5Xlsx
	#}
	
	#Archivo de recursos con icono de aplicacion
	#RC_ICONS += ..\\Resources\\Images\ControlMotor.ico
	
	# Recursos especificos para version Windows (Estilos)
	#RESOURCES+= ControlMotor_win.qrc
} ####win32

####Especificos para version Linux GNU G++
linux-g++ {
	message(HMI.pro - Compilacion para LINUX G++)
	TEMPLATE = app
	LIBS += -L../libs -lmodbus -lxml2 -lz -llzma -lm -lpthread
	QMAKE_LFLAGS += -Wl,-rpath,../libs
	
	# Recursos especificos para version Linux (Estilos)
	#RESOURCES+= HMI_linux.qrc
	
	#QMAKE_CXXFLAGS+= -fexceptions -mthreads
} ####linux-g++

SOURCES += \
    ./src/main.cpp \
    ./src/MainWindow.cpp \
    ./src/MasterDriver/ModbusMasterDrv.cpp \
    ./src/ControlWdg.cpp \
    ./src/ControlOptWdg.cpp \
    ./src/AlarmsWdg.cpp \
    ./src/TurbineViewWdg.cpp \
    ./src/TendencyWdg.cpp \
    ./src/VarsViewWdg.cpp

HEADERS += \
    ./src/MainWindow.h \
    ./src/MasterDriver/ModbusMasterDrv.h \
    ./src/ControlWdg.h \
    ./src/ControlOptWdg.h \
    ./src/AlarmsWdg.h \
    ./src/TurbineViewWdg.h \
    ./src/TendencyWdg.h \
    ./src/VarsViewWdg.h

FORMS += \
    ./src/MainWindow.ui \
    ./src/ControlWdg.ui \
    ./src/ControlOptWdg.ui \
    ./src/AlarmsWdg.ui \
    ./src/TurbineViewWdg.ui \
    ./src/TendencyWdg.ui \
    ./src/VarsViewWdg.ui



 
