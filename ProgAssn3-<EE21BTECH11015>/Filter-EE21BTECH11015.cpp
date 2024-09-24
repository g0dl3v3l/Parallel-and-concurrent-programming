#include <iostream>
#include <thread>
#include <atomic>
#include <algorithm>
#include <unordered_map>
#include <unistd.h>
#include <memory>
#include <fstream>
#include <sstream> // For stringstream
#include <random>        // for exponential_distribution
#include <chrono>        // for chrono literals
#include <iomanip>       // for std::put_time

using namespace std;

typedef struct threads{
    atomic<int> level;
    atomic<thread::id> threadId;
    threads(int l, thread::id id) : level(l), threadId(id) {}
} th;

typedef struct victim{
    atomic<thread::id> victimThread;
    victim(thread::id id) : victimThread(id){}
} victim;

int search(vector<threads> &ths, size_t n, thread::id th) {
    for (int i = 0; i < n; i++) {
        if (ths[i].threadId == th) {
            return i;
        }
    }
    return -1;
}

class filterLock {
    int num_threads;
    std::vector<std::atomic<int>> level;    // Each thread's level
    std::vector<std::atomic<int>> victim;   // Victim for each level

public:
    filterLock(int n) : num_threads(n), level(n), victim(n) {
        for (int i = 0; i < n; ++i) {
            level[i] = -1;  // Initially, all threads are at level -1
            victim[i] = -1; // No victim at the start
        }
    }

    void lock(int thread_id) {
        for (int l = 0; l < num_threads - 1; ++l) {
            level[thread_id].store(l);  // Set the level of the thread
            victim[l].store(thread_id); // This thread is the victim for this level

            // Wait while there is any thread at the same level and the current thread is the victim
            while (is_conflict(thread_id, l)) {
                // Busy-wait (spinlock) until it's safe to proceed
            }
        }
    }

    void unlock(int thread_id) {
        level[thread_id].store(-1); // Reset the thread's level
    }

private:
    // Check if there is a conflict with other threads at the same level
    bool is_conflict(int thread_id, int current_level) {
        for (int i = 0; i < num_threads; ++i) {
            if (i != thread_id && level[i].load() >= current_level && victim[current_level].load() == thread_id) {
                return true; // Conflict exists
            }
        }
        return false; // No conflict
    }
};

double getExponentialDelay(double lambda) {
    random_device rd;  // Random number generator
    mt19937 gen(rd()); // Seed the generator
    exponential_distribution<> dist(1.0 / lambda); // Exponential distribution

    return dist(gen); // Return a delay time in milliseconds
}

// Function to get the current time formatted as HH:MM:SS
string getSysTime(const chrono::system_clock::time_point& now) {
    
    time_t now_time = chrono::system_clock::to_time_t(now);
    tm* localTime = localtime(&now_time);

    stringstream ss;
    ss << put_time(localTime, "%H:%M:%S"); // Format as HH:MM:SS
    return ss.str();
}

void testCS(int k,int id,float &avgEnterTime,float &worst_case, int lambda1, int lambda2, filterLock &Lock , ofstream &FilterLockFile) {
    //hread::id id = this_thread::get_id(); // Get the thread ID
    chrono::duration<float> thread_worst_time(0);
    chrono::duration<float> csEnterTime;
    chrono::duration<float> csEnterTimeTotal;
    chrono::duration<float> csExiteTime ;
    for (int i = 0; i < k; ++i) {
        // Time of CS entry request
        auto reqEnterTime = chrono::system_clock::now();
        //auto reqEnterTimeString = getSysTime(reqEnterTime);
        //cout << i+1 << "th CS Requested Entry at " << reqEnterTime << " by thread " << id << endl;

        Lock.lock(id);
        //FilterLockFile<<"-----------------------------------------------------------------------"<<endl;
        // Time of actual CS entry
        auto actEnterTime = chrono::system_clock::now();
        csEnterTime = actEnterTime - reqEnterTime;
        csEnterTimeTotal += csEnterTime;
        thread_worst_time = max(thread_worst_time, csEnterTime);
        auto actEnterTimeString = getSysTime(actEnterTime);
        FilterLockFile << i+1 << "th CS Entered at " << actEnterTimeString << " by thread " << id << endl;

        // Generate exponential delay with average lambda1 milliseconds
        double t1 = getExponentialDelay(lambda1);
        this_thread::sleep_for(chrono::milliseconds(static_cast<int>(t1)));

        // Time of CS exit request
        auto reqExitTime = chrono::system_clock::now();
        auto reqExitTimeString = getSysTime(reqExitTime);
        FilterLockFile << i+1 << "th CS Requested Exit at " << reqExitTimeString << " by thread " << id << endl;
        //FilterLockFile<<"--------------------------------------------------------------------------"<<endl;

        Lock.unlock(id);
        
        // Time of actual CS exit
        auto actExitTime = chrono::system_clock::now();
       // cout << i+1 << "th CS Exited at " << actExitTime << " by thread " << id << endl;
        //csEnterTime += actEnterTime - reqEnterTime;
        //cout<<"The cs EnterTime is "<<csEnterTime.count()<<endl;
        csExiteTime += actExitTime - reqExitTime;
        // Generate exponential delay with average lambda2 milliseconds
        double t2 = getExponentialDelay(lambda2);
        this_thread::sleep_for(chrono::milliseconds(static_cast<int>(t2)));
    }

    avgEnterTime = csEnterTimeTotal.count();
    worst_case = thread_worst_time.count();
    //cout<<"The avg Enter Time: "<<avgEnterTime<<endl;
}

