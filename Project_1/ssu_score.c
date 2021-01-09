#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ssu_score.h"
#include "blank.h"

extern struct ssu_scoreTable score_table[QNUM];
extern char id_table[SNUM][10];

struct ssu_scoreTable score_table[QNUM];
char id_table[SNUM][10];

char stuDir[BUFLEN];
char ansDir[BUFLEN];
char errorDir[BUFLEN];
char threadFiles[ARGNUM][FILELEN];
char cIDs[ARGNUM][FILELEN];

int eOption = false;
int tOption = false;
int mOption = false;
int iOption = false;

void ssu_score(int argc, char *argv[])
{
	char saved_path[BUFLEN];
	int i;

	for(i = 0; i < argc; i++){
		if(!strcmp(argv[i], "-h")){
			/*입력받은 인자중 '-h'라는것이 있을경우 strcmp에의해 0을 return한다.*/
			print_usage();
			/*print_usage함수를 이용해 사용법을 출력함.*/
			return;
		}
	}

	memset(saved_path, 0, BUFLEN);
	/*saved_path 배열을 다 0으로 설정함.*/
	if(argc >= 3){
		strcpy(stuDir, argv[1]);
		strcpy(ansDir, argv[2]);
	}
/*stuDir, ansDir 입력받음*/

	if(!check_option(argc, argv))
		exit(1);
/*입력된 옵션을 확인한다*/

if(!eOption && !tOption && !mOption && iOption){
		do_cOption(cIDs);
		return;
	}
/*옵션으로 -i만 입력 받은경우*/

	getcwd(saved_path, BUFLEN);
/*현재 작업중인 디렉토리의 경로를 saved_path에  얻는다*/

	if(chdir(stuDir) < 0){
		fprintf(stderr, "%s doesn't exist\n", stuDir);
		return;
	}
/*stuDir로 디렉토리 변경*/

	getcwd(stuDir, BUFLEN);
/*stuDir에 현재 디렉토리 경로를 저장한다.*/

	chdir(saved_path);
	/*saved_path로 디렉토리 변경*/
	
	if(chdir(ansDir) < 0){
/*ansDir로 디렉토리 변경*/
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN);
/*ansDir에 현재 디렉토리 경로를 저장한다.*/

	chdir(saved_path);
/*다시 처음의 saved_path 디렉토리로 바꿔줌*/

	set_scoreTable(ansDir);
	/*scoreTable 설정 완료*/
	
	set_idTable(stuDir);
/*idtable 설정해준다.*/


	printf("grading student's test papers..\n");
	score_students();

	if(iOption)
		do_cOption(cIDs);

	return;
}

int check_option(int argc, char *argv[])
{
	int i, j;
	int c;

	while((c = getopt(argc, argv, "e:thmi")) != -1)
	{
		switch(c){
			case 'e':
				eOption = true;
				strcpy(errorDir, optarg);
/*e인경우 eOption을 true로 하고, 입력받은 optarg를 errorDir에 카피함*/

				if(access(errorDir, F_OK) < 0)
					mkdir(errorDir, 0755);
				/*errorDir 없는경우 생성*/
				else{
					rmdirs(errorDir);
					mkdir(errorDir, 0755);
					/*errorDir를 unlink 해주고 새로 생성한다.*/
				}
				break;
			case 't':
				tOption = true;
				i = optind;
				j = 0;
/*t인경우 tOption을 true로함*/
				while(i < argc && argv[i][0] != '-'){

					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
				/*가변인자를 센 j가 5가 넘을 경우 예외처리해줌*/
					else
						strcpy(threadFiles[j], argv[i]);
					i++; 
					j++;
/*가변인자 개수를 세어준다.*/
/*threadFiles에 입력받은 인자를 카피함*/
				}
				break;
			case 'm':
				mOption = true;
				break;
/*m옵션을 입력받는경우 mOption을 true로 변경*/
			case 'i':
				iOption = true;
				i = optind;
				j = 0;
/*i옵션을 입력받는경우 iOption을 true로 변경*/
				while(i < argc && argv[i][0] != '-'){

					if(j >= ARGNUM)
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					/*가변인자를 센 j가 5가 넘을경우 예외처리해준다.*/
					else
						strcpy(cIDs[j], argv[i]);
					i++; 
					j++;/*가변인자 개수를 세어준다.*/
/*cID배열에 입력받은 학번을 입력*/
				}
				break;
			case '?':
				printf("Unkown option %c\n", optopt);
				return false;

		/*알수없는 인자가 들어왔을 경우 예외처리*/
			}
	}

	return true;
}


