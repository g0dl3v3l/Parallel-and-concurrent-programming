#include <iostream>
#include <fstream>
#include <sstream> // For stringstream
#include <vector>
#include <thread>
#include <mutex> 
#include <algorithm>
#include <dirent.h>
#include <chrono>
using namespace std;

// Class to represent a sparse matrix
class sparseMatrix {
public:
    int N, S, K, rowInc; // Matrix dimensions and parameters
    vector<vector<int>> vectorMatrix; // 2D vector to store the matrix
    
    // Constructor to initialize matrix (optional, but not used here)
    // sparseMatrix()  {} 
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

// Function to count zero elements in a matrix segment
void SparseCounting(int start, int end, sparseMatrix* mat, int* totalSum, mutex* mtx, vector<int>& threadCount, int threadIndex) {
    int sum = 0;
    for (int i = start; i < end; i++) { 
        sum += count(mat->vectorMatrix[i].begin(), mat->vectorMatrix[i].end(), 0);  
    }
    mtx->lock();
    *(totalSum) += sum; // Update the total sum of zeros
    threadCount[threadIndex-1] = sum; // Store the count for this thread
    //cout<<"The value of threadcout "<<threadCount[threadIndex-1]<<" for thread : "<<threadIndex<<endl;
    // cout<<"---------------------------------------------------------------------------"<<endl;
    // cout<<"the thread being excuted is "<<threadIndex<<endl;
    // cout<<"the counting for the set start " <<start<< " and "<<end<<" is "<<sum<<endl;
    // cout<<"The count of zeros for the thread so far is "<<threadCount[threadIndex-1]<<endl;
    // cout<<"the total zeros counted till now is "<<*totalSum<<endl;
    // cout<<"---------------------------------------------------------------------------"<<endl;
    mtx->unlock();
}

// Function to divide the matrix into chunks and process them with threads
void chunk(sparseMatrix* mat, vector<thread>& ths, ofstream& outFile) {
    int p = mat->N / mat->K; // Number of rows per chunk
    int m = 0; // Start index for each chunk
    int totalSum = 0; // Total number of zeros
    vector<int> threadCount(mat->K, 0); // Vector to store zero counts for each thread
    mutex mtx; // Mutex to synchronize access to totalSum
    auto time_avg = 0;
    for(int f = 0 ; f < 5;f++){
       cout<<"Iteration number : "<<f<<endl;
        totalSum = 0; // Total number of zeros
        //vector<int> threadCount(mat->K, 0); // Reset thread count vector for each iteration
        mutex mtx; // Mutex to synchronize access to totalSum
        ths.clear(); // Clear previous threads before starting new iteration
        // Create and start threads for each chunk
         auto startTime = chrono::high_resolution_clock::now(); 
         m=0;
        for (int i = 1; i <= mat->K; i++) {
            if (i == mat->K) {
                // Last chunk may be larger if N is not perfectly divisible by K
                ths.push_back(thread([m, i, p, &totalSum, mat, &mtx, &threadCount]() {
                    SparseCounting(m, mat->N, mat, &totalSum, &mtx, threadCount, i);
                }));
            } else {
                ths.push_back(thread([m, i, p, &totalSum, mat, &mtx, &threadCount]() {
                    SparseCounting(m, i * p, mat, &totalSum, &mtx, threadCount, i);
                }));
                m = m + p; // Update start index for next chunk
            }
        } 

        // Wait for all threads to finish
        for (auto& th : ths) {
            if (th.joinable()) {
                th.join();
            }
        }

        auto endTime = chrono::high_resolution_clock::now();
        time_avg += std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
      
    }
    outFile << "Time taken to count the number of zeros: " << time_avg/5 << " milliseconds" << endl;
    outFile << "Total Number of zero-valued elements in the matrix: " << totalSum << endl;

    // Output the count of zero elements for each thread
    for (int i = 0; i < mat->K; i++) {
        outFile << "Number of zero-valued elements counted by thread" << i + 1 << ": " << threadCount[i] << endl;
    }
    outFile << endl;
}

// Function to process each file and execute the chunk processing
void processingFiles(const string& filePath, ofstream& outFile, ofstream& outFile2) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return;
    }
    sparseMatrix* mat1 = new sparseMatrix();
    PopulateMatrix(file, mat1);
    vector<thread> threads;
    cout << "Matrix Size: " << mat1->N << endl;
    outFile<<"The matrix size is: "<<mat1->N<<endl;
    outFile2<<"The matrix size is: "<<mat1->N<<endl;
    cout << endl;
    chunk(mat1, threads, outFile);
    for(int i = 2 ; i<=32 ; i=i*2){
        mat1->K = i;
        cout<<"The number of threads is: "<<mat1->K<<endl; 
        outFile2<<"Number of threads is: "<<mat1->K<<endl;
        chunk(mat1, threads , outFile2);
    }
    cout<<endl;


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
    vector<thread> threads;
    cout << "Matrix Size: " << mat1->N << endl;
    //outFile<<"The matrix size is: "<<mat1->N<<endl;
    outFile<<"The sparsity is: "<<mat1->S<<endl;
    
    cout << endl;
    chunk(mat1, threads, outFile);
    cout << "--------------------------------------" << endl;


    delete mat1;
    file.close();
}

int main() {    
    cout << "--------------------------------------" << endl;
    cout << "The execution of chunk method" << endl;
    cout << endl;
    string inputFolder_1 = "./inputfiles"; // process the input files for experiment 1,2 and 4
    string inputFolder_2 = "./SparseInputFiles"; // process the input files for experiment 3
    DIR* dir;
    struct dirent* ent;
    ofstream outFile("./Assgn1-Chunk-EE21BTECH11015/output.txt");
    ofstream outFile2("./Assgn1-Chunk-EE21BTECH11015/output_threads.txt");
    ofstream outFile3("./Assgn1-Chunk-EE21BTECH11015/output_Sparse.txt");

    // Open the directory and process each file
    if ((dir = opendir(inputFolder_1.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            string fileName = ent->d_name;
            if (fileName != "." && fileName != "..") { // Skip . and .. directories
                string filePath = inputFolder_1 + "/" + fileName;
                cout << "Processing file: " << filePath << endl;
                processingFiles(filePath, outFile,outFile2);
            }
        }
        outFile.close();
        closedir(dir);
    } else {
        cerr << "Could not open directory: " << inputFolder_1 << endl;
        return EXIT_FAILURE;
    }

    cout<<"generating data for experiment 3"<<endl;

    if ((dir = opendir(inputFolder_2.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            string fileName = ent->d_name;
            if (fileName != "." && fileName != "..") { // Skip . and .. directories
                string filePath = inputFolder_2 + "/" + fileName;
                cout << "Processing file: " << filePath << endl;
                processingFiles(filePath,outFile3);
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