void LockExperiment(string inputfilePath , ofstream &outputfile1 , ofstream &outputfile2 , ofstream &outputfile3, ofstream&FilterLock){
    int k, n, lambda1, lambda2;
    // Read parameters from fil
    ifstream inputFile(inputfilePath);
    
    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << inputfilePath << endl;
        return;
    }
    
    string line;
    stringstream ss;
    
    while (getline(inputFile, line)) {
        ss.str(line);
        ss >> k >> n >> lambda1 >> lambda2;
        cout << "k: " << k << " n: " << n << " lambda1: " << lambda1 << " lambda2: " << lambda2 << endl;
        float total_throughput = 0.0f;
        float total_avgEntertime = 0.0f;
        float total_worst_entry_time = 0.0f;
        for(int f = 1 ; f < 5;f++){
        vector<thread> ths;
        filterLock lock(n);
        vector<float> avgEnterTimes(n);
        vector<float> worstEntryTime(n);
        float worst_entry_time = 0.0;
        float csEntrytime;
        auto StartTime =  chrono::system_clock::now();
        for (int i = 0; i < n; i++) {
            
            ths.push_back(thread(testCS, k,i,ref(avgEnterTimes[i]),ref(worstEntryTime[i]), lambda1, lambda2, ref(lock), ref(FilterLock)));
            //cout<<"THe avgEnterTime :"<<avgEnterTimes[i]<<endl;
        }
        auto EndTime =  chrono::system_clock::now();
         chrono::duration<float> duration = EndTime - StartTime;
        // auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(EndTime - StartTime);
        
        // Calculate throughput
        float throughput = duration.count() / (k * n);
        total_throughput += throughput;
        float totalEnterTime = 0.0f;
        
        
        //outputfile1<< throughput<<endl;
        
        for (auto& th : ths) {
            if (th.joinable()) {
                th.join();
            }
        }

        for (int i = 0; i < n; ++i) {
            totalEnterTime += avgEnterTimes[i];  // Manually sum all elements
            worst_entry_time = max(worst_entry_time, worstEntryTime[i]);
        }
    
        float avgEntertime = totalEnterTime / (k*n);  // Calculate average
        total_avgEntertime += avgEntertime;
        total_worst_entry_time += worst_entry_time;

    }   

        float avg_throughput = total_throughput / 5.0f;
        float avg_avgEntertime = total_avgEntertime / 5.0f;
        float avg_worst_entry_time = total_worst_entry_time / 5.0f;
        outputfile1<< avg_throughput<<endl;
        outputfile2<<avg_avgEntertime<<endl;
        outputfile3<<avg_worst_entry_time<<endl;
        ss.clear();
    }
    
}




int main() {
    
    
    // Read parameters from file
    string inputFilePathExp1 = "./InputFiles/experiment_1_input_file.txt";
    string inputFilePathExp2 = "./InputFiles/experiment_2_input_file.txt";
    string inputFilePathExp3 = "./InputFiles/experiment_3_input_file.txt";
    string inputFilePathExp4 = "./InputFiles/experiment_4_input_file.txt";
    
    ofstream outFileFilter("./OutputFiles/LockFilter.txt");
    outFileFilter <<"Filter lock Output:"<<endl;


     // Experiment 1 & 3
    ofstream outFile("./OutputFiles/experiment_1_output_file_filterLock.txt");
    ofstream outFile3("./OutputFiles/experiment_3_output_file_filterLock.txt");
    ofstream outFile3_1("./OutputFiles/experiment_3_1_output_file_filterLock_worst_Entry.txt");
    
    LockExperiment(inputFilePathExp1 , outFile , outFile3,outFile3_1,outFileFilter);
     //Experiment 2 & 4
    ofstream outFile2("./OutputFiles/experiment_2_output_file_filterLock.txt");
    ofstream outFile4("./OutputFiles/experiment_4_output_file_filterLock.txt"); 
    ofstream outFile3_2("./OutputFiles/experiment_3_2_output_file_filterLock_worst_Entry.txt");
    LockExperiment(inputFilePathExp2 , outFile2,outFile4,outFile3_2,outFileFilter);

 
    return 0;
}
