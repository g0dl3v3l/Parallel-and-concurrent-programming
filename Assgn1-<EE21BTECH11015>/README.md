# Parallel & Concurrent Programming: Autumn 2024 Programming Assignment 1 - Measuring Matrix Sparsity

This repository contains the code and resources for the assignment "Measuring Matrix Sparsity." The project is organized into several folders, each with specific content and purpose.

## Project Structure

### Source Folders

- **Assgn1-Chunk-EE21BTECH11015**
  - Contains the source file `Assgn1-Chunk-EE21BTECH11015.cpp`, which implements the Chunk Method.
- **Assgn1-Dynamic-EE21BTECH11015**

  - Contains the source file `Assgn1-Dynamic-EE21BTECH11015.cpp`, which implements the Dynamic Method.

- **Assgn1-Mixed-EE21BTECH11015**
  - Contains the source file `Assgn1-Mixed-EE21BTECH11015.cpp`, which implements the Mixed Method.

### Input Files

Each source folder has two input directories:

- **inputfiles**

  - Contains matrix inputs for experiments 1, 2, and 4.

- **SparseInputFiles**
  - Contains matrix inputs for experiment 3.

**Note:** In every matrix input file, the first line contains metadata: Matrix Size, Sparsity of the Matrix, default number of threads to be used for execution, and `rowInc` (only for the Dynamic method).

### Output Folder

Each source folder generates the following output files:

- **output.txt**

  - Data generated for experiment 1.

- **output_threads.txt**

  - Data generated for experiment 2.

- **output_Sparse.txt**

  - Data generated for experiment 3.

- **output_rowIncrement.txt**
  - Data generated for experiment 4 (exists only in `Assgn1-Dynamic-EE21BTECH11015`).

### Data Processing

- **data_processing/data_processing.ipynb**
  - A Jupyter notebook containing the data processing steps for the experiments. This file generates graphs and tables used in the report.

### Execution Script

- **execute.py**
  - A Python script to compile and run all the source files.

### Report

- **report**
  - Contains the report for the project, which includes low-level program design explanations for each method, as well as observations and analysis of each experiment.

## Running the Program

To execute the program, run the following command in the terminal:

```bash
python execute.py
```
