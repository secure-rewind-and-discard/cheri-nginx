pipeline {
  agent none
  stages {
    stage("Build") {
      steps {
        cheribuildProject('nginx', '--install-prefix /tmp/benchdir/nginx-$CPU --with-libstatcounters --nginx/no-debug-info')
      }
    }
  }
}
