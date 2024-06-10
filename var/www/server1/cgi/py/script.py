# Import modules for CGI handling
import cgi, cgitb
import cgitb
cgitb.enable()

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
for key in form.keys():
	print(form[key])
	

print("Content-type:text/html\r\n\r\n")
print("<html>")
print("<head>")
print("<title>Checkbox - Third CGI Program</title>")
print("</head>")
print("<body>")
# print("<h2> CheckBox Maths is : %s</h2>" % math_flag)
# print("<h2> CheckBox Physics is : %s</h2>" % physics_flag)
print("</body>")
print("</html>")
