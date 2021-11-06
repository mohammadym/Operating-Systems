#include <stdio.h>
#include <math.h>
#include <string.h>
int main()
{
int plus_opp_index;
int hash_opp_index;
int equal_opp_index;
int null_opp_index;
int first_num=0;
int second_num=0;
int third_num=0;
int first_lenght=0;
int second_lenght=0;
int third_lenght=0;
int a=0;
char input[33];
int counter;
for (counter=0;counter<33;counter++)
{
    input[counter]='n';
}
scanf("%s", input);
for (counter=0;counter<33;counter++)
{
if (input[counter]=='#')
	hash_opp_index=counter;
if (input[counter]=='+')
	plus_opp_index=counter;
if (input[counter]=='=')
	equal_opp_index=counter;
if (input[counter]=='n'|| input[counter]=='\0')
{
    null_opp_index=counter;
    break;
}
}
int power=0;
for(counter=plus_opp_index-1;counter>=0;counter--)
{
	a=input[counter]-'0';
	first_num+=((int)pow(10,power))*a;
	power++;
	first_lenght++;
}
power=0;
for(counter=equal_opp_index-1;counter>=plus_opp_index+1;counter--)
{
	a=input[counter]-'0';
	second_num+=((int)pow(10,power))*a;
	power++;
	second_lenght++;
}
power=0;
for(counter=null_opp_index-1;counter>=equal_opp_index+1;counter--)
{
	a=input[counter]-'0';
	third_num+=((int)pow(10,power))*a;
	power++;
	third_lenght++;
}
if (hash_opp_index<plus_opp_index)
{
    int temp=third_num-second_num;
    int i=0;
    int copy_num=temp;
    while(copy_num>0)
    {
        copy_num/=10;
        i++;
    }
    char str_temp[i];
    char new_num[i];
    for(counter=0;counter<i;counter++)
    {
        str_temp[counter]=temp%10+'0';
        temp/=10;
    }
    strrev(str_temp);
    int ekhtelaf=i-first_lenght;
    if(hash_opp_index==0)
    {
        for(counter=0;counter<ekhtelaf;counter++)
            new_num[counter]='#';
        int k=0;
        for(counter=ekhtelaf;counter<i;counter++)
        {
            new_num[counter]=input[k];
            k++;
        }
    }
    if(hash_opp_index==first_lenght-1)
    {
        for(counter=0;counter<first_lenght;counter++)
            new_num[counter]=input[counter];
        int k=counter;
        counter=0;
        while(counter<ekhtelaf)
        {
            new_num[k]='#';
            counter++;
            k++;
        }
    }
    else
    {
        for(counter=0;counter<hash_opp_index+1;counter++)
            new_num[counter]=input[counter];
        int k=counter;
        counter=0;
        while(counter<ekhtelaf)
        {
            new_num[k]='#';
            counter++;
            k++;
        }
        for(counter=k;counter<i;counter++)
            new_num[counter]=input[counter];
    }
    printf("%s\n",str_temp);
    printf("%s\n",new_num);
    for(counter=0;counter<i;counter++)
    {
        if(new_num[counter]=!str_temp[counter])
        {
        	printf("%d\n",1);
            printf("%d\n",-1);
            return 0;
        }
        if(new_num[counter]='#')
        {
            continue;
        }
    }
    printf("%d+%d=%d",third_num-second_num,second_num,third_num);
}
if (hash_opp_index>plus_opp_index && hash_opp_index<equal_opp_index)
{
    int temp=third_num-first_num;
    int i=0;
    int copy_num=temp;
    while(copy_num>0)
    {
        copy_num/=10;
        i++;
    }
    char str_temp[i];
    char new_num[i];
    for(counter=0;counter<i;counter++)
    {
        str_temp[counter]=temp%10+'0';
        temp/=10;
    }
    strrev(str_temp);
    int ekhtelaf=i-second_lenght;
    if(hash_opp_index==plus_opp_index+1)
    {
        counter=0;
        while(counter<ekhtelaf)
        {
            new_num[counter]='#';
            counter++;
        }
        int j=1;
        for(counter;counter<i;counter++)
        {
            new_num[counter]=input[plus_opp_index+j];
        }
    }
    if(hash_opp_index==second_lenght-1)
    {
        for(counter=0;counter<second_lenght;counter++)
            new_num[counter]=input[counter+plus_opp_index+1];
        int k=counter;
        counter=0;
        while(counter<ekhtelaf)
        {
            new_num[k]='#';
            counter++;
            k++;
        }
    }
    else
    {
        int k=0;
        for(counter=plus_opp_index+1;counter<hash_opp_index+1;counter++)
        {
            new_num[k]=input[counter];
            k++;
        }
        counter=0;
        while(counter<ekhtelaf)
        {
            new_num[k]='#';
            counter++;
            k++;
        }
        int j=0;
        for(counter=k;counter<i;counter++)
        {
            new_num[counter]=input[hash_opp_index+1+j];
            j++;
        }
    }
    printf("%s\n",new_num);
    printf("%s\n",str_temp);
    for(counter=0;counter<i;counter++)
    {
        if(new_num[counter]=!str_temp[counter])
        {
            printf("%d\n",-1);
            return 0;
        }
        if(new_num[counter]='#')
        {
            continue;
        }
    }
    printf("%d+%d=%d",first_num,third_num-first_num,third_num);
}
if (hash_opp_index>equal_opp_index)
{
    int temp=first_num+second_num;
    int i=0;
    int copy_num=temp;
    while(copy_num>0)
    {
        copy_num/=10;
        i++;
    }
    char str_temp[i];
    char new_num[i];
    for(counter=0;counter<i;counter++)
    {
        str_temp[counter]=temp%10+'0';
        temp/=10;
    }
    strrev(str_temp);
    int ekhtelaf=i-third_lenght;
    if(hash_opp_index==equal_opp_index+1)
    {
        counter=0;
        while(counter<ekhtelaf)
        {
            new_num[counter]='#';
            counter++;
        }
        int j=1;
        for(counter;counter<i;counter++)
        {
            new_num[counter]=input[equal_opp_index+j];
        }
    }
    if(hash_opp_index==third_lenght-1)
    {
        for(counter=0;counter<third_lenght;counter++)
            new_num[counter]=input[counter+equal_opp_index+1];
        int k=counter;
        counter=0;
        while(counter<ekhtelaf)
        {
            new_num[k]='#';
            counter++;
            k++;
        }
    }
    else
    {
        int k=0;
        for(counter=equal_opp_index+1;counter<hash_opp_index+1;counter++)
        {
            new_num[k]=input[counter];
            k++;
        }
        counter=0;
        while(counter<ekhtelaf)
        {
            new_num[k]='#';
            counter++;
            k++;
        }
        int j=0;
        for(counter=k;counter<i;counter++)
        {
            new_num[counter]=input[hash_opp_index+1+j];
            j++;
        }
    }
    for(counter=0;counter<i;counter++)
    {
        if(new_num[counter]=!str_temp[counter])
        {
            printf("%d\n",-1);
            return 0;
        }
        if(new_num[counter]='#')
        {
            continue;
        }
    }
    printf("%d+%d=%d\n",first_num,second_num,first_num+second_num);
}
}
