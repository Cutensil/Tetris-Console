#include<stdio.h>
#include<iostream>
#include<fstream>
#include<windows.h>
#include<conio.h>
#pragma comment(lib, "winmm.lib")
using namespace std;

HANDLE output;
char warning = 0;
char _hold_flag = 0;
char t_spin_flag = 0;
char t_mini_flag = 1;
char b2b_flag = 0;
char combo_flag = 0;
char gameover = 0;
int score = 0;
int lines = 0;
int garbage_lines = 0;
int combo = 0;
int attack_temp = 0;
int attack = 0;
int lock_count = 0;
int fall_count = 0;
int garbage_count = 0;
int attack_count = 0;
int attack_goal = 0;
int attack_delay = 1800;
int garbage_raise = 0;
int fps;
char current_color = -1;

struct DELAY
{
	int garbage = 0;
	int lock = 0;
	int fall = 0;
	int DAS = 0;
	int ARR = 0;
}_left, _right, soft_drop, delay;
class CONFIG
{
public:
	char mode = 0;//0.Marathon 1.Survival 2.Dig Challange
	char difficulty = 0;//0.Easy 1.Normal 2.Hard 3.Insane 4.Impossible
	int height = 20;
	int width = 10;
	char backfire = 0;//0.Off 1.1x 2.2x
	char shadow = 1;
	char hidden = 0;
	int left = 37;
	int right = 39;
	int soft_drop = 40;
	int hard_drop = 32;
	int left_rotate = 38;
	int right_rotate = 'D';
	int _180_rotate = 'S';
	int hold = 'F';
	int retry = 'R';
	int DAS = 7;
	int ARR = 2;
	int SDARR = 0;
public:
	void Save();
public:
	void Load();
}config;
struct MAP
{
	char wall = 0;
	char block = 0;
	char death = 0;
}map[50][100];
struct CLEAR
{
	char t_mini = 0;
	char t_spin = 0;
	char b2b = 0;
	char pc = 0;
	int lines = 0;
	int timer = 0;
}clear, clear_temp;
class BLOCK
{
public:
	char type;
	char state;
	int position_x;
	int position_y;
	char appearance[4][4];
public:
	void Move(char direction);
public:
	void Move(int delta_x, int delta_y);
public:
	void Rotate(char direction);
public:
	void Generate(char type);
public:
	void Generate(char type_generate, char if_death);
public:
	void Lock();
}block, hold, _next[14], ghost, death;
class GARBAGE
{
public:
	int queue[100];
	int count;
public:
	void Buffer(int count);
public:
	void Generate();
}garbage;

