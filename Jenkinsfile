pipeline {
    agent {
        dockerfile true
    }
    stages {
        stage('Build') {
            steps {
                sh 'make'
                archiveArtifacts artifacts: '**/build/WbMsw/WbMsw_ino_signed.bin', fingerprint: true
            }
        }
    }
}