void do_cOption(char (*ids)[FILELEN])
{/*score.csv 파일을 출력하는 함수*/
	FILE *fp;
	char tmp[BUFLEN];
	int i = 0;
	char *p, *saved;
	char tmp2[BUFLEN];

	sprintf(tmp2,"%s/%s",ansDir,"score_table.csv");

	read_scoreTable(tmp2);

	printf("%s",score_table[i].qname);
	if((fp = fopen("score.csv", "r")) == NULL){
		fprintf(stderr, "file open error for score.csv\n");
		return;
	}

	fscanf(fp, "%s\n", tmp);
/*score.csv 파일의 첫행먼저 입력 받음*/
	while(fscanf(fp, "%s\n", tmp) != EOF)
	{/*파일이 끝날때까지 score.csv파일을 한줄씩 입력받음.*/
		p = strtok(tmp, ",");
/*,쉼표로 strtok 해줌*/
		if(!is_exist(ids, tmp))
			continue;

		i=0;
/*is_exist로 입력받은 ids랑 tmp를 비교해서 원하는 학번을 찾음*/

		printf("%s's wrong answer : \n", tmp);

		while((p = strtok(NULL,","))!=NULL){
			
			if(atof(p)==0){
				printf("%s, ",score_table[i].qname);
			}

			i++;
		
		}

	}
	fclose(fp);
}

int is_exist(char (*src)[FILELEN], char *target)
{/*입력받은 cID중 target이 있는지 확인하는 함수*/
	int i = 0;

	while(1)
	{
		if(i >= ARGNUM)
			return false;
/*i가 5이상인경우 false*/
		else if(!strcmp(src[i], ""))
			return false;
/*src가 NULL일경우 false*/
		else if(!strcmp(src[i++], target))
			return true;
/*target과 같은 src가 있을 경우 true*/
	}
	return false;
}

void set_scoreTable(char *ansDir)
{
	char filename[FILELEN];

	sprintf(filename, "%s/%s", ansDir, "score_table.csv");
/*filename에 ANS_Dir경로/score_table.csv를 넣어줌*/
	if(access(filename, F_OK) == 0)
		read_scoreTable(filename);
	/*score_table.csv파일의 존재여부 확인하고 있으면 read_scoreTable.*/

	else{
		make_scoreTable(ansDir);
		/*score_table.csv파일이 존재하지않을경우 생성함*/
		write_scoreTable(filename);
		/*scoretable에 문제이름과 문제점수 입력완료*/
	}
}

void read_scoreTable(char *path)
{/*문제별 점수를 입력받는 함수이다.*/

	FILE *fp;
	char qname[FILELEN];
	char score[BUFLEN];
	int idx = 0;
	char probnum[50];
	char pscore[50];
	char *p;

	if((fp = fopen(path, "r")) == NULL){
		fprintf(stderr, "file open error for %s\n", path);
		return ;
		/*path를 읽기모드로 open함 , 실패시 NULL반환 + 에러처리*/
	}

	while(fscanf(fp, "%[^,],%s\n",qname,score) != EOF){
/*fp파일의 내용을 qname과 score에 반복하여 입력받음*/
		
		strcpy(score_table[idx].qname, qname);
		score_table[idx++].score =atof(score);
		/*score_table배열에 문제별 문제이름과 점수를 입력받는다.*/
	}

	fclose(fp);
		
	if(mOption){
		/*mOption일 경우*/
		while(1){
			printf("Input question's number to modify >> ");
			scanf("%s",probnum);
			if(!strcmp(probnum,"no")) break;
/*점수 수정을 원하는 probnum을 입력받는다. no일경우 break*/
			else{
				fp=fopen(path,"r");
				idx=0;
				/*score_table.csv파일을 읽기모드로 open함*/
				while(fscanf(fp,"%[^,],%s\n",qname,score)!=EOF){
					p=strtok(qname,".");
/*한줄씩입력받아 qname과 score로 문제이름과 점수를 입력받는다.*/
					/*입력받은 qname과 probnum을 비교해준다*/
					if(!strcmp(probnum,qname)){
						printf("Current score : %s\n",score);
						printf("New score : ");
						scanf("%s",pscore);
						score_table[idx].score=atof(pscore);
					}
/*비교했을때 문제이름이 같으면 현재점수 출력후 새 점수를 입력받는다.*/
/*현재 프로그램의 score_table배열의 점수를 수정해준다*/
					idx++;
				}

				fclose(fp);
			}
		}
	}
}

