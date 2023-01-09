pipeline {
    agent {
        dockerfile {
            filename 'Dockerfile'
            args '-v ${dir()}:/wb-zwave-msw -w /wb-zwave-msw --rm'
        }
    }
    stages {
        stage('Build') {
            steps {
                sh 'make'
            }
        }
    }
}
