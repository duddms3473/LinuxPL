#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<dirent.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<time.h>
#include<sys/time.h>

#define BUFF_SIZE 1024
#define DIRECTORY_SIZE MAXNAMLEN
#define SECOND_TO_MICRO 1000000

extern void main2(void);

void set_trash();
void delete_(char *token1,int index);
int timecheck(char *token2, char *token3,int index);
void tree();
void un_link(char *token1);
void infocheck();
int findfilename(char *token1);
void help();
void recover(char *token1);
void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);

char tmp1[BUFF_SIZE];
char tmp2[BUFF_SIZE];

int select_(const struct dirent *dirent);

int main(void)
{
	pid_t pid;
	char choose[BUFF_SIZE];
	char tokens[BUFF_SIZE][BUFF_SIZE];
	char *token;
	char del[]=" ";
	int n=0,i,leng;
	int timetosleep;
	struct timeval begin_t,end_t;
	
	gettimeofday(&begin_t,NULL);
	/*프로그램 작동시간을 알기위해 시작시간 저장*/

	if((pid=fork())<0){
		fprintf(stderr,"fork error\n");
		exit(1);
	}

	/*fork해서 자식프로세스는 main2인 디몬프로세스를 실행하도록 해줌*/
	else if (pid==0)
	{
		main2();
	}

	while(1)
	{
	
		char buf[BUFF_SIZE]={'\0'};
		n=0;
		fflush(stdin);

		while(1)
		{

			printf("20172532>");
			fgets(buf,BUFF_SIZE,stdin);
		
			if(strlen(buf)==1)
				continue;
			else
				break;
		}
/*프롬프트 실행*/
/*엔터만 입력된 경우 while문으로 계속 반복*/
		token=strtok(buf,del);
		while(token!=NULL)
		{
			strcpy(tokens[n++],token);
			token=strtok(NULL,del);
		}
/*입력받은 것들을 token으로 분리해서 tokens배열에 저장함*/
		if(strcmp(tokens[0],"delete\n")==0)
		{
			printf("No filename Input.\n");
		}
/*delete만 입력받고, 파일명은 입력하지 않은경우 예외출력*/
		else if(strcmp(tokens[0],"delete")==0)
		{
/*delete인 경우*/
			if(strcmp(tokens[2],"")==0)//바로삭제의 경우 index를 1로해줌
			{
			delete_(tokens[1],1);
			}
			else
			{
				if((pid=fork())<0)
				{
					fprintf(stderr,"fork error\n");
					exit(1);
				}
				/*시간이 입력된경우 fork해서 시간이 지난후 delete하도록 처리해줌*/

				else if(pid>0)
					continue;
				/*부모프로세스의경우 다시 처음프롬프트를 진행*/
				else
				{

					if(strcmp(tokens[4],"")==0)//지정시간+옵션없는경우.
					{
						timetosleep=timecheck(tokens[2],tokens[3],1);
						/*입력된 시간을 tokens로 보내 timecheck 함수로 남은시간을 계산해준다*/
						sleep(timetosleep);
						delete_(tokens[1],2);
						/*sleep으로 기다린후 delete함*/
						
					}
					else
					{
						if(strcmp(tokens[4],"-i\n")==0)//trash이동없이 바로삭제
						{/*i옵션인경우*/
							
							timetosleep=timecheck(tokens[2],tokens[3],2);
							/*timecheck로 남은시간 계산후 sleep*/
							sleep(timetosleep);
							/*시간이지나면 un_link로 바로 삭제해준다*/
							un_link(tokens[1]);
							
						}
						else if(strcmp(tokens[4],"-r\n")==0)//지정시간 삭제시 재확인
						{/*r옵션인 경우*/
							printf("Delete [y/n]? ");
							/*우선 delete할것인지 물어본다.*/
							fgets(choose,BUFF_SIZE,stdin);
							if(strcmp(choose,"y\n")==0)
							{/*y를 입력받은경우 sleep후 delete함*/
								
								timetosleep=timecheck(tokens[2],tokens[3],3);
								
								sleep(timetosleep);
								delete_(tokens[1],2);
								
							}
							else
								exit(0);
								
						
						}
					}
				}
			}
		}

		else if(strcmp(tokens[0],"size\n")==0)
		{
			printf("size input\n");
		}
		else if(strcmp(tokens[0],"recover")==0)
		{	/*recover입력받은경우 입력받은 파일명 recover 함수로 전달*/
			recover(tokens[1]);
		}
		else if(strcmp(tokens[0],"tree\n")==0)
		{/*tree입력받은 경우 tree함수이용해 디렉터리 tree형태로 출력*/
			tree();
		}
		else if(strcmp(tokens[0],"exit\n")==0)
		{/*exit입력받은 경우, 프로그램 실행에 걸린시간 출력후 종료메시지출력후 종료*/
			gettimeofday(&end_t,NULL);
			ssu_runtime(&begin_t,&end_t);

			printf("The end monitoring program..\n");
			exit(0);
		}
		else/*이외에 명령어를 입력받은경우 help로 메세지 출력*/
			help();
	
	}
}
void recover(char *token1)
{
	char filename[BUFF_SIZE],filetype[BUFF_SIZE];
	char *pch;
	char del[]=".";
	DIR *dirp;
	char *dirp2="./files";
	struct dirent *dentry;
	char name[BUFF_SIZE];
	int i=0;
	char tmp[BUFF_SIZE];
	char path[BUFF_SIZE],path1[BUFF_SIZE];
	int len;
	char tmptoken[BUFF_SIZE];
	struct dirent **namelist;
	int i2,count;
	struct stat statbuf;
	char tm1[BUFF_SIZE],tm2[BUFF_SIZE];
	char buff[BUFF_SIZE];
	FILE *fp;
	int choose;
	char choicefile[BUFF_SIZE],choicepath[BUFF_SIZE];

	len=strlen(token1);
	token1[len-1]=0;
/*입력받은 파일명 끝에 엔터를 0으로해줌*/
	strcpy(tmptoken,token1);

	
	pch=strtok(tmptoken,del);
	strcpy(filename,pch);
	pch=strtok(NULL,del);
	strcpy(filetype,pch);
/*파일명과 확장자를 token으로 분리해서 filename,filetype에 각각 저장*/
	
	if((dirp=opendir("./trash/info"))==NULL||chdir("./trash/info")==-1)
	{
		fprintf(stderr,"opendir,chdir error for trash/info\n");
		exit(1);
	}
/*info디렉터리에 있는 파일들 dentry에 저장하여 filename 비교함*/
	while((dentry=readdir(dirp))!=NULL)
	{
		if(dentry->d_ino==0)
			continue;
		
		strcpy(name,dentry->d_name);

		if((strcmp(name,".")==0)||strcmp(name,"..")==0)
			continue;
		else
		{
			pch=strtok(name,"_,.");
			/*중복되는 파일의경우 _로 구분해줬기때문에 delimiter를 _ .으로 구별함*/
			if(strcmp(pch,filename)==0)
			{
				while(pch!=NULL)
				{
					strcpy(tmp,pch);
					pch=strtok(NULL,"_,.");
				
				}
				if(strcmp(tmp,filetype)==0)
				{
					i++;
				}
			}
		}
	}
/*파일명이 같은게 몇개있는지 i에 저장함*/
	closedir(dirp);

	if(i==0) 
	{
		printf("There is no '%s' in the 'trash' directory\n",token1);

		chdir("..");
		chdir("..");
	}
/*입력받은 파일명에 해당하는 파일이 없는경우 없다고 출력해줌*/
	else if(i==1)
	{/*중복된 파일이 없는경우*/
		
		strcpy(path,"../../check/");
		strcat(path,token1);
		
		strcpy(path1,"../files/");
		strcat(path1,token1);

		rename(path1,path);
		unlink(token1);
/*info파일은 unlink해주고, files에 있는 파일은 rename으로 경로 변경해준다*/
		chdir("..");
		chdir("..");
		
	}

	else
	{/*파일이 중복되는경우*/
		strcpy(tmp1,filename);
		strcpy(tmp2,filetype);

		chdir("..");
	

		count=scandir(dirp2,&namelist,select_,NULL);

		chdir("./info");
		
		for(i2=0;i2<count;i2++)
		{
			fp=fopen(namelist[i2]->d_name,"r");
			fgets(buff,BUFF_SIZE,fp);
			fgets(buff,BUFF_SIZE,fp);
			fgets(buff,BUFF_SIZE,fp);
			len=strlen(buff);
			buff[len-1]=0;
			printf("%d. %s.%s %s ",i2+1,filename,filetype,buff);
			fgets(buff,BUFF_SIZE,fp);
			printf("%s",buff);
			fclose(fp);
		}
		/*info디렉터리에서 파일이름이 같은파일을 열어 파일에 있는 D,M시간을 출력해준다*/
		printf("Choose : ");
		scanf("%d",&choose);
/*recover를 원하는 파일 번호 입력받음*/
		strcpy(choicefile,namelist[choose-1]->d_name);
		fp=fopen(choicefile,"r");
		fgets(buff,BUFF_SIZE,fp);
		fgets(buff,BUFF_SIZE,fp);
		fclose(fp);

		len=strlen(buff);
		buff[len-1]=0;

		
		unlink(choicefile);
/*해당info파일 unlink해서 없애줌*/
		chdir("..");
		strcpy(choicepath,"./files/");
		strcat(choicepath,choicefile);

		rename(choicepath,buff);
/*files에 있는 해당파일 recover를 위해 rename해줌*/
	}


}
int select_(const struct dirent *dirent)
{
	/*namelist중 원하는 file이 있는지 찾기위한 함수, 미리 저장해둔 tmp1,tmp2로 filenmae과 filetype을 비교해줌*/
	char *pch;
	char name[100];
	char name2[100];
	char name3[100];
	char del[]=".,_";

	strcpy(name,dirent->d_name);
	
	if(strcmp(name,".")==0||strcmp(name,"..")==0)
		return 0;

	pch=strtok(name,del);
	strcpy(name2,pch);

	while(pch!=NULL)
	{
		strcpy(name3,pch);
		pch=strtok(NULL,del);
	}

	if(strcmp(name2,tmp1)==0 && strcmp(name3,tmp2)==0)
		return 1;
/*filename과 filetype이 같으면 1을 리턴함*/
	else return 0;

}

