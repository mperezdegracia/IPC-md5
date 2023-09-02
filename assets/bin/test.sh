#!/bin/bash


# Create a test folder if it doesn't exist
mkdir -p test_files

# Create test files with predefined names
for i in {1..10}; do
    filename= "test_files/file$i.txt"
    echo "This is a test file $i" > $filename
done

echo "Test files created in the test_files folder."