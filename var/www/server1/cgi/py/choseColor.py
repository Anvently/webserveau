#!/usr/bin/python
import cgi, cgitb
form = cgi.FieldStorage()

if form.getvalue('color'):
	color = form.getvalue('color')
else:
	color = 'red'
print("Set-Cookie: color=%s" % color, end='\r\n')
print("Content-Type: text/html\r\n", end='\r\n')
print("<html><head><title> Colors </title></head><body><style>")
if (color == 'red'):
	print("#one {color:darkred;text-align: center;font-size:150%;}")
elif (color == 'blue'):
	print("#one {color:cornflowerblue;text-align: center;font-size:150%;}")
else:
	print("#one {color:green;text-align: center;font-size:150%;}")
print("</style><p id=\"one\">")
print("I love sockets !")
print("</p></body></html>")
