Assignment Overview

This project is an implementation of a basic storage manager in C. The storage manager is a low-level module responsible for handling page-based file I/O. It provides a clean interface for creating, opening, closing, and deleting page files, as well as reading from and writing to specific pages within those files.

The core of the implementation is in storage_mgr.c, which uses standard C library functions to perform file operations. The file handle (SM_FileHandle) is used to maintain the state of an open file, including its name, total number of pages, current page position, and a FILE pointer for I/O operations.
Code Structure

The project is organized into the following key files:

•	storage_mgr.h: The public interface for the storage manager. It defines the SM_FileHandle and SM_PageHandle data structures and declares all the functions that can be called by a client of this module.

•	storage_mgr.c: The core implementation of the storage manager. It contains the logic for all the functions declared in the header file, from creating and opening files to complex read/write operations. It encapsulates all direct file manipulations.

•	dberror.h / dberror.c: These files define and manage the custom error codes used throughout the project. They provide a standardized way to handle and report errors, such as RC_FILE_NOT_FOUND or RC_WRITE_FAILED.

•	test_assign1_1.c: The main test suite for the storage manager. It includes tests for creating, opening, closing, and destroying files, as well as for verifying the content of pages after read and write operations.

•	test_helper.h: A helper file for the test suite that provides useful macros (TEST_CHECK, ASSERT_TRUE, etc.) to simplify the process of writing and running tests.

•	Makefile: The build script for the project. It compiles all the necessary .c files and links them to create the final test_assign1 executable. It also includes a clean command to remove generated object files and the executable.


How to Compile and Run

To compile the project, navigate to the project's root directory in your terminal and run this command

  “make” or “mingw32-make”
  
This will generate an executable file named test_assign1_1.exe. To run the tests, execute this file

“./test_assign1_1.exe”

If all tests pass, it will print a series of "OK" messages, confirming that the storage manager is functioning correctly.
VIDEO RECORDING OF THE OUTPUT

   https://www.loom.com/share/63c56d78c31345baa93d00da467621ad?sid=ad337f4f-da7d-4434-89ec-ae4974ca596b
