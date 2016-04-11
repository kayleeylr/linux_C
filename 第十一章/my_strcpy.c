#include <stdio.h>
#include <string.h>
#include <stdlib.h>
char *my_strcpy (char *strDest ,const char *strSrc)
{
		char *p_return = strDest;

		//检查参数指针是否为空
		if(strDest == NULL||strSrc == NULL)	{
				fprintf(stderr ,"NULL POINT !");
				return NULL;
		}
		while ((*strDest++ = *strSrc++) != '\0') ;
		return p_return ;
}

int main()
{
		char	string1[32];
		char	string2[32];
		int		c;
		int		i = 0;
		
		printf("please input your string :");
		//对输入字符串的长度进行检查
		while(((c = getchar() ) != '\n' ) &&(c != EOF ) &&( i < 31) ) {
				string2[i] = c;
				i++;
		}
		string2[i] = '\0';

		//对返回值也进行合法性检查
		if(my_strcpy(string1,string2 ) == NULL)	{
				fprintf(stderr ,"return from my_strcpy ");
				exit(1);
		}
		printf("string1 : %s \n",string1);
		printf("string2 : %s \n",string2);
		return 0;

}
