# Script to generate input files

# Function to write data to a file
def write_to_file(filename, data):
    with open(filename, 'w') as f:
        for line in data:
            f.write(' '.join(map(str, line)) + '\n')

# 1st Input File: Vary n from 2 to 64 (powers of 2), k=15, lambda1=1, lambda2=2
n_values = [2**i for i in range(1, 7)]  # 2, 4, 8, 16, 32, 64
data1 = [(10,n, 1, 2) for n in n_values]
write_to_file('./InputFiles/experiment_1_input_file.txt', data1)

# 2nd Input File: Vary k from 5 to 25, n=16, lambda1=1, lambda2=2
k_values = range(5, 26)
data2 = [(k, 16, 1, 2) for k in k_values]
write_to_file('./InputFiles/experiment_2_input_file.txt', data2)

# 3rd Input File: Vary n from 2 to 64 (powers of 2), k=10, lambda1=1, lambda2=2
data3 = [(10, n, 1, 2) for n in n_values]
write_to_file('./InputFiles/experiment_3_input_file.txt', data3)

# 4th Input File: Vary k from 5 to 25, n=16, lambda1=1, lambda2=2
data4 = [(k, 16, 1, 2) for k in k_values]
write_to_file('./InputFiles/experiment_4_input_file.txt', data4)

print("Input files generated successfully.")
