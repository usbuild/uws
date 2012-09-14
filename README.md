#UWS(Usbuild Web Server)

Introduction
=================================================
UWS is a light weight http server written in C, It can ONLY work under Linux(3.2.0 or upper).It's a practise while reading <Advanced Programming in Unix Environment> and <Unix Net Programming>, just for fun, so DON'T use it in production environment.

Features
=================================================
* Work under HTTP/1.1
* ONLY Support GET and POST Method
* Support 304 status and If-Modified-Since header
* Gzip and deflate support
* Basic Fast-CGI support
* Index module can determine what to show when access a directory
* Vhost, at different port

TODO
================================================
* HTTPS Support
* Rewrite Engine
* Wsgi Support
* Reverse Proxy and Load Distribution
* ...

Depends
===============================================
* zlib1g-dev
* libpcre3-dev

How to Use
===============================================
Clone this respository, Run `make` command in the project directory. `uws.conf` is the configuration file, you can change it to follow your needs.

Copying
==============================================
The software distributes under BSD license, See COPYING file.

Contact
==============================================
* GitHub [http://github.com/usbuild](http://github.com)
* Sina-Weibo [http://weibo.com/usbuild](http://weibo.com/usbuild)
* Email [njuzhangqichao@gmail.com](mailto:njuzhangqichao@gmail.com)