void make_scoreTable(char *ansDir)
{
	int type, num;
	double score, bscore, pscore;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int idx = 0;
	int i;

	num = get_create_type();
/*get_create_type에서 어떤유형으로 scoreTable만들건지 입력받음*/

	if(num == 1)
	{/*1로 입력받았을경우 빈칸유형,프로그램유형의 점수를 몇점으로할지 bscore와 pscore에 입력받음*/
		printf("Input value of blank question : ");
		scanf("%lf", &bscore);
		printf("Input value of program question : ");
		scanf("%lf", &pscore);
	}

	if((dp = opendir(ansDir)) == NULL){
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}	/*dp에 ansDir을 open함*/
	
	while((dirp = readdir(dp)) != NULL)
	{/*dp디렉터리의 포인터를 반환하며 dp내 파일을 읽게함.*/

		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		if((type = get_file_type(dirp->d_name)) < 0)
			continue;
/*dirp->d_name에 해당하는 파일타입을 찾는다. .c .txt 가아닐경우 continue*/
		strcpy(score_table[idx++].qname, dirp->d_name);
/*score_table[idx].qname에 dirp->d_name 카피해줌. 그리고 idx++해줌*/
	}

	closedir(dp);
/*문제이름 입력 완료*/
	sort_scoreTable(idx);
/*scoreTable을 sort해줌*/

	for(i = 0; i < idx; i++)
	{
		type = get_file_type(score_table[i].qname);
/*파일타입 입력받음*/
		if(num == 1)
		{
			if(type == TEXTFILE)
				score = bscore;
			else if(type == CFILE)
				score = pscore;
		}
		/*문제 type에따라 해당 점수 다르게해줌*/
		else if(num == 2)/*점수입력 유형이 2일경우*/
		{
			printf("Input of %s: ", score_table[i].qname);
			scanf("%lf", &score);
			/*한문제씩 점수를 입력받음*/
		}

		score_table[i].score = score;
		/*해당하는 score를 문제의 점수로 설정함*/
	}
}

void write_scoreTable(char *filename)
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]);
/*문제 몇개인지*/
	if((fd = creat(filename, 0666)) < 0){
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}
	/*filename에 해당하는 파일 creat함*/

	for(i = 0; i < num; i++)
	{
		if(score_table[i].score == 0)
			break;

		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score);
		/*tmp에 문제이름과 문제점수를 저장*/
		write(fd, tmp, strlen(tmp));
		/*파일에 문제이름과 문제점수를 저장*/
	}

	close(fd);
}


void set_idTable(char *stuDir)
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int num = 0;

	if((dp = opendir(stuDir)) == NULL){
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}
/*stuDir 오픈함.*/
	while((dirp = readdir(dp)) != NULL){
		/*학생dir 끝까지 read함*/
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name);
		/*tmp에 '디렉토리경로/학생디렉토리경로' 저장*/
		stat(tmp, &statbuf);
