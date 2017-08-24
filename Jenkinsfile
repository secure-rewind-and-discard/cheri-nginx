properties([
    disableConcurrentBuilds(),
    pipelineTriggers([]),
])

stage("Build") {
    def before = {
        dir('nginx-tests') {
            git 'https://github.com/nginx/nginx-tests.git'
        }
        sh '''
            apt-get install -y --no-install-recommends git wget
            tar cJf nginx-tests.tar.xz  --exclude nginx-tests/.git nginx-tests
            # TODO: use perl from pkg.FreeBSD.org? (for some reason can't extract it properly though)
            wget https://transfer.sh/AmikH/perl-mips-junit.tar.xz
            ls -la
        '''
    }
    cheribuildProject(name:'nginx', extraArgs: '--with-libstatcounters --nginx/no-debug-info',
                      targets: ['mips'],  // only run one for now
                      testScript: 'cd /opt/$CPU/ && sh -xe ./run-nginx-tests.sh',
                      beforeTests: before, testExtraArgs: '--test-archive nginx-tests.tar.xz --test-archive perl-mips-junit.tar.xz' 
                      /* minimalTestImage: true */) // TODO: once we need perl this will no longer be true
}
