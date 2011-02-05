#include "HttpHandlerTest.h"
#include "HttpHandler.h"
#include "HttpRequest.h"
#include <QtCore/QBuffer>
#include <QtCore/QCoreApplication>
#include <QtTest/QtTest>
using namespace Pillow;

Pillow::HttpRequest * HttpHandlerTestBase::createGetRequest(const QByteArray &path)
{
	QByteArray data = QByteArray().append("GET ").append(path).append(" HTTP/1.0\r\n\r\n");
	QBuffer* inputBuffer = new QBuffer(); inputBuffer->open(QIODevice::ReadWrite);
	QBuffer* outputBuffer = new QBuffer(); outputBuffer->open(QIODevice::ReadWrite);
	
	Pillow::HttpRequest* request = new Pillow::HttpRequest(inputBuffer, outputBuffer, this);
	connect(request, SIGNAL(completed(Pillow::HttpRequest*)), this, SLOT(requestCompleted(Pillow::HttpRequest*)));
	inputBuffer->setParent(request);
	outputBuffer->setParent(request);
	
	inputBuffer->write(QByteArray().append("GET ").append(path).append(" HTTP/1.0\r\n\r\n"));
	inputBuffer->seek(0);
	
	while (request->state() != Pillow::HttpRequest::SendingHeaders)
		QCoreApplication::processEvents();
	
	return request;
}

void HttpHandlerTestBase::requestCompleted(Pillow::HttpRequest *request)
{
	response = readResponse(request);
}

QByteArray HttpHandlerTestBase::readResponse(Pillow::HttpRequest *request)
{
	QBuffer* buffer = static_cast<QBuffer*>(request->outputDevice());
	buffer->seek(0);
	QByteArray data = buffer->buffer();
	buffer->buffer().clear();
	return data;	
}

class MockHandler : public Pillow::HttpHandler
{
public:
	MockHandler(const QByteArray& acceptPath, int statusCode, QObject* parent)
		: Pillow::HttpHandler(parent), acceptPath(acceptPath), statusCode(statusCode), handleRequestCount(0)
	{}
	
	QByteArray acceptPath;
	int statusCode;
	int handleRequestCount;	
	
	bool handleRequest(Pillow::HttpRequest *request)
	{
		++handleRequestCount;
		
		if (acceptPath == request->requestPath())
		{
			request->writeResponse(statusCode);
			return true;
		}
		return false;
	}
};

void HttpHandlerTest::testHandlerStack()
{
	HttpHandlerStack handler;
	MockHandler* mock1 = new MockHandler("/1", 200, &handler);
	MockHandler* mock1_1 = new MockHandler("/", 403, mock1);
	new QObject(&handler); // Some dummy object, also child of handler.
	MockHandler* mock2 = new MockHandler("/2", 302, &handler);
	MockHandler* mock3 = new MockHandler("/", 500, &handler);
	MockHandler* mock4 = new MockHandler("/", 200, &handler);
	
	bool handled = handler.handleRequest(createGetRequest("/"));
	QVERIFY(handled);
	QVERIFY(response.startsWith("HTTP/1.0 500"));
	QCOMPARE(mock1->handleRequestCount, 1);
	QCOMPARE(mock1_1->handleRequestCount, 0);
	QCOMPARE(mock2->handleRequestCount, 1);
	QCOMPARE(mock3->handleRequestCount, 1);
	QCOMPARE(mock4->handleRequestCount, 0);
	
	handled = handler.handleRequest(createGetRequest("/2"));
	QVERIFY(handled);
	QVERIFY(response.startsWith("HTTP/1.0 302"));
	QCOMPARE(mock1->handleRequestCount, 2);
	QCOMPARE(mock1_1->handleRequestCount, 0);
	QCOMPARE(mock2->handleRequestCount, 2);
	QCOMPARE(mock3->handleRequestCount, 1);
	QCOMPARE(mock4->handleRequestCount, 0);
}

void HttpHandlerTest::testHandlerFixed()
{
	bool handled = HttpHandlerFixed(403, "Fixed test").handleRequest(createGetRequest());
	QVERIFY(handled);
	QVERIFY(response.startsWith("HTTP/1.0 403"));
	QVERIFY(response.endsWith("\r\n\r\nFixed test"));
}

void HttpHandlerTest::testHandler404()
{
	bool handled = HttpHandler404().handleRequest(createGetRequest("/some_path"));
	QVERIFY(handled);
	QVERIFY(response.startsWith("HTTP/1.0 404"));
	QVERIFY(response.contains("/some_path"));	
}

void HttpHandlerTest::testHandlerLog()
{
	QBuffer buffer; buffer.open(QIODevice::ReadWrite);
	Pillow::HttpRequest* request1 = createGetRequest("/first");
	Pillow::HttpRequest* request2 = createGetRequest("/second");
	Pillow::HttpRequest* request3 = createGetRequest("/third");
	
	HttpHandlerLog handler(&buffer, &buffer);
	QVERIFY(!handler.handleRequest(request1));
	QVERIFY(!handler.handleRequest(request2));
	QVERIFY(!handler.handleRequest(request3));
	QVERIFY(buffer.data().isEmpty());
	request3->writeResponse(302);
	request1->writeResponse(200);
	request2->writeResponse(500);
	
	// The log handler should write the log entries as they are completed.
	buffer.seek(0);
	QVERIFY(buffer.readLine().contains("GET /third"));
	QVERIFY(buffer.readLine().contains("GET /first"));
	QVERIFY(buffer.readLine().contains("GET /second"));
	QVERIFY(buffer.readLine().isEmpty());
}

