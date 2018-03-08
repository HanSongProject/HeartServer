#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include "readFile.h"

#define  MAX_PATH 260

ReadFile::ReadFile()
{

}


void deblank(char string[])  
{  
    int i=0;
    int j=0;
    while(string[j]!='\0'){
        if(string[j]!=' '){
            string[i]=string[j];
            i++;
            j++;
        }
        else
        {
            // string[i]=string[j+1];
            j++;
        }
    }
    string[i]='\0';
} 

int ReadFile::GetCurrentPath(char *pFileName)
{

    static char buf[MAX_PATH];
    memset(buf,0,sizeof(buf));
#ifdef WIN32
    GetModuleFileName(NULL,buf,MAX_PATH);
#else
    char pidfile[64];
    int bytes;
    int fd;

    sprintf(pidfile, "/proc/%d/cmdline", getpid());

    fd = open(pidfile, O_RDONLY, 0);
    bytes = read(fd, buf, 256);
    close(fd);
    buf[MAX_PATH] = '\0';
#endif
    char * p = &buf[strlen(buf)];

    do
    {
        *p = '\0';
        p--;
#ifdef WIN32
    } while( '\\' != *p );
#else
    } while( '/' != *p );
#endif

    p++;
    //配置文件目录
    memcpy(p,pFileName,strlen(pFileName));
    pName = buf;
    return 0;

}

char* ReadFile::GetIniKeyString(char *title, char *key)
{
    FILE *fp;
    char szLine[1024];
    static char tmpstr[1024];
    int rtnval;
    int i = 0;
    int flag = 0;
    char *tmp;
    //  string retStr;
    const char *filename = pName;

    if((fp = fopen(filename, "r")) == NULL)
    {
        printf("have   no   such   file \n");
        return NULL;
    }
    while(!feof(fp))
    {
        rtnval = fgetc(fp);
        if(rtnval == EOF && sizeof(szLine) != 0)
        {
            break;
        }
        else
        {
            szLine[i++] = rtnval;
        }
        if(rtnval == '\n')
        {
#ifdef WIN32
            i--;
            szLine[--i] = '\0';
#endif
            szLine[i-1] = '\0';
            i = 0;
            tmp = strchr(szLine, '=');
            if(( tmp != NULL )&&(flag == 1))
            {
                if(strstr(szLine,key)!=NULL)
                {
                    //注释行
                    if ('#' == szLine[0])
                    {
                    }
                    else if ( '/' == szLine[0] && '/' == szLine[1] )
                    {

                    }
                    else
                    {
                        //找打key对应变量
                        strncpy(tmpstr,tmp+1,strlen(tmp-2));
                        deblank(tmpstr);
                        fclose(fp);
                        //  retStr = tmpstr;
                        return tmpstr;
                    }
                }

            }
            else
            {
                strcpy(tmpstr,"[");
                strcat(tmpstr,title);
                strcat(tmpstr,"]");
                if( strncmp(tmpstr,szLine,strlen(tmpstr)) == 0 )
                {
                    //找到title
                    flag = 1;
                }
            }
            memset(szLine,0,1024);

        }
    }
    fclose(fp);
    return NULL;
}

int ReadFile::SetIniKeyString(char *title, char *key, char *value)
{
    FILE *fp;
    char szLine[1024];
    static char tmpStr[1024];
    int rtnval;
    int i = 0;
    int flag = 0;
    char *tmp;

    const char *filename = pName;
    if((fp = fopen(filename, "rb+")) == NULL)
    {
        printf("have   no   such   file \n");
        return 0;
    }
    while(!feof(fp))
    {
        rtnval = fgetc(fp);
        if(rtnval == EOF && sizeof(szLine) != 0)
        {
            break;
        }
        else
        {
            szLine[i++] = rtnval;
        }
        if(rtnval == '\n')
        {
#ifdef WIN32
            i--;
            szLine[--i] = '\0';
#endif
            szLine[i-1] = '\0';
            i = 0;
            tmp = strchr(szLine, '=');
            if(( tmp != NULL )&&(flag == 1))
            {
                if(strstr(szLine,key)!=NULL)
                {
                    //注释行
                    if ('#' == szLine[0])
                    {
                    }
                    else if ( '/' == szLine[0] && '/' == szLine[1] )
                    {

                    }
                    else
                    {
                        //找打key对应变量
                        int length = strlen(szLine);
                        int pos = tmp + 1 - szLine;
                        fseek(fp,-(length - pos +1),SEEK_CUR);
                        //strncpy(tmpStr,tmp+1,strlen(tmp-2));


                        fputs(value,fp);

                        int tail = length - pos - strlen(value);
                        printf("%d  %d\n",length - pos +1,tail);
                        if(tail > 0)
                        {
                            while(tail--)
                            {
                                fputc(' ',fp);
                            }

                        }


                        fclose(fp);
                        return 1;
                    }
                }

            }
            else
            {
                strcpy(tmpStr,"[");
                strcat(tmpStr,title);
                strcat(tmpStr,"]");
                if( strncmp(tmpStr,szLine,strlen(tmpStr)) == 0 )
                {
                    //找到title
                    flag = 1;
                }
            }
            memset(szLine,0,1024);

        }
    }
    fclose(fp);
    return 0;
}

int ReadFile::WriteIniKeyString(char *title,char *key, char *value)
{
    FILE *fp;
    char szLine[1024];
    static char tmpstr[1024];
    int rtnval;
    int i = 0;
    int ret = 0;

    const char *filename = pName;
    if((fp = fopen(filename, "rb+")) == NULL)
    {
        printf("have   no   such   file \n");
        return ret;
    }
    while(!feof(fp))
    {
        rtnval = fgetc(fp);
        if(rtnval == EOF && sizeof(szLine) != 0)
        {
            break;
        }
        else
        {
            szLine[i++] = rtnval;
        }
        if(rtnval == '\n')
        {
#ifdef WIN32
            i--;
            szLine[--i] = '\0';
#endif
            szLine[i-1] = '\0';
            i = 0;
            strcpy(tmpstr,"[");
            strcat(tmpstr,title);
            strcat(tmpstr,"]");
            if( strncmp(tmpstr,szLine,strlen(tmpstr)) == 0 )
            {
                //找到title
                ret = 1;
                fseek(fp,0,SEEK_END);
                fprintf(fp,"%s=%s\n",key,value);
                fclose(fp);
                return ret;
            }

            memset(szLine,0,1024);

        }
    }
    ret = 1;
    fprintf(fp,"[%s]\n",title);
    fseek(fp,0,SEEK_END);
    fprintf(fp,"%s=%s\n",key,value);
    fclose(fp);
    return ret;
}

#if 0
int main()
{
    GetCurrentPath("Config.ini");
    char Ip[1024];
    strcpy(Ip,GetIniKeyString("FTP", "serverIp"));
    char *Name = GetIniKeyString("FTP", "userName");
    SetIniKeyString("FTP","serverIp","192.68.1.1");
    printf("%s\n", Ip);
    printf("%s\n", Name);
    printf("%s\n",GetIniKeyString("FTP", "serverIp"));

    char *ipA = GetIniKeyString("FTP", "serverIp");
    printf("%d\n", strlen(ipA));

    //  PutIniKeyString("CAT", "name", "ccc", "config.ini");
    //  PutIniKeyInt("DOG", "age", 56, "config.ini");
    return 0;

}
#endif
