include(../config.pri)

TEMPLATE = lib
DESTDIR = ../lib

QT       += core network script
QT       -= gui

CONFIG   += static

DEPENDPATH = .
INCLUDEPATH = .

SOURCES += \
	parser/parser.c \
	parser/http_parser.c \
	HttpServer.cpp \
	HttpHandler.cpp \
	HttpHandlerQtScript.cpp \
	HttpHelpers.cpp \
	HttpsServer.cpp \
	HttpHandlerSimpleRouter.cpp \
	HttpConnection.cpp \
	HttpHandlerProxy.cpp \
    HttpClient.cpp

HEADERS += \
	parser/parser.h \
	parser/http_parser.h \
	HttpServer.h \
	HttpHandler.h \
	HttpHandlerQtScript.h \
	HttpHelpers.h \
	HttpsServer.h \
	HttpHandlerSimpleRouter.h \
	HttpConnection.h \
	HttpHandlerProxy.h \
	ByteArrayHelpers.h \
	private/ByteArray.h \
    HttpClient.h

OTHER_FILES +=