void CursorJump(int x, int y)
{
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(output, pos);
}
void HideCursor()
{
	CONSOLE_CURSOR_INFO curInfo;
	curInfo.dwSize = 1;
	curInfo.bVisible = FALSE;
	SetConsoleCursorInfo(output, &curInfo);
}
char CheckMove(char direction)
{
	if (direction == 'L')
	{
		for (int x = 0;x < 4;x++)
		{
			for (int y = 0;y < 4;y++)
			{
				if (block.appearance[x][y] == 1)
				{
					if (map[block.position_x + x - 1][block.position_y + y].wall == 1 || block.position_x + x - 1 < 0 || block.position_x + x - 1 >= config.width || block.position_y + y < 0)
						return 0;
				}
			}
		}
	}
	else if (direction == 'R')
	{
		for (int x = 0;x < 4;x++)
		{
			for (int y = 0;y < 4;y++)
			{
				if (block.appearance[x][y] == 1)
				{
					if (map[block.position_x + x + 1][block.position_y + y].wall == 1 || block.position_x + x + 1 < 0 || block.position_x + x + 1 >= config.width || block.position_y + y < 0)
						return 0;
				}
			}
		}
	}
	else
	{
		for (int x = 0;x < 4;x++)
		{
			for (int y = 0;y < 4;y++)
			{
				if (block.appearance[x][y] == 1)
				{
					if (map[block.position_x + x][block.position_y + y - 1].wall == 1 || block.position_x + x < 0 || block.position_x + x >= config.width || block.position_y + y - 1 < 0)
						return 0;
				}
			}
		}
	}
	return 1;
}
char CheckMove(int delta_x, int delta_y)
{
	for (int x = 0;x < 4;x++)
	{
		for (int y = 0;y < 4;y++)
		{
			if (block.appearance[x][y] == 1)
			{
				if (map[block.position_x + x + delta_x][block.position_y + y + delta_y].wall == 1 || block.position_x + x + delta_x < 0 || block.position_x + x + delta_x >= config.width || block.position_y + y + delta_y < 0)
					return 0;
			}
		}
	}
	return 1;
}
void Ghost(char mode)
{
	if (mode == 'N')
	{
		for (int i = 0;;i++)
		{
			if (!CheckMove(0, -i))
			{
				ghost.position_x = block.position_x;
				ghost.position_y = block.position_y - i + 1;
				memcpy(ghost.appearance, block.appearance, sizeof(block.appearance));
				return;
			}
		}
	}
	else
	{
		for (int x = 0;x < 4;x++)
		{
			for (int y = 0;y < 4;y++)
			{
				if (ghost.appearance[x][y] == 1 && map[ghost.position_x + x][ghost.position_y + y].block == 'G')
					map[ghost.position_x + x][ghost.position_y + y].block = 0;
			}
		}
	}
}
void SetColor(char block_type)
{
	if (current_color == block_type)
		return;
	else
		current_color = block_type;
	switch (block_type)
	{
	case 'I':
		if (warning == 0)
			SetConsoleTextAttribute(output, 7 | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		else
			SetConsoleTextAttribute(output, BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case 'O':
		if (warning == 0)
			SetConsoleTextAttribute(output, 7 | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY);
		else
			SetConsoleTextAttribute(output, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case 'T':
		if (warning == 0)
			SetConsoleTextAttribute(output, 7 | BACKGROUND_RED | BACKGROUND_BLUE);
		else
			SetConsoleTextAttribute(output, BACKGROUND_RED | BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case 'S':
		if (warning == 0)
			SetConsoleTextAttribute(output, 7 | BACKGROUND_GREEN | BACKGROUND_INTENSITY);
		else
			SetConsoleTextAttribute(output, BACKGROUND_GREEN | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case 'Z':
		if (warning == 0)
			SetConsoleTextAttribute(output, 7 | BACKGROUND_RED);
		else
			SetConsoleTextAttribute(output, BACKGROUND_RED | FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case 'L':
		if (warning == 0)
			SetConsoleTextAttribute(output, 7 | BACKGROUND_RED | BACKGROUND_GREEN);
		else
			SetConsoleTextAttribute(output, BACKGROUND_RED | BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case 'J':
		if (warning == 0)
			SetConsoleTextAttribute(output, 7 | BACKGROUND_BLUE);
		else
			SetConsoleTextAttribute(output, BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case 'B':
		if (warning == 0)
			SetConsoleTextAttribute(output, 7 | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
		else
			SetConsoleTextAttribute(output, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case 'G':
		if (warning == 0)
			SetConsoleTextAttribute(output, 7 | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
		else
			SetConsoleTextAttribute(output, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	default:
		if (warning == 0)
			SetConsoleTextAttribute(output, 7);
		else
			SetConsoleTextAttribute(output, FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	}
}
void Print()
{
	HideCursor();
	CursorJump(0, 0);
	for (int x = 0;x < 4;x++)
	{
		for (int y = 0;y < 4;y++)
		{
			if (block.appearance[x][y] == 1)
				map[block.position_x + x][block.position_y + y].block = block.type;
			if (ghost.appearance[x][y] == 1 && map[ghost.position_x + x][ghost.position_y + y].block == 0)
				map[ghost.position_x + x][ghost.position_y + y].block = 'G';
		}
	}
	current_color = -1;
	for (int y = config.height + 2;y >= config.height;y--)
	{
		SetColor(0);
		printf("¡¡¡¡¡¡¡¡¡¡¡¡");
		for (int x = 0;x < config.width;x++)
		{
			SetColor(map[x][y].block);
			if (map[x][y].death == 1 && warning == 1)
				printf("¡Á");
			else
				printf("¡¡");
		}
		printf("\n");
	}
	for (int y = config.height - 1;y >= 0 || y >= config.height - 20;y--)
	{
		SetColor(0);
		if (y <= config.height - 13 || y == config.height - 11 || y == config.height - 9)
			printf("¡¡¡¡¡¡¡¡¡¡¡¡");
		else if (y == config.height - 12)
		{
			if (clear.pc == 1)
			{
				SetConsoleTextAttribute(output, FOREGROUND_RED | FOREGROUND_GREEN);
				current_color = -1;
				printf("   All Clear");
			}
			else
				printf("¡¡¡¡¡¡¡¡¡¡¡¡");
		}
		else if (y == config.height - 10)
		{
			if (clear.b2b == 1)
			{
				SetConsoleTextAttribute(output, 7);
				current_color = -1;
				printf("Back to Back");
			}
			else
				printf("¡¡¡¡¡¡¡¡¡¡¡¡");
		}
		else if (y == config.height - 8)
		{
			if (clear.lines == 0)
				printf("¡¡¡¡¡¡¡¡¡¡¡¡");
			else if (clear.lines == 1)
			{
				SetConsoleTextAttribute(output, 7);
				current_color = -1;
				printf("£Ó£É£Î£Ç£Ì£Å");
			}
			else if (clear.lines == 2)
			{
				SetConsoleTextAttribute(output, 7);
				current_color = -1;
				printf("£Ä£Ï£Õ£Â£Ì£Å");
			}
			else if (clear.lines == 3)
			{
				SetConsoleTextAttribute(output, 7);
				current_color = -1;
				printf("£Ô£Ò£É£Ð£Ì£Å");
			}
			else
			{
				SetConsoleTextAttribute(output, FOREGROUND_GREEN | FOREGROUND_BLUE);
				current_color = -1;
				printf("£Ô£Å£Ô£Ò£É£Ó");
			}
		}
		else if (y == config.height - 7)
		{
			if (clear.t_spin == 0)
				printf("¡¡¡¡¡¡¡¡¡¡¡¡");
			else
			{
				SetConsoleTextAttribute(output, FOREGROUND_RED | FOREGROUND_BLUE);
				current_color = -1;
				printf("¡¡¡¡¡¡T-SPIN");
			}
		}
		else if (y == config.height - 6)
		{
			if (clear.t_mini == 0)
				printf("¡¡¡¡¡¡¡¡¡¡¡¡");
			else
			{
				SetConsoleTextAttribute(output, FOREGROUND_RED | FOREGROUND_BLUE);
				current_color = -1;
				printf("¡¡¡¡¡¡¡¡MINI");
			}
		}
		else if (y >= config.height - 5 && y <= config.height - 2)
		{
			printf("¡¡¡¡");
			for (int i = 0;i < 4;i++)
			{
				if (hold.appearance[i][(17 - config.height + y) % 4] != 0)
				{
					if (_hold_flag == 0)
						SetColor(hold.type);
					else
						SetColor('G');
				}
				else
					SetColor(0);
				printf("¡¡");
			}
		}
		else
		{
			SetColor(0);
			printf("¡¡¡¡£È£Ï£Ì£Ä");
		}
		for (int x = 0;x < config.width;x++)
		{
			SetColor(map[x][y].block);
			if (y < 0)
			{
				for (int x = 0;x < config.width;x++)
					printf("¡¡");
				break;
			}
			if (map[x][y].death == 1)
				printf("¡Á");
			else if (map[x][y].wall == 0 && map[x][y].block == 0)
				printf("¡õ");
			else
				printf("¡¡");
		}
		if (y < garbage_lines && y >= 0)
		{
			SetConsoleTextAttribute(output, BACKGROUND_RED | BACKGROUND_INTENSITY);
			current_color = -1;
		}
		else
			SetColor(0);
		printf("¡¡");
		SetColor(0);
		if (y == config.height - 1)
		{
			SetColor(0);
			printf("£Î£Å£Ø£Ô");
		}
		else if (y >= config.height - 17 && y <= config.height - 2)
		{
			for (int i = 0;i < 4;i++)
			{
				if (_next[(config.height - y - 2) / 4].appearance[i][(17 - config.height + y) % 4] != 0)
					SetColor(_next[(config.height - y - 2) / 4].type);
				else
					SetColor(0);
				printf("¡¡");
			}
		}
		else if (y == config.height - 18)
		{
			if (combo != 0)
			{
				SetConsoleTextAttribute(output, 7);
				current_color = -1;
				printf("%d Combo      ",combo);
			}
			else
				printf("              ");
		}
		else if (y == config.height - 19)
		{
			if (attack != 0)
			{
				if (attack < 10)
				{
					SetConsoleTextAttribute(output, 7);
					current_color = -1;
				}
				else
				{
					SetConsoleTextAttribute(output, FOREGROUND_RED);
					current_color = -1;
				}
				printf("%d Attack      ",attack);
			}
			else
				printf("               ");
		}
		else if (y == 0)
		{
			if (garbage_lines != 0)
			{
				SetConsoleTextAttribute(output, FOREGROUND_RED | FOREGROUND_INTENSITY);
				current_color = -1;
				printf("%d   ", garbage_lines);
			}
			else
				printf("        ");
		}
		printf("\n");
	}
	SetConsoleTextAttribute(output, 7);
	current_color = -1;
	printf("Score:%d\n", score);
	printf("Lines:%d\n", lines);
	if (config.mode == 3)
	{
		printf("Attack:%d/%d          \n", attack_count, attack_goal);
		printf("Time:%d          \n", delay.garbage);
	}
	if (fps < 60)
	{
		SetConsoleTextAttribute(output, FOREGROUND_RED | FOREGROUND_GREEN);
		current_color = -1;
	}
	printf("Fps:%d          \n", fps);
}
void BLOCK::Generate(char type_generate)
{
	state = 0;
	position_x = config.width / 2 - 2;
	if (type_generate == 'I')
		position_y = config.height - 1;
	else
		position_y = config.height;
	for (int x = 0;x < 4;x++)
	{
		for (int y = 0;y < 4;y++)
			appearance[x][y] = 0;
	}
	switch (type_generate)
	{
	case 'I':
		type = 'I';
		for (int i = 0;i < 4;i++)
			appearance[i][2] = 1;
		break;
	case 'O':
		type = 'O';
		for (int i = 1;i < 3;i++)
		{
			for (int j = 1;j < 3;j++)
				appearance[i][j] = 1;
		}
		break;
	case 'T':
		type = 'T';
		for (int i = 0;i < 3;i++)
			appearance[i][1] = 1;
		appearance[1][2] = 1;
		break;
	case 'S':
		type = 'S';
		for (int i = 0;i < 2;i++)
		{
			appearance[i][1] = 1;
			appearance[i + 1][2] = 1;
		}
		break;
	case 'Z':
		type = 'Z';
		for (int i = 0;i < 2;i++)
		{
			appearance[i][2] = 1;
			appearance[i + 1][1] = 1;
		}
		break;
	case 'J':
		type = 'J';
		for (int i = 0;i < 3;i++)
			appearance[i][1] = 1;
		appearance[0][2] = 1;
		break;
	case 'L':
		type = 'L';
		for (int i = 0;i < 3;i++)
			appearance[i][1] = 1;
		appearance[2][2] = 1;
		break;
	}
	for (int x = 0;x < 4;x++)
	{
		for (int y = 0;y < 4;y++)
		{
			if (appearance[x][y] == 1)
			{
				if (map[position_x + x][position_y + y].wall == 1)
				{
					gameover = 1;
					return;
				}
			}
		}
	}
}
void BLOCK::Generate(char type_generate, char if_death)
{
	state = 0;
	position_x = config.width / 2 - 2;
	if (type_generate == 'I')
		position_y = config.height - 1;
	else
		position_y = config.height;
	for (int x = 0;x < 4;x++)
	{
		for (int y = 0;y < 4;y++)
			appearance[x][y] = 0;
	}
	switch (type_generate)
	{
	case 'I':
		type = 'I';
		for (int i = 0;i < 4;i++)
			appearance[i][2] = 1;
		break;
	case 'O':
		type = 'O';
		for (int i = 1;i < 3;i++)
		{
			for (int j = 1;j < 3;j++)
				appearance[i][j] = 1;
		}
		break;
	case 'T':
		type = 'T';
		for (int i = 0;i < 3;i++)
			appearance[i][1] = 1;
		appearance[1][2] = 1;
		break;
	case 'S':
		type = 'S';
		for (int i = 0;i < 2;i++)
		{
			appearance[i][1] = 1;
			appearance[i + 1][2] = 1;
		}
		break;
	case 'Z':
		type = 'Z';
		for (int i = 0;i < 2;i++)
		{
			appearance[i][2] = 1;
			appearance[i + 1][1] = 1;
		}
		break;
	case 'J':
		type = 'J';
		for (int i = 0;i < 3;i++)
			appearance[i][1] = 1;
		appearance[0][2] = 1;
		break;
	case 'L':
		type = 'L';
		for (int i = 0;i < 3;i++)
			appearance[i][1] = 1;
		appearance[2][2] = 1;
		break;
	}
}
void BLOCK::Move(char direction)
{
	Ghost('C');
	if (CheckMove(direction))
	{
		t_spin_flag = 0;
		for (int x = 0;x < 4;x++)
		{
			for (int y = 0;y < 4;y++)
			{
				if (appearance[x][y] == 1 && map[position_x + x][position_y + y].wall == 0)
					map[position_x + x][position_y + y].block = 0;
			}
		}
		if (direction == 'L')
		{
			position_x--;
			lock_count = 0;
		}
		else if (direction == 'R')
		{
			position_x++;
			lock_count = 0;
		}
		else
		{
			position_y--;
			fall_count = 0;
		}
	}
}
void BLOCK::Move(int delta_x, int delta_y)
{
	Ghost('C');
	if (CheckMove(delta_x, delta_y))
	{
		t_spin_flag = 0;
		for (int x = 0;x < 4;x++)
		{
			for (int y = 0;y < 4;y++)
			{
				if (appearance[x][y] == 1 && map[position_x + x][position_y + y].wall == 0)
					map[position_x + x][position_y + y].block = 0;
			}
		}
		position_x += delta_x;
		position_y += delta_y;
	}
}
void GARBAGE::Buffer(int _lines)
{
	if (_lines == 0)
		return;
	else if (_lines > 0)
	{
		for (int i = 0;i < 100;i++)
		{
			if (queue[i] == 0)
			{
				queue[i] = _lines;
				return;
			}
		}
	}
	else
	{
		for (;_lines < 0;_lines++)
		{
			if (queue[0] != 0)
			{
				queue[0]--;
				if (queue[0] == 0)
				{
					for (int i = 0;queue[i + 1] != 0;i++)
					{
						queue[i] = queue[i + 1];
						queue[i + 1] = 0;
					}
				}
			}
			else
			{
				if (queue[1] == 0)
				{
					Buffer(-_lines * config.backfire);
					if (config.mode == 3)
						attack_count += -_lines;
					return;
				}
				else
				{
					for (int i = 0;queue[i + 1] != 0;i++)
					{
						queue[i] = queue[i + 1];
						queue[i + 1] = 0;
					}
				}
			}
		}
	}
}
void GARBAGE::Generate()
{
	int temp;
	static int temp_rand = 0;
	if (queue[0] == 0)
		temp_rand = rand() % config.width;
	if (garbage_lines <= 8)
		temp = garbage_lines;
	else if (garbage_lines > 8)
		temp = 8;
	else
		return;
	for (;temp >= 0;temp--)
	{
		if (queue[0] != 0 && temp != 0)
		{
			int temp_count = 0;
			for (int y = 0;y < 50;y++)
			{
				for (int x = 0;x < config.width;x++)
				{
					if (map[x][y].wall == 1)
					{
						temp_count++;
						break;
					}
				}
			}
			for (int y = temp_count;y > 0;y--)
			{
				for (int x = 0;x < config.width;x++)
				{
					map[x][y].wall = map[x][y - 1].wall;
					map[x][y].block = map[x][y - 1].block;
				}
			}
			for (int x = 0;x < config.width;x++)
			{
				map[x][0].block = 0;
				map[x][0].wall = 0;
				if (x == temp_rand)
					continue;
				map[x][0].block = 'B';
				map[x][0].wall = 1;
			}
			queue[0]--;
		}
		else if (queue[0] == 0)
		{
			for (int i = 0;queue[i + 1] != 0;i++)
			{
				queue[i] = queue[i + 1];
				queue[i + 1] = 0;
			}
			temp_rand = rand() % config.width;
			if (temp != 0)
				temp++;
		}
		else
			break;
	}
}
void TSpin()
{
	if (t_spin_flag == 1)
	{
		int count = 0;
		for (int x = 0;x < 3;x += 2)
		{
			for (int y = 0;y < 3;y += 2)
			{
				if (map[block.position_x + x][block.position_y + y].wall == 1 || block.position_x + x < 0 || block.position_x + x >= config.width || block.position_y + y < 0)
					count++;
			}
		}
		if (count >= 3)
		{
			clear_temp.t_spin = 1;
			clear_temp.timer = 120;
			if (t_mini_flag == 1)
			{
				if (block.state == 0)
				{
					if (map[block.position_x][block.position_y + 2].wall == 0 || map[block.position_x + 2][block.position_y + 2].wall == 0)
						clear_temp.t_mini = 1;
				}
				else if (block.state == 1)
				{
					if (map[block.position_x + 2][block.position_y].wall == 0 || map[block.position_x + 2][block.position_y + 2].wall == 0)
						clear_temp.t_mini = 1;
				}
				else if (block.state == 2)
				{
					if (map[block.position_x][block.position_y].wall == 0 || map[block.position_x + 2][block.position_y].wall == 0)
						clear_temp.t_mini = 1;
				}
				else if (block.state == 3)
				{
					if (map[block.position_x][block.position_y].wall == 0 || map[block.position_x][block.position_y + 2].wall == 0)
						clear_temp.t_mini = 1;
				}
			}
		}
	}
}
void Score()
{
	int level = (200 - delay.fall) / 10;
	if (clear.lines == 0)
	{
		if (clear.t_spin == 1)
		{
			if (clear.t_mini == 1)
			{
				score += (100 * level);
			}
			else
			{
				score += (400 * level);
			}
		}
	}
	else if (clear.lines == 1)
	{
		if (clear.t_spin == 1)
		{
			if (clear.t_mini == 1)
			{
				if (clear.b2b == 1)
					score += (200 * level * 1.5);
				else
					score += (200 * level);
			}
			else
			{
				if (clear.b2b == 1)
					score += (800 * level * 1.5);
				else
					score += (800 * level);
			}
		}
		else
			score += (100 * level);
	}
	else if (clear.lines == 2)
	{
		if (clear.t_spin == 1)
		{
			if (clear.t_mini == 1)
			{
				if (clear.b2b == 1)
					score += (400 * level * 1.5);
				else
					score += (400 * level);
			}
			else
			{
				if (clear.b2b == 1)
					score += (1200 * level * 1.5);
				else
					score += (1200 * level);
			}
		}
		else
			score += (300 * level);
	}
	else if (clear.lines == 3)
	{
		if (clear.t_spin == 1)
		{
			if (clear.b2b == 1)
				score += (1600 * level * 1.5);
			else
				score += (1600 * level);
		}
		else
			score += (500 * level);
	}
	else
	{
		if (clear.b2b == 1)
			score += (800 * level * 1.5);
		else
			score += (800 * level);
	}
}
void Attack()
{
	if (clear_temp.timer == 120)
	{
		if (clear_temp.lines != 0)
		{
			if (clear_temp.lines == 1)
			{
				attack_temp += clear_temp.b2b;
				if (clear_temp.t_spin == 1 && clear_temp.t_mini == 0)
				{
					attack_temp += 2;
					attack_temp *= (1 + 0.25 * combo);
				}
				else
				{
					if (combo <= 1);
					else if (combo >= 2 && combo <= 5)
						attack_temp += 1;
					else if (combo >= 6 && combo <= 15)
						attack_temp += 2;
					else
						attack_temp += 3;
				}
			}
			else
			{
				if (clear_temp.lines > 1 && clear_temp.lines <= 3 && (clear_temp.t_spin == 0 || clear_temp.t_mini == 1))
				{
					attack_temp = clear_temp.lines - 1;
				}
				else if (clear_temp.lines == 4)
					attack_temp = 4;
				else
					attack_temp = clear_temp.lines * 2;
				attack_temp += clear_temp.b2b;
				attack_temp *= (1 + 0.25 * combo);
			}
			int count = 0;
			for (int x = 0; x < config.width; x++)
			{
				if (map[x][0].wall == 1)
					break;
				else
					count++;
			}
			if (count == config.width)
			{
				attack_temp += 10;
				clear_temp.pc = 1;
				if (clear_temp.b2b == 1)
					score += (3500 * 1.5 * (200 - delay.fall) / 10);
				else
					score += (3500 * (200 - delay.fall) / 10);
			}
			attack += attack_temp;
			garbage.Buffer(-attack_temp);
			attack_temp = 0;
		}
		memcpy(&clear, &clear_temp, sizeof(clear));
		Score();
		clear_temp.timer--;
	}
	else
		memset(&clear_temp, 0, sizeof(clear_temp));
	if (clear.timer != 0)
		clear.timer--;
	else
	{
		memset(&clear, 0, sizeof(clear));
		attack = 0;
	}
}
void ClearLine()
{
	int temp_lines = 0;
	for (int y = 0;y < 50;y++)
	{
		int count = 0;
		for (int x = 0;x < config.width;x++)
		{
			if (map[x][y].wall == 1)
				count++;
		}
		if (count == 0)
		{
			if (temp_lines == 0)
			{
				combo_flag = 0;
				combo = 0;
				garbage.Generate();
			}
			else
			{
				lines += temp_lines;
				if (combo_flag == 1)
					combo++;
				combo_flag = 1;
				if (clear_temp.t_spin == 1 || temp_lines == 4)
				{
					if (b2b_flag == 1)
						clear_temp.b2b = 1;
					b2b_flag = 1;
				}
				else
				{
					b2b_flag = 0;
				}
				clear_temp.lines = temp_lines;
				clear_temp.timer = 120;
			}
			return;
		}
		if (count == config.width)
		{
			temp_lines++;
			count = 0;
			for (int j = y;j < 50;j++)
			{
				for (int i = 0;i < config.width;i++)
				{
					if (map[i][j].wall == 1)
						count = 1;
					map[i][j].block = map[i][j + 1].block;
					map[i][j].wall = map[i][j + 1].wall;
				}
				if (count == 0)
					break;
			}
			y--;
		}
	}
}
void BLOCK::Lock()
{
	char gameover_flag = 0;
	int count = 0;
	lock_count = 0;
	if (type == 'T')
		TSpin();
	_hold_flag = 0;
	Ghost('C');
	for (int x = 0;x < 4;x++)
	{
		for (int y = 0;y < 4;y++)
		{
			if (appearance[x][y] == 1)
				map[position_x + x][position_y + y].block = 0;
		}
	}
	for (int i = 0;;i++)
	{
		if (!CheckMove(0, -i))
		{
			for (int x = 0;x < 4;x++)
			{
				for (int y = 0;y < 4;y++)
				{
					if (appearance[x][y] == 1)
					{
						map[position_x + x][position_y + y - i + 1].block = type;
						map[position_x + x][position_y + y - i + 1].wall = 1;
						if (position_y + y - i + 1 >= config.height)
							count++;
					}
				}
			}
			if (count == 4)
				gameover_flag = 1;
			break;
		}
		else
			score += 2;
	}
	score -= 2;
	ClearLine();
	block.Generate(_next[0].type);
	for (int i = 0;i < 13;i++)
		_next[i] = _next[i + 1];
	if (clear_temp.lines == 0 && gameover_flag == 1)
		gameover = 1;
}
void Hold()
{
	if (_hold_flag == 0)
	{
		_hold_flag = 1;
		lock_count = 0;
		Ghost('C');
		for (int x = 0;x < 4;x++)
		{
			for (int y = 0;y < 4;y++)
			{
				if (block.appearance[x][y] == 1)
				{
					map[block.position_x + x][block.position_y + y].block = 0;
				}
			}
		}
		if (hold.type == 0)
		{
			hold.Generate(block.type);
			block.Generate(_next[0].type);
			for (int i = 0;i < 13;i++)
				_next[i] = _next[i + 1];
		}
		else
		{
			char temp = block.type;
			block.Generate(hold.type);
			hold.Generate(temp);
		}
	}
	else
		return;
}
char SRS(char state_begin, char state_after, int* delta_x, int* delta_y)
{
	if (state_begin == 0 && state_after == 2)
	{
		if (CheckMove(0, 0))
		{
			*delta_x = 0;
			*delta_y = 0;
			return 1;
		}
		else if (CheckMove(0, 1))
		{
			*delta_x = 0;
			*delta_y = 1;
			return 1;
		}
		else if (CheckMove(1, 1))
		{
			*delta_x = 1;
			*delta_y = 1;
			return 1;
		}
		else if (CheckMove(-1, 1))
		{
			*delta_x = -1;
			*delta_y = 1;
			return 1;
		}
		else if (CheckMove(1, 0))
		{
			*delta_x = 1;
			*delta_y = 0;
			return 1;
		}
		else if (CheckMove(-1, 0))
		{
			*delta_x = -1;
			*delta_y = 0;
			return 1;
		}
	}
	else if (state_begin == 2 && state_after == 0)
	{
		if (CheckMove(0, 0))
		{
			*delta_x = 0;
			*delta_y = 0;
			return 1;
		}
		else if (CheckMove(0, -1))
		{
			*delta_x = 0;
			*delta_y = -1;
			return 1;
		}
		else if (CheckMove(-1, -1))
		{
			*delta_x = -1;
			*delta_y = -1;
			return 1;
		}
		else if (CheckMove(1, -1))
		{
			*delta_x = 1;
			*delta_y = -1;
			return 1;
		}
		else if (CheckMove(-1, 0))
		{
			*delta_x = -1;
			*delta_y = 0;
			return 1;
		}
		else if (CheckMove(1, 0))
		{
			*delta_x = 1;
			*delta_y = 0;
			return 1;
		}
	}
	else if (state_begin == 1 && state_after == 3)
	{
		if (CheckMove(0, 0))
		{
			*delta_x = 0;
			*delta_y = 0;
			return 1;
		}
		else if (CheckMove(1, 0))
		{
			*delta_x = 1;
			*delta_y = 0;
			return 1;
		}
		else if (CheckMove(1, 2))
		{
			*delta_x = 1;
			*delta_y = 2;
			return 1;
		}
		else if (CheckMove(1, 1))
		{
			*delta_x = 1;
			*delta_y = 1;
			return 1;
		}
		else if (CheckMove(0, 2))
		{
			*delta_x = 0;
			*delta_y = 2;
			return 1;
		}
		else if (CheckMove(0, 1))
		{
			*delta_x = 0;
			*delta_y = 1;
			return 1;
		}
	}
	else if (state_begin == 3 && state_after == 1)
	{
		if (CheckMove(0, 0))
		{
			*delta_x = 0;
			*delta_y = 0;
			return 1;
		}
		else if (CheckMove(-1, 0))
		{
			*delta_x = -1;
			*delta_y = 0;
			return 1;
		}
		else if (CheckMove(-1, 2))
		{
			*delta_x = -1;
			*delta_y = 2;
			return 1;
		}
		else if (CheckMove(-1, 1))
		{
			*delta_x = -1;
			*delta_y = 1;
			return 1;
		}
		else if (CheckMove(0, 2))
		{
			*delta_x = 0;
			*delta_y = 2;
			return 1;
		}
		else if (CheckMove(0, 1))
		{
			*delta_x = 0;
			*delta_y = 1;
			return 1;
		}
	}
	else
	{
		if (block.type == 'I')
		{
			if (CheckMove(0, 0))
			{
				*delta_x = 0;
				*delta_y = 0;
				return 1;
			}
			else
			{
				if ((state_begin == 0 && state_after == 1) || (state_begin == 3 && state_after == 2))
				{
					if (CheckMove(-2, 0))
					{
						*delta_x = -2;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(1, 0))
					{
						*delta_x = 1;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(-2, -1))
					{
						*delta_x = -2;
						*delta_y = -1;
						return 1;
					}
					else if (CheckMove(1, 2))
					{
						*delta_x = 1;
						*delta_y = 2;
						return 1;
					}
				}
				else if ((state_begin == 1 && state_after == 0) || (state_begin == 2 && state_after == 3))
				{
					if (CheckMove(2, 0))
					{
						*delta_x = 2;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(-1, 0))
					{
						*delta_x = -1;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(2, 1))
					{
						*delta_x = 2;
						*delta_y = 1;
						return 1;
					}
					else if (CheckMove(-1, -2))
					{
						*delta_x = -1;
						*delta_y = -2;
						return 1;
					}
				}
				else if ((state_begin == 1 && state_after == 2) || (state_begin == 0 && state_after == 3))
				{
					if (CheckMove(-1, 0))
					{
						*delta_x = -1;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(2, 0))
					{
						*delta_x = 2;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(-1, 2))
					{
						*delta_x = -1;
						*delta_y = 2;
						return 1;
					}
					else if (CheckMove(2, -1))
					{
						*delta_x = 2;
						*delta_y = -1;
						return 1;
					}
				}
				else if ((state_begin == 2 && state_after == 1) || (state_begin == 3 && state_after == 0))
				{
					if (CheckMove(1, 0))
					{
						*delta_x = 1;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(-2, 0))
					{
						*delta_x = -2;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(1, -2))
					{
						*delta_x = 1;
						*delta_y = -2;
						return 1;
					}
					else if (CheckMove(-2, 1))
					{
						*delta_x = -2;
						*delta_y = 1;
						return 1;
					}
				}
			}
		}
		else
		{
			if (CheckMove(0, 0))
			{
				*delta_x = 0;
				*delta_y = 0;
				return 1;
			}
			else
			{
				if ((state_begin == 0 && state_after == 1) || (state_begin == 2 && state_after == 1))
				{
					if (CheckMove(-1, 0))
					{
						*delta_x = -1;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(-1, 1))
					{
						*delta_x = -1;
						*delta_y = 1;
						return 1;
					}
					else if (CheckMove(0, -2))
					{
						*delta_x = 0;
						*delta_y = -2;
						return 1;
					}
					else if (CheckMove(-1, -2))
					{
						*delta_x = -1;
						*delta_y = -2;
						t_mini_flag = 0;
						return 1;
					}
				}
				else if ((state_begin == 1 && state_after == 0) || (state_begin == 1 && state_after == 2))
				{
					if (CheckMove(1, 0))
					{
						*delta_x = 1;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(1, -1))
					{
						*delta_x = 1;
						*delta_y = -1;
						return 1;
					}
					else if (CheckMove(0, 2))
					{
						*delta_x = 0;
						*delta_y = 2;
						return 1;
					}
					else if (CheckMove(1, 2))
					{
						*delta_x = 1;
						*delta_y = 2;
						t_mini_flag = 0;
						return 1;
					}
				}
				else if ((state_begin == 2 && state_after == 3) || (state_begin == 0 && state_after == 3))
				{
					if (CheckMove(1, 0))
					{
						*delta_x = 1;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(1, 1))
					{
						*delta_x = 1;
						*delta_y = 1;
						return 1;
					}
					else if (CheckMove(0, -2))
					{
						*delta_x = 0;
						*delta_y = -2;
						return 1;
					}
					else if (CheckMove(1, -2))
					{
						*delta_x = 1;
						*delta_y = -2;
						t_mini_flag = 0;
						return 1;
					}
				}
				else if ((state_begin == 3 && state_after == 2) || (state_begin == 3 && state_after == 0))
				{
					if (CheckMove(-1, 0))
					{
						*delta_x = -1;
						*delta_y = 0;
						return 1;
					}
					else if (CheckMove(-1, -1))
					{
						*delta_x = -1;
						*delta_y = -1;
						return 1;
					}
					else if (CheckMove(0, 2))
					{
						*delta_x = 0;
						*delta_y = 2;
						return 1;
					}
					else if (CheckMove(-1, 2))
					{
						*delta_x = -1;
						*delta_y = 2;
						t_mini_flag = 0;
						return 1;
					}
				}
			}
		}
	}
	return 0;
}
void BLOCK::Rotate(char direction)
{
	t_mini_flag = 1;
	Ghost('C');
	char state_temp;
	char appearance_temp[4][4];
	memcpy(appearance_temp, appearance, sizeof(appearance));
	for (int x = 0;x < 4;x++)
	{
		for (int y = 0;y < 4;y++)
		{
			if (appearance[x][y] == 1)
				map[position_x + x][position_y + y].block = 0;
		}
	}
	int delta_x, delta_y;
	if (direction == 'L')
		state_temp = (state + 3) % 4;
	else if (direction == 'R')
		state_temp = (state + 1) % 4;
	else
		state_temp = (state + 2) % 4;
	if (type == 'I')
	{
		if (state_temp == 0)
		{
			char temp[4][4] = { {0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 1)
		{
			char temp[4][4] = { {0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 2)
		{
			char temp[4][4] = { {0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else
		{
			char temp[4][4] = { {0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
	}
	else if (type == 'O')
	{
		char temp[4][4] = { {0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0} };
		memcpy(appearance, temp, sizeof(temp));
	}
	else if (type == 'T')
	{
		if (state_temp == 0)
		{
			char temp[4][4] = { {0,1,0,0},{0,1,1,0},{0,1,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 1)
		{
			char temp[4][4] = { {0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 2)
		{
			char temp[4][4] = { {0,1,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else
		{
			char temp[4][4] = { {0,1,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
	}
	else if (type == 'S')
	{
		if (state_temp == 0)
		{
			char temp[4][4] = { {0,1,0,0},{0,1,1,0},{0,0,1,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 1)
		{
			char temp[4][4] = { {0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 2)
		{
			char temp[4][4] = { {1,0,0,0},{1,1,0,0},{0,1,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else
		{
			char temp[4][4] = { {0,1,1,0},{1,1,0,0},{0,0,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
	}
	else if (type == 'Z')
	{
		if (state_temp == 0)
		{
			char temp[4][4] = { {0,0,1,0},{0,1,1,0},{0,1,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 1)
		{
			char temp[4][4] = { {0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 2)
		{
			char temp[4][4] = { {0,1,0,0},{1,1,0,0},{1,0,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else
		{
			char temp[4][4] = { {1,1,0,0},{0,1,1,0},{0,0,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
	}
	else if (type == 'J')
	{
		if (state_temp == 0)
		{
			char temp[4][4] = { {0,1,1,0},{0,1,0,0},{0,1,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 1)
		{
			char temp[4][4] = { {0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 2)
		{
			char temp[4][4] = { {0,1,0,0},{0,1,0,0},{1,1,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else
		{
			char temp[4][4] = { {1,0,0,0},{1,1,1,0},{0,0,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
	}
	else
	{
		if (state_temp == 0)
		{
			char temp[4][4] = { {0,1,0,0},{0,1,0,0},{0,1,1,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 1)
		{
			char temp[4][4] = { {0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else if (state_temp == 2)
		{
			char temp[4][4] = { {1,1,0,0},{0,1,0,0},{0,1,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
		else
		{
			char temp[4][4] = { {0,0,1,0},{1,1,1,0},{0,0,0,0},{0,0,0,0} };
			memcpy(appearance, temp, sizeof(temp));
		}
	}
	if (SRS(state, state_temp, &delta_x, &delta_y))
	{
		block.Move(delta_x, delta_y);
		state = state_temp;
		t_spin_flag = 1;
		lock_count = 0;
	}
	else
		memcpy(appearance, appearance_temp, sizeof(appearance));
}
void Bag7(char mode)
{
	char order[7] = { 'I','J','L','O','S','Z','T' }, temp;
	for (int i = 0; i < 30; i++)
	{
		int p = rand() % 7, q = rand() % 7;
		temp = order[p];
		order[p] = order[q];
		order[q] = temp;
	}
	if (mode == 0)
	{
		for (int i = 0; i < 7; i++)
		{
			_next[i].Generate(order[i]);
		}
	}
	else
	{
		for (int i = 13; i >= 7; i--)
		{
			_next[i].Generate(order[13 - i]);
		}
	}
}
void CONFIG::Save()
{
	ofstream config_file;
	config_file.open("config.dat", ios::binary);
	if (!config_file.is_open())
	{
		SetConsoleTextAttribute(output, FOREGROUND_RED);
		cout << "Error:Failed to save config data! Exiting in 5 seconds." << endl;
		SetConsoleTextAttribute(output, 7);
		Sleep(5000);
	}
	else
		config_file.write((const char*)&config, sizeof(config));
}
void CONFIG::Load()
{
	ifstream config_file;
	config_file.open("config.dat", ios::binary);
	if (!config_file.is_open())
	{
		SetConsoleTextAttribute(output, FOREGROUND_RED);
		cout << "Error:Cannot find config data! Now using the default settings." << endl;
		cout << "(If it is the first time you run this program, please ignore.)" << endl;
		SetConsoleTextAttribute(output, 7);
		ofstream config_file;
		config_file.open("config.dat", ios::binary);
		if (!config_file.is_open())
		{
			SetConsoleTextAttribute(output, FOREGROUND_RED);
			cout << "Error:Failed to create config data!" << endl;
			SetConsoleTextAttribute(output, 7);
		}
		else
			config_file.write((const char*)&config, sizeof(config));
	}
	else
		while (config_file.read((char*)&config, sizeof(config)));
}
void PrintKeyboardConfig(char key)
{
	if (key == 38)
		cout << "Up Arrow" << endl;
	else if (key == 37)
		cout << "Left Arrow" << endl;
	else if (key == 39)
		cout << "Right Arrow" << endl;
	else if (key == 40)
		cout << "Down Arrow" << endl;
	else if (key == 32)
		cout << "Space" << endl;
	else
		cout << key << endl;
}
void GetInput()
{
	unsigned static int left_count = 0, right_count = 0, left_rotate_count = 0, right_rotate_count = 0;
	static char left_rotate_flag = 0, right_rotate_flag = 0, _180_rotate_flag = 0, hold_flag = 0, hard_drop_flag = 0, left_DAS_flag = 0, right_DAS_flag = 0, left_ARR_flag = 0, right_ARR_flag = 0, soft_drop_ARR_flag = 0;
	if (GetKeyState(config.left) < 0)
		left_count++;
	if (GetKeyState(config.right) < 0)
		right_count++;
	if (GetKeyState(config.left) >= 0)
	{
		left_DAS_flag = 0;
		left_ARR_flag = 0;
		left_count = 0;
		_left.DAS = 0;
		_left.ARR = 0;
	}
	if (GetKeyState(config.right) >= 0)
	{
		right_DAS_flag = 0;
		right_ARR_flag = 0;
		right_count = 0;
		_right.DAS = 0;
		_right.ARR = 0;
	}
	if (GetKeyState(config.soft_drop) >= 0)
	{
		soft_drop.ARR = 0;
		soft_drop_ARR_flag = 0;
	}
	if (GetKeyState(config.left_rotate) >= 0)
		left_rotate_flag = 0;
	if (GetKeyState(config.right_rotate) >= 0)
		right_rotate_flag = 0;
	if (GetKeyState(config._180_rotate) >= 0)
		_180_rotate_flag = 0;
	if (GetKeyState(config.hold) >= 0)
		hold_flag = 0;
	if (GetKeyState(config.hard_drop) >= 0)
		hard_drop_flag = 0;
	if (GetKeyState(config.left) < 0)
	{
		if (GetKeyState(config.right) < 0 && right_count < left_count)
		{
			if (right_DAS_flag == 0)
			{
				block.Move('R');
				right_DAS_flag = 1;
			}
			if (_right.DAS < config.DAS)
				_right.DAS++;
			else
			{
				if (right_ARR_flag == 0)
				{
					if (config.ARR != 0)
						block.Move('R');
					else
					{
						while (CheckMove('R'))
							block.Move('R');
					}
					right_ARR_flag = 1;
				}
				if (_right.ARR < config.ARR - 1)
					_right.ARR++;
				else
				{
					block.Move('R');
					_right.ARR = 0;
				}
			}
		}
		else
		{
			if (left_DAS_flag == 0)
			{
				block.Move('L');
				left_DAS_flag = 1;
			}
			if (_left.DAS < config.DAS)
				_left.DAS++;
			else
			{
				if (left_ARR_flag == 0)
				{
					if (config.ARR != 0)
						block.Move('L');
					else
					{
						while (CheckMove('L'))
							block.Move('L');
					}
					left_ARR_flag = 1;
				}
				if (_left.ARR < config.ARR - 1)
					_left.ARR++;
				else
				{
					block.Move('L');
					_left.ARR = 0;
				}
			}
		}
	}
	else
	{
		if (GetKeyState(config.right) < 0)
		{
			if (right_DAS_flag == 0)
			{
				block.Move('R');
				right_DAS_flag = 1;
			}
			if (_right.DAS < config.DAS)
				_right.DAS++;
			else
			{
				if (right_ARR_flag == 0)
				{
					if (config.ARR != 0)
						block.Move('R');
					else
					{
						while (CheckMove('R'))
							block.Move('R');
					}
					right_ARR_flag = 1;
				}
				if (_right.ARR < config.ARR - 1)
					_right.ARR++;
				else
				{
					block.Move('R');
					_right.ARR = 0;
				}
			}
		}
	}
	if (GetKeyState(config.left_rotate) < 0 && left_rotate_flag == 0)
	{
		block.Rotate('L');
		left_rotate_flag = 1;
	}
	if (GetKeyState(config.right_rotate) < 0 && right_rotate_flag == 0)
	{
		block.Rotate('R');
		right_rotate_flag = 1;
	}
	if (GetKeyState(config._180_rotate) < 0 && _180_rotate_flag == 0)
	{
		block.Rotate('B');
		_180_rotate_flag = 1;
	}
	if (GetKeyState(config.hold) < 0 && hold_flag == 0)
	{
		Hold();
		hold_flag = 1;
		t_spin_flag = 0;
	}
	if (GetKeyState(config.soft_drop) < 0)
	{
		if (soft_drop_ARR_flag == 0)
		{
			if (config.SDARR != 0)
			{
				if (CheckMove('D'))
					score++;
				block.Move('D');
			}
			else
			{
				while (CheckMove('D'))
				{
					score++;
					block.Move('D');
				}
			}
			soft_drop_ARR_flag = 1;
		}
		if (soft_drop.ARR < config.SDARR - 1)
			soft_drop.ARR++;
		else
		{
			if (CheckMove('D'))
				score++;
			block.Move('D');
			soft_drop.ARR = 0;
		}
	}
	if (GetKeyState(config.hard_drop) < 0 && hard_drop_flag == 0)
	{
		block.Lock();
		hard_drop_flag = 1;
	}
}
void Initialize()
{
	memset(&map, 0, sizeof(map));
	memset(&delay, 0, sizeof(delay));
	memset(&clear, 0, sizeof(clear));
	memset(&clear_temp, 0, sizeof(clear_temp));
	memset(&garbage, 0, sizeof(garbage));
	memset(&hold, 0, sizeof(hold));
	warning = 0;
	_hold_flag = 0;
	t_spin_flag = 0;
	t_mini_flag = 1;
	b2b_flag = 0;
	combo_flag = 0;
	gameover = 0;
	score = 0;
	lines = 0;
	garbage_lines = 0;
	combo = 0;
	attack_temp = 0;
	attack = 0;
	lock_count = 0;
	fall_count = 0;
	garbage_count = 0;
	attack_count = 0;
	attack_goal = 0;
	Bag7(0);
	Bag7(1);
	block.Generate(_next[0].type);
	for (int i = 0;i < 13;i++)
		_next[i] = _next[i + 1];
	if (config.mode == 0)
		delay.fall = 200 - 50 * config.difficulty;
	else
	{
		delay.fall = 90;
		if (config.mode == 1)
			delay.garbage = 900 - 180 * config.difficulty;
		else if (config.mode == 3)
		{
			delay.garbage = attack_delay;
			attack_goal = 5 * config.difficulty + 5;
		}
		else
		{
			delay.garbage = 300;
			garbage_raise = config.difficulty + 1;
		}
	}
	delay.lock = 60;
}
int Fps(int fps, DWORD btime)
{
	DWORD etime = timeGetTime();
	int sleep_time = 1000 / fps + btime - etime;
	if (sleep_time > 0)
		Sleep(sleep_time);
	return 1000 / (etime - btime + (sleep_time > 0 ? sleep_time : 0));
}
char Menu()
{
	char error_flag = 0;
retry0:
	system("cls");
	config.Load();
	SetConsoleTextAttribute(output, 7);
	cout << "----------Tetris in C++----------" << endl;
	cout << "        1.Start" << endl;
	cout << "        2.Game Setting" << endl;
	cout << "        3.Visual Setting" << endl;
	cout << "        4.Keyboard Setting" << endl;
	cout << "        5.Control Setting" << endl;
	cout << "        0.Exit" << endl;
	cout << "---------------------------------" << endl;
	cout << "Version: 2.6" << endl;
	cout << "Author: Cute_Monster" << endl;
	int temp = _getch();
	switch (temp)
	{
	case '1':
		break;
	case '2':
	retry2:
		system("cls");
		cout << "----------Game Setting----------" << endl;
		cout << "        1.Mode:";
		switch (config.mode)
		{
		case 0:
			cout << "Marathon" << endl;
			break;
		case 1:
			cout << "Survival" << endl;
			break;
		case 2:
			cout << "Dig Challange" << endl;
			break;
		case 3:
			cout << "Attack Challange" << endl;
			break;
		}
		cout << "        2.Difficulty:";
		switch (config.difficulty)
		{
		case 0:
			cout << "Easy" << endl;
			break;
		case 1:
			cout << "Normal" << endl;
			break;
		case 2:
			cout << "Hard" << endl;
			break;
		case 3:
			cout << "Insane" << endl;
			break;
		case 4:
			cout << "Impossible" << endl;
			break;
		}
		cout << "        3.Board Width:" << config.width << endl;
		cout << "        4.Board Height:" << config.height << endl;
		cout << "        5.Backfire:";
		switch (config.backfire)
		{
		case 0:
			cout << "Off" << endl;
			break;
		case 1:
			cout << "1x" << endl;
			break;
		case 2:
			cout << "2x" << endl;
			break;
		}
		cout << "        0.Save and Exit" << endl;
		cout << "--------------------------------" << endl;
		temp = _getch();
		switch (temp)
		{
		case '1':
			config.mode = (config.mode + 1) % 4;
			goto retry2;
		case '2':
			config.difficulty = (config.difficulty + 1) % 5;
			goto retry2;
		case '3':
		retry23:
			cout << "Input the width you want, then press Enter" << endl;
			if (error_flag == 1)
			{
				SetConsoleTextAttribute(output, FOREGROUND_RED);
				error_flag = 0;
			}
			cout << "(Must between 4 and 50)" << endl;
			SetConsoleTextAttribute(output, 7);
			cin >> temp;
			if (temp < 4 || temp > 50)
			{
				error_flag = 1;
				goto retry23;
			}
			else
			{
				config.width = temp;
				goto retry2;
			}
		case '4':
		retry24:
			cout << "Input the height you want, then press Enter" << endl;
			if (error_flag == 1)
			{
				SetConsoleTextAttribute(output, FOREGROUND_RED);
				error_flag = 0;
			}
			cout << "(Must between 1 and 30)" << endl;
			SetConsoleTextAttribute(output, 7);
			cin >> temp;
			if (temp < 1 || temp > 30)
			{
				error_flag = 1;
				goto retry24;
			}
			else
			{
				config.height = temp;
				goto retry2;
			}
		case '5':
			config.backfire = (config.backfire + 1) % 3;
			goto retry2;
		case '0':
			config.Save();
			goto retry0;
		default:
			goto retry2;
		}
	case '3':
	retry3:
		system("cls");
		cout << "----------Visual Setting----------" << endl;
		cout << "        1.Shadow:";
		switch (config.shadow)
		{
		case 0:
			cout << "Off" << endl;
			break;
		case 1:
			cout << "On" << endl;
			break;
		}
		cout << "        2.Hidden:";
		switch (config.hidden)
		{
		case 0:
			cout << "Off" << endl;
			break;
		case 1:
			cout << "On" << endl;
			break;
		}
		cout << "        0.Save and Exit" << endl;
		cout << "----------------------------------" << endl;
		temp = _getch();
		switch (temp)
		{
		case '1':
			config.shadow = (config.shadow + 1) % 2;
			goto retry3;
		case '2':
			config.hidden = (config.hidden + 1) % 2;
			goto retry3;
		case '0':
			config.Save();
			goto retry0;
		default:
			goto retry3;
		}
	case '4':
	retry4:
		system("cls");
		if (error_flag == 1)
		{
			SetConsoleTextAttribute(output, FOREGROUND_RED);
			cout << "Error:Duplicate keys are not allowed!" << endl;
			error_flag = 0;
			SetConsoleTextAttribute(output, 7);
		}
		cout << "----------Keyboard Setting----------" << endl;
		cout << "        1.Left Move:";
		PrintKeyboardConfig(config.left);
		cout << "        2.Right Move:";
		PrintKeyboardConfig(config.right);
		cout << "        3.Soft Drop:";
		PrintKeyboardConfig(config.soft_drop);
		cout << "        4.Hard Drop:";
		PrintKeyboardConfig(config.hard_drop);
		cout << "        5.Left Rotate:";
		PrintKeyboardConfig(config.left_rotate);
		cout << "        6.Right Rotate:";
		PrintKeyboardConfig(config.right_rotate);
		cout << "        7.180¡ã Rotate:";
		PrintKeyboardConfig(config._180_rotate);
		cout << "        8.Hold:";
		PrintKeyboardConfig(config.hold);
		cout << "        9.Retry:";
		PrintKeyboardConfig(config.retry);
		cout << "        0.Save and Exit" << endl;
		cout << "------------------------------------" << endl;
		temp = _getch();
		if (temp >= '1' && temp <= '9')
		{
		retry40:
			cout << "Press the key you want to set" << endl;
			if (error_flag == 1)
			{
				SetConsoleTextAttribute(output, FOREGROUND_RED);
				error_flag = 0;
			}
			cout << "(Must be A-Z / Space / Four Arrows)" << endl;
			SetConsoleTextAttribute(output, 7);
			int temp_0 = _getch();
			if (temp_0 == 224)
			{
				temp_0 = _getch();
				switch (temp_0)
				{
				case 75:
					temp_0 = 37;
					break;
				case 72:
					temp_0 = 38;
					break;
				case 77:
					temp_0 = 39;
					break;
				case 80:
					temp_0 = 40;
					break;
				}
			}
			else if ((temp_0 >= 'A' && temp_0 <= 'Z') || temp_0 == 32);
			else if (temp_0 >= 'a' && temp_0 <= 'z')
			{
				temp_0 -= 32;
			}
			else
			{
				error_flag = 1;
				goto retry40;
			}
			switch (temp)
			{
			case '1':
				config.left = temp_0;
				goto retry4;
			case '2':
				config.right = temp_0;
				goto retry4;
			case '3':
				config.soft_drop = temp_0;
				goto retry4;
			case '4':
				config.hard_drop = temp_0;
				goto retry4;
			case '5':
				config.left_rotate = temp_0;
				goto retry4;
			case '6':
				config.right_rotate = temp_0;
				goto retry4;
			case '7':
				config._180_rotate = temp_0;
				goto retry4;
			case '8':
				config.hold = temp_0;
				goto retry4;
			case '9':
				config.retry = temp_0;
				goto retry4;
			}
		}
		else if (temp == '0')
		{
			int check[8] = { config.left, config.right ,config.soft_drop ,config.hard_drop ,config.left_rotate ,config.right_rotate ,config._180_rotate,config.hold };
			for (int i = 0;i < 7;i++)
			{
				for (int j = i + 1;j < 8;j++)
				{
					if (check[i] == check[j])
					{
						error_flag = 1;
						goto retry4;
					}
				}
			}
			config.Save();
			goto retry0;
		}
		else
			goto retry4;
	case '5':
	retry5:
		system("cls");
		SetConsoleTextAttribute(output, FOREGROUND_RED | FOREGROUND_GREEN);
		cout << "Warning:These settings are not recommanded to change if you don't understand." << endl;
		SetConsoleTextAttribute(output, 7);
		cout << "----------Control Setting----------" << endl;
		cout << "    1.DAS(Delayed Auto Shift):" << config.DAS << "F" << endl;
		cout << "    2.ARR(Automatic Repeat Rate):" << config.ARR << "F" << endl;
		cout << "    3.Soft Drop ARR:" << config.SDARR << "F" << endl;
		cout << "    0.Save and Exit" << endl;
		cout << "-----------------------------------" << endl;
		temp = _getch();
		if (temp >= '1' && temp <= '3')
		{
		retry50:
			cout << "Input the number you want, then press Enter" << endl;
			if (error_flag == 1)
			{
				SetConsoleTextAttribute(output, FOREGROUND_RED);
				error_flag = 0;
			}
			cout << "(Must be 0 or above)" << endl;
			SetConsoleTextAttribute(output, 7);
			int temp_0;
			cin >> temp_0;
			if (temp_0 < 0)
			{
				error_flag = 1;
				goto retry50;
			}
			else
			{
				switch (temp)
				{
				case '1':
					config.DAS = temp_0;
					goto retry5;
				case '2':
					config.ARR = temp_0;
					goto retry5;
				case '3':
					config.SDARR = temp_0;
					goto retry5;
				}
			}
		}
		else if (temp == '0')
		{
			config.Save();
			goto retry0;
		}
		else
			goto retry5;
	case '0':
		return 1;
	default:
		goto retry0;
	}
	return 0;
}
void Warning()
{
	char death_flag = 0;
	warning = 0;
	for (int y = config.height;y > config.height - 4;y--)
	{
		for (int x = 0;x < config.width;x++)
		{
			if (map[x][y - garbage_lines].wall == 1 || garbage_lines >= config.height)
			{
				warning = 1;
				goto _break;
			}
		}
	}
_break:
	if ((warning == 1 && death_flag == 0) || death.position_y < config.height)
	{
		death_flag = 1;
		for (int x = 0;x < 4;x++)
		{
			for (int y = 0;y < 4;y++)
			{
				if (death.appearance[x][y] == 1)
					map[death.position_x + x][death.position_y + y].death = 0;
			}
		}
		death.Generate(_next[0].type, 'D');
		death.position_y -= (garbage_lines > 8 ? 8 : garbage_lines);
		for (int x = 0;x < 4;x++)
		{
			for (int y = 0;y < 4;y++)
			{
				if (death.appearance[x][y] == 1)
					map[death.position_x + x][death.position_y + y].death = 1;
			}
		}
	}
	else
		death_flag = 0;
}
void Timer()
{
	if (fall_count < delay.fall)
		fall_count++;
	else
	{
		block.Move('D');
		fall_count = 0;
	}
	if (!CheckMove('D'))
		lock_count++;
	if (lock_count >= delay.lock)
		block.Lock();
	if (config.mode == 1)
	{
		if (garbage_count < delay.garbage)
			garbage_count++;
		else
		{
			garbage.Buffer(4);
			garbage_count = 0;
		}
	}
	else if (config.mode == 2)
	{
		if (garbage_count < delay.garbage)
			garbage_count++;
		else
		{
			for (int i = 0;i < garbage_raise;i++)
				garbage.Buffer(1);
			garbage_count = 0;
		}
	}
	else if (config.mode == 3)
	{
		if (attack_count < attack_goal)
		{
			if (delay.garbage > 0)
				delay.garbage--;
			else
			{
				delay.garbage = attack_delay;
				garbage.Buffer(attack_goal - attack_count);
				attack_count -= attack_goal;
			}
		}
		else
		{
			attack_count -= attack_goal;
			attack_goal++;
			delay.garbage += attack_delay;
			if (delay.garbage > attack_delay + 300)
				delay.garbage = attack_delay + 300;
		}
	}
}

int main()
{
	srand((unsigned int)time(NULL));
	output = GetStdHandle(STD_OUTPUT_HANDLE);
retry0:
	if (Menu())
		return 0;
	char retry_flag = 0;
retry:
	Initialize();
	system("cls");
	while (!gameover)
	{
		DWORD btime = timeGetTime();
		garbage_lines = 0;
		for (int i = 0;garbage.queue[i] != 0;i++)
			garbage_lines += garbage.queue[i];
		if (_next[6].type == _next[13].type)
			Bag7(1);
		Ghost('N');
		Warning();
		Print();
		GetInput();
		if (GetKeyState(config.retry) < 0 && retry_flag == 0)
		{
			retry_flag = 1;
			goto retry;
		}
		if (GetKeyState(config.retry) >= 0)
			retry_flag = 0;
		if (GetKeyState(VK_ESCAPE) < 0)
			goto retry0;
		if (config.mode == 0)
		{
			if (delay.fall > 0)
			{
				delay.fall = 200 - 50 * config.difficulty - lines;
				if (delay.fall < 0)
					delay.fall = 0;
			}
			else
				delay.lock = 60 - (lines - 200 + config.difficulty * 50) / 2;
		}
		Attack();
		Timer();
		fps = Fps(60, btime);
		if (_kbhit())
			_getch();
	}
	garbage_lines = 0;
	for (int i = 0;garbage.queue[i] != 0;i++)
		garbage_lines += garbage.queue[i];
	Ghost('C');
	Print();
	SetConsoleTextAttribute(output, FOREGROUND_RED);
	cout << "Game Over!" << endl;
	Sleep(3000);
	cout << "1.Retry" << endl;
	cout << "2.Menu" << endl;
	cout << "3.Exit" << endl;
	SetConsoleTextAttribute(output, 7);
retry_over:
	int temp = _getch();
	switch (temp)
	{
	case '2':
		goto retry0;
	case '3':
		return 0;
	case '1':
		retry_flag = 1;
		goto retry;
	default:
		goto retry_over;
	}
}