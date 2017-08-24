Navigate to the compression_service folder

Open ternminal and run make clean;make

Test Scripts:
	Navigate to Test_cases folder->select the test case to run->open terminal->run the shell scripts(use chmod +x "shellscriptname")
	Run first server.sh on one terminal and other scripts on different terminals[Try to run APPx.sh scripts at same time]

Running Independent test cases:
	Open Terminal-> type in "./Server <specify the max size of shared memory>"
	Open terminal-> Navigate to Client1 or Client2 type in "./App* <filenames>"
	* is 1,2,3,4

Note:
*--->enumerate with numbers, Application_C1.c and Application_C2.c uses synchronuous calls and Application_C3.c and Application_C4.c uses asynchrounous calls
filenames must be separated with commas not spaces(eg: yuy.txt,oiuy.txt)
While Running independent test cases, make sure all the files are in the compression_service folder


To break out of Server, Press Ctrl+C. This will safely delete the shared size variable so that no application shared memory segments persists on the system.  