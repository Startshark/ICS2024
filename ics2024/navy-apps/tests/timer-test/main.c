#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <NDL.h>

int main(){
    NDL_Init(0);
    const int period = 500;
    uint32_t timeus = NDL_GetTicks();
    int cnt = 0;
    while(1){
        uint32_t now = NDL_GetTicks();
        if((now - timeus) / period > cnt){
            printf("Hello, World! %d\n", cnt++);
            cnt = (now - timeus) / period;
        }
    }
    NDL_Quit();
    return 0;
}