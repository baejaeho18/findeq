#include <stdio.h>

int main()
{
    int start ;
    int end ;
    int flag_cnt ;

    int file_count = 18 ;
    int num_threads = 10 ;

    for (int t = 0 ; t < num_threads ; t++)
    {
        for (int i = 0 ; i < file_count ; i++)
        {
            // start, end by divide
            // distribute by %
            start = i + 1 + t * (file_count - i - 1) / num_threads ;
            end = i + 1 + (t + 1) * (file_count - i - 1) / num_threads ;
            
            for (int j = start ; j < end ; j++)
            { 
                printf("%d ", j) ;
            }
            printf("\t");
        }
        printf("\n");
    }
}