#!/bin/sh -e

NGINX=/usr/local/nginx/sbin/nginx
FETCHBENCH=/usr/local/nginx/sbin/fetchbench

echo "${0}: nginx binary details:"
file ${NGINX}
echo "${0}: starting nginx..."
${NGINX}
echo "${0}: running benchmark..."
${FETCHBENCH} http://127.0.0.1/ 5 200
echo "${0}: stopping nginx..."
${NGINX} -s stop
echo "${0}: done"
