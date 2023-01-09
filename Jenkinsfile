pipeline {
    agent {
        dockerfile {
            filename 'Dockerfile'
            args '-v $(pwd):/wb-zwave-msw -w /wb-zwave-msw --rm'
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
