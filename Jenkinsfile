stage("Build") {
    cheribuildProject(name:'nginx', extraArgs: '--with-libstatcounters --nginx/no-debug-info'
                      testScript: 'cd /opt/$CPU/ && sh -xe ./nginx-benchmark.sh')
}
