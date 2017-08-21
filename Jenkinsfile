pipeline {
    agent none
    stage("Build") {
        parallel cheribuildProject('nginx', '--install-prefix /tmp/benchdir/nginx-$CPU --with-libstatcounters --nginx/no-debug-info')
    }
}
