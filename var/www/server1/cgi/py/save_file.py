#!/usr/bin/env python
import cgi, os
import cgitb; cgitb.enable()

# try: # Windows needs stdio set for binary mode.
#     import msvcrt
#     msvcrt.setmode (0, os.O_BINARY) # stdin  = 0
#     msvcrt.setmode (1, os.O_BINARY) # stdout = 1
# except ImportError:
#     pass

form = cgi.FieldStorage()

# A nested FieldStorage instance holds the file
fileitem = form['file']

# Test if the file was uploaded
if fileitem.filename:

    # strip leading path from file name
    # to avoid directory traversal attacks
    fn = os.path.basename(fileitem.filename)
    open('../../data/images/' + fn, 'wb').write(fileitem.file.read())
    message = 'The file "' + fn + '" was uploaded successfully'

else:
    message = 'No file was uploaded'

print("""\
Content-Type: text/html\r\n

<html>
      <head>
      <meta http-equiv="refresh"content="0; url=http://localhost:8080/images/%s">
      </head>
      <body>
<p>%s</p>
</body></html>
""" % (fn, message))
