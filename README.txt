We decorate our implementation with mostly mutexes (possibly looking for mutex-conditions) to control the concurrency over threads.
We use just one semaphore for any commentator thread to wait for other commentator threads' decision of answering.
We ensure that all mutex-wait functions are backed by mutex-locks, before mutex-wait calls in their scopes.
Detailed explanations of concepts and objects used in the program are commented in the main.c file, for further observations.

We also created a makefile thanks to which we have below commands from terminal:
make all --> compiles program
make compile --> compiles program
make test --> compiles then performs sample test run with parameters -n 4 -p 0.75 -q 5 -t 3 -b 0.05
make clean --> deletes previous compiled file

Current main.c does not contain breaking news functionality. We had implemented it very basically and worked actually but, due to the time left, we
have not tested and improved the code and logging enough, so we choose not to include c file with the breaking news functionality in the submission zip. 


