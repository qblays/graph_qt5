QT += widgets
CONFIG += release c++1z
QMAKE_CXXFLAGS += --fast-math -O3 -funroll-loops
HEADERS       = window.h \
    approximator.h \
    graphMethod.h
SOURCES       = main.cpp \
                approximator.cpp \
                window.cpp \
                trap.cpp