/*tmp에 해당하는 파일을 stat구조체로 가져옴*/
		if(S_ISDIR(statbuf.st_mode))
			strcpy(id_table[num++], dirp->d_name);
		/*해당파일이 디렉토리일경우 id_table에 디렉토리이름을 넣고, num추가함.. 이때 num은 학생 디렉토리 수를 가리키게됨. id_table은 디렉토리 이름인 학번이 담기게된다.*/
		else
			continue;
	}

	sort_idTable(num);
/*idTable을 sort해줌*/
}

void sort_idTable(int size)
{
	int i, j;
	char tmp[10];

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 -i; j++){
			if(strcmp(id_table[j], id_table[j+1]) > 0){
				strcpy(tmp, id_table[j]);
				strcpy(id_table[j], id_table[j+1]);
				strcpy(id_table[j+1], tmp);
				/*id_table에 저장된 이름들을 비교해서 sort해줌*/
			}
		}
	}
}

void sort_scoreTable(int size)
{/*문제번호를 쪼개서 score_table에 저장한다.*/
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 - i; j++){

			get_qname_number(score_table[j].qname, &num1_1, &num1_2);
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2);
/*num1_1,num1_2,num2_1,num2_2로 쪼개서 문제번호를 받는다.*/

			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){
/*문제번호를 sort해줌*/
				memcpy(&tmp, &score_table[j], sizeof(score_table[0]));
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0]));
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0]));
			}
		}
	}
}

void get_qname_number(char *qname, int *num1, int *num2)
{
	/*하위문제가 있는 문제를 위해, token으로 쪼개서 저장하는 함수이다..*/
	char *p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname));
	/*dup에 qname qname의 length만큼 copy함*/
	*num1 = atoi(strtok(dup, "-."));
	/*dup을 token으로 나눠서 num1에 정수로 바꿔 저장*/

	p = strtok(NULL, "-.");
	if(p == NULL)
		*num2 = 0;
	else
		*num2 = atoi(p);
}

int get_create_type()
{/*score table을 어떤 유형으로 만들건지 선택하게하는 함수이다.*/
	int num;

	while(1)
	{
		printf("score_table.csv file doesn't exist!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &num);
/*유형은 2가지로 1,2 둘중 선택하지않을경우 예외처리해준다.*/
		if(num != 1 && num != 2)
			printf("not correct number!\n");
		else
			return num;
	}

	return num;
	/*선택한 유형을 반환한다.*/
}

void score_students()
{
	double score = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]);
	/*size는 학생수*/

	if((fd = creat("score.csv", 0666)) < 0){
		fprintf(stderr, "creat error for score.csv");
		return;
	}
/*score.csv라는 파일을 생성함*/
	write_first_row(fd);
/*score 첫행 입력해준다.*/

	for(num = 0; num < size; num++)
	{
		if(!strcmp(id_table[num], ""))
			break;

		sprintf(tmp, "%s,", id_table[num]);
		write(fd, tmp, strlen(tmp)); 
/*id_table에서 학생학번 불러와서 첫column에저장*/

		score += score_student(fd, id_table[num]);
		/*학생들 점수 합산*/
	}

	printf("Total average : %.2f\n", score / num);

	close(fd);
}

double score_student(int fd, char *id)
{
	int type;
	double result;
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);
/*size는 문제수*/
	for(i = 0; i < size ; i++)
	{
		if(score_table[i].score == 0)
			break;

		sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname);
/*학생의 디렉토리에 답안파일 경로 tmp에 저장*/
		if(access(tmp, F_OK) < 0)
			result = false;
		/*tmp에 접근 가능여부 판단*/
		else
		{
			if((type = get_file_type(score_table[i].qname)) < 0)
				continue;

/*파일의 type 판단하여 txt파일이면 score_blank로, c파일이면 score_program 함수로 채점함*/
			if(type == TEXTFILE)
				result = score_blank(id, score_table[i].qname);
			
			else if(type == CFILE)
				result = score_program(id, score_table[i].qname);
		}

		if(result == false)
			write(fd, "0,", 2);
		/*0점일경우 fd에 0, write함*/
		else{
			if(result == true){/*맞았을경우*/
				score += score_table[i].score;
				sprintf(tmp, "%.2f,", score_table[i].score);
			}
			else if(result < 0){/*감점일경우*/
				score = score + score_table[i].score + result;
				sprintf(tmp, "%.2f,", score_table[i].score + result);
			}
			write(fd, tmp, strlen(tmp));
			/*score입력함*/
		}
	}

	printf("%s is finished.. score : %.2f\n", id, score); 

	sprintf(tmp, "%.2f\n", score);
	write(fd, tmp, strlen(tmp));
