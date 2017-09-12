#include <boost/asio.hpp>

#include "CBaseHttpRequest.h"
#include "CHttpResponseWriter.h"
#include "HttpMockServer.h"

using namespace boost::asio;
using namespace boost::asio::ip;

int Server::gtcnt = 0;

void Server::run()
{
	int port = 8080;
	io_service io_serv;

	tcp::acceptor acceptor(io_serv, tcp::endpoint(tcp::v4(), port));

	for (;;)
	{
		tcp::socket socket(io_serv);
		acceptor.accept(socket);

		CHttpRequest request(&socket);
		std::string route = request.getRoute();

		CHttpResponseWriter responseWriter(&socket);

		if (route.compare(route_500_error) == 0)
		{
			responseWriter.writeStatus("500 Internal Server Error");
		}
		else if (route.compare(route_bad_json) == 0)
		{
			responseWriter.writeJSON("{{}");
		}
		else if (route.compare(route_timeout) == 0)
		{
			sleep(10);
			responseWriter.writeStatus("200 OK");
		}
		else if (route.compare(route_no_csp_no_csp_ro) == 0)
		{
			responseWriter.writeJSON("{\"csp\":15}");
		}
		else if (route.compare(route_get_method) == 0
				&& request.getMethod() == CBaseHttpRequest::RequestMethod::GET)
		{
			responseWriter.writeStatus("200 OK");
		}
		else if (route.compare(route_check_headers) == 0)
		{
			responseWriter.writeString(request.getString(), "text/plain");
		}
		else
		{
			responseWriter.writeStatus("400 Bad Request");
		}
	}
}
