#!/bin/sh -e

NGINX=/usr/local/nginx/sbin/nginx
FETCHBENCH=/usr/local/nginx/sbin/fetchbench
NTIMES=10

echo "${0}: uname:"
uname -a

echo "${0}: invariants/witness:"
sysctl -a | grep -E '(invariants|witness)'

echo "${0}: nginx binary details:"
if ! command -v file > /dev/null; then
	echo "file binary not installed"
else
	file "${NGINX}"
fi

echo "${0}: starting nginx..."
${NGINX}

echo "${0}: running benchmark ${NTIMES} times..."
for i in `jot ${NTIMES}`; do
	${FETCHBENCH} http://127.0.0.1/ 5 200
done

echo "${0}: stopping nginx..."
${NGINX} -s stop

echo "${0}: done"