/*sum 점수 입력*/
	return score;
	/*학생 1명의 점수 리턴*/
}

void write_first_row(int fd)
{
	/*score파일의 첫행을 쓰는 함수이다.*/
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);
/*size는 문제수*/
	write(fd, ",", 1);
/*fd에해당하는 파일에 , 입력*/
	for(i = 0; i < size; i++){
		if(score_table[i].score == 0)
			break;

		sprintf(tmp, "%s,", score_table[i].qname);
		/*문제입력을 tmp에 저장*/
		write(fd, tmp, strlen(tmp));
		/*문제이름을 파일에 쓴다*/
	}
	write(fd, "sum\n", 4);
	/*첫행 마지막에는 sum을 써준다.*/
}

char *get_answer(int fd, char *result)
{
	/*답 파일을 배열로 입력받는함수다.*/
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN);
/*입력받은 result 메모리설정해줌*/
	while(read(fd, &c, 1) > 0)
	{
		/*답을 1바이트씩 읽음*/
		if(c == ':')
			break;
		/*입력받은 캐릭터가 :일경우 break*/
		result[idx++] = c;
		/*result파일에 1byte씩 입력해줌, idx도 하나씩 추가해줌*/
	}
	if(result[strlen(result) - 1] == '\n')
		result[strlen(result) - 1] = '\0';
	/*마지막이 \n일경우 \0으로 바꿔준다*/

	return result;
	/*입력받은 답안 리턴*/
}

int score_blank(char *id, char *filename)
{
	char tokens[TOKEN_CNT][MINLEN];
	node *std_root = NULL, *ans_root = NULL;
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN];
	char qname[FILELEN];
	int fd_std, fd_ans;
	int result = true;
	int has_semicolon = false;

	memset(qname, 0, sizeof(qname));
/*qname 메모리 할당*/
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));
/*.txt를 제외한 filename을 qname에 copy함*/
	sprintf(tmp, "%s/%s/%s", stuDir, id, filename);
	/*tmp에 stuDir/id/filename 입력*/
	fd_std = open(tmp, O_RDONLY);
	/*학생 답파일 읽기전용으로 open함*/
	strcpy(s_answer, get_answer(fd_std, s_answer));
/*get_answer로 학생파일에서 학생답을 입력받고 s_answer로 카피함*/
	if(!strcmp(s_answer, "")){
		close(fd_std);
		return false;
	}
/*s_answer파일이 null이면 파일닫고 false반환.*/
	if(!check_brackets(s_answer)){
		/*blank.c의 함수이용. 괄호를 판단하는함수이다.
		 괄호개수가 맞지않을경우 0을 return해주는함수이다.*/
		close(fd_std);
		return false;
		/*괄호개수가 맞지않을경우 파일닫고 false반환*/
	}

	strcpy(s_answer, ltrim(rtrim(s_answer)));
/*s_answer를 앞,뒷부분을 trim 해주고 s_answer에 카피함*/
	if(s_answer[strlen(s_answer) - 1] == ';'){
		has_semicolon = true;
		/*s_answer의 끝이 ;일경우 has_semicolon을 true로 함*/
		s_answer[strlen(s_answer) - 1] = '\0';
		/*그리고 끝을 \0으로 바꿔줌*/
	}

	if(!make_tokens(s_answer, tokens)){
		close(fd_std);
		return false;
	}
/*학생의 답안을 tokens로 만드는 함수*/

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0);
/*답안을 tree로 쪼갬*/
	sprintf(tmp, "%s/%s", ansDir, filename);
	fd_ans = open(tmp, O_RDONLY);