void set_trash()
{
	/*trash폴더 설정을위해 만든 함수*/
	DIR *dirp;
	if((dirp=opendir("trash"))==NULL)
	{
		
		mkdir("trash",0777);
		chdir("./trash");
		mkdir("files",0777);
		mkdir("info",0777);
		chdir("..");
		}

}
void delete_(char *token1,int index)
{

	char filename[100];
	char trashfile[100];
	char *pch,*pch2;
	char del[]="/";
	FILE *fp;
	char name[100],infoname[100];
	int len=strlen(token1);
	char real[BUFF_SIZE],copy[BUFF_SIZE];
	struct tm *timeinfo;
	struct stat statbuf;
	time_t prev_time,now_time;
	char now_string[BUFF_SIZE];
	int ind;
	char realfilename[100];
	int str_len;
	char copyfilename[100];
	char stor1[BUFF_SIZE],stor2[BUFF_SIZE];
	if(index==1) 
		token1[len-1]=0;
	else 
		token1[len]=0;
	
	set_trash();
/*delete전 trash 디렉터리 설정*/
	pch=strtok(token1,del);
	while(pch!=NULL)
	{
		strcpy(filename,pch);
		pch=strtok(NULL,del);
	}	

	strcpy(realfilename,filename);
	strcpy(copyfilename,filename);

	if((ind=findfilename(copyfilename))>0)
	{/*해당 filename이 여러개인경우, 기존 delete할 파일명에 _숫자 를 넣어준다.*/
		pch=strtok(realfilename,".");
		strcpy(stor1,pch);
		
		pch2=strtok(NULL,".");
		strcpy(stor2,pch2);
		

		realfilename[0]=0;
		strcpy(realfilename,stor1);
		strcat(realfilename,"_");

		str_len=strlen(realfilename);
		
		realfilename[str_len]=ind+48;

		strcat(realfilename,".");
		strcat(realfilename,stor2);
	}


	strcpy(name,"./check/");
	strcat(name,filename);

	realpath(name,real);
	stat(name,&statbuf);

	prev_time=statbuf.st_mtime;

	strcpy(trashfile,"./trash/files/");
	strcat(trashfile,realfilename);

	strcpy(infoname,"./trash/info/");
	strcat(infoname,realfilename);

	fp=fopen(infoname,"a+");
	strcpy(copy,"[Trash info]\n");
	strcat(copy,real);
	
	timeinfo=localtime(&prev_time);
	strftime(now_string,BUFF_SIZE,"\nD : %Y-%m-%d %H:%M:%S\n",timeinfo);
	strcat(copy,now_string);
	/*info파일에 해당파일의 최근수정시간을 입력*/
	
	time(&now_time);

	timeinfo=localtime(&now_time);
	strftime(now_string,BUFF_SIZE,"M : %Y-%m-%d %H:%M:%S\n",timeinfo);
/*info파일에 삭제시간 입력*/
	strcat(copy,now_string);
	fputs(copy,fp);
	fclose(fp);

	rename(name,trashfile);
/*rename으로 trash로 이동해줌*/
	infocheck();


}

