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
    atomic<bool> flag;
    atomic<int> level;
    atomic<thread::id> threadId;
    atomic<int> idt;
    threads(bool f, int l,thread::id id,int threadId) : flag(f),level(l), threadId(id) ,idt(threadId){}
} th;

class bakeryLock {
    int num_threads;
    std::vector<std::atomic<int>> label;  // Ticket numbers for each thread
    std::vector<std::atomic<bool>> flag;  // Flag indicating if thread is choosing a number

public:
    bakeryLock(int n) : num_threads(n), label(n), flag(n) {
        for (int i = 0; i < n; ++i) {
            label[i] = 0;
            flag[i] = false;
        }
    }

    void lock(int thread_id) {
        flag[thread_id].store(true);  // Indicate that this thread is choosing a number

        // Assign the thread the next largest ticket number
        int max_label = 0;
        for (int i = 0; i < num_threads; ++i) {
            max_label = std::max(max_label, label[i].load());
        }
        label[thread_id].store(max_label + 1);  // Set the label (ticket number)
        flag[thread_id].store(false);           // Finished choosing a number

        // Wait until it's this thread's turn
        for (int i = 0; i < num_threads; ++i) {
            if (i == thread_id) continue;  // Skip self-check

            // Wait while the other thread is choosing or has a lower label
            while (flag[i].load() || 
                   (label[i].load() != 0 && 
                    (label[i].load() < label[thread_id].load() || 
                     (label[i].load() == label[thread_id].load() && i < thread_id)))) {
                // Busy-wait (spinlock)
            }
        }
    }

    void unlock(int thread_id) {
        label[thread_id].store(0);  // Release the ticket
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

void testCS(int k,int id,float &avgEnterTime,float &worst_case, int lambda1, int lambda2, bakeryLock &Lock, ofstream &bakeryLockFile) {
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
        //bakeryLockFile<<"-----------------------------------------------------------------------"<<endl;
        // Time of actual CS entry
        auto actEnterTime = chrono::system_clock::now();
        csEnterTime = actEnterTime - reqEnterTime;
        csEnterTimeTotal += csEnterTime;
        thread_worst_time = max(thread_worst_time, csEnterTime);
        auto actEnterTimeString = getSysTime(actEnterTime);
        bakeryLockFile << i+1 << "th CS Entered at " << actEnterTimeString << " by thread " << id << endl;

        // Generate exponential delay with average lambda1 milliseconds
        double t1 = getExponentialDelay(lambda1);
        this_thread::sleep_for(chrono::milliseconds(static_cast<int>(t1)));

        // Time of CS exit request
        auto reqExitTime = chrono::system_clock::now();
        auto reqExitTimeString = getSysTime(reqExitTime);
        bakeryLockFile << i+1 << "th CS Requested Exit at " << reqExitTimeString << " by thread " << id << endl;
        //bakeryLockFile<<"--------------------------------------------------------------------------"<<endl;

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

void LockExperiment(string inputfilePath , ofstream &outputfile1 , ofstream &outputfile2 , ofstream &outputfile3, ofstream &bakeryLockFile){
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
        for(int i = 1; i <=5;i++){
        vector<thread> ths;
        bakeryLock lock(n);
        vector<float> avgEnterTimes(n);
        vector<float> worstEntryTime(n);
        float worst_entry_time = 0.0;
        float csEntrytime;
        auto StartTime =  chrono::system_clock::now();
        for (int i = 0; i < n; i++) {
            
            ths.push_back(thread(testCS, k,i,ref(avgEnterTimes[i]),ref(worstEntryTime[i]), lambda1, lambda2, ref(lock), ref(bakeryLockFile)));
            //cout<<"THe avgEnterTime :"<<avgEnterTimes[i]<<endl;
        }
        auto EndTime =  chrono::system_clock::now();
        chrono::duration<float> duration = EndTime - StartTime;
        //auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(EndTime - StartTime);
        // Calculate throughput
        float throughput = duration.count() / (k * n);
        total_throughput += throughput;
        float totalEnterTime = 0.0f;
        
        
      //  outputfile1<< throughput<<endl;
        
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
     
    ofstream outFileBakery("./OutputFiles/LockBakery.txt");
    outFileBakery <<"Filter lock Output:"<<endl;

     // Experiment 1 & 3
    ofstream outFile("./OutputFiles/experiment_1_output_file_bakeryLock.txt");
     ofstream outFile3("./OutputFiles/experiment_3_output_file_bakeryLock.txt");
      ofstream outFile3_1("./OutputFiles/experiment_3_1_output_file_bakeryLock_worst_Entry.txt");
    LockExperiment(inputFilePathExp1 , outFile , outFile3 , outFile3_1,outFileBakery);
     //Experiment 2 & 4
    ofstream outFile2("./OutputFiles/experiment_2_output_file_bakeryLock.txt");
    ofstream outFile4("./OutputFiles/experiment_4_output_file_bakeryLock.txt");
      ofstream outFile3_2("./OutputFiles/experiment_3_2_output_file_bakeryLock_worst_Entry.txt");
    LockExperiment(inputFilePathExp2 , outFile2,outFile4,outFile3_2,outFileBakery);

 
    return 0;
}
