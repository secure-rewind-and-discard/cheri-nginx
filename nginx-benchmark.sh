#!/bin/sh -e

NGINX=/usr/local/nginx/sbin/nginx
FETCHBENCH=/usr/local/nginx/sbin/fetchbench
NTIMES=10

echo "${0}: uname:"
uname -a

echo "${0}: invariants/witness:"
sysctl -a | grep -E '(invariants|witness)' || true

echo "${0}: nginx binary details:"
if ! command -v file > /dev/null; then
	echo "file binary not installed"
else
	file "${NGINX}"
fi

if ! command -v jot> /dev/null; then
  echo "jot not found, cannot run benchmark!"
  exit 1
fi

echo "${0}: running benchmark ${NTIMES} times..."
for i in `jot ${NTIMES}`; do
	echo "${0}: starting nginx..."
	export STATCOUNTERS_OUTPUT="/tmp/nginx-${i}.statcounters"
	${NGINX}

	${FETCHBENCH} http://127.0.0.1/ 5 200

	echo "${0}: stopping nginx..."
	${NGINX} -s stop
done

echo "${0}: done"
