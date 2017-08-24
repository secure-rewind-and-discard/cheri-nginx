#!/bin/sh -xe
# TODO: run the NGINX test suite
# pkg bootstrap && pkg install -y perl5 p5-TAP-Formatter-JUnit
which perl
which prove
NGINX_DIR=$(realpath .)
export TEST_NGINX_BINARY=${NGINX_DIR}/sbin/nginx
export TEST_NGINX_VERBOSE=1
export TEST_NGINX_CATLOG=1
# http://blogs.perl.org/users/confuseacat/2011/09/perl-testing-with-jenkinshudson-avoiding-some-pitfalls.html
prove -v --formatter=TAP::Formatter::JUnit /opt/nginx-tests | tee /tmp/test_output.xml
