#!/bin/sh -xe
# TODO: run the NGINX test suite
# pkg bootstrap && pkg install -y perl5 p5-TAP-Formatter-JUnit
which perl
which prove
NGINX_DIR=$(realpath .)
export TEST_NGINX_BINARY=${NGINX_DIR}/sbin/nginx
# http://blogs.perl.org/users/confuseacat/2011/09/perl-testing-with-jenkinshudson-avoiding-some-pitfalls.html
prove --formatter=TAP::Formatter::JUnit /opt/nginx-tests | tee /tmp/test_output.xml
