import os
import subprocess
import platform

# Updated list of source files with safer names
source_files = [
    "Assgn1-Chunk-EE21BTECH11015/Assgn1-Chunk-EE21BTECH11015.cpp", 
    "Assgn1-Mixed-EE21BTECH11015/Assgn1-Mixed-EE21BTECH11015.cpp", 
    "Assgn1-Dynamic-EE21BTECH11015/Assgn1-Dynamic-EE21BTECH11015.cpp"
]

# Detect the platform
current_platform = platform.system()
print(f"Detected platform: {current_platform}")

# Compilation and execution command based on platform
if current_platform == "Windows":
    compile_cmd = "g++ -std=c++11 -pthread {source_file} -o executable.exe"
    run_cmd = "executable.exe"
else:
    compile_cmd = "g++ -std=c++11 -pthread {source_file} -o executable"
    run_cmd = "./executable"

# Loop through each source file, compile, and run
for source_file in source_files:
    # Print the current source file being processed
    print(f"Processing source file: {source_file}")
    
    # Check if the source file exists
    if not os.path.exists(source_file):
        print(f"Error: Source file {source_file} does not exist!")
        continue
    
    # Extract the program name from the source file path
    program_name = os.path.splitext(os.path.basename(source_file))[0]
    print(f"Program name extracted: {program_name}")

    # Construct the compile command
    compile_command = compile_cmd.format(source_file=source_file, program_name=program_name)
    print(f"Compilation command: {compile_command}")

    # Compile the source file
    compile_process = subprocess.run(compile_command, shell=True)
    
    # Check if the compilation was successful
    if compile_process.returncode == 0:
        print(f"Compilation of {source_file} successful. Running {program_name}...")
        
        # Construct the run command
        run_command = run_cmd.format(program_name=program_name)
        print(f"Run command: {run_command}")

        # Run the compiled program
        subprocess.run(run_command, shell=True)
    else:
        print(f"Compilation of {source_file} failed. Return code: {compile_process.returncode}")

print("Finished processing all source files.")
