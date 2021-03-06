#include "../headers/response.hpp"
#include "../headers/cgi.hpp"
Response::Response(std::vector<dataserver> server, Request &request, int port) : _request(request), _data_servers(server)
{
	_status = _request.get_status();
	_headers = "HTTP/1.1 ";
	_body = "";
	_index_path = "";
	_autoIndex_page = "";
	_dir_path = "";
	_autoIndex = false;
	_not_found = false;
	_is_location = false;
	_port = 0;
	_redirected_location = "";
	_cgi_body = "";
	this->_errors[200] = "OK";
	this->_errors[301] = "Moved Permanently";
	this->_errors[307] = "Temporary Redirect";
	this->_errors[400] = "Bad Request";
	this->_errors[403] = "Forbidden";
	this->_errors[404] = "Not Found";
	this->_errors[405] = "Not Allowed";
	this->_errors[411] = "Length Required";
	this->_errors[413] = "Request Entity Too Large";
	this->_errors[414] = "Uri Too Long";
	this->_errors[415] = "Unsupported Media Type";
	this->_errors[500] = "Internal Server Error";
	this->_errors[501] = "Not Implemented";
	this->_errors[502] = "Bad Gateway";
	this->_errors[504] = "Gateway Timeout";
	this->_errors[505] = "HTTP Version Not Supported";
}
void Response::build_error_header(int status)
{
	time_t rawTime;
	std::string tm;

	this->_headers.clear();
	time(&rawTime);
	tm = ctime(&rawTime);
	tm.pop_back();
	this->_headers.append(this->_request.get_protocol());
	this->_headers.append(" ");
	this->_headers.append(std::to_string(_status));
	this->_headers.append(" ");
	this->_headers.append(this->_errors[_status]);
	this->_headers.append("\r\n");
	this->_headers.append("Server: webServ\r\n");
	this->_headers.append("Date: " + tm.append(" GMT"));
	this->_headers.append("\r\n");
	this->_headers.append("Connection: " + _request.get_header_value("Connection:"));
	this->_headers.append("\r\n");
	this->_headers.append("Content-Type: text/html; charset=UTF-8");
	this->_headers.append("\r\n");
	this->_headers.append("Content-Length: " + std::to_string(_body.length()));
	this->_headers.append("\r\n\r\n");
	this->_headers.append(_body);
}
void Response::parse_cgi_header(std::string &cgiResp)
{
	std::string buffer;
	std::istringstream s(cgiResp);
	time_t rawTime;
	std::string tm;

	time(&rawTime);
	tm = ctime(&rawTime);
	tm.pop_back();
	this->_headers.append(std::to_string(_status));
	this->_headers.append(" ");
	this->_headers.append(this->_errors[_status]);
	this->_headers.append("\r\n");
	this->_headers.append("Server: webServ\r\n");
	this->_headers.append("Date: " + tm.append(" GMT"));
	this->_headers.append("\r\n");
	this->_headers.append("Connection: " + _request.get_header_value("Connection:"));
	this->_headers.append("\r\n");
	if (_cgi_location.getLocationExtention() == "php")
	{
		while (std::getline(s, buffer))
		{
			if (buffer.find("X-Powered-By:") != npos)
			{
				this->_headers.append("X-Powered-By: " + buffer.substr(buffer.find(": ") + 2));
			}
			else if (buffer.find("Set-Cookie:") != npos)
			{
				this->_headers.append("Set-Cookie: " + buffer.substr(buffer.find(": ") + 2));
			}
			else if (buffer.find("Expires:") != npos)
			{
				this->_headers.append("Expires: " + buffer.substr(buffer.find(": ") + 2));
			}
			else if (buffer.find("Cache-Control:") != npos)
			{
				this->_headers.append("Cache-Control: " + buffer.substr(buffer.find(": ") + 2));
			}
			else if (buffer.find("Pragma:") != npos)
			{
				this->_headers.append("Pragma: " + buffer.substr(buffer.find(": ") + 2));
			}
			else if (buffer.find("Content-type:") != npos)
			{
				this->_headers.append("Content-type: " + buffer.substr(buffer.find(": ") + 2));
			}
			else if (buffer.compare("\r\n\r\n") == 0)
				break;
		}
	}
	else if (_cgi_location.getLocationExtention() == "py")
	{
		while (std::getline(s, buffer))
		{
			if (buffer.find("Content-type:") != npos)
			{
				this->_headers.append("Content-type: " + buffer.substr(buffer.find(":") + 1));
				this->_headers.pop_back();
			}
		}
		this->_body = cgiResp.substr(cgiResp.find("\r\n") + 2);
	}
	this->_body = cgiResp.substr(cgiResp.find("\r\n\r\n") + 4);
	this->_headers.append("\r\n");
	this->_headers.append("Content-Length: " + std::to_string(_body.size()));
	this->_headers.append("\r\n\r\n");
	this->_headers.append(_body);
}
void Response::read_error_file(std::string error_path)
{
	std::ifstream file(error_path);
	if (file)
	{

		std::ostringstream ss;
		ss << file.rdbuf();
		_body = ss.str();
	}
	else
		read_default_error_file(_status);
}
void Response::read_default_error_file(int status)
{
	std::string error = "<!DOCTYPE html>\n\
						 <html>\n\
						 <head>\n\
						 <meta charset=\"UTF-8\" />\n\
						 <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" />\n\
						 <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n\
						 <title>Document</title>\n\
						 <style>\n\
						 .container {\n\
							 margin: 10%;\n\
								 text-align: center;\n\
								 color: rgb(0, 0, 0);\n\
						 }\n\
	h1 {\n\
		font-size: 3rem;\n\
			font-family: \"Courier New\", Courier, monospace;\n\
			font-weight: bold;\n\
			margin:-5rem 0 0 0;\n\
	}\n\
	.parag {\n\
		margin:0;\n\
			font-weight: bold;\n\
			font-size: 2rem;\n\
			font-family: \"Courier New\", Courier, monospace;\n\
	}\n\
	</style>\n\
		</head>\n\
		<body>\n\
		<div class=\"container\">\n\
		<h1>\"status code : $1\"</h1>\n\
		<p class=\"parag\">$2</p>\n\
		</div>\n\
		</body>\n\
		</html>";
	error.replace(error.find("$1"), 2, std::to_string(status));
	error.replace(error.find("$2"), 2, _errors[status]);
	_body = error;
}
void Response::set_error_page(int code)
{
	_status = code;
	if (data_server.getError_page().find(code) != data_server.getError_page().end())
	{
		read_error_file(data_server.getError_page()[_status]);
	}
	else
	{
		read_default_error_file(_status);
	}

	build_error_header(_status);
}
bool Response::is_cgi()
{
	for (std::map<std::string, location>::iterator i = data_server.Location.begin(); i != data_server.Location.end(); i++)
	{
		if ((i->first.find("php") != npos || i->first.find("py") != npos) && _request.get_url().find(i->second.getLocationExtention()) != npos)
		{
			if (!i->second.isCgi)
				return false;
			else
			{
				_cgi_location = i->second;
				return true;
			}
		}
	}
	return false;
}
bool Response::is_directory(const std::string &path)
{
	std::string s = path;
	DIR *r;

	if ((r = opendir(s.c_str())))
	{
		closedir(r);
		return true;
	}
	return false;
}

void Response::read_file(std::string file_path)
{
	std::ostringstream streambuff;
	// exit(1);
	if (access(file_path.c_str(), F_OK) != 0)
	{
		set_error_page(NOT_FOUND);
	}
	else
	{
		if (access(file_path.c_str(), R_OK) == 0)
		{
			std::ifstream file(file_path);
			if (file)
			{
				std::ostringstream ss;
				ss << file.rdbuf();
				_body = ss.str();
			}
			else
				set_error_page(INTERNAL_SERVER_ERROR);
			file.close();
		}
		else
			set_error_page(FORBIDEN);
	}
}
std::string Response::getHtmlCode()
{
	std::string htmlcode = "<!DOCTYPE html>\n\
							<html>\n\
							<head>\n\
							<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
							<title>WEBSERV</title>\n\
							<style>\n\
							.container{margin:0;top:0;}\n\
							.div{font-size: 2.5rem;text-align: center;margin-top: 10%;color:black;}\n\
							</style>\n\
							</head>\n\
							<body>\n\
							<div class=\"container\">\n\
							<div class=\"div\">\n\
							<h1>this is my web-server</h1>\n\
							</div>\n\
							</div>\n\
							</body>\n\
							</html>";

	return htmlcode;
}
std::string Response::getContentType()
{
	std::string extension = _request.get_url().substr(_request.get_url().rfind(".") + 1);
	if (_request.get_header_value("Content-Type:").size())
		return _request.get_header_value("Content-Type:");
	else if (extension.compare("html") == 0 || extension.compare("php") == 0 || extension.compare("py") == 0)
		return "text/html; charset=UTF-8";
	else if (extension.compare("json") == 0)
		return "application/json";
	else if (extension.compare("ico") == 0)
		return "image/x-icon";
	else if (extension.compare("jpeg") == 0)
		return "image/jpeg";
	else if (extension.compare("jpg") == 0)
		return "image/jpg";
	else if (extension == "mp4")
		return "video/mp4";
	else if (extension == "aac")
		return "audio/aac";
	else if (extension == "css")
		return "text/css";
	else if (extension == "gif")
		return "image/gif";
	else
		return "text/plain";
}
void Response::build_header()
{
	time_t rawTime;
	std::string tm;

	time(&rawTime);
	tm = ctime(&rawTime);
	tm.pop_back();
	this->_headers.append(std::to_string(_status));
	this->_headers.append(" ");
	this->_headers.append(this->_errors[_status]);
	this->_headers.append("\r\n");
	if (_status == MOVED_PERMANENTLY)
	{
		this->_headers.append("Location: " + _location.getL_Return_value());
		this->_headers.append("\r\n");
		this->_headers.append("\r\n\r\n");
	}
	else
	{
		this->_headers.append("Server: webServ\r\n");
		this->_headers.append("Date: " + tm.append(" GMT"));
		this->_headers.append("\r\n");
		this->_headers.append("Connection: " + _request.get_header_value("Connection:"));
		if (_location.getL_AutoIndex() == false)
		{
			this->_headers.append("\r\n");
			this->_headers.append("Content-Type: " + getContentType());
		}
		this->_headers.append("\r\n");
		this->_headers.append("Content-Length: " + std::to_string(_body.length()));
		if (_request.get_header().count("Cookie:"))
		{
			this->_headers.append("\r\n");
			this->_headers.append("Set-cookie: " + _request.get_header_value("Cookie"));
		}
		this->_headers.append("\r\n\r\n");
		this->_headers.append(_body);
	}
}
std::string Response::autoindex_run(std::string rooted_path)
{
	DIR *directory = opendir(rooted_path.c_str());
	struct dirent *en;
	std::string fileName;
	std::string _autoIndexPage;
	_autoIndexPage = "<!DOCTYPE html>\n<html lang=\"en\">\n\
					  <head>\n\
					  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
					  <title>AUTO INDEX</title>\n\
					  </head>\n\
					  <body>\n\
					  <div style=\"margin-left: 5%; margin-top:10%;\">\n\
					  <hr>\n";

	if (directory)
	{
		while ((en = readdir(directory)) != nullptr)
		{
			fileName = en->d_name;
			if (en->d_type == DT_DIR)
				fileName.append("/");
			std::string url = _request.get_url();
			std::string str;
			if (url[0] != '/')
				str = url + fileName;
			else if (url == "/")
				str = fileName;
			else
				str = url + '/' + fileName;
			_autoIndexPage.append("\t\t\t<p><a href=\"" + str + "\">" + fileName + "</a></p>\n");
		}
		closedir(directory);
	}
	_autoIndexPage += "\
					   <hr>\n\
					   </div>\n\
					   </body>\n\
					   </html>\n";
	return _autoIndexPage;
}
std::string Response::get_root()
{
	if (_LocExist && _location.isRoot)
		return _location.getL_Root();
	else
		return data_server.getRoot();
}
void Response::get_method()
{
	std::string rooted_path = get_root() + _request.get_url();
	if (_LocExist)
	{
		if (is_directory(rooted_path) == true)
		{
			if (_location.getL_AutoIndex() == false)
			{
				if (_location.getL_Index().size())
				{
					_request.setUrl(_location.getL_Index());
					if (rooted_path[rooted_path.length() - 1] != '/' && _location.getL_Index()[0] != '/')
						rooted_path = rooted_path + "/" + _location.getL_Index();
					else if (rooted_path[rooted_path.length() - 1] == '/' && _location.getL_Index()[0] == '/')
					{
						rooted_path.pop_back();
						rooted_path = rooted_path + _location.getL_Index();
					}
					else
						rooted_path = rooted_path + _location.getL_Index();
					read_file(rooted_path);
				}
				else
				{
					_body = getHtmlCode();
					_request.setUrl(_request.get_url() + ".html");
				}
			}
			else if (_location.getL_AutoIndex() == true)
				_body = autoindex_run(rooted_path);
		}
		else
			read_file(rooted_path);
	}
	else
		read_file(rooted_path);
}
std::string Response::find_file_name(std::string dispo)
{
	std::string tmp = dispo.substr(dispo.find("filename=") + 10);
	tmp = tmp.substr(0, tmp.find("\""));
	return tmp;
}
std::string Response::get_upload_path()
{

	if (_location.get_L_upload_store().size())
	{
		std::string upload;
		if (_location.get_L_upload_store()[0] != '/')
			upload = '/' + _location.get_L_upload_store();
		else
			upload = _location.get_L_upload_store();
		return get_root() + upload;
	}
	return get_root();
}

void Response::post_method()
{
	std::string file_path;
	std::string buffer;
	for (size_t i = 0; i < _request.getBodys().size(); i++)
	{
		if (!_location.get_L_upload_enb())
			set_error_page(UNAUTHORIZED);
		else
		{
			file_path = get_upload_path();
			if (is_directory(file_path) == false)
				set_error_page(NOT_FOUND);
			else
			{
				std::string dispo = file_path + '/' + find_file_name(_request.getBodys()[i].Content_Disposition);
				if (access(dispo.c_str(), F_OK) == 0 && access(dispo.c_str(), W_OK) != 0)
				{
					set_error_page(FORBIDEN);
					return;
				}
				std::ofstream file(dispo);
				std::stringstream ss(_request.getBodys()[i].content);
				while (std::getline(ss, buffer))
				{
					file << buffer.append("\n");
				}
				file.close();
				_body = "<html><head><body><div><h5>File Uploaded successfully</h5></div></body></head></html>";
			}
		}
	}
}
bool Response::find_location()
{
	std::string uri = _request.get_url();
	if (uri[0] != '/')
		uri = "/" + uri;
	std::map<std::string, location>::iterator it = data_server.Location.begin(); // Location.begin();
	// looking for exact match
	for (; it != data_server.Location.end(); it++)
	{
		if (it->first == uri)
		{
			_location = it->second;
			return (true);
		}
	}

	// no exact match found: looking for the longest match
	it = data_server.Location.begin();
	size_t pos = uri.length();

	while (pos != 0)
	{
		pos = uri.rfind("/", pos - 1);
		for (; it != data_server.Location.end(); it++)
		{
			// if (strncmp(uri.c_str(), it->first.c_str(), pos) == 0)
			if (it->first.compare(uri.substr(0, pos)) == 0)
			{
				_location = it->second;
				return true;
			}
			if (it->first == "/")
			{
				_location = it->second;
				return true;
			}
		}
	}
	return (false);
}
void Response::delete_method()
{
	std::string directoryPath = get_root() + _request.get_url();

	if (is_directory(directoryPath))
		set_error_page(NOT_FOUND);
	else
	{
		if (access(directoryPath.c_str(), F_OK) != 0)
			set_error_page(NOT_FOUND);
		else
		{
			if (access(directoryPath.c_str(), W_OK) == 0)
			{
				if (std::remove(directoryPath.c_str()) != 0)
					set_error_page(INTERNAL_SERVER_ERROR);
			}
			else
				set_error_page(FORBIDEN);
		}
	}
}
void Response::handle_cgi()
{
	struct stat st;
	std::map<std::string, int> test;
	std::string filePath = get_root() + _request.get_url();
	test = _cgi_location.getL_Allowed_Methods();

	if (!test[_request.get_method()])
		set_error_page(METHOD_NOT_ALLOWED);
	if (access(filePath.c_str(), F_OK) == 0)
	{
		if (access(filePath.c_str(), R_OK) == 0 && access(filePath.c_str(), W_OK) == 0)
		{
			if (stat(this->_cgi_location.getL_Fastcgi_Pass().c_str(), &st) == -1)
				set_error_page(INTERNAL_SERVER_ERROR);
			else
			{
				_body = LaunchCGI(_cgi_location, filePath, _request);
				parse_cgi_header(_body);
			}
		}
		else
			set_error_page(FORBIDEN);
	}
	else
		set_error_page(NOT_FOUND);
}
void Response::generate_response()
{
	_LocExist = find_location();

	if (_LocExist && _location.getL_Return_value().size())
	{
		_status = MOVED_PERMANENTLY;
		build_header();
		return;
	}
	if (_LocExist && is_cgi())
	{
		handle_cgi();
	}
	else
	{
		std::map<std::string, int> test;
		test = _location.getL_Allowed_Methods();
		if (!test[_request.get_method()])
		{
			set_error_page(METHOD_NOT_ALLOWED);
			return ;
		}
		if (_request.get_method().compare("GET") == 0)
			get_method();
		else if (_request.get_method().compare("POST") == 0)
			post_method();
		else if (_request.get_method().compare("DELETE") == 0)
			delete_method();
		if (_status == OK)
			build_header();
	}
}

void Response::FindServer()
{
	std::string host = _request.get_header_value("Host:");
	std::string server_name = host.substr(0, host.find(':'));
	int port;
	if (host.find(':') == npos)
		port = 80;
	else
		port = std::stoi(host.substr(host.find(':') + 1));

	size_t i = this->_data_servers.size();
	for (size_t j = 0; j < i; j++)
	{
		if (server_name == _data_servers[j].getServer_name())
		{
			for (std::vector<int>::iterator it = _data_servers[j].getListens().begin(); it != _data_servers[j].getListens().begin(); it++)
			{
				if (*it == port)
				{
					serverIndex = j;
					return;
				}
			}
		}
	}
	for (size_t j = 0; j < i; j++)
	{
		for (std::vector<int>::iterator it = _data_servers[j].getListens().begin(); it != _data_servers[j].getListens().begin(); it++)
		{
			if (*it == port)
			{
				serverIndex = j;
				return;
			}
		}
	}
	serverIndex = 0;
}
void Response::init_response()
{
	FindServer();
	data_server = _data_servers[serverIndex];
	if (_status == OK)
	{
		generate_response();
	}
	else
		set_error_page(_status);
}
std::string Response::getHeader()
{
	return this->_headers;
}
Response::~Response()
{
	_status = 200;
	_headers.clear();
	_body.clear();
	_index_path.clear();
	_autoIndex_page.clear();
	_dir_path.clear();
	_autoIndex = false;
	_not_found = false;
	_is_location = false;
	_port = 0;
	_redirected_location.clear();
	_cgi_body.clear();
}

void Response::setLocExist(bool bl)
{
	_LocExist = bl;
}

bool Response::getLocExist()
{
	return _LocExist;
}
