#include "MDR32Fx.h"			// подключение библиотеки МК
#define F_CPU 8000000			//указание тактовой частоты МК
#include "milkites_delay.h"		//подключение библиотеки задержек
#include "milkites_display.h"	//подключение библиотеки дисплея
#include "pwm_step_motor.h"
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>

#define baseL 84		//победа
#define startSize 5		//стартовая длина
int HighScore = 0;		//рекорд

#define LCD_led_en MDR_PORTE->RXTX |= (1<<2) // вкл. подсветки
#define LCD_led_dis MDR_PORTE->RXTX &= ~(1<<2) // выкл. подсветки

#define PE7  1 << 7
#define PB4  1 << 4											// bit of the button PB4
#define PE7  1 << 7											// bit of the button PE7
#define PE6  1 << 6											// bit of the button PE6
#define PE3  1 << 3											// bit of the button PE3
#define Read_PB4 (MDR_PORTB->RXTX & PB4)					// Reading pin PB4
#define Read_PE3 (MDR_PORTE->RXTX & PE3)					// Reading pin PE3
#define Read_PE7 (MDR_PORTE->RXTX & PE7)					// Reading pin PE7
#define Read_PE6 (MDR_PORTE->RXTX & PE6)					// Reading pin PE6

enum CellType { Empty, Wall, Snake, SnakeHead, Apple, };
enum State { NewGame, Standard, Death, Restart, Win};

struct s_Cell
{
	int		y, x;
	int prevY, prevX;
	enum CellType cellType;

};
char c_Wall[8] =		{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }; //графика
char c_Empty[8] =		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
char c_Snake[8] =		{ 0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00 };
char c_SnakeHead[8] =	{ 0x00, 0x7E, 0x5A, 0x66, 0x66, 0x5a, 0x7E, 0x00 };
char c_Apple[8] =		{ 0x00, 0x7C, 0x7C, 0x7E, 0x7F, 0x65, 0x7C, 0x00 };	 

enum CellType clr_map[8][16] = 
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
enum CellType m_map[8][16] =
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


void drawBlock(int Y, int X, char* what);										//отрисовка клетки на дисплее
void drawMap(struct s_Cell* snake, size_t size, struct s_Cell* apple);			//заполнение карты

void repeatTail(struct s_Cell* snake, size_t size);								//повторить шаг для хвоста
void step(struct s_Cell* snake, size_t size, char dir);							//перемещение головы змейки

void spawnApple(struct s_Cell* apple);											//переместить яблоко
void eatApple(struct s_Cell* snake, size_t* size, struct s_Cell* apple);		//событие - съесть яблоко

int sec_counter = 0;															//переменная-счетчик секунд
void Timer1_init(void);
void Timer1_IRQHandler(void);
void Timer1_start(void);														//команда запуска Таймера 1

enum State stateNewGame(bool start);											//состояние начала игры
enum State stateStandart(struct s_Cell* snake, size_t* size, struct s_Cell* apple, char* dir); //событие обычной игры
enum State stateGameOver(size_t score, bool start);								//событие проигрыша игры
void stateRestart(struct s_Cell* snake, size_t* size, struct s_Cell* apple);	//событие рестарта
enum State stateWin(size_t score, bool start);									//событие победа

