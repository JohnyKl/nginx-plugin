#ifndef CBASEHTTPRESPONSEWRITER_H_
#define CBASEHTTPRESPONSEWRITER_H_

#include <boost/asio.hpp>

using namespace boost::asio::ip;

class CBaseHttpResponseWriter
{
protected:
	tcp::socket* sock;
public:
	CBaseHttpResponseWriter(tcp::socket* sock)
	{
		this->sock = sock;
	}
	virtual ~CBaseHttpResponseWriter(){}
	virtual void writeStatus(std::string status) = 0;
	virtual void writeString(std::string message, std::string contentType) = 0;
	virtual void writeJSON(std::string jsonContent) = 0;
	virtual void writeFile(std::string filePath) = 0;
};

#endif
