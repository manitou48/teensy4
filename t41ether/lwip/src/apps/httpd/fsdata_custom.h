#ifndef LWIP_FSDATA_CUSTOM_H
#define LWIP_FSDATA_CUSTOM_H

#include "lwip/init.h"
#include "httpd_structs.h"
#include "fsdata.h"

#define file_NULL (struct fsdata_file *) NULL

const struct fsdata_file file__404_html[] = { {
file_NULL,
"/404.html",
"HTTP/1.0 404 File not found\n"
"Server: " HTTPD_SERVER_AGENT "\n" HTTP_HDR_HTML
"<html>\n"
"<head><title>lwIP - A Lightweight TCP/IP Stack</title></head>\n"
"<body bgcolor='white' text='black'>\n"
" <table width='100%'>\n"
" <tr valign='top'><td width='500'>\n"
"  <h1>lwIP - A Lightweight TCP/IP Stack</h1>\n"
"  <h2>404 - Page not found</h2>\n"
"   <p>\n"
"Sorry, the page you are requesting was not found on this server.\n" 
"   </p>\n"
"  </td><td>&nbsp;</td></tr>\n"
" </table>\n"
"</body>\n"
"</html>",
0,
1,
}};

const struct fsdata_file file__index_html[] = { {
file__404_html,
"/index.html",
"HTTP/1.0 200 OK\n"
"Server: " HTTPD_SERVER_AGENT "\n" HTTP_HDR_HTML
"<html>\n"
"<head><title>lwIP - A Lightweight TCP/IP Stack</title></head>\n"
"<body bgcolor='white' text='black'>\n"
" <table width='100%'>\n"
" <tr valign='top'><td width='500'>\n"
"  <h1>lwIP - A Lightweight TCP/IP Stack</h1>\n"
"   <p>\n"
"The web page you are watching was served by a simple web server running on top of the lightweight TCP/IP stack <a href='http://www.sics.se/~adam/lwip/'>lwIP</a>.\n"
"   </p>\n"
"  </td><td>&nbsp;</td></tr>\n"
" </table>\n"
"</body>\n"
"</html>",
0,
1,
}};

#define FS_ROOT file__index_html
#define FS_NUMFILES 2

#endif