#include "isEqual.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;;  // 파일 목록에 대한 락
pthread_mutex_t termination_lock = PTHREAD_MUTEX_INITIALIZER;;  // 쓰레드 종료에 대한 락
pthread_mutex_t starter_lock = PTHREAD_MUTEX_INITIALIZER;;  // 쓰레드 시작에 대한 락
pthread_cond_t  termination_cond  = PTHREAD_COND_INITIALIZER;

file_list file_list_head = {0x0, 0x0, 0x0} ;
file_list_ptr diff_line = &file_list_head ;
file_list_ptr same_line ;

int dup_cnt ;
int thread_cnt ;

void print_file_list() 
{
    printf("The numbef of identical file : %d\n", dup_cnt) ;
    file_list_ptr itr ;
    file_list_ptr ptr ;
    if (output_file == NULL)
    {
        printf("[\n") ;
        for (itr = file_list_head.diff ; itr != NULL ; itr = itr->diff)
        {
            printf("\t[\n") ;
            for (ptr = itr ; ptr != NULL ; ptr = ptr->same)
                printf("\t\t%s,\n", ptr->filename) ;
            printf("\t],\n") ;
        }
        printf("]\n") ;
    }
    else
    {
        FILE *file = fopen(output_file, "w") ;
        if (file == NULL)
        {
            perror(output_file) ;
            return ;
        }
        int cnt = 0 ;
        fprintf(file, "[\n") ;
        for (itr = file_list_head.diff ; itr != NULL ; itr = itr->diff)
        {
            fprintf(file, "\tlevel : %d\n\t[\n", cnt++) ;
            for (ptr = itr ; ptr != NULL ; ptr = ptr->same)
                fprintf(file, "\t\t%s,\n", ptr->filename) ;
            fprintf(file, "\t],\n") ;
        }
        fprintf(file, "]\n") ;

        fclose(file) ;
    }
}

void handle_sigint() 
{
    printf("Interrupt signal received\n") ;
    printf("Files found:\n") ;
    print_file_list() ;
    exit(0) ;
}

void keycontrol(int sig)
{
    if(sig == SIGINT)
        puts("   CTRL+C pressed") ;
    // print current results
    handle_sigint() ;
}

void timeout(int sig)
{
    if (sig == SIGINT)
    {    
        printf("The numbef of identical file : %d\n", dup_cnt) ;
        alarm(5) ; 
    }
}

// are_files_equal 함수는 파일 하나와 파일 목록 전체를 비교합니다.
int are_files_equal(const char * path1, const char * path2)
{
    // 두 파일을 열고, 파일 디스크립터를 얻는다.
    FILE * file1 = fopen(path1, "rb") ;
    if (file1 == NULL)
    {
        perror(path1) ;
        return 0 ;
    }

    FILE * file2 = fopen(path2, "rb") ;
    if (file2 == NULL)
    {
        perror(path2) ;
        fclose(file1) ;
        return 0 ;
    }

    // 두 파일의 크기를 비교한다.
    fseek(file1, 0, SEEK_END) ;
    fseek(file2, 0, SEEK_END) ;
    long size1 = ftell(file1) ;
    long size2 = ftell(file2) ;

    if (size1 != size2)
    {
        // 크기가 다르면 내용도 다르다고 판단한다.
        fclose(file1) ;
        fclose(file2) ;
        return 0 ;
    }

    // 파일의 크기가 같으면, 각 바이트를 비교한다.
    rewind(file1) ;
    rewind(file2) ;
    char buf1[BUF_SIZE], buf2[BUF_SIZE] ;
    size_t bytes_read1, bytes_read2 ;
    do
    { 
        bytes_read1 = fread(buf1, 1, BUF_SIZE, file1) ;  
        bytes_read2 = fread(buf2, 1, BUF_SIZE, file2) ;

        if (bytes_read1 != bytes_read2 || memcmp(buf1, buf2, bytes_read1) != 0)
        {
            // 바이트 수가 다르거나, 메모리 비교 결과가 다르면 두 파일은 다르다.
            fclose(file1) ;
            fclose(file2) ;
            return 0 ;
        }
    } while (bytes_read1 > 0) ;

    // 파일을 비교한 후에는 반드시 닫아준다.
    fclose(file1) ;
    fclose(file2) ;

    // 파일의 크기와 모든 바이트가 동일하므로 두 파일은 같다.
    return 1 ;
}

void *add_file_to_list(char *filename)
{
    file_list_ptr itr = (file_list_ptr)malloc(sizeof(file_list)) ;
    itr->filename = filename ;
    itr->diff = NULL ;
    itr->same = NULL ;

    return (void * )itr ;    
}

