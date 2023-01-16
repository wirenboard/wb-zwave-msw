pipeline {
    agent {
        label 'devenv'
    }

    stages {
        stage('Build') {
            agent {
                dockerfile true
            }
            steps {
                sh 'make'
            }
            post {
                success {
                    archiveArtifacts artifacts: '**/build/WbMsw/*.bin', fingerprint: true
                }
            }
        }
        stage('Github release') {
            when { branch 'master' }
            steps {
                withCredentials([usernamePassword(credentialsId: 'jenkins-github-token',
                                                  passwordVariable: 'GITHUB_TOKEN',
                                                  usernameVariable: 'DUMMY')]) {
                    sh "wbci-git -v -t $GITHUB_TOKEN publish-release build/WbMsw/*.bin"
                }
            }
        }
    }
}
