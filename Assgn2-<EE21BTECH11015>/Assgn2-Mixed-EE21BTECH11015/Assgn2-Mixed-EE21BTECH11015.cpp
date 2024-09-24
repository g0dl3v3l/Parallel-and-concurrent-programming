#include <iostream>
#include <fstream>
#include <sstream> // For stringstream
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <chrono>
#include <omp.h> // Include OpenMP header
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
    
    // Read the file line by line
    while (getline(f, line)) {
        if (i == 0) {
            // First line contains matrix parameters: N, S, K, rowInc
            ss.str(line);
            ss >> obj->N >> obj->S >> obj->K >> obj->rowInc;
            cout << "Matrix dimensions: N=" << obj->N << ", S=" << obj->S 
                 << ", K=" << obj->K << ", rowInc=" << obj->rowInc << endl;

            // Allocate memory for the matrix
            obj->vectorMatrix.resize(obj->N, vector<int>(obj->N));
            i++;
        } else {
            // Reading matrix data from subsequent lines
            ss.clear();
            ss.str(line);

            rowBuffer.clear();
            int num;
            while (ss >> num) {
                rowBuffer.push_back(num);
            }
            
            // Check if the row has the expected number of elements
            if (rowBuffer.size() != obj->N) {
                cerr << "Error: Row " << i-1 << " does not have " << obj->N << " elements." << endl;
                return;
            }
            
            // Assign row data to the matrix
            obj->vectorMatrix[i-1] = rowBuffer;
            i++;
        }
    }

    // Verify that the total number of rows matches the expected number
    if (i - 1 != obj->N) {
        cerr << "Error: Number of rows read (" << i-1 << ") does not match N (" << obj->N << ")." << endl;
    }
    
    cout << "Matrix population completed." << endl;
}

// OpenMP version of the SparseCounting function
void mixed_omp(sparseMatrix* mat, ofstream& outFile) {
    int num_threads = mat->K; // Number of threads
    int totalSum = 0; // Total number of zeros
    vector<int> threadCount(num_threads, 0); // Vector to store zero counts for each thread
    auto time_avg = 0;

    for(int f = 0; f < 5; f++){
        totalSum = 0;
        fill(threadCount.begin(), threadCount.end(), 0); // Reset thread counts
        auto startTime = chrono::high_resolution_clock::now();

        // Parallel region using OpenMP
        #pragma omp parallel for num_threads(num_threads) reduction(+:totalSum)
        for (int i = 1; i <= mat->N; ++i) {
            int thread_id = omp_get_thread_num(); // Get the current thread ID
            int localSum = count(mat->vectorMatrix[i-1].begin(), mat->vectorMatrix[i-1].end(), 0);
            
            threadCount[thread_id] += localSum; // Update the local count for the thread
            totalSum += localSum; // Use reduction to update the total sum
        }

        auto endTime = chrono::high_resolution_clock::now();
        time_avg += std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    }

    outFile << "Time taken to count the number of zeros: " << time_avg << " milliseconds" << endl;
    outFile << "Total Number of zero-valued elements in the matrix: " << totalSum << endl;

    // Output the count of zero elements for each thread
    for (int i = 0; i < num_threads; i++) {
        outFile << "Number of zero-valued elements counted by thread " << i + 1 << ": " << threadCount[i] << endl;
    }
    outFile << endl;
}

// Function to process each file and execute the mixed processing
void processingFiles(const string& filePath, ofstream& outFile, ofstream& outFile2) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }
    sparseMatrix* mat1 = new sparseMatrix();
    PopulateMatrix(file, mat1);
    cout << "Matrix Size: " << mat1->N << endl;
    outFile << "The matrix size is: " << mat1->N << endl;
    outFile2 << "The matrix size is: " << mat1->N << endl;
    cout << endl;

    // Run the mixed OpenMP version
    mixed_omp(mat1, outFile);

    // Test with varying thread counts
    for(int i = 2 ; i <= 32 ; i = i * 2){
        mat1->K = i;
        cout << "The number of threads is: " << mat1->K << endl; 
        outFile2 << "Number of threads is: " << mat1->K << endl;
        mixed_omp(mat1, outFile2);
    }
    cout << endl;

    delete mat1;
    file.close();
}

// Processing sparse files (unchanged)
void processingFiles(const string& filePath, ofstream& outFile) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }
    sparseMatrix* mat1 = new sparseMatrix();
    PopulateMatrix(file, mat1);
    cout << "Matrix Size: " << mat1->N << endl;
    outFile << "The sparsity is: " << mat1->S << endl;
    cout << endl;

    mixed_omp(mat1, outFile);
    cout << "--------------------------------------" << endl;

    delete mat1;
    file.close();
}

int main() {    
    cout << "--------------------------------------" << endl;
    cout << "The execution of mixed method using OpenMP" << endl;
    cout << endl;
    string inputFolder_1 = "./inputfiles"; // Process the input files for experiments 1, 2, and 4
    string inputFolder_2 = "./SparseInputFiles"; // Process the input files for experiment 3  
    DIR* dir;
    struct dirent* ent;
    ofstream outFile("./Assgn2-Mixed-EE21BTECH11015/output.txt");
    ofstream outFile2("./Assgn2-Mixed-EE21BTECH11015/output_threads.txt");
    ofstream outFile3("./Assgn2-Mixed-EE21BTECH11015/output_Sparse.txt");

    // Open the directory and process each file
    if ((dir = opendir(inputFolder_1.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            string fileName = ent->d_name;
            if (fileName != "." && fileName != "..") { // Skip . and .. directories
                string filePath = inputFolder_1 + "/" + fileName;
                cout << "Processing file: " << filePath << endl;
                processingFiles(filePath, outFile, outFile2);
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
                processingFiles(filePath, outFile3);
            }
        }
        outFile.close();
        closedir(dir);
    } else {
        cerr << "Could not open directory: " << inputFolder_2 << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
