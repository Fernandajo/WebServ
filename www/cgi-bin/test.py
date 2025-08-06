#!/usr/bin/env python3

import os
import sys

# Output the CGI HTTP headers
print("Content-Type: text/plain")
print()  # Empty line to separate headers from body

# Print basic environment info
print("=== CGI Environment ===")
print(f"REQUEST_METHOD: {os.environ.get('REQUEST_METHOD')}")
print(f"QUERY_STRING: {os.environ.get('QUERY_STRING', '')}")
print(f"CONTENT_LENGTH: {os.environ.get('CONTENT_LENGTH', '')}")
print(f"CONTENT_TYPE: {os.environ.get('CONTENT_TYPE', '')}")

# Read and print body if it's a POST request
if os.environ.get('REQUEST_METHOD') == 'POST':
    try:
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        body = sys.stdin.read(content_length)
        print("\n=== Body ===")
        print(body)
    except Exception as e:
        print(f"Error reading POST body: {e}")