void *compare_files_thread(void *arg)
{
    ThreadArgs *args = (ThreadArgs *)arg ; // arg를 ThreadArgs 형으로 변환

    printf("Thread %d started\n", args->thread_id) ;  // 스레드가 시작될 때 메시지 출력

    int start ;
    int end ;
    int flag_cnt ;
    
    for (int i = 0 ; i < args->file_count ; i++)
    {
        pthread_mutex_lock(&starter_lock) ;
        thread_cnt = args->num_threads ;
        if (args->file_list[i].flag != 0)
        {
            pthread_mutex_unlock(&starter_lock) ;
            if (args->file_list[i].flag != dup_cnt - 1) 
                continue ;
        }
        else
        {
            flag_cnt = dup_cnt++ ;
            args->file_list[i].flag = flag_cnt ;
            diff_line->diff = (file_list_ptr) add_file_to_list(args->file_list[i].filepath) ;
            diff_line = diff_line->diff ;
            same_line = diff_line ;
            pthread_mutex_unlock(&starter_lock) ;
        }

        // 이 스레드가 담당할 파일 범위를 계산
        start = i + 1 + args->thread_id * (args->file_count - i - 1) / args->num_threads ;
        end = i + 1 + (args->thread_id + 1) * (args->file_count - i - 1) / args->num_threads ;
        printf("1") ;
        for (int j = start ; j < end ; j++)
        {   // j를 i 이후의 파일을 가리키도록 초기화
            // 두 파일이 같은지 검사한다.
            if (are_files_equal(args->file_list[i].filepath, args->file_list[j].filepath))
            {
                printf("'%s' and '%s' are equal, thread number: %d\n", args->file_list[i].filepath, args->file_list[j].filepath, args->thread_id) ;
                
                pthread_mutex_lock(&lock);
                args->file_list[j].flag = flag_cnt ;
                same_line->same = (file_list_ptr) add_file_to_list(args->file_list[j].filepath) ;
                same_line = same_line->same ;
                pthread_mutex_unlock(&lock) ;
            }
        }
        
        pthread_mutex_lock(&termination_lock) ;
        thread_cnt-- ;
        // printf("thread number: %d, %d : %d, %d\n", args->thread_id, thread_cnt, start, end) ;
        pthread_mutex_unlock(&termination_lock) ;

        while (thread_cnt > 0) ;
    }
    
    printf("Thread %d finished\n", args->thread_id) ;  // 스레드 작업 완료 시 메시지 출력
    return NULL ;
}

// 지정된 디렉토리 내의 모든 파일을 검사하고 파일 목록에 추가하는 함수
void check_files_in_dir(const char * dir_path, file_path file_list[], int * file_count, int bound_size)
{
    // opendir 함수를 통해 디렉토리를 열고, 이 디렉토리의 스트림 정보를 얻는다.
    DIR * dir = opendir(dir_path) ;
    if (dir == NULL)
    {
        perror(dir_path) ; 
        return ;
    }
    // readdir 함수를 통해 디렉토리 내부의 파일 또는 디렉토리를 하나씩 읽는다.
    struct dirent * entry ;
    while ((entry = readdir(dir)) != NULL)
    {
        // "."과 ".."은 현재 디렉토리와 상위 디렉토리를 가리키므로, 이를 건너뛴다.
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue ;
        }

        // 전체 파일 경로를 만든다.
        char full_path[MAX_PATH] ;
        snprintf(full_path, MAX_PATH, "%s/%s", dir_path, entry->d_name) ;

        // stat 함수를 통해 해당 파일의 정보를 얻는다.
        struct stat statbuf ;
        if (stat(full_path, &statbuf) == -1)
        {
            perror(full_path) ;
            continue ;
        }

        if (S_ISDIR(statbuf.st_mode))
        {
            // 만약 디렉토리라면, 재귀적으로 이 함수를 호출하여 그 디렉토리 내부를 탐색한다.
            check_files_in_dir(full_path, file_list, file_count, bound_size) ;
        }
        else if (S_ISREG(statbuf.st_mode) && bound_size < statbuf.st_size)
        {
            // 만약 일반 파일이라면, 해당 파일 경로를 파일 목록에 추가한다.
            strncpy(file_list[*file_count].filepath, full_path, MAX_PATH) ;
            file_list[*file_count].flag = 0 ;
            (* file_count)++ ;
        }
    }

    // opendir로 열었던 디렉토리는 반드시 closedir을 통해 닫아준다.
    closedir(dir) ;

    dup_cnt = 0 ;
}