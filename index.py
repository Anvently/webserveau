#!C:\Python311\python.exe
 
import cgi, cgitb
 
form = cgi.FieldStorage()
 
username = form["first_name"].value
emailaddress = form["last_name"].value
 
print("Content-type:text/html\r\n\r\n")
 
print("<html>")
print("<head>")
print("<title> MY FIRST CGI FILE </title>")
print("</head>")
print("<body>")
print("<h3> This is HTML's Body Section </h3>")
print(username)
print(emailaddress)
print("</body>")
print("</html>")