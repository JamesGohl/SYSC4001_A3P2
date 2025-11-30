#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fstream>
#include <unistd.h>
#include <semaphore.h>
#include <cstring> 

struct SharedExamData{
    char rubric[5]; // keeps track of the letters in rubric
    char student_number[5];   // std::string doesn't work with shared memory
    bool questions_marked[5]; //knows if an exam has been marked or not
    sem_t sem_rubric; //rubric semaphore
    sem_t sem_exam; //exam semaphore
    sem_t sem_question; //question semaphore
};

//loads the rubric into shared memory
void loadRubric(std:: string file_name, SharedExamData *shared){
    std::ifstream file(file_name);
    std::string line;
    int file_line = 0;
    while(std::getline(file, line) && file_line < 5) {
        shared->rubric[file_line++] = line[2]; //line[2] is the 3rd char after number and comma
    }
    file.close();

}
void randomDelay(double min, double max){
    min *= 1000000; //convert to seconds
    max *= 1000000;
    int delay = (rand() % ((int)max - (int)min)) + min;
    usleep(delay);
    // usleep(1); // testing purpouses so dont have to wait

}

//loads the exam into shared memory
void loadExam(std:: string file_name, SharedExamData *shared){
    std::ifstream file(file_name);
    int file_line = 0;
    std::string line;
    std::getline(file, line);
    // copy into char array safely
    strncpy(shared->student_number, line.c_str(), sizeof(shared->student_number));
    shared->student_number[4] = '\0';    
    for(int i = 0; i < 5; i ++){
        shared->questions_marked[i] = false; //questions have not been marked when first loaded
    }
    file.close();
}




int main(int argc, char *argv[]){
    if (argc < 2){
        std::cout <<"Enter num tas\n";
        exit(0);
    }

    int num_TAs = std::stoi(argv[1]);

    if(num_TAs < 3){
        std::cout <<"To little tas\n";
        exit(0);
    }
    

    //create the shared memory
    SharedExamData *shared = (SharedExamData *) mmap(NULL, sizeof(SharedExamData),
                                                PROT_READ | PROT_WRITE, 
                                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);


    //Initializes the semaphore     
    sem_init(&shared->sem_rubric, 1, 1); // 1 shared between process, 1 initial value 
    sem_init(&shared->sem_exam, 1, 1);
    sem_init(&shared->sem_question, 1, 1);     


    sem_wait(&shared->sem_rubric);                
    loadRubric("rubric.txt", shared); // loads the rubric into shared memory
    sem_post(&shared->sem_rubric);
    
    //load the first exam into memory
    int exam_number = 1;
    sem_wait(&shared->sem_exam);  
    loadExam("exam_folder/exam" + std::to_string(exam_number), shared);
    sem_post(&shared->sem_exam);

    //fork n tas
    for (int n = 0; n < num_TAs; n++){
        int ta_num = n + 1;
        pid_t pid = fork(); //forks the as
        if(pid == 0){// child
            while(true){
                for(int i = 0; i < 5; i++){
                    std::cout << "TA " + std::to_string(ta_num) + " reviewing rubric q " + std::to_string(i+1) +"\n";
                    randomDelay(0.5, 1);
                    if(rand() % 5 == 0) { //1/5 chances of changing rubric
                        shared->rubric[i]++; // adds 1 to asci code
                        // rewrite new rubric
                        sem_wait(&shared->sem_rubric);   
                        std::ofstream write_rubric("rubric.txt");
                        std::cout << "TA " + std::to_string(ta_num) + " changing rubric for question " + std::to_string(i+1) +"\n";

                        for(int j = 0; j < 5; j++){
                            write_rubric << std::to_string((j+1)) + "," + std::string(1, shared->rubric[j]) + "\n";
                        }
                        sem_post(&shared->sem_rubric);

                    } 

                }
                //mark exam questions

                for(int i = 0; i < 5; i ++){
                    sem_wait(&shared->sem_question); // so that 2 aren't marked true at same time
                    if(!shared->questions_marked[i]) {
                        shared->questions_marked[i] = true;
                        sem_post(&shared->sem_question);
                        std::cout << "TA " + std::to_string(ta_num) + " marking student " <<
                                    shared->student_number << " question" + std::to_string(i+1) + " \n";
                        randomDelay(1, 2);

                    }
                    sem_post(&shared->sem_question);
                }
                
                
                exam_number++;
                if(strcmp(shared->student_number, "9999") != 0){
                    sem_wait(&shared->sem_exam); // wait until can load exam
                    sem_wait(&shared->sem_question);
                    loadExam("exam_folder/exam" + std::to_string(exam_number), shared);
                    sem_post(&shared->sem_exam);
                    sem_post(&shared->sem_question);
                }
                else{
                    break; //end of loop reached last student
                }
                
                
            }
            exit(0); //child exits

        }
       
    }
     //parent waits for all children to finish
        for(int i = 0; i < num_TAs; i ++){
            wait(NULL);
        } 
}