int findfilename(char *filename)
{
	/*trash에 같은 filename이 같은 파일이 몇개있는지 알아내는 함수*/
	struct dirent *dentry;
	DIR *dirp;

	char filename2[BUFF_SIZE];
	char *pch;
	char newname[BUFF_SIZE];
	int len;
	int c=48;
	if((dirp=opendir("./trash/files"))==NULL||chdir("./trash/files")==-1)
	{
		fprintf(stderr,"opendir, chdir error\n");
		exit(1);
	}
	strcpy(filename2,filename);

	while ((dentry=readdir(dirp))!=NULL){
		if(strcmp(filename2,dentry->d_name)==0)
		{
			c++;
			
			rewinddir(dirp);
			pch=strtok(filename2,".");
			strcpy(newname,pch);
			strcat(newname,"_");
			
			len=strlen(newname);
			newname[len]=c;

			strcat(newname,".");

			pch=strtok(NULL,".");
			strcat(newname,pch);

			strcpy(filename2,newname);
			continue;
		}
	}
	closedir(dirp);

	chdir("..");
	chdir("..");
	return c-48;
			

}
void infocheck()
{/*info디렉터리가 2KB가 넘었는지 확인하고 넘으면 가장 오래된 파일을 delete한다*/
	struct dirent *dentry;
	struct stat statbuf;
	DIR *dirp;
	char filename[BUFF_SIZE];
	char minfile[BUFF_SIZE];
	int filesize;
	time_t mintime;
	char delfile[BUFF_SIZE];
	if((dirp=opendir("./trash/info"))==NULL||chdir("./trash/info")==-1)
	{
		fprintf(stderr,"opendir, chdir error for trash/info\n");
		exit(1);
	}

	while(1)/*info파일이 2KB보다 작아질때까지 가장 오래된파일 삭제함*/
	{
		filesize=0;
		mintime=1000000000000000;
		while((dentry=readdir(dirp))!=NULL){
			if(dentry->d_ino==0)
				continue;

			memcpy(filename,dentry->d_name,BUFF_SIZE-1);

			if(strcmp(filename,".")==0||strcmp(filename,"..")==0) continue;

			if(stat(filename,&statbuf)==-1){
				fprintf(stderr,"stat error for %s\n",filename);
				exit(1);
			}
/*해당 filenmae파일을 statbuf 구조체로 받아 시간을 알아낸다*/

			filesize+=statbuf.st_size;
			if(statbuf.st_mtime<mintime) 
			{
				strcpy(minfile,filename);
				mintime=statbuf.st_mtime;
			}
		}

		if(filesize<2048) 
		{
			closedir(dirp);
			chdir("..");
			chdir("..");
			break;
		}
		else
		{
			unlink(minfile);
			strcpy(delfile,"../files/");
			strcat(delfile,minfile);
			unlink(delfile);
			rewinddir(dirp);
		}

	}

}
int timecheck(char *token2, char *token3,int index)
{/*입력받은 delete시간을 token으로 구별하여 time으로 바꿔줘서 delete까지 남은시간을 계산해준다*/
	char *pch;
	char del[]="-,:";
	int i=0;
	char input_time[8][8];
	int len=strlen(token3);
	time_t now;
	struct tm setime;
	time_t tt;

	pch=strtok(token2,del);

	while(pch!=NULL)
	{
		strcpy(input_time[i++],pch);
		pch=strtok(NULL,del);
	}

	if(index==1) token3[len-1]=0;
	pch=strtok(token3,del);

	while(pch!=NULL)
	{
		strcpy(input_time[i++],pch);
		pch=strtok(NULL,del);
	}

	setime.tm_year=atoi(input_time[0])-1900;
	setime.tm_mon=atoi(input_time[1])-1;
	setime.tm_mday=atoi(input_time[2]);
	setime.tm_hour=atoi(input_time[3]);
	setime.tm_min=atoi(input_time[4]);
	setime.tm_sec=0;
	setime.tm_isdst=0;

	time(&now);
	tt=mktime(&setime);

	return (tt-now);

}

