#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html\r")
print("\r")
print("<html><body>")
print("<h1>CGI Test - Python</h1>")
print("<h2>Environment Variables:</h2>")
print("<pre>")
for key, value in sorted(os.environ.items()):
    print(f"{key} = {value}")
print("</pre>")

print("<h2>Request Info:</h2>")
print(f"<p>Method: {os.environ.get('REQUEST_METHOD', 'N/A')}</p>")
print(f"<p>Query: {os.environ.get('QUERY_STRING', 'N/A')}</p>")

# Si POST, lire le body
if os.environ.get('REQUEST_METHOD') == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        body = sys.stdin.read(content_length)
        print(f"<p>Body: {body}</p>")

print("</body></html>")