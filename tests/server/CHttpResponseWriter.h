#ifndef CHTTPRESPONSEWRITER_H_
#define CHTTPRESPONSEWRITER_H_

#include <sstream>
#include <fstream>
#include <iostream>

#include <boost/array.hpp>
#include "CBaseHttpResponseWriter.h"

class CHttpResponseWriter: public CBaseHttpResponseWriter
{
	void putBasicHttpHeader(std::stringstream &strm);
public:
	CHttpResponseWriter(tcp::socket* sock) :
			CBaseHttpResponseWriter(sock)
	{
	}

	virtual ~CHttpResponseWriter()
	{
	}
	virtual void writeStatus(std::string status);
	virtual void writeString(std::string message, std::string contentType =
			"text/plain");
	virtual void writeJSON(std::string jsonContent);
	virtual void writeFile(std::string filePath);
};

void CHttpResponseWriter::writeStatus(std::string status)
{
	boost::system::error_code ignored_error;

	status = "HTTP/1.1 " + status;
	status +="\r\n\r\n";

	boost::asio::write(*sock, boost::asio::buffer(status),
			ignored_error);
}

void CHttpResponseWriter::writeString(std::string message,
		std::string contentType)
{
	std::stringstream strm;
	boost::system::error_code ignored_error;

	putBasicHttpHeader(strm);

	strm << "Content-Type: " << contentType << "\r\n";
	strm << "Connection: close\r\n";
	strm << "Content-Length: " << message.size() << "\r\n\r\n";
	strm << message << "\r\n";

	boost::asio::write(*sock, boost::asio::buffer(strm.str()), ignored_error);
}

void CHttpResponseWriter::putBasicHttpHeader(std::stringstream &strm)
{
	strm << "HTTP/1.1 200 OK\r\n";
	strm << "Server: PQTestServer\r\n";
	strm << "Cache-Control: no-cache\r\n";
	strm << "Pragma: no-cache\r\n";
}

void CHttpResponseWriter::writeJSON(std::string jsonContent)
{
	writeString(jsonContent, "application/json");
}

void CHttpResponseWriter::writeFile(std::string filePath)
{
	std::string shortFileName;

	std::size_t found = filePath.find_last_of("/\\");
	if (found != std::string::npos)
	{
		shortFileName = filePath.substr(found + 1);
	}
	else
	{
		shortFileName = filePath;
	}

	std::stringstream strm;
	boost::system::error_code error;

	std::ifstream source_file(filePath,
			std::ios_base::binary | std::ios_base::ate);

	if (!source_file)
	{
		std::cout << "Cannot open a file \"" << filePath << "\". Skipped.\n";

		return;
	}

	size_t fileLength = source_file.tellg();
	source_file.seekg(0);

	putBasicHttpHeader(strm);

	strm << "Content-Type: application/force-download\r\n";
	strm << "Content-Disposition: attachment; filename=\"" << shortFileName
			<< "\"\r\n";
	strm << "Content-Transfer-Encoding: binary\r\n";
	strm << "Accept-Ranges: bytes\r\n";
	strm << "Content-Length: " << fileLength << "\r\n\r\n";

	boost::asio::write(*sock, boost::asio::buffer(strm.str()), error);

	boost::array<char, 2048> buf;
	while (true)
	{
		if (source_file.eof() == false)
		{
			source_file.read(buf.c_array(), (std::streamsize) buf.size());
			if (source_file.gcount() <= 0)
			{
				std::cout << "read file error " << std::endl;
				break;
			}

			boost::asio::write(*sock,
					boost::asio::buffer(buf.c_array(), source_file.gcount()),
					boost::asio::transfer_all(), error);

			if (error)
			{
				std::cout << "send error:" << error << std::endl;
				break;
			}
		}
		else
		{
			std::cout << "send file " << filePath
					<< " completed successfully.\n";
			break;
		}
	}
}

#endif /* CHTTPRESPONSEWRITER_H_ */