/*답안 파일을 open함*/
	while(1)
	{
		ans_root = NULL;
		result = true;

		for(idx = 0; idx < TOKEN_CNT; idx++)
			memset(tokens[idx], 0, sizeof(tokens[idx]));

		strcpy(a_answer, get_answer(fd_ans, a_answer));
/*a_answer를 입력받음*/
		if(!strcmp(a_answer, ""))
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer)));
/*a_answer의 양옆 trim해줌*/
		if(has_semicolon == false){
			if(a_answer[strlen(a_answer) -1] == ';')
				continue;
		}

		else if(has_semicolon == true)
		{
			if(a_answer[strlen(a_answer) - 1] != ';')
				continue;
			else
				a_answer[strlen(a_answer) - 1] = '\0';
		}
/*semicolon이 있는지 확인후, 있으면 \0값으로 바꿔준다.*/
		if(!make_tokens(a_answer, tokens))
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0);
/*입력받은 답을 tree로 만들어서 ans_root로 rootnode 반환*/
		compare_tree(std_root, ans_root, &result);
/*학생답안의 root와 답안의 root를 비교한다. 결과는 result에 나옴*/
		if(result == true){
			close(fd_std);
			close(fd_ans);

			if(std_root != NULL)
				free_node(std_root);
			if(ans_root != NULL)
				free_node(ans_root);
			return true;
/*답이 맞는경우 result는 true이므로 std_root와 ans_root는 free node 해줌*/

		}
	}
	
	close(fd_std);
	close(fd_ans);

	if(std_root != NULL)
		free_node(std_root);
	if(ans_root != NULL)
		free_node(ans_root);
/*std_root와 ans_root 모두 null로 반환시킴, false 틀린답.*/
	return false;
}

double score_program(char *id, char *filename)
{
	double compile;
	int result;

	compile = compile_program(id, filename);
/*입력받은 파일을 컴파일함*/
	if(compile == ERROR || compile == false)
		return false;
	/*comile 결과가 에러일경우 false일경우 false반환.*/
	result = execute_program(id, filename);

	if(!result)
		return false;

	if(compile < 0)
		return compile;

	return true;
}

int is_thread(char *qname)
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]);
/*size는 입력받은 thread개수*/
	for(i = 0; i < size; i++){
		if(!strcmp(threadFiles[i], qname))
			return true;
		/*threadFiles이랑 qname을 비교해서 같으면 true, 아니면 false*/
	}
	return false;
}

double compile_program(char *id, char *filename)
{
/*입력받은 답을 컴파일하는 함수이다.*/
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN];
	char command[BUFLEN];
	char qname[FILELEN];
	int isthread;
	off_t size;
	double result;

	memset(qname, 0, sizeof(qname));
/*우선 qname배열 메모리세팅함*/
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));
	/*qname 배열에 filename을 카피함. 입력받을 byte는 .뒤에이름은 지우고 크기반환*/
	isthread = is_thread(qname);

	sprintf(tmp_f, "%s/%s", ansDir, filename);
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname);
/*tmpf,tmpe에 컴파일할 파일경로와, 실행파일을 입력*/
	if(tOption && isthread)
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	/*tOption과 thread가 있을경우, 컴파일문 형식 command에 저장*/
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);
	/*thread가 없을경우 command에 컴파일문 형식 저장*/

	sprintf(tmp_e, "%s/%s_error.txt", ansDir, qname);
	fd = creat(tmp_e, 0666);
/*ansDir/qname으로 tmp_e 에러텍스트를 생성한다.*/
	redirection(command, fd, STDERR);
	/*command를 실행해서 에러가났을경우fd에 저장*/
	size = lseek(fd, 0, SEEK_END);
	/*에러파일의 크기를 알아냄*/
	close(fd);
	unlink(tmp_e);

	if(size > 0)
		return false;
	/*에러가있을경우 false로 리턴함.*/

	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename);
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname);
/*위에 ansDir의 컴파일과 동일*/
	if(tOption && isthread)
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);
/*컴파일문 생성*/
	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname);
	fd = creat(tmp_f, 0666);
