#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<signal.h>
#include<syslog.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include<string.h>
#include<time.h>

#define BUFF_SIZE 1024
#define DIRECTORY_SIZE MAXNAMLEN

void main2(void);
int ssu_daemon_init(void);
void monitoring();

void main2(void){
	/*디몬프로세스를 실행하고 모니터링하는 함수*/
	char *buf;
	
	mkdir("check",0777);

	ssu_daemon_init();

	monitoring();

}

int ssu_daemon_init(void)
{
	pid_t pid;
	int fd, maxfd;
	
	if((pid=fork())<0){
		fprintf(stderr,"fork error\n");
		exit(1);
		}
	else if(pid!=0)
		exit(0);

	pid=getpid();
	
	setsid();
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	maxfd=getdtablesize();

	for(fd=0;fd<maxfd;fd++)
		close(fd);

	umask(0);

	fd=open("log.txt",O_RDWR);
	dup(0);
	dup(0);
	return 0;
}


void monitoring()
{
	FILE *fp;
	char *p=".";
	struct dirent *dentry,**namelist,**namelist2;
	struct stat statbuf[BUFF_SIZE];
	char filename[DIRECTORY_SIZE+1];
	int filecount,filecount2,i,j;
	time_t now;
	struct tm *tm_p;
	char buf[BUFF_SIZE];
	int exist,modify;
	char newfile[BUFF_SIZE],newfile2[BUFF_SIZE];
	struct stat statbuf2[BUFF_SIZE];

	chdir("./check");
	
	filecount=scandir(p,&namelist,NULL,alphasort);
	for(i=0;i<filecount;i++)
	{
		stat(namelist[i]->d_name,&statbuf[i]);
	}
/*해당디렉터리 파일들을 namelist로 dentry로 입력받음*/
	/*그리고 이때 stat구조체로도 statbuf에 저장*/
	while(1){
		
		fp=fopen("../log.txt","a+");

		filecount2=scandir(p,&namelist2,NULL,alphasort);
		for(i=0;i<filecount2;i++)
		{
			stat(namelist2[i]->d_name,&statbuf2[i]);
		}
	/*namelist2로 해당 디렉터리 스캔함*/

		if(filecount<filecount2)//파일생성됐을경우
		{
			for(i=0;i<filecount2;i++)
			{
				exist=0;
				if(strcmp(namelist2[i]->d_name,".")==0||strcmp(namelist2[i]->d_name,"..")==0)
					continue;
				for(j=0;j<filecount;j++)
				{
					if(strcmp(namelist[j]->d_name,".")==0||strcmp(namelist[j]->d_name,"..")==0)
						continue;

					if(strcmp(namelist2[i]->d_name,namelist[j]->d_name)==0)
					{
						exist=1;
					}
				}
/*이전 namelist와 namelist2의 d_name을 비교해서 새로운게 있는지 확인*/
				if(exist==0)
				{
					strcpy(newfile,namelist2[i]->d_name);
					break;
				}
			}

			
			time(&now);
			tm_p=localtime(&now);
			strftime(buf,BUFF_SIZE,"[%Y-%m-%d %X][create_",tm_p);
			fputs(buf,fp);
			strcpy(newfile2,newfile);
			strcat(newfile2,"]\n");
			fputs(newfile2,fp);
			
/*fp파일로 출력함*/
		}
		else if(filecount>filecount2)//파일삭제됐을경우
		{
			for(i=0;i<filecount;i++)
			{
				exist=0;
				if(strcmp(namelist[i]->d_name,".")==0||strcmp(namelist[i]->d_name,"..")==0)
					continue;
				for(j=0;j<filecount2;j++)
				{
					if(strcmp(namelist2[j]->d_name,".")==0||strcmp(namelist2[j]->d_name,"..")==0)
						continue;
					if(strcmp(namelist[i]->d_name,namelist2[j]->d_name)==0)
					{
						exist=1;
					}
				}
			/*이전 namelist와 namelist2의 filecount를 비교해서 줄어들었을경우 비교해서 삭제된파일 이름알아냄*/
				if(exist==0)
				{
					strcpy(newfile,namelist[i]->d_name);
					break;
				}
			}
				time(&now);
				tm_p=localtime(&now);
				strftime(buf,BUFF_SIZE,"[%Y-%m-%d %X][delete_",tm_p);
				fputs(buf,fp);
				strcpy(newfile2,newfile);
				strcat(newfile2,"]\n");
				fputs(newfile2,fp);

		}
		
		else//파일수정되거나 그대로인경우
		{
			modify=0;
			for(i=0;i<filecount2;i++)
			{
				stat(namelist2[i]->d_name,&statbuf2[i]);
			}

			for(i=0;i<filecount;i++)
			{
				if(strcmp(namelist[i]->d_name,".")==0||strcmp(namelist[i]->d_name,"..")==0)
					continue;
				for(j=0;j<filecount2;j++)
				{
					if(strcmp(namelist2[j]->d_name,".")==0||strcmp(namelist2[j]->d_name,"..")==0)
						continue;
					if(strcmp(namelist[i]->d_name,namelist2[j]->d_name)==0)
					{
						if(statbuf[i].st_mtime!=statbuf2[j].st_mtime)
							{
								strcpy(newfile,namelist[i]->d_name);
								modify=1;
								
							}
					}
				}
				if(modify==1)
				{
					time(&now);
					tm_p=localtime(&now);
					strftime(buf,BUFF_SIZE,"[%Y-%m-%d %X][modify_",tm_p);
					fputs(buf,fp);
					strcpy(newfile2,newfile);
					strcat(newfile2,"]\n");
					fputs(newfile2,fp);
					
				}
			}
		}

		filecount=scandir(p,&namelist,NULL,alphasort);
		for(i=0;i<filecount;i++)
		{
			stat(namelist[i]->d_name,&statbuf[i]);
		}

		fclose(fp);
	}
		
}