void un_link(char *token1)
{/*check폴더에 있는 token파일을 unlink로 완전히 delete해줌*/
	char filename[100];
	char *pch;
	char del[]="/";
	int len=strlen(token1);
	
	token1[len]=0;
	pch=strtok(token1,del);
	
	while(pch!=NULL)
	{
		strcpy(filename,pch);
		pch=strtok(NULL,del);
	}
	
	chdir("./check");
	unlink(filename);
	chdir("..");

}

void tree(){/*tree모양으로 해당 디렉터리를 출력한다*/
	struct dirent *dentry,*dentry2;
	struct stat statbuf;
	char filename[DIRECTORY_SIZE+1];
	DIR *dirp,*dirp2;
	
	dirp=opendir("./check");
	chdir("./check");
	printf("check------------");
	while((dentry=readdir(dirp))!=NULL){
	
		if(dentry->d_ino==0)
			continue;

		if(strcmp(dentry->d_name,".")==0||strcmp(dentry->d_name,"..")==0)
			continue;
	
		memcpy(filename,dentry->d_name,DIRECTORY_SIZE);
		stat(filename,&statbuf);

		if((statbuf.st_mode&S_IFMT)==S_IFREG)
			printf("-%s\n\t\t|\n\t\t|\n\t\t|",filename);
		else{/*디렉터리 파일인경우*/

				printf("-%-10s",filename);
				
				dirp2=opendir(filename);
				chdir(filename);
				printf("-------------");
				while((dentry2=readdir(dirp2))!=NULL){
					if(dentry2->d_ino==0)
						continue;
					if(strcmp(dentry2->d_name,".")==0||strcmp(dentry2->d_name,"..")==0)
						continue;
					memcpy(filename,dentry2->d_name,DIRECTORY_SIZE);

					printf("-%s\n\t\t|\t\t\t|\n\t\t|\t\t\t|",filename);
				}
				printf("\n\t\t|");
				chdir("..");
		}

	}
	chdir("..");
	printf("\n");
}

void help(){/*help명령어 출력*/
      printf("Delete : delete [FILENAME] [END_TIME] [OPTION]\n");
      printf("Size : size [FILENAME] [OPTION]\n");
	  printf("Recover : recover [FILENAME] [OPTION]\n");
      printf("Tree : tree\n");
      printf("Exit : exit\n");
      printf("Help : help\n");
  }

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t)
{/*입력받은 시작시간과 끝시간을 이용해 프로그램 작동시간 계산*/
	
	end_t->tv_sec-=begin_t->tv_sec;
	if(end_t->tv_usec<begin_t->tv_usec){
		end_t->tv_sec--;
		end_t->tv_usec+=SECOND_TO_MICRO;
	}

	end_t->tv_usec-=begin_t->tv_usec;
	printf("Runtime: %ld:%06ld(sec:usec)\n",end_t->tv_sec,end_t->tv_usec);
}