/*tmpf 텍스트파일 생성*/
	redirection(command, fd, STDERR);
	/*에러발생시 tmp_f 파일에 출력*/
	size = lseek(fd, 0, SEEK_END);
	close(fd);

	if(size > 0){/*에러가있을경우*/
		if(eOption)
		{
			sprintf(tmp_e, "%s/%s", errorDir, id);
			if(access(tmp_e, F_OK) < 0)
				mkdir(tmp_e, 0755);

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname);
			rename(tmp_f, tmp_e);
/*e옵션일경우 errorDir의 존재여부확인후 없으면 생성, tmp_f파일을 tmp_e로 재명명*/
			result = check_error_warning(tmp_e);
			
		}
		else{ /*eOption없을경우*/
			result = check_error_warning(tmp_f);
			unlink(tmp_f);
		}

		return result;
	}

	unlink(tmp_f);
	return true;
	/*에러없는경우 true리턴*/
}

double check_error_warning(char *filename)
{/*error존재유무 판단과 warning의 개수를 세어준다.*/
	FILE *fp;
	char tmp[BUFLEN];
	double warning = 0;

	if((fp = fopen(filename, "r")) == NULL){
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}
	

	while(fscanf(fp, "%s", tmp) > 0){
		/*error파일 입력받음*/
		if(!strcmp(tmp, "error:"))
			return ERROR;
		/*error: 문장이 있을경우 ERROR 리턴*/
		else if(!strcmp(tmp, "warning:"))
			warning += WARNING;
		/*warning:이 있을경우 warning 개수를 세어준다.*/
	}

	return warning;
	/*error없을때, warning 개수 리턴.*/
}

int execute_program(char *id, char *filename)
{
	char std_fname[BUFLEN], ans_fname[BUFLEN];
	char tmp[BUFLEN];
	char qname[FILELEN];
	time_t start, end;
	pid_t pid;
	int fd;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));
/*qname에 filename 카피함*/
	sprintf(ans_fname, "%s/%s.stdout", ansDir, qname);
	fd = creat(ans_fname, 0666);
/*ansDir/qname.stdout이라는 파일을 생성*/

	sprintf(tmp, "%s/%s.exe", ansDir, qname);
	redirection(tmp, fd, STDOUT);
	close(fd);
/*ansDir/qname.exe라는 이름을 tmp에 입력후, 실행결과를 fd로 출력하도록 바꿈*/

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname);
	fd = creat(std_fname, 0666);
/*stuDir/id/qname.stdout 파일 생성*/
	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname);
/*tmp에 stuDir/id/pname.stdexe & 입력*/
	start = time(NULL);
	/*시간측정 시작*/
	redirection(tmp, fd, STDOUT);
	/*실행파일을 fd에 stdout하도록 바꿔줌*/
	
	sprintf(tmp, "%s.stdexe", qname);
	/*tmp에 qname.stdexe*/
	while((pid = inBackground(tmp)) > 0){
		end = time(NULL);
/*시간측정 종료.*/
		if(difftime(end, start) > OVER){
			kill(pid, SIGKILL);
			/*해당하는 pid 종료*/
			close(fd);
			return false;
			/*5초이상 걸린경우 false 리턴*/
		}
	}

	close(fd);

	return compare_resultfile(std_fname, ans_fname);
	/*stuDir/id/qname.stdout과 ansDir/qname.stdout 비교*/
}

pid_t inBackground(char *name)
{
	pid_t pid;
	char command[64];
	char tmp[64];
	int fd;
	off_t size;
	
	memset(tmp, 0, sizeof(tmp));
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);
/*background.txt 파일open함*/
	sprintf(command, "ps | grep %s", name);
	/*command에 'ps|grep name' 저장*/
	redirection(command, fd, STDOUT);
	/*command 실행해서 stdout결과를 background파일 fd에 저장*/

	lseek(fd, 0, SEEK_SET);
	/*fd파일 offset 처음으로함*/
	read(fd, tmp, sizeof(tmp));
