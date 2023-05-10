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
                stash includes: '**/build/WbMsw/*.bin', name: 'fw'
            }
            post {
                success {
                    archiveArtifacts artifacts: '**/build/WbMsw/*.bin', fingerprint: true
                }
            }
        }
        stage('Github release') {
            when { 
                branch 'master'
                expression {
                    def lastVersion = wb.getLatestTagVersionVisibleFromBranch(GIT_BRANCH)
                    def newVersion = wb.getVersionFromChangelog()           
                    def status = sh script: "wbdev user dpkg --compare-versions ${newVersion} gt ${lastVersion}",
                                    returnStatus: true
                    return (status == 0)
                }
            }
            steps {
                withCredentials([usernamePassword(credentialsId: 'jenkins-github-token',
                                                  passwordVariable: 'GITHUB_TOKEN',
                                                  usernameVariable: 'DUMMY')]) {
                    unstash 'fw'
                    sh 'wbdev user wbci-git -v -t $GITHUB_TOKEN publish-release build/WbMsw/*.bin'                    
                }
            }
        }
    }
}