int main()
{
	MDR_RST_CLK->PER_CLOCK = 0xffffffff; 
	MDR_PORTA->OE = 0xffff;						// настройка PORTA на выход
	MDR_PORTA->FUNC = 0x0000;					// функция - порт, основная функция
	MDR_PORTA->PWR = 0xffff;					// максимально быстрый фронт
	MDR_PORTA->ANALOG = 0xffff;				// режим работы - цифровой ввод/вывод
	MDR_PORTA->RXTX = 0x0000;						// запись числа 255 для включения
	
	MDR_PORTB->FUNC = 0x0000;						// функция - порт, основная функция
	MDR_PORTB->ANALOG = 0xffff;					// режим работы - цифровой ввод/вывод
	
	MDR_RST_CLK->PER_CLOCK = 0xffffffff;			// вкл. тактирования периферии МК 
	MDR_PORTE->OE = 0xff37;						// биты 7,6,3 PORTE - входы (кнопки), др. - выходы
	MDR_PORTE->FUNC = 0x0000;						// функция - порт, основная функция
	MDR_PORTE->PWR = 0xff37;						// макс. быстрый фронт
	MDR_PORTE->ANALOG = 0xffff;					// режим работы порта - цифровой ввод/вывод
	
	delay_init();									// инициализация системы задержек
	LCD_init();									// инициализация дисплея
	LCD_clear();									// первая очистка
	LCD_led_en;									// вкл. подсветки дисплея
	
	Timer1_init();								// инициализация Таймера 1
	NVIC_EnableIRQ(Timer1_IRQn);					// разрешение прерывания от Таймера 1
	__enable_irq();								// глобальное разрешение прерываний
	Timer1_start();								// запуск Таймера 1
	
	size_t snake_Length = startSize;						//стартовая длина змейки
	char dir = 'r';											//начальное направление змейки
	struct s_Cell *snake = 
		(struct s_Cell*)calloc(85, sizeof(struct s_Cell));	//выделение памяти для змейки
	struct s_Cell apple;									//объявление яблока

	enum State state = NewGame;								//определение стартового статуса

	while (true)
	{
		switch (state)
		{
		case NewGame:
			if(stateNewGame((!Read_PB4)||(!Read_PE6)||(!Read_PE7)||(!Read_PE3)) == Restart) state = Restart;
			break;
		case Standard:
			state = stateStandart(snake, &snake_Length, &apple, &dir);
			break;

		case Death:
			if(stateGameOver(snake_Length - startSize, (!Read_PB4) || (!Read_PE6) || (!Read_PE7) || (!Read_PE3)) == Restart) state = Restart;
			break;
		case Win:
			stateWin(snake_Length - startSize, (!Read_PB4) || (!Read_PE6) || (!Read_PE7) || (!Read_PE3));
			state = Standard;
			break;
		case Restart:
			stateRestart(snake, &snake_Length, &apple);
			state = Standard;
			break;
		default:
			break;
		}
	}

	free(snake);
	return 0;
}

