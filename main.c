#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


enum cellTypes { Empty, Wall, Snake, Apple,};

struct s_Cell
{
	int		y, x;
	int prevY, prevX;
	enum cellTypes cellType;
};

char c_Wall[8] =  { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
char c_Empty[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
char c_Snake[8] = { 0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00 };
char c_Apple[8] = { 0x00, 0x3E, 0x3E, 0x7E, 0xFE, 0xA6, 0x3E, 0x00 };	


enum cellTypes clr_map[8][16] = 
{
	{Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall} 
};

enum cellTypes m_map[8][16] =
{  
	{Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty, Wall},
	{Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall, Wall}
};

spawApple(struct s_Cell* apple)
{
	int y = 1, x = 1;
	while (true)
	{
		y = 1 + rand() % 6;
		x = 1 + rand() % 14;
		if (m_map[y][x] == Empty)
		{
			apple->x = x; apple->y = y; break;
		}
	}
}

void drawBlock(int Y, int X, char* what) //what - по сути палка, тут надо либо делать такое дл¤ вадратов, либо более сложное
{
	//LCD_page_set(Y);
	for (size_t i = 0; i < 8; i++)
	{
		//LCD_column_set(8 * X + i);
		//LCD_wr_byte(what[i]);
	}
}


void fillMap(struct s_Cell* snake, size_t size, struct s_Cell *apple)
{
	for (int i = 1; i < 7; i++) //очищение, копирование карты
		for (int j = 1; j < 15; j++)
			m_map[i][j] = clr_map[i][j];

	for (int i = 0; i < size; i++) //добавл¤ем змейку
		m_map[snake[i].y][snake[i].x] = snake->cellType;

	m_map[apple->y][apple->x] = apple->cellType;

	for (int Y = 1; Y < 7; Y++) //отрисовка карты на дисплее !!!!!!!!!!!!!!!!!!!!!!!!!!
		for (int X = 1; X < 15; X++)
		{
			switch (m_map[Y][X])
			{
			case Wall:
				drawBlock(Y, X, 0xff); break;
			case Snake:
				drawBlock(Y, X, 0xff); break;
			case Apple:
				drawBlock(Y, X, 0xff); break;
			default:
				drawBlock(Y, X, 0x00); break;
				break;
			}
			if (m_map[Y][X] == 1)
				drawBlock(Y, X, 0xff);
			else if (m_map[Y][X] == 0)
				drawBlock(Y, X, 0x00);
		}
}

void repeatTail(struct s_Cell* snake, size_t size)
{
	for (size_t i = 1; i < size; i++)
	{
		snake[i].prevX = snake[i].x;
		snake[i].prevY = snake[i].y;
		snake[i].x = snake[i - 1].prevX;
		snake[i].y = snake[i - 1].prevY;
	}
}


void step(struct s_Cell* snake, size_t size, char dir)
{
	snake[0].prevX = snake[0].x;
	snake[0].prevY = snake[0].y;
	if (dir == 'r') //ѕ≈–≈ƒ≈Ћј“№  Ќќѕ ”!!!!!
	{
		snake[0].x = snake[0].x + 1;
	}
	else if (dir == 'l') //ѕ≈–≈ƒ≈Ћј“№  Ќќѕ ”!!!!!
	{
		snake[0].x = snake[0].x - 1;
	}
	else if (dir == 'u') //ѕ≈–≈ƒ≈Ћј“№  Ќќѕ ”!!!!!
	{
		snake[0].y = snake[0].y - 1;
	}
	else if (dir == 'd') //ѕ≈–≈ƒ≈Ћј“№  Ќќѕ ”!!!!!
	{
		snake[0].y = snake[0].y + 1;
	}
	repeatTail(snake, size);
}

enum cellTypes doAction(struct s_Cell* snake, size_t* size, struct s_Cell* apple)
{
	switch (m_map[snake[0].y][snake[0].x])
	{
	case Empty: break;
	case Wall: return Wall; break;
	case Snake: return Wall; break;
	case Apple: 
		snake[*size].x = snake[*size - 1].x;
		snake[*size].y = snake[*size - 1].y;
		*size += 1; 
		spawApple(apple);
		return Apple; break;
	default:
		break;
	}
}

int main()
{
	size_t snake_Length = 3;
	struct s_Cell snake[86];
	for (size_t i = 0; i < sizeof(snake) / sizeof(snake[0]); i++)
	{
		snake[i].x = 5;
		snake[i].y = 2;
		snake[i].prevX = snake[i].x;
		snake[i].prevY = snake[i].y;
		snake[i].cellType = Snake;
	}

	struct s_Cell apple; apple.x = 1; apple.y = 1; apple.cellType = Apple;
	spawApple(&apple);
	fillMap(snake, snake_Length, &apple);

	char dir = 'r'; //в проекте убрать иль нет, хммммммммммммм
	printf("\n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n \n"); //прост опускаю в низ сразу, забить

	while (true)
	{
		//заменить на отрисовку на дисплее
		for (size_t i = 0; i < 8; i++)
		{
			for (size_t j = 0; j < 16; j++)
			{
				printf("%d", m_map[i][j]);
			}
			printf("\n");
		}

		dir = getchar(); //заменить на 
		//if (!Read_PE3)
		//	step('r');
		getchar(); //костыль против enter
		
		step(snake, snake_Length, dir);
		if (doAction(snake, &snake_Length, &apple) == Wall)
		{
			printf("LALKA!!!\n");
			break;
		}
		fillMap(snake, snake_Length, &apple);
		printf("----------------------------------------- \n");
}
