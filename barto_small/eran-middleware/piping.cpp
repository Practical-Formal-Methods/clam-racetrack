#include<iostream>
#include<vector>
#include<cstdlib>
#include<utility>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fstream>

using namespace std;

int main(){
    int fd1[2];
    int fd2[2];
    pid_t p;

    if(pipe(fd1)==-1 || pipe(fd2)==-1){
        cout << "Could not create pipes." << endl;
        exit(1);
    }

    std::vector<std::pair<int, int>> input_box_int_old = {
        {19,19},
        {7,7},
        {-2,-2},
        {-4,-4},
        {5,5},
        {5,5},
        {5,5},
        {7,7},
        {16,16},
        {4,4},
        {4,4},
        {4,4},
        {0,0},
        {7,7}
    };

    std::vector<std::pair<int, int>> input_box_int = {
 {19,19},
 {11,11},
 {0,0},
 {0,0},
 {6,6},
 {7,7},
 {0,0},
 {6,6},
 {0,0},
 {6,6},
 {15,15},
 {0,0},
 {13,13},
 {11,11}

    };
    p = fork();

    if(p < 0){
        cout << "Fork failed." << endl;
        exit(1);
    }

    //Parent process - CRAB
    //Assuming we have formed our input in input_box_int 
    else if(p > 0){
        close(fd1[0]); //close reading end of first pipe

        int arr[28];
        for(int i=0;i<14;i++){
            arr[2*i] = input_box_int[i].first;
            arr[2*i+1] = input_box_int[i].second;
        }

        //Write the input to the pipe and close writing end
        write(fd1[1], (const void* )(arr), 28*sizeof(int));
        close(fd1[1]); // close writing end of first pipe

        //Wait for child
        wait(NULL);

        close(fd2[1]); //close writing end of second pipe

        //Read from the input of second pipe
        int out_size;
        read(fd2[0], (void *)(&out_size), sizeof(int));
        int *out_arr = (int *)(calloc(2*out_size, sizeof(int)));
        read(fd2[0], (void *)(out_arr), 2*out_size*sizeof(int));

        std::vector<std::pair<int, int>> output_box_int;
        for(int i=0;i<2*out_size;i+=2){
            output_box_int.push_back(pair<int, int>(out_arr[i], out_arr[i+1]));
        }

        close(fd2[0]); //close reading end of second pipe
        //Print the output - will be converted to disjuncts
        std::cout << "Number of actions received : " << out_size << std::endl;
        int count=1;
        for(auto it: output_box_int){
            cout << "Action number " << count << " : " << it.first << "," << it.second << endl;
            count++;
        }
        
    }

    //Child process - after reqd manipulation, we exec to deepsymbol middleware
    else{
        // For deepsymbol, we can map fd1[0] and fd2[1] to stdin and stdout
        close(fd1[1]); //close writing end of the first pipe
        close(fd2[0]); //close reading end of second pipe
        
        //Read bound integers from first pipe
        int *bounds_arr = (int *)(malloc(28*sizeof(int)));

        read(fd1[0], (void *)(bounds_arr), 28*sizeof(int));

        //Convert to char * for exec
        //31 args to middleware: 1 path, 1 fd, 28 bounds, NULL
        char *args[31];
        args[0] = (char *)(malloc(43*sizeof(char)));
        args[0] = "/eran-middleware/middleware";
        args[1] = (char *)(malloc(sizeof(int)+sizeof(char)));
        sprintf(args[1], "%d\0", fd2[1]); //File descriptor for child process to write to
        for(int i=2; i<30; i++){
            args[i] = (char *)(malloc(sizeof(int)+sizeof(char)));
            sprintf(args[i], "%d\0", bounds_arr[i-2]);
        }
        args[30] = NULL;

        close(fd1[0]);
        execv(args[0], args);
        cout << "Failed to execute deepsymbol" << endl;
        exit(1);

    }

    return 0;

}