void drawBlock(int Y, int X, char* what)				//what - по сути массив из восьми палок(пикслей)
{
	LCD_page_set(Y);
	for (size_t i = 0; i < 8; i++)
	{
		LCD_column_set(8 * X + i);
		LCD_wr_byte(what[i]);
	}
}
void drawMap(struct s_Cell* snake, size_t size, struct s_Cell* apple) //перезапись карты 
{
	for (int i = 1; i < 7; i++)							//очищение карты копированием шаблона
		for (int j = 1; j < 15; j++)
			m_map[i][j] = clr_map[i][j];

	m_map[(*apple).y][(*apple).x] = (*apple).cellType;	//добавляем яблоко на карту

	for (int i = 0; i < size; i++)						//добавляем змейку на карту
		m_map[snake[i].y][snake[i].x] = snake[i].cellType;

	for (int Y = 0; Y < 8; Y++)							//отрисовка карты на дисплее
		for (int X = 0; X < 16; X++)
		{
			switch (m_map[Y][X])
			{
			case Wall:
				drawBlock(Y, X, c_Wall); break;
			case Snake:
				drawBlock(Y, X, c_Snake); break;
			case SnakeHead:
				drawBlock(Y, X, c_SnakeHead); break;
			case Apple:
				drawBlock(Y, X, c_Apple); break;
			default:
				drawBlock(Y, X, c_Empty); break;
				break;
			}
		}

	for (size_t i = 0; i < 8; i++)
	{
		for (size_t j = 0; j < 16; j++)
		{
			printf("%d", m_map[i][j]);
		}
		printf("\n");
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
	if (dir == 'r')
	{
		snake[0].x = snake[0].x + 1;
	}
	else if (dir == 'l')
	{
		snake[0].x = snake[0].x - 1;
	}
	else if (dir == 'u')
	{
		snake[0].y = snake[0].y - 1;
	}
	else if (dir == 'd')
	{
		snake[0].y = snake[0].y + 1;
	}
	repeatTail(snake, size);
}

	void spawnApple(struct s_Cell* apple)
	{
		while (true)	//перемещение в любое место
		{
			int y = 1 + rand() % 6;
			int x = 1 + rand() % 14;
			if (m_map[y][x] == Empty)
			{
				apple->x = x; apple->y = y; break;
			}
		}
	}
	void eatApple(struct s_Cell* snake, size_t* size, struct s_Cell* apple)
	{
		snake[*size].x = snake[*size - 1].prevX;
		snake[*size].y = snake[*size - 1].prevY;
		*size += 1;			//увличение размера змейки
		spawnApple(apple);
	}

	enum State stateNewGame(bool start)
	{
		LCD_set_cursor(1);
		LCD_column_set(8*6);
		LCD_print_text("Hello");
		LCD_set_cursor(2);
		LCD_print_text("PB4 - DOWN");
		LCD_set_cursor(3);
		LCD_print_text("PE6 - UP");
		LCD_set_cursor(4);
		LCD_print_text("PE7 - LEFT");
		LCD_set_cursor(5);
		LCD_print_text("PE3 - RIGHT");
		LCD_set_cursor(6);
		LCD_print_text("POT - SPEED CONTROL");
		LCD_set_cursor(7);
		LCD_print_text("PRESS ANY BUTTON");
		delay_ms(20);
		if (start) return Restart;
		else return NewGame;
	}
	enum State stateStandart(struct s_Cell* snake, size_t* size, struct s_Cell* apple, char* dir)
	{
		if(!Read_PB4) *dir = 'd';		//перезапись направления по кнопке
		else if(!Read_PE6) *dir = 'u';
		else if(!Read_PE7) *dir = 'l';
		else if(!Read_PE3) *dir = 'r';
		else;

		MCU_ADC_set_ch(4);							//чтение порта
		float ch_4_counts = MCU_ADC_read() - 38;	//приведение к нулю
		if(sec_counter > ch_4_counts)				//шаг змейки от значения напряжени на потенциометре
		{
			step(snake, *size, *dir);				//движение змейки

			switch (m_map[snake[0].y][snake[0].x])	//если клетка на которую наступили 
			{
			case Wall:								//стена - смерть
				return Death;
				break;
			case Snake:								//сама змейка - смерть
				return Death;
				break;
			case Apple:								//яблоко -  
				eatApple(snake, size, apple);		//добавляем размер и перемещаяем яблоко
				drawMap(snake, *size, apple);		//отрисовка карты
				if (size >= baseL - 1) return Win;	//условие победы выполняется - победа
				else return Standard;
				break;
			default:								//ничего не произошло
				drawMap(snake, *size, apple);		//отрисовка карты
				return Standard;					//продолжаем игру как обычно
				break;
			}
		}
		return Standard;
	}
	enum State stateGameOver(size_t score, bool start)
	{
		if (HighScore > score) HighScore = score;
		LCD_set_cursor(3);
		LCD_column_set(8*6);
		LCD_print_text("Death");
		LCD_set_cursor(4);
		LCD_column_set(8*2);
		LCD_print_text("Your score is: ");
		LCD_print_num(score);
		LCD_set_cursor(5);
		LCD_column_set(8*2);
		LCD_print_text("Highscore is: ");
		LCD_print_num(HighScore);
		if (start) return Restart;
		else return Death;
	}
	void stateRestart(struct s_Cell* snake, size_t* size, struct s_Cell* apple)
	{
		*size = startSize;
		for (size_t i = 0; i < baseL; i++)	//переизициализация всей змейки
		{
			snake[i].x = 5;
			snake[i].y = 1;
			snake[i].prevX = snake[i].x;
			snake[i].prevY = snake[i].y;
			snake[i].cellType = Snake;
		}
		snake[0].cellType = SnakeHead;		//переизициализация всей змейки
		apple->cellType = Apple;
		spawnApple(apple);					//функция перемещения змейки

		drawMap(snake, *size, apple);		//отрисовка карты
	}
	enum State stateWin(size_t score, bool start)
	{
		HighScore = score;
		LCD_set_cursor(3);
		LCD_column_set(8*6);
		LCD_print_text("WIN");
		LCD_set_cursor(4);
		LCD_column_set(8*2);
		LCD_print_text("Your score is: ");
		LCD_print_num(score);
		if (start) return Restart;
		else return Win;
	}

	void Timer1_init(void)					//инициализация таймера
	{
	 MDR_RST_CLK->TIM_CLOCK |= (1<< 24);	//вкл. тактирование Таймера 1
	 MDR_TIMER1->CNTRL = 0x00000000; 
	 MDR_TIMER1->PSG = 1;					//частоты равный двум тактам
	 MDR_TIMER1->ARR = 999;					//основание счета = CNT + 1 = 1000
	 MDR_TIMER1->CNT = 0;					//начальное значение счетчика
	 MDR_TIMER1->IE = 2;					//разрешение генерир. прерывание при CNT=ARR
	}
	void Timer1_IRQHandler(void)			//обраотчик прерываний
	{
	 sec_counter++;							//инкремент счетчика секунд
	 MDR_TIMER1->CNT = 0					//сброс счетчика
	 MDR_TIMER1->STATUS = 0;				// сброс статуса прерывания
	 NVIC_ClearPendingIRQ(Timer1_IRQn);		// сброс статуса прерывания
	} 
	void Timer1_start(void) // команда запуска Таймера 1
	{ 
	 MDR_TIMER1->CNTRL = 1;
	}
