/*
 *
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <windows.h>
#define SWAP(type,a,b) { type temp = a; a = b; b = temp; }
#define MEN 6
#define ROW 3
#define COL 3
#define CMD_NUM 18
#define CMD_LEN 3
 //#define STACK_MAX 35791394  // MAX=(7FFFFFFF=2,147,483,647)バイトまで。
#define STACK_MAX 30000000  // MAX=(7FFFFFFF=2,147,483,647)バイトまで。

void upperstring(char*, const char*);
void lowerstring(char*, const char*);
char upper(char);
char lower(char);
void print_cube(void);
void move_cube(char*);
int test_cube(void);
int rand_cube(int);
int save_history(int);
int back_track_cube(void);
int haba_yusen_cube(void);
int isComplete(char CUBE_CLR[][ROW][COL]);
int cmd2int(char*);
void stack_push(int);
int stack_pop(void);
int stack_top = 0;
int get_undo(int);
void rewind_cube(void);
int char_sw = 0;
//
typedef struct tagPTN {
	char kyokuclr[MEN * ROW * COL];
	int pattern_from;
} PTN;
//PTN* history;
PTN history[STACK_MAX];

int history_count = 0;
int queue_bottom;

char CUBE_CLR[MEN][ROW][COL] = { { {'0', '0', '0'},
								   {'0', '0', '0'},
								   {'0', '0', '0'} },
								 { {'1', '1', '1'},
								   {'1', '1', '1'},
								   {'1', '1', '1'} },
								 { {'2', '2', '2'},
								   {'2', '2', '2'},
								   {'2', '2', '2'} },
								 { {'3', '3', '3'},
								   {'3', '3', '3'},
								   {'3', '3', '3'} },
								 { {'4', '4', '4'},
								   {'4', '4', '4'},
								   {'4', '4', '4'} },
								 { {'5', '5', '5'},
								   {'5', '5', '5'},
								   {'5', '5', '5'} } };

char coltbl[MEN][9] = { "\x1b[40m",   // 黒背景
						"\x1b[41m",   // 赤背景
						"\x1b[42m",   // 緑背景
						"\x1b[43m",   // 黄背景
						"\x1b[44m",   // 青背景
						"\x1b[45m" }; // 紫背景

char messeage[ROW + 1][COL][2][24] = { {{"上行を左へ(←):TL", "上行を右へ(→):TR"},
									   {"中行を左へ(←):ML", "中行を右へ(→):MR"},
									   {"下行を左へ(←):BL", "下行を右へ(→):BR"}},
									  {{"左列を上へ(↑):LU", "左列を下へ(↓):LD"},
									   {"中列を上へ(↑):CU", "中列を下へ(↓):CD"},
									   {"右列を上へ(↑):RU", "右列を下へ(↓):RD"}},
									  {{"手前を左へ(←):FL", "手前を右へ(→):FR"},
									   {"中奥を左へ(←):NL", "中奥を右へ(→):NR"},
									   {"最奥を左へ(←):RL", "最奥を右へ(→):RR"}},
									  {{"***", "***" },
									   {"テスト:TS", "ランダム実行:RN" },
									   {"***", "***" }} };

char cmdtbl[MEN * ROW][CMD_LEN] = { "TL", "TR", "ML", "MR", "BL", "BR",
									"LU", "LD", "CU", "CD", "RU", "RD",
									"FL", "FR", "NL", "NR", "RL", "RR" };

int cmd_stack[STACK_MAX];
int stack_ctr = 0;
FILE* fp;

int main(void) {
	// ログファイルを準備
	fp = fopen("RubicCubeLog.txt", "w");
	if (fp == NULL) {
		printf("ファイルをオープンできませんでした。\n");
		return 1;  // main()が異常終了した場合1を返す
	}
	fprintf(fp, "<<< RubicCubeの処理を開始しました。 >>>\n");

	int judge = 0;
	do {
		print_cube();

		char inp[CMD_LEN];
		char INP[CMD_LEN];

		int rtn = scanf("%s", inp);
		upperstring(INP, inp);
		printf("入力はinp=%s INP=%s\n", inp, INP);

		if (strcmp(INP, "QQ") == 0) {
			judge = 1;
		}
		else if (strcmp(INP, "ZZ") == 0) {
			if (stack_top <= 0) {
				printf("戻せる状態はありません！\n");
			}
			else {
				int rtn1 = stack_pop();
				int rtn2 = get_undo(rtn1);
				//fprintf(fp, "cmd=%s undo_cmd=%s\n", cmdtbl[rtn1], cmdtbl[rtn2]);
				move_cube(cmdtbl[rtn2]);
				print_cube();
			}
		}
		else if (strcmp(INP, "TS") == 0) {
			char_sw = 1;
			int rtn = test_cube();
			if (rtn == 0) {
				printf("テストは正常終了しました。\n");
			}
			judge = 1;
		}
		else if (strcmp(INP, "RN") == 0) {
			printf("ランダム実行する回数を入力してください。");
			int kai;
			int rtn = scanf("%d", &kai);
			int rtn1 = rand_cube(kai);
			int rtn2 = isComplete(CUBE_CLR);
			if (rtn2 == 0) {
				printf("完成形です。\n");
			}
			if (rtn1 == 0) {
				printf("ランダム実行は完成形にたどり着きませんでした。\n");
				printf("5秒後に戻し処理を行います。\n");
				Sleep(5000);
				rewind_cube();
				printf("戻し処理が修了しました。\n");
			}
			judge = 1;
		}
		else if (strcmp(INP, "BT") == 0) {
			//history = NULL;
			save_history(-1);  // 現時点を登録
			int rtn = back_track_cube();
			if (rtn == 1) {
				printf("完成形です。\n");
			}
			else {
				printf("解が見つかりませんでした。\n");
			}
			judge = 1;
		}
		else if (strcmp(INP, "HB") == 0) {
			//history = NULL;
			save_history(-1);  // 現時点を登録
			int rtn = haba_yusen_cube();
			if (rtn == 1) {
				printf("解が見つかりました。\n");
				//while (getchar() != '\n');
				//int w = getchar();
				_getch();
				// 
				int last = -1;
				while (last != queue_bottom) {
					int i;
					for (i = queue_bottom; history[i].pattern_from != last;) {
						i = history[i].pattern_from;
					}
					last = i;
					//
					memcpy(CUBE_CLR, history[last].kyokuclr, sizeof(CUBE_CLR));
					print_cube();
					//while (getchar() != '\n');
					//int w = getchar();
					_getch();
				}
			}
			else {
				printf("解が見つかりませんでした。\n");
			}
			judge = 1;
		}
		else if ((strcmp(INP, "TL") == 0) || (strcmp(INP, "TR") == 0) || (strcmp(INP, "ML") == 0) || (strcmp(INP, "MR") == 0) || (strcmp(INP, "BL") == 0) || (strcmp(INP, "BR") == 0)
			|| (strcmp(INP, "LU") == 0) || (strcmp(INP, "LD") == 0) || (strcmp(INP, "CU") == 0) || (strcmp(INP, "CD") == 0) || (strcmp(INP, "RU") == 0) || (strcmp(INP, "RD") == 0)
			|| (strcmp(INP, "FL") == 0) || (strcmp(INP, "FR") == 0) || (strcmp(INP, "NL") == 0) || (strcmp(INP, "NR") == 0) || (strcmp(INP, "RL") == 0) || (strcmp(INP, "RR") == 0)) {
			move_cube(INP);
			print_cube();
			int rtn = cmd2int(INP);
			stack_push(rtn);
		}
		else {
			printf("対応するコマンドなし:%s\n", INP);
		}
	} while (judge == 0);

	//while (getchar() != '\n');
	//int w = getchar();
	_getch();

	fprintf(fp, "<<< RubicCubeの処理が終了しました。 >>>\n");
	fclose(fp);

	// free(history);
	return 0;
}

void print_cube(void) {
	system("cls");
	// 1
	printf("      -------      \n");
	for (int i = 0; i < ROW; i++) {
		printf("      ");
		for (int j = 0; j < COL; j++) {
			printf("|");
			printf("%s", coltbl[CUBE_CLR[4][i][j] - '0']);
			printf("O");
			printf("\x1b[49m");
		}
		printf("|      ");
		printf("     ");
		printf("%s", messeage[0][i][0]);
		printf(" / ");
		printf("%s", messeage[0][i][1]);
		printf("\n");
	}
	// 2
	printf("------+-----+------\n");
	for (int i = 0; i < ROW; i++) {
		for (int j = 0; j < COL; j++) {
			printf("|");
			printf("%s", coltbl[CUBE_CLR[2][i][j] - '0']);
			printf("O");
			printf("\x1b[49m");
		}
		for (int j = 0; j < COL; j++) {
			printf("|");
			printf("%s", coltbl[CUBE_CLR[0][i][j] - '0']);
			printf("O");
			printf("\x1b[49m");
		}
		for (int j = 0; j < COL; j++) {
			printf("|");
			printf("%s", coltbl[CUBE_CLR[3][i][j] - '0']);
			printf("O");
			printf("\x1b[49m");
		}
		printf("|");
		printf("     ");
		printf("%s", messeage[1][i][0]);
		printf(" / ");
		printf("%s", messeage[1][i][1]);
		printf("\n");
	}
	printf("------+-----+------\n");
	// 3
	for (int i = 0; i < ROW; i++) {
		printf("      ");
		for (int j = 0; j < COL; j++) {
			printf("|");
			printf("%s", coltbl[CUBE_CLR[1][i][j] - '0']);
			printf("O");
			printf("\x1b[49m");
		}
		printf("|      ");
		printf("     ");
		printf("%s", messeage[2][i][0]);
		printf(" / ");
		printf("%s", messeage[2][i][1]);
		printf("\n");
	}
	printf("      -------      \n");
	// 4
	for (int i = 0; i < ROW; i++) {
		printf("      ");
		for (int j = 0; j < COL; j++) {
			printf("|");
			printf("%s", coltbl[CUBE_CLR[5][i][j] - '0']);
			printf("O");
			printf("\x1b[49m");
		}
		printf("|      ");
		printf("     ");
		printf("%s", messeage[3][i][0]);
		printf(" / ");
		printf("%s", messeage[3][i][1]);
		printf("\n");
	}
	printf("      -------      ");
	printf("     戻す:ZZ                       終了:QQ\n");
	printf("                        ");
}

void turn_left(int men) {
	char temC[COL];
	for (int i = 0; i < COL; i++) {
		temC[i] = CUBE_CLR[men][0][i];
	}
	CUBE_CLR[men][0][0] = CUBE_CLR[men][0][2];
	CUBE_CLR[men][0][1] = CUBE_CLR[men][1][2];
	CUBE_CLR[men][0][2] = CUBE_CLR[men][2][2];
	CUBE_CLR[men][1][2] = CUBE_CLR[men][2][1];
	CUBE_CLR[men][2][2] = CUBE_CLR[men][2][0];
	CUBE_CLR[men][2][1] = CUBE_CLR[men][1][0];
	for (int i = 0; i < COL; i++) {
		CUBE_CLR[men][COL - 1 - i][0] = temC[i];
	}
}

void turn_right(int men) {
	char temC[COL];
	for (int i = 0; i < COL; i++) {
		temC[i] = CUBE_CLR[men][0][COL - 1 - i];
	}
	CUBE_CLR[men][0][2] = CUBE_CLR[men][0][0];
	CUBE_CLR[men][0][1] = CUBE_CLR[men][1][0];
	CUBE_CLR[men][0][0] = CUBE_CLR[men][2][0];
	CUBE_CLR[men][1][0] = CUBE_CLR[men][2][1];
	CUBE_CLR[men][2][0] = CUBE_CLR[men][2][2];
	CUBE_CLR[men][2][1] = CUBE_CLR[men][1][2];
	for (int i = 0; i < COL; i++) {
		CUBE_CLR[men][COL - 1 - i][2] = temC[i];
	}
}

void move_cube(char* cmd) {

	char temC[COL];
	if (strcmp(cmd, "TL") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][0][i];
			CUBE_CLR[0][0][i] = CUBE_CLR[3][0][i];
			CUBE_CLR[3][0][i] = CUBE_CLR[5][0][i];
			CUBE_CLR[5][0][i] = CUBE_CLR[2][0][i];
			CUBE_CLR[2][0][i] = temC[i];
		}
		//
		turn_left(4);
	}
	else if (strcmp(cmd, "TR") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][0][i];
			CUBE_CLR[0][0][i] = CUBE_CLR[2][0][i];
			CUBE_CLR[2][0][i] = CUBE_CLR[5][0][i];
			CUBE_CLR[5][0][i] = CUBE_CLR[3][0][i];
			CUBE_CLR[3][0][i] = temC[i];
		}
		//
		turn_right(4);
	}
	else if (strcmp(cmd, "ML") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][1][i];
			CUBE_CLR[0][1][i] = CUBE_CLR[3][1][i];
			CUBE_CLR[3][1][i] = CUBE_CLR[5][1][COL - 1 - i];
			CUBE_CLR[5][1][COL - 1 - i] = CUBE_CLR[2][1][i];
			CUBE_CLR[2][1][i] = temC[i];
		}
	}
	else if (strcmp(cmd, "MR") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][1][i];
			CUBE_CLR[0][1][i] = CUBE_CLR[2][1][i];
			CUBE_CLR[2][1][i] = CUBE_CLR[5][1][COL - 1 - i];
			CUBE_CLR[5][1][COL - 1 - i] = CUBE_CLR[3][1][i];
			CUBE_CLR[3][1][i] = temC[i];
		}
	}
	else if (strcmp(cmd, "BL") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][2][i];
			CUBE_CLR[0][2][i] = CUBE_CLR[3][2][i];
			CUBE_CLR[3][2][i] = CUBE_CLR[5][2][i];
			CUBE_CLR[5][2][i] = CUBE_CLR[2][2][i];
			CUBE_CLR[2][2][i] = temC[i];
		}
		//
		turn_left(1);
	}
	else if (strcmp(cmd, "BR") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][2][i];
			CUBE_CLR[0][2][i] = CUBE_CLR[2][2][i];
			CUBE_CLR[2][2][i] = CUBE_CLR[5][2][i];
			CUBE_CLR[5][2][i] = CUBE_CLR[3][2][i];
			CUBE_CLR[3][2][i] = temC[i];
		}
		//
		turn_right(1);
	}
	else if (strcmp(cmd, "LU") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][i][0];
			CUBE_CLR[0][i][0] = CUBE_CLR[1][i][0];
			CUBE_CLR[1][i][0] = CUBE_CLR[5][i][0];
			CUBE_CLR[5][i][0] = CUBE_CLR[4][i][0];
			CUBE_CLR[4][i][0] = temC[i];
		}
		//
		turn_left(2);
	}
	else if (strcmp(cmd, "CU") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][i][1];
			CUBE_CLR[0][i][1] = CUBE_CLR[1][i][1];
			CUBE_CLR[1][i][1] = CUBE_CLR[5][i][1];
			CUBE_CLR[5][i][1] = CUBE_CLR[4][i][1];
			CUBE_CLR[4][i][1] = temC[i];
		}
	}
	else if (strcmp(cmd, "RU") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][i][2];
			CUBE_CLR[0][i][2] = CUBE_CLR[1][i][2];
			CUBE_CLR[1][i][2] = CUBE_CLR[5][i][2];
			CUBE_CLR[5][i][2] = CUBE_CLR[4][i][2];
			CUBE_CLR[4][i][2] = temC[i];
		}
		//
		turn_right(3);
	}
	else if (strcmp(cmd, "LD") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][i][0];
			CUBE_CLR[0][i][0] = CUBE_CLR[4][i][0];
			CUBE_CLR[4][i][0] = CUBE_CLR[5][i][0];
			CUBE_CLR[5][i][0] = CUBE_CLR[1][i][0];
			CUBE_CLR[1][i][0] = temC[i];
		}
		//
		turn_right(2);
	}
	else if (strcmp(cmd, "CD") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][i][1];
			CUBE_CLR[0][i][1] = CUBE_CLR[4][i][1];
			CUBE_CLR[4][i][1] = CUBE_CLR[5][i][1];
			CUBE_CLR[5][i][1] = CUBE_CLR[1][i][1];
			CUBE_CLR[1][i][1] = temC[i];
		}
	}
	else if (strcmp(cmd, "RD") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[0][i][2];
			CUBE_CLR[0][i][2] = CUBE_CLR[4][i][2];
			CUBE_CLR[4][i][2] = CUBE_CLR[5][i][2];
			CUBE_CLR[5][i][2] = CUBE_CLR[1][i][2];
			CUBE_CLR[1][i][2] = temC[i];
		}
		//
		turn_left(3);
	}
	else if (strcmp(cmd, "FL") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[4][2][i];
			CUBE_CLR[4][2][i] = CUBE_CLR[3][i][0];
			CUBE_CLR[3][i][0] = CUBE_CLR[1][0][COL - 1 - i];
			CUBE_CLR[1][0][COL - 1 - i] = CUBE_CLR[2][COL - 1 - i][2];
			CUBE_CLR[2][COL - 1 - i][2] = temC[i];
		}
		//
		turn_left(0);
	}
	else if (strcmp(cmd, "FR") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[4][2][COL - 1 - i];
			CUBE_CLR[4][2][COL - 1 - i] = CUBE_CLR[2][i][2];
			CUBE_CLR[2][i][2] = CUBE_CLR[1][0][i];
			CUBE_CLR[1][0][i] = CUBE_CLR[3][COL - 1 - i][0];
			CUBE_CLR[3][COL - 1 - i][0] = temC[i];
		}
		//
		turn_right(0);
	}
	else if (strcmp(cmd, "NL") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[4][1][i];
			CUBE_CLR[4][1][i] = CUBE_CLR[3][i][1];
			CUBE_CLR[3][i][1] = CUBE_CLR[1][1][COL - 1 - i];
			CUBE_CLR[1][1][COL - 1 - i] = CUBE_CLR[2][COL - 1 - i][1];
			CUBE_CLR[2][COL - 1 - i][1] = temC[i];
		}
	}
	else if (strcmp(cmd, "NR") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[4][1][COL - 1 - i];
			CUBE_CLR[4][1][COL - 1 - i] = CUBE_CLR[2][i][1];
			CUBE_CLR[2][i][1] = CUBE_CLR[1][1][i];
			CUBE_CLR[1][1][i] = CUBE_CLR[3][COL - 1 - i][1];
			CUBE_CLR[3][COL - 1 - i][1] = temC[i];
		}
	}
	else if (strcmp(cmd, "RL") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[4][0][i];
			CUBE_CLR[4][0][i] = CUBE_CLR[3][i][2];
			CUBE_CLR[3][i][2] = CUBE_CLR[1][2][COL - 1 - i];
			CUBE_CLR[1][2][COL - 1 - i] = CUBE_CLR[2][COL - 1 - i][0];
			CUBE_CLR[2][COL - 1 - i][0] = temC[i];
		}
		//
		turn_right(5);
	}
	else if (strcmp(cmd, "RR") == 0) {
		for (int i = 0; i < COL; i++) {
			temC[i] = CUBE_CLR[4][0][COL - 1 - i];
			CUBE_CLR[4][0][COL - 1 - i] = CUBE_CLR[2][i][0];
			CUBE_CLR[2][i][0] = CUBE_CLR[1][2][i];
			CUBE_CLR[1][2][i] = CUBE_CLR[3][COL - 1 - i][2];
			CUBE_CLR[3][COL - 1 - i][2] = temC[i];
		}
		//
		turn_left(5);
	}
	else {
		//
		printf("該当するコマンドがありませんcmd=%s\n", cmd);
		exit(9);
	}
	//
	fprintf(fp, "<move_cube> (%s) CUBE_CLR=%s\n", cmd, CUBE_CLR);
}

int test_cube(void) {
	char SHOKI[MEN][ROW][COL];
	char cmd1[CMD_LEN] = "";
	char cmd2[CMD_LEN] = "";

	// memcpyで配列をコピー  
	memcpy(SHOKI, CUBE_CLR, sizeof(CUBE_CLR));

	for (int cmdctr = 0; cmdctr < CMD_NUM; cmdctr++) {  // コマンド数
		if (cmdctr % 2 == 0) {
			for (int i = 0; i < CMD_LEN; i++) {
				cmd1[i] = cmdtbl[cmdctr][i];
				cmd2[i] = cmdtbl[cmdctr + 1][i];
			}
		}
		else {
			for (int i = 0; i < CMD_LEN; i++) {
				cmd1[i] = cmdtbl[cmdctr - 1][i];
				cmd2[i] = cmdtbl[cmdctr][i];
			}
		}
		//printf("%d回目 cmd1=%s cmd2=%s\n", cmdctr, cmd1, cmd2);
		move_cube(cmd1);
		print_cube();
		Sleep(500);
		move_cube(cmd2);
		print_cube();
		Sleep(500);
		printf("\n");

		// memcpyで配列を比較
		if (memcmp(SHOKI, CUBE_CLR, sizeof(CUBE_CLR)) == 0) {
			// 一致
		}
		else {
			printf("%d回目 cmd1=%s cmd2=%s\n", cmdctr, cmd1, cmd2);
			printf("SHOKI=%s\n", SHOKI);
			printf(" CUBE=%s\n", CUBE_CLR);
			exit(9);
		}

		for (int i = 0; i < 4; i++) {
			move_cube(cmd1);
			print_cube();
			Sleep(500);
		}
		if (memcmp(SHOKI, CUBE_CLR, sizeof(CUBE_CLR)) == 0) {
			// 一致
		}
		else {
			printf("%d回目 cmd1=%s cmd2=%s\n", cmdctr, cmd1, cmd2);
			printf("SHOKI=%s\n", SHOKI);
			printf(" CUBE=%s\n", CUBE_CLR);
			exit(9);
		}
	}
	return 0;
}

int rand_cube(int kai) {
	char cmd[CMD_LEN];
	srand((unsigned int)time(NULL));

	for (int ctr = 0; ctr < kai; ctr++) {
		int cmdnum = rand() % CMD_NUM;
		for (int i = 0; i < CMD_LEN; i++) {
			cmd[i] = cmdtbl[cmdnum][i];
		}
		move_cube(cmd);
		print_cube();
		int rtn = cmd2int(cmd);
		stack_push(rtn);
		Sleep(100);
		printf("ctr=%3d\n", ctr);
	}
	return 0;
}

int save_history(int pattern_from) {
	// 今までの局面と比較する
	for (int i = 0; i < history_count; i++) {
		if (memcmp(history[i].kyokuclr, CUBE_CLR, sizeof(CUBE_CLR)) == 0) {
			fprintf(fp, "過去の局面にありましたので戻ります。\n");
			return 0;
		}
	}
	// 新しい局面を保存する
	history_count++;
	//history = (PTN*)realloc(history, sizeof(PTN) * (history_count));
	//if (history == NULL) {
	//	printf("メモリが足りません。");
	//	exit(EXIT_FAILURE);
	//}

	memcpy(history[history_count - 1].kyokuclr, CUBE_CLR, sizeof(CUBE_CLR));
	history[history_count - 1].pattern_from = pattern_from;
	fprintf(fp, "新たな局面として登録しました。\n");

	return 1;
}

int back_track_cube(void) {

	printf("history_count=%d\n", history_count);
	//
	if (isComplete(CUBE_CLR) == 0) {
		return 1;
	}
	//
	char cmd[CMD_LEN] = "";
	for (int cmdctr = 0; cmdctr < CMD_NUM; cmdctr += 2) {  // コマンドの半分
		for (int i = 0; i < CMD_LEN; i++) {
			cmd[i] = cmdtbl[cmdctr][i];
		}
		move_cube(cmd);
		print_cube();
		int rtn = cmd2int(cmd);
		stack_push(rtn);
		if (save_history(0) == 1) {
			int rtn = back_track_cube();
			if (rtn == 1) {
				return 1;
			}
		}
		else {
			int rtn1 = stack_pop();
			int rtn2 = get_undo(rtn1);
			fprintf(fp, "cmd=%s undo_cmd=%s\n", cmdtbl[rtn1], cmdtbl[rtn2]);
			move_cube(cmdtbl[rtn2]);
			print_cube();
		}
		// for DEBUG
		//if (history_count > 100) {
		//    return 0;
		//}
	}
	return 0;
}

int haba_yusen_cube(void) {

	queue_bottom = 0;
	while (queue_bottom != history_count) {
		fprintf(fp, "queue_bottom=%d history_count=%d\n", queue_bottom, history_count);
		// キューから１つ取得する
		memcpy(CUBE_CLR, history[queue_bottom].kyokuclr, sizeof(CUBE_CLR));
		print_cube();
		fprintf(fp, "<haba_yusen>(GQ) CUBE_CLR=%s\n", CUBE_CLR);
		//
		switch (queue_bottom) {
		case 18:
			fprintf(fp, "****************************** 1手の読み完了 ******************************\n");
			break;
		case (18 + 18 * 18):
			fprintf(fp, "****************************** 2手の読み完了 ******************************\n");
			break;
		case (18 + 18 * 18 + 18 * 18 * 18):
			fprintf(fp, "****************************** 3手の読み完了 ******************************\n");
			break;
		case (18 + 18 * 18 + 18 * 18 * 18 + 18 * 18 * 18 * 18):
			fprintf(fp, "****************************** 4手の読み完了 ******************************\n");
			break;
		case (18 + 18 * 18 + 18 * 18 * 18 + 18 * 18 * 18 * 18 + 18 * 18 * 18 * 18 * 18):
			fprintf(fp, "****************************** 5手の読み完了 ******************************\n");
			break;
		case (18 + 18 * 18 + 18 * 18 * 18 + 18 * 18 * 18 * 18 + 18 * 18 * 18 * 18 * 18 + 18 * 18 * 18 * 18 * 18 * 18):
			fprintf(fp, "****************************** 6手の読み完了 ******************************\n");
			break;
		default:
			break;
		}
		//
		//if (isComplete(CUBE_CLR) == 0) {
		//	return 1;
		//}
		//
		for (int i = 0; i < CMD_NUM; i++) {
			move_cube(cmdtbl[i]);
			int rtn = save_history(queue_bottom);
			//
			if (isComplete(CUBE_CLR) == 0) {
				print_cube();
				return 1;
			}
			//
			int rtn2 = get_undo(i);
			fprintf(fp, "cmd=%s undo_cmd=%s\n", cmdtbl[i], cmdtbl[rtn2]);
			move_cube(cmdtbl[rtn2]);
		}
		queue_bottom++;
	}
	return 0;  // 解なし
}

void rewind_cube(void) {

	while (stack_top != 0) {
		int rtn1 = stack_pop();
		int rtn2 = get_undo(rtn1);
		fprintf(fp, "cmd=%s undo_cmd=%s\n", cmdtbl[rtn1], cmdtbl[rtn2]);
		move_cube(cmdtbl[rtn2]);
		print_cube();
	}
}

int isComplete(char cube[][ROW][COL]) {
	//
	for (int men = 0; men < MEN; men++) {
		for (int row = 0; row < ROW; row++) {
			for (int col = 0; col < COL; col++) {
				if (cube[men][row][col] != cube[men][0][0]) {
					// fprintf(fp, "cube[men][row][col](%d) != cube[men][0][0](%d) / men=%d row=%d col=%d\n", cube[men][row][col], cube[men][0][0], men, row, col);
					return 9;
				}
			}
		}
	}
	return 0;
}

int cmd2int(char* cmd) {
	if (strcmp(cmd, "QQ") == 0) {
		return 99;
	}
	else if (strcmp(cmd, "TL") == 0) { return  0; }
	else if (strcmp(cmd, "TR") == 0) { return  1; }
	else if (strcmp(cmd, "ML") == 0) { return  2; }
	else if (strcmp(cmd, "MR") == 0) { return  3; }
	else if (strcmp(cmd, "BL") == 0) { return  4; }
	else if (strcmp(cmd, "BR") == 0) { return  5; }
	else if (strcmp(cmd, "LU") == 0) { return  6; }
	else if (strcmp(cmd, "LD") == 0) { return  7; }
	else if (strcmp(cmd, "CU") == 0) { return  8; }
	else if (strcmp(cmd, "CD") == 0) { return  9; }
	else if (strcmp(cmd, "RU") == 0) { return 10; }
	else if (strcmp(cmd, "RD") == 0) { return 11; }
	else if (strcmp(cmd, "FL") == 0) { return 12; }
	else if (strcmp(cmd, "FR") == 0) { return 13; }
	else if (strcmp(cmd, "NL") == 0) { return 14; }
	else if (strcmp(cmd, "NR") == 0) { return 15; }
	else if (strcmp(cmd, "RL") == 0) { return 16; }
	else if (strcmp(cmd, "RR") == 0) { return 17; }
	else {
		return 99;
	}
}

void upperstring(char* out, const char* in) {
	int i;

	i = 0;
	while (in[i] != '\0') {
		out[i] = upper(in[i]);
		i++;
	}
	out[i] = '\0';
}

void lowerstring(char* out, const char* in) {
	int i;

	i = 0;
	while (in[i] != '\0') {
		out[i] = lower(in[i]);
		i++;
	}
	out[i] = '\0';
}

char upper(char c) {
	if ('a' <= c && c <= 'z') {
		c = c - ('a' - 'A');
	}
	return c;
}

char lower(char c) {
	if ('A' <= c && c <= 'Z') {
		c = c + ('a' - 'A');
	}
	return c;
}

void stack_push(int val) {
	if (stack_top >= STACK_MAX) {
		// スタックが満杯
		printf("エラー：スタックが満杯です。(Stack overflow)\n");
		exit(EXIT_FAILURE);
	}
	else {
		// 渡された値をスタックに積む
		cmd_stack[stack_top] = val;
		stack_top++;
	}
}

int stack_pop(void) {
	if (stack_top <= 0) {
		// スタックには何もない
		printf("エラー：スタックが空なのにpopが呼ばれました。(Stack underflow)\n");
		exit(EXIT_FAILURE);
	}
	else {
		// 一番上の値を返す
		stack_top--;
		return cmd_stack[stack_top];
	}
}

int get_undo(int val) {

	if (val % 2 == 0) {
		return val + 1;
	}
	else {
		return val - 1;
	}
}