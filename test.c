#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int parseRegNum(char test[])
{
    char res[strlen(test)-1];
    int intRes;
    for (int i = 1; i < strlen(test); i++)
    {
        res[i - 1] = test[i];
    }
    intRes = atol(res);
    return intRes;
}
int parseInt(char test[])
{
    int intRes;
    intRes = atol(test);
    return intRes;
}
int main()
{
    char test[7] = "R3288";
    printf("%d",parseRegNum(test));
    return 0;
}