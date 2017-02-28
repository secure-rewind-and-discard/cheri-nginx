#!/bin/sh -e

echo "${0}: starting nginx..."
/usr/local/nginx/sbin/nginx
echo "${0}: running benchmark..."
/usr/local/nginx/sbin/fetchbench http://127.0.0.1/ 5 200
echo "${0}: done"
