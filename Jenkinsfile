pipeline {
    agent {
        dockerfile true
    }
    stages {
        stage('Build') {
            steps {
                sh 'make'
            }
            post {
                success {
                    archiveArtifacts artifacts: '**/build/WbMsw/*.bin', fingerprint: true
                }
            }
        }
    }
}
