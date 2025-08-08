#!/usr/bin/env python3
import os
import sys
import html

# Required CGI headers
print("Content-Type: text/html")
print()  # Blank line to separate headers from body

print("<html><style> body {background-color: pink;}</style><body>")
print("<h1>CGI Test Script</h1>")

# Show method and query
method = os.environ.get("REQUEST_METHOD", "")
print(f"<p><strong>Method:</strong> {html.escape(method)}</p>")

if method == "GET":
    query = os.environ.get("QUERY_STRING", "")
    print(f"<p><strong>Query String:</strong> {html.escape(query)}</p>")

elif method == "POST":
    try:
        length = int(os.environ.get("CONTENT_LENGTH", 0))
    except ValueError:
        length = 0

    body = sys.stdin.read(length) if length > 0 else ""
    print("<h2>POST Body</h2>")
    print(f"<pre>{html.escape(body)}</pre>")

# Show some CGI environment variables
print("<h2>Environment Variables</h2>")
print("<ul>")
for key in ["CONTENT_LENGTH", "CONTENT_TYPE", "QUERY_STRING", "REQUEST_METHOD", "SCRIPT_NAME"]:
    print(f"<li>{html.escape(key)} = {html.escape(os.environ.get(key, ''))}</li>")
print("</ul>")

print("</body></html>")
