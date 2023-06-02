#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define MAX_PATH 1024
#define BUF_SIZE 8192
#define MAX_FILES 1024
#define MAX_PATH 1024
#define MAX_THREADS 64 // 스레드 최대 개수를 정의

char *output_file ;

// 같은 내용이라는 flag를 위한 구조체 정의
typedef struct
{
    char filepath[MAX_PATH] ;
    int flag ;
} file_path;

// 여러개의 인자를 스레드로 보내기 위한 구조체 정의
typedef struct
{
    file_path (*file_list) ; // 파일 리스트
    int file_count ; // 파일 개수
    int thread_id ; // 스레드 ID
    int num_threads ; // 스레드 개수
} ThreadArgs ;

struct _file_list
{
    char *filename ;
    struct _file_list *diff ;
    struct _file_list *same ;
} ;

typedef struct _file_list   file_list ;
typedef struct _file_list * file_list_ptr;

void * add_file_to_list(char *filename) ;
  
void print_file_list() ;

void handle_sigint() ;
void keycontrol(int sig) ;
void timeout(int sig) ;

// are_files_equal 함수는 파일 하나와 파일 목록 전체를 비교합니다.
int are_files_equal(const char *path1, const char *path2) ;
void *compare_files_thread(void *arg) ;

// 지정된 디렉토리 내의 모든 파일을 검사하고 파일 목록에 추가하는 함수
void check_files_in_dir(const char *dir_path, file_path file_list[], int *file_count, int bound_size) ;