stage("Build") {
    cheribuildProject(name:'nginx', extraArgs: '--with-libstatcounters --nginx/no-debug-info',
                      testScript: 'cd /opt/$CPU/ && sh -xe ./run-nginx-tests.sh',
                      /* minimalTestImage: true */) // TODO: once we need perl this will no longer be true
}
