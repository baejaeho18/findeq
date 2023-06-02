#include "isEqual.h"

pthread_mutex_t thread_lock = PTHREAD_MUTEX_INITIALIZER;;  // 쓰레드에 대한 락

int main(int argc, char * argv[])
{
    printf("start the program. 👍\n") ;
    
    clock_t start, end ;
    double cpu_time_used ;

    signal(SIGINT, keycontrol) ;
    signal(SIGALRM, timeout) ;
    alarm(5) ;

    start = clock() ;
    
    int opt ;
    int num_threads ;
    int bound_size = 1024;
    output_file = NULL ;
    char * target_directory ;

    int i = 1 ;
    while ((opt = getopt(argc, argv, "t:m:o:")) != -1) {
        switch (opt) {
            case 't':
                strtok(argv[i++], "=") ;
                num_threads = atoi(strtok(NULL, "=")) ;
                break;
            case 'm':
                strtok(argv[i++], "=") ;
                bound_size = atoi(strtok(NULL, "=")) ;
                break ;
            case 'o':
                strtok(argv[i++], "=") ;
                output_file = strtok(NULL, "=") ;
                break ;
            default:
                exit(EXIT_FAILURE) ;
        }
    }
    // target directory path는 항상 마지막에 입력한다는 전제 하에 가능
    target_directory = argv[i] ;    // i -> optind also okay
    printf("%d %d %s %s\n", num_threads, bound_size, output_file, target_directory) ;

    if (num_threads <= 0 || MAX_THREADS < num_threads)
    {
        fprintf(stderr, "Invalid number of threads: %d\n", num_threads) ;
        return EXIT_FAILURE ;
    }

    struct stat statbuf ;
    if (stat(target_directory, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode))
    {
        fprintf(stderr, "%s is not a valid path\n", target_directory) ;
        return EXIT_FAILURE ;
    }

    // 파일 리스트를 저장할 배열 선언
    file_path file_list[MAX_FILES] ;
    int file_count = 0 ; // 파일 개수를 0으로 초기화

    // 디렉토리에서 파일을 찾고 리스트에 추가
    check_files_in_dir(target_directory, file_list, &file_count, bound_size) ;

    // 스레드 객체와 인자를 저장할 배열 생성
    pthread_t threads[MAX_THREADS] ;
    ThreadArgs thread_args[MAX_THREADS] ;

    // 각 스레드 생성 및 시작
    for (int i = 0 ; i < num_threads ; i++)
    {
        thread_args[i].file_list = file_list ;
        thread_args[i].file_count = file_count ;
        thread_args[i].thread_id = i ;
        thread_args[i].num_threads = num_threads ;

        if (pthread_create(&threads[i], NULL, compare_files_thread, &thread_args[i]) != 0)
        {
            perror("pthread_create") ;
            return EXIT_FAILURE ;
        }
    }

    // 모든 스레드가 끝날 때까지 기다림
    for (int i = 0 ; i < num_threads ; i++)
    {
        pthread_mutex_lock(&thread_lock);
        if (pthread_join(threads[i], NULL) != 0)
        {
            perror("pthread_join") ;
            return EXIT_FAILURE ;
        }
        pthread_mutex_unlock(&thread_lock);
    }
    
    print_file_list();

    end = clock() ;
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC ;
    printf("Execution time: %f seconds\n", cpu_time_used) ;

    return EXIT_SUCCESS ;
}