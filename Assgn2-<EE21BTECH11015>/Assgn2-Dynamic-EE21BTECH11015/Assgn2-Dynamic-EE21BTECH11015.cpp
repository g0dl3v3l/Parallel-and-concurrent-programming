#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <omp.h> // OpenMP header
#include <dirent.h>
#include <chrono>

using namespace std;

// Class to represent a sparse matrix
class sparseMatrix {
public:
    int N, S, K, rowInc; // Matrix dimensions and parameters
    vector<vector<int>> vectorMatrix; // 2D vector to store the matrix
};

// Function to populate the matrix from a file
void PopulateMatrix(ifstream& f, sparseMatrix* obj) {
    string line;
    int i = 0;
    vector<int> rowBuffer;
    stringstream ss;
    
    cout << "Starting matrix population..." << endl;
    
    while (getline(f, line)) {
        if (i == 0) {
            ss.str(line);
            ss >> obj->N >> obj->S >> obj->K >> obj->rowInc;
            cout << "Matrix dimensions: N=" << obj->N << ", S=" << obj->S 
                 << ", K=" << obj->K << ", rowInc=" << obj->rowInc << endl;
            obj->vectorMatrix.resize(obj->N, vector<int>(obj->N));
            i++;
        } else {
            ss.clear();
            ss.str(line);
            rowBuffer.clear();
            int num;
            while (ss >> num) {
                rowBuffer.push_back(num);
            }
            
            if (rowBuffer.size() != obj->N) {
                cerr << "Error: Row " << i-1 << " does not have " << obj->N << " elements." << endl;
                return;
            }
            
            obj->vectorMatrix[i-1] = rowBuffer;
            i++;
        }
    }

    if (i - 1 != obj->N) {
        cerr << "Error: Number of rows read (" << i-1 << ") does not match N (" << obj->N << ")." << endl;
    }
    
    cout << "Matrix population completed." << endl;
}

void dynamic_omp(sparseMatrix* mat, ofstream &outFile) {
    auto time_avg = 0;
    int totalSum = 0;
    vector<int> threadCount(mat->K, 0);

    for (int i = 0; i < 5; i++) {
        totalSum = 0;
        fill(threadCount.begin(), threadCount.end(), 0);

        auto startTime = chrono::high_resolution_clock::now();

        #pragma omp parallel for num_threads(mat->K) schedule(dynamic, mat->rowInc) reduction(+:totalSum)
        for (int i = 0; i < mat->N; ++i) {
            int localSum = count(mat->vectorMatrix[i].begin(), mat->vectorMatrix[i].end(), 0);
            totalSum += localSum;

            // Optional: capture individual thread counts if needed
            int threadId = omp_get_thread_num();
            threadCount[threadId] += localSum;
        }

        auto endTime = chrono::high_resolution_clock::now();
        time_avg += chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
    }

    outFile << "Time taken to count the number of zeros: " << time_avg / 5 << " milliseconds" << endl;
    outFile << "Total Number of zero-valued elements in the matrix: " << totalSum << endl;

    for (int i = 0; i < mat->K; i++) {
        outFile << "Number of zero-valued elements counted by thread " << i + 1 << ": " << threadCount[i] << endl;
    }
    outFile << endl;
}

// Function to process each file and execute the chunk processing
void processingFiles(const string& filePath, ofstream& outFile , ofstream& outFile2 ,ofstream& outFile3) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }
    sparseMatrix* mat1 = new sparseMatrix();
    PopulateMatrix(file, mat1);
    cout << "Matrix Size: " << mat1->N << endl;
    outFile<<"The matrix size is: "<<mat1->N<<endl;
    outFile2<<"The matrix size is: "<<mat1->N<<endl;
    outFile3<<"The matrix size is: "<<mat1->N<<endl;
    cout << endl;
    dynamic_omp(mat1, outFile);
    for(int i = 2 ; i<=32 ; i=i*2){
        mat1->K = i;
        cout<<"The number of threads is: "<<mat1->K<<endl; 
        outFile2<<"Number of threads is: "<<mat1->K<<endl;
        dynamic_omp(mat1, outFile2);
    }
    cout<<endl;
    mat1->K = 16;
    for(int i = 10 ; i <= 50 ; i+=10){
        mat1->rowInc = i;
        cout<<"rowInc Value is: "<<mat1->rowInc<<endl;
        outFile3<<"rowInc Value is: "<<mat1->rowInc<<endl;
        dynamic_omp(mat1, outFile3);
    }
    mat1->K = 16;
    
    delete mat1;
    file.close();
}

void processingFiles(const string& filePath, ofstream& outFile) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }
    sparseMatrix* mat1 = new sparseMatrix();
    PopulateMatrix(file, mat1);
    cout << "Matrix Size: " << mat1->N << endl;
    outFile<<"The sparsity is: "<<mat1->S<<endl;
    
    cout << endl;
    dynamic_omp(mat1, outFile);
    cout << "--------------------------------------" << endl;

    delete mat1;
    file.close();
}

int main() {    
    cout << "--------------------------------------" << endl;
    cout << "The execution of Dynamic method using OpenMP" << endl;
    cout << endl;
    string inputFolder_1 = "./inputfiles"; // process the input files for experiment 1,2 and 4
    string inputFolder_2 = "./SparseInputFiles"; // process the input files for experiment 3   
    DIR* dir;
    struct dirent* ent;
    ofstream outFile("./Assgn2-Dynamic-EE21BTECH11015/output.txt");
    ofstream outFile2("./Assgn2-Dynamic-EE21BTECH11015/output_threads.txt");
    ofstream outFile3("./Assgn2-Dynamic-EE21BTECH11015/output_rowIncrement.txt");
    ofstream outFile4("./Assgn2-Dynamic-EE21BTECH11015/output_Sparse.txt");
    

    // Open the directory and process each file
    if ((dir = opendir(inputFolder_1.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            string fileName = ent->d_name;
            if (fileName != "." && fileName != "..") { // Skip . and .. directories
                string filePath = inputFolder_1 + "/" + fileName;
                cout << "Processing file: " << filePath << endl;
                processingFiles(filePath, outFile, outFile2, outFile3);
            }
        }
        outFile.close();
        closedir(dir);
    } else {
        cerr << "Could not open directory: " << inputFolder_1 << endl;
        return EXIT_FAILURE;
    }

    cout << "Generating data for experiment 3" << endl;

    if ((dir = opendir(inputFolder_2.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            string fileName = ent->d_name;
            if (fileName != "." && fileName != "..") { // Skip . and .. directories
                string filePath = inputFolder_2 + "/" + fileName;
                cout << "Processing file: " << filePath << endl;
                processingFiles(filePath, outFile4);
            }
        }
        outFile.close();
        closedir(dir);
    } else {
        cerr << "Could not open directory: " << inputFolder_2 << endl;
        return EXIT_FAILURE;
    }

    cout << "Dynamic OpenMP execution completed successfully!" << endl;
    cout << "--------------------------------------" << endl;
    
    return 0;
}
