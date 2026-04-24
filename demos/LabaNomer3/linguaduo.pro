QT += core gui widgets
# Подключится автоматически, если установлен qt6-multimedia-dev.
# Без него код соберётся без аудио (файл main.cpp защищён __has_include).
qtHaveModule(multimedia): QT += multimedia

CONFIG += c++17
TARGET = linguaduo
TEMPLATE = app

SOURCES += main.cpp

# Копировать 01.mp3 рядом с бинарём, чтобы плеер его нашёл.
CONFIG += file_copies
COPIES += audio
audio.files = 01.mp3
audio.path  = $$OUT_PWD
