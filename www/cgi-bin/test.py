#!/usr/bin/env python3
import os
import sys
import html
import urllib.parse

# Required CGI headers
print("Content-Type: text/html")
print()  # Blank line to separate headers from body

print("<html><head><style>body {background-color: pink; font-family: sans-serif;}</style></head><body>")
print("<h1>CGI Test Script</h1>")

# Show request method
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
    
    # Parse form data
    params = urllib.parse.parse_qs(body)
    message = params.get("message", [""])[0].strip()

    if message:
        # Append message to file
        file_path = os.path.join(os.path.dirname(__file__), "messages.txt")
        with open(file_path, "a", encoding="utf-8") as f:
            f.write(message + "\n")

    # Display decoded message
    print("<h2>Message Received</h2>")
    print(f"<pre>{html.escape(message)}</pre>")
    print('<p><a href="../messages.html">Back to Message Board</a></p>')

# Show some CGI environment variables
print("<h2>Environment Variables</h2>")
print("<ul>")
for key in ["CONTENT_LENGTH", "CONTENT_TYPE", "QUERY_STRING", "REQUEST_METHOD", "SCRIPT_NAME"]:
    print(f"<li>{html.escape(key)} = {html.escape(os.environ.get(key, ''))}</li>")
print("</ul>")

print("</body></html>")
