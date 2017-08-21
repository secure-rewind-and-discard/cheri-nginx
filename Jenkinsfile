pipeline {
  agent none
  stages {
    stage('Build') {
      steps {
        agent docker
        sh 'pwd'
        cheribuildProject 'nginx', '--install-prefix /tmp/benchdir/nginx-$CPU --with-libstatcounters --nginx/no-debug-info'
      }
    }
  }
}
