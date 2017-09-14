#ifndef CBASEHTTPREQUEST_H_
#define CBASEHTTPREQUEST_H_

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/asio/read_until.hpp>
#include <iostream>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

class CBaseHttpRequest
{
protected:
	tcp::socket* sock;
	string route;
	string httpVersion;
	string requestStr;
	vector<pair<string, string>> params;
	virtual void readRequest() = 0;

public:

	typedef enum
	{
		POST, GET, UNKNOWN
	} RequestMethod;
protected:
	RequestMethod method;

public:
	CBaseHttpRequest(tcp::socket* sock)
	{
		this->method = UNKNOWN;
		this->sock = sock;
		this->httpVersion = "HTTP/1.1";
	}

	virtual ~CBaseHttpRequest()
	{
	}

	virtual string getRoute()
	{
		return route;
	}

	virtual string getString()
	{
		return requestStr;
	}

	virtual RequestMethod getMethod()
	{
		return method;
	}

	virtual vector<pair<string, string>> getParams()
	{
		return params;
	}

	virtual void setRoute(string route)
	{
		this->route = route;
	}

	virtual void setString(string req)
	{
		this->requestStr = req;
	}

	virtual void setMethod(RequestMethod method)
	{
		this->method = method;
	}

	virtual void setParams(vector<pair<string, string>> params)
	{
		this->params = params;
	}

	virtual void write() = 0;
};

class CHttpRequest: public CBaseHttpRequest
{
	string decodeSpaces(string str);
	virtual void readRequest();
	void parseMethod();
	void parseRoute();
	void parseParams();
public:
	CHttpRequest(tcp::socket* sock) :
			CBaseHttpRequest(sock)
	{
		readRequest();
	}

	virtual ~CHttpRequest()
	{
	}

	virtual vector<pair<string, string>> getParams()
	{
		if (params.size() == 0)
		{
			parseParams();
		}

		return params;
	}

	virtual string getString();

	virtual void write();
};

void CHttpRequest::write()
{
	boost::system::error_code ignored_error;

	boost::asio::write(*sock, boost::asio::buffer(getString()), ignored_error);
}

string CHttpRequest::getString()
{
	if (requestStr.empty() && method != RequestMethod::UNKNOWN)
	{
		requestStr = method == RequestMethod::GET ? "GET " :
						method == RequestMethod::POST ? "POST " : "";

		requestStr += route;

		if (params.size() != 0)
			requestStr += "?";

		for (int i = 0; i < params.size(); i++)
		{
			requestStr += params[i].first + "=" + params[i].second;

			if (i != params.size() - 1)
				requestStr += "&";
		}

		requestStr += httpVersion + "\r\n";
	}

	return requestStr;
}

string CHttpRequest::decodeSpaces(string str)
{
	stringstream s;
	size_t pos = string::npos;

	do
	{
		pos = str.find("%20");

		if (pos != string::npos)
			str = str.replace(pos, 3, " ");

	} while (pos != string::npos);

	return str;
}

void CHttpRequest::readRequest()
{
	boost::asio::streambuf b;
	boost::system::error_code ec;

	boost::asio::read_until(*sock, b, "\r\n\r\n");
	requestStr = boost::asio::buffer_cast<const char*>(b.data());

	parseMethod();
	parseRoute();
}

void CHttpRequest::parseMethod()
{
	if (requestStr.find("GET ") == 0)
	{
		method = RequestMethod::GET;
	}
	else if (requestStr.find("POST ") == 0)
	{
		method = RequestMethod::POST;
	}
	else
	{
		cout << "Unknown request method: " << requestStr;
	}
}

void CHttpRequest::parseRoute()
{
	size_t posStart = requestStr.find('/');
	size_t posEnd = string::npos;

	if (posStart != string::npos)
	{
		posEnd = requestStr.find_first_of(" ?", posStart);

		if (posEnd != string::npos)
		{
			route = requestStr.substr(posStart, posEnd - posStart);
		}
	}
}

void CHttpRequest::parseParams()
{
	size_t posStart = requestStr.find('?');
	size_t posEnd = requestStr.find(" HTTP/1");
	size_t currPos = posStart + 1;

	if (posStart != string::npos && posEnd != string::npos)
	{
		while (currPos < posEnd)
		{
			size_t eqwPos = requestStr.find('=', currPos);
			size_t delimPos = requestStr.find('&', currPos);

			string key;
			string value;

			if (eqwPos != string::npos)
			{
				key = requestStr.substr(currPos, eqwPos - currPos);

				if (delimPos == string::npos)
					delimPos = posEnd;

				value = requestStr.substr(eqwPos + 1, delimPos - eqwPos - 1);
			}

			value = decodeSpaces(value);

			if (!key.empty())
			{
				params.push_back(pair<string, string>(key, value));
			}

			if (delimPos != string::npos)
			{
				currPos = delimPos + 1;
			}
			else
			{
				break;
			}
		}
	}
}

#endif /* CBASEHTTPREQUEST_H_ */
