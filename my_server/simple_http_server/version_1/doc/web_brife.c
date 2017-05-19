/*####################################################
# File Name: web_brife.c
# Author: xxx
# Email: xxx@126.com
# Create Time: 2017-05-18 12:21:57
# Last Modified: 2017-05-18 12:56:34
####################################################*/
1、Web Client 和 Server之间是基于 HTTP交互的

	HTTP ：Hypertext Transfer Protocol
	简单的理解：一个 Web Client(如 浏览器)打开一个到 Server的因特网连接，并请求某些内容。
				Server响应所请求的内容，然后关闭连接，浏览器读到这些内容，将其显示在屏幕上

	Web服务 与 文件检索服务(如 FTP)
		主要区别：Web内容可以用 HTML语言来编写。

		HTML 的作用：告诉浏览器 如何显示内容(文本、图像等)

2、Web内容
	对于 Web Client 和 Server而言，内容 是与一个 MIME类型相关的字节序列。

	(1) Web Server向 Client提供内容的两种方式：
		静态：取一个磁盘文件，将其内容返回给 Client
			磁盘文件 称为 静态内容
		动态：运行一个可执行文件，将其输出结果 返回给Client
			运行可执行文件产生的输出 称为 动态内容

	(2) URL：Universal Resource Locator
		每条 Web Server返回的内容 都是和它管理的某个文件相关联的。每个文件都有一个唯一的名字 -- URL

		静态文件的URL
			http://www.google.com:80/index.html
			表示：因特网主机 www.google.com 上一个名为 /index.html 的 HTML文件，它是由一个监听80端口的 Web Server管理的
			( 端口号是可选的 )

		动态文件的URL
			可以在文件名后包括参数，'?' 用于分隔 文件名 和 参数，参数之间用 '&'分隔
			http://bluefish.ics.cs.cmu.edu:8000/cgi-bin/adder?150&312
			表示：标识了一个 /cgi-bin/adder的可执行文件，带有两个参数 150、312 来调用它

	
	(3) 在事务过程中，Client 和 Server使用的是 URL的不同部分
		http://www.google.com:80/index.html
			Client使用前缀：http://www.google.com:80
				来决定与哪类 Server联系，Server在哪里，及 Server监听的端口号是多少

			Server使用后缀：/index.html
				来查找在它文件系统中的文件，并确定是 静态内容 还是 动态内容

	(4) 关于 Server如何解释 URL的后缀
		a、确定是静态还是动态内容，没有统一的标准。
			一种常见的方法：确定一组目录，如 cgi-bin，所有可执行文件都必须放在该目录下

		b、后缀中那个最开始的 '/'，不表示 Unix根目录
			它表示的是被请求内容类型的主目录
				如：将所有静态内容存放在 /usr/httpd/html目录
					将所有动态内容存放在 /usr/httpd/cig-bin目录

		c、最小的 URL后缀是 '/'，所有服务器将其扩展为某个默认的主页
			如：扩展为 /index.html
				这就解释了为什么输入了一个域名就可以取出一个网站的主页
				浏览器会在 URL后添加缺失的 '/'，并将其传递给Server，Server又将 '/' 扩展为某个默认的文件名

3、用 telnet来和 Web Server执行事务
	每次输入一个文本行，并键入回车，telnet会读取该行，在后面加上 回车 和 换行符(在 C语言中表示为 "\r\n")，并将这一行发送到服务器。
	HTTP标准要求 每个文本行 都由一对 回车、换行符来结束。

	具体如下：
		telnet www.baidu.com 80	//用 telnet打开一个到 www.baidu.com服务器的连接
		Trying 220.181.112.244...	//这 3行是 telnet打开连接之后的输出，然后等待我们输入
		Connected to www.a.shifen.com.
		Escape character is '^]'.
		GET / HTTP/1.1	//为了发起事务，这三行是我们的一个 HTTP请求
		Host: www.baidu.com
		
		HTTP/1.1 200 OK	//接下来直到结尾的 </html> 都是 Server的响应
		Date: Thu, 18 May 2017 04:13:40 GMT
		Content-Type: text/html
		Content-Length: 14613
		Last-Modified: Mon, 08 May 2017 03:48:00 GMT
		Connection: Keep-Alive
		Vary: Accept-Encoding
		Set-Cookie: BAIDUID=17A41DF57BB19E8D99F8D840D7502F9F:FG=1; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
		Set-Cookie: BIDUPSID=17A41DF57BB19E8D99F8D840D7502F9F; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
		Set-Cookie: PSTM=1495080820; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
		P3P: CP=" OTI DSP COR IVA OUR IND COM "
		Server: BWS/1.1
		X-UA-Compatible: IE=Edge,chrome=1
		Pragma: no-cache
		Cache-control: no-cache
		Accept-Ranges: bytes
		
		<!DOCTYPE html><!--STATUS OK-->
		<html>
		...
		<html>
		Connection closed by foreign host.	//关闭连接

	(1) HTTP请求
		三部分组成：一个请求行 + 零个/多个请求头 + 一个空的文本行来终止报头列表
		a、一个请求行
			形式：<method> <uri> <version>
				如：GET / HTTP/1.1
			GET方法指导 Server生成和返回 URI表示的内容
				如：GET / HTTP/1.1
					要求 Server取出并返回 HTML文件 /index.html，它也告知 Server请求剩下的部分是 HTTP/1.1格式的
			version 表示：该请求遵循的 HTTP版本
			/*
			 * URI : Uniform Rescource Identifier
			 * URI 是相应的 URL的后缀，包括文件名、可选参数
			 *
			 * HTTP1.1
			 * 增加了一些附加报头
			 * 支持一种机制：允许 Client 和 Server在同一条持久连接上执行多个事务
			 * */
		b、零个/多个请求头
			形式：<header name>: <header data>
				针对我们的目的，唯一需要关注的是 Host报头
				如：Host: www.baidu.com
			

		c、一个空的文本行来终止报头

	(2) HTTP响应
		三部分组成：一个响应行 + 零个/多个响应报头 + 响应主体
		形式：<version> <status code> <status message>
			version ：响应遵循的 HTTP版本
			status code ：对请求的处理结果
			status message ：给出与错误代码对应的英文描述

		一些常见的状态码：
			200 成功	处理请求无误
			301 永久移动	内容已经移动到位置头中指明的主机
			400 错误请求	服务器不能理解请求
			403 禁止	服务器无权访问所请求的文件
			404 未找到	服务器不能找到所请求的文件
			501 未实现	服务器不支持请求的方法
			505 HTTP版本不支持	服务器不支持请求的版本