/*fd파일 tmp로 read함*/
	if(!strcmp(tmp, "")){
		/*tmp파일이 null이면. return 0*/
		unlink("background.txt");
		close(fd);
		return 0;
	}

	pid = atoi(strtok(tmp, " "));
	/*tmp에 " "까지 분리해서 pid에 저장*/
	close(fd);

	unlink("background.txt");
	return pid;
	/*pid를 리턴함*/
}

int compare_resultfile(char *file1, char *file2)
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY);
	fd2 = open(file2, O_RDONLY);

	while(1)
	{
		while((len1 = read(fd1, &c1, 1)) > 0){
			/*fd1파일을 1byte씩 read함*/
			if(c1 == ' ') 
				continue;
			else 
				break;
			/*빈칸없는 곳까지 offset 이동*/
		}
		while((len2 = read(fd2, &c2, 1)) > 0){
			if(c2 == ' ') 
				continue;
			else 
				break;
		}
		/*빈칸없는곳 까지 offset 이동*/
		if(len1 == 0 && len2 == 0)
			break;
		/*둘다 파일의 끝일경우break*/

		to_lower_case(&c1);
		to_lower_case(&c2);
/*둘다 대문자일경우 소문자로 변경해줌*/

		if(c1 != c2){
			close(fd1);
			close(fd2);
			return false;
			/*두개의 값이 다를경우 false 리턴함*/
		}
	}
	close(fd1);
	close(fd2);
	return true;
	/*답과 맞는경우 true 리턴*/
}

void redirection(char *command, int new, int old)
{/*시스템상에서 수행결과를 저장위치를 바꿔준다*/
	int saved;

	saved = dup(old);
/*old 파일디스크립터 saved에 복사*/
	dup2(new, old);
/*new는 old파일디스크립터를 가리킴*/
	system(command);
/*system call로 command 수행*/
	dup2(saved, old);
	/*saved는 old를 파일디스크립터 가리킴*/
	close(saved);
}

int get_file_type(char *filename)
{
	char *extension = strrchr(filename, '.');
/*strrchr은 filename중 .이 가장마지막에 위치하는곳 포인터 반환*/
	if(!strcmp(extension, ".txt"))
		return TEXTFILE;
	else if (!strcmp(extension, ".c"))
		return CFILE;
	else
		return -1;
	/*ssu_score.h에 define된거에 따라, .txt일 경우 3리턴, .c일경우 4리턴*/
	/*그렇지않을경우 -1 리턴*/
}

void rmdirs(const char *path)
{
	/*연결된 path를 unlink 하는 함수*/
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[BUFLEN];
	
	if((dp = opendir(path)) == NULL)
		return;
/*path open했는데 NULL일경우 리턴*/
	while((dirp = readdir(dp)) != NULL)
	{
		/*dp 를 read했을때,null이 아닌경우*/
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", path, dirp->d_name);
/*tmp에 path/dirp->d_name으로 경로 저장*/
		if(lstat(tmp, &statbuf) == -1)
			continue;
/*tmp파일을 stat구조체로 리턴, 에러면 continue*/
		if(S_ISDIR(statbuf.st_mode))
			rmdirs(tmp);
		/*경로값이 디렉토리인경우 다시 rmdirs 실행*/
		else
			unlink(tmp);
		/*디렉토리가 아닌경우 unlink 해줌*/
	}

	closedir(dp);
	rmdir(path);
}

void to_lower_case(char *c)
{/*대문자를 소문자로 바꿔주는 함수*/
	if(*c >= 'A' && *c <= 'Z')
		*c = *c + 32;
}

void print_usage()
{
	printf("Usage : ssu_score <STD_DIR> <ANS_DIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -m			modify question's score \n");
	printf(" -e <DIRNAME>		print error on 'DIRNAME/ID/qname_error.txt' file\n");
	printf(" -t <QNAMES>		compile QNAME.C with -lpthread option\n");
	printf(" -i <IDS>		print ID's wrong questions\n");
	printf(" -h			print usage\n");
}
