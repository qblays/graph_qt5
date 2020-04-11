QT += widgets
TARGET = a.out
CONFIG += c++1z optimize_full
QMAKE_CXXFLAGS -= -O2
QMAKE_CXXFLAGS += --fast-math -O3 -funroll-loops -Wextra -pedantic -Wall -Wcast-qual -march=native
HEADERS       = window.h \
    approximator.h \
    graphMethod.h
SOURCES       = main.cpp \
                approximator.cpp \
                window.cpp \
                trap.cpp
