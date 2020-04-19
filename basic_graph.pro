QT += widgets
TARGET = a.out
CONFIG += c++1z \
          optimize_full
QMAKE_CXXFLAGS -= -O2
#QMAKE_CXX = clang++
#QMAKE_LINK = clang++
QMAKE_CXXFLAGS += -g
#LIBS += -lleaktracer
#QMAKE_CXXFLAGS += -fsanitize=address
#QMAKE_LFLAGS += -lm
#QMAKE_LFLAGS += -fsanitize=address
#QMAKE_CXXFLAGS += -fsanitize=memory -fsanitize-recover=memory -fsanitize-blacklist=blist.txt
#QMAKE_LFLAGS += -fsanitize=memory -fsanitize-recover=memory -fsanitize-blacklist=blist.txt
#QMAKE_CXXFLAGS += -fsanitize=undefined
#QMAKE_LFLAGS += -fsanitize=undefined

QMAKE_CXXFLAGS += -ffast-math -Ofast -funroll-loops -Wextra -pedantic -Wall -Wcast-qual -march=native -lm
HEADERS       = window.h \
    approximator.h \
    graphMethod.h
SOURCES       = main.cpp \
                approximator.cpp \
                window.cpp \
                trap.cpp
