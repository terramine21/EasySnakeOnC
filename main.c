/*------------------------------------------------------------*/
//#include "MDR32Fx.h" // подключение библиотеки МК
//#define F_CPU 8000000 // указание тактовой частоты МК
//#include "milkites_delay.h" // подключение библиотеки задержек
//#include "milkites_display.h" // подключение библиотеки дисплея
//#include "pwm_step_motor.h"
#include "stdbool.h"
#include <stdio.h>
#include <stdlib.h>

/*------------------------------------------------------------*/
//#define LCD_led_en MDR_PORTE->RXTX |= (1<<2) // вкл. подсветки
//#define LCD_led_dis MDR_PORTE->RXTX &= ~(1<<2) // выкл. подсветки
/*------------------------------------------------------------*/
//кнопочки
//#define PE7  1 << 7
//#define PB4  1 << 4																					// bit of the button PB4
//#define PB5  0xff																						// bit of the SENSOR
//#define PE7  1 << 7																					// bit of the button PE7
//#define PE6  1 << 6																					// bit of the button PE6
//#define PE3  1 << 3																					// bit of the button PE3
//#define Read_PB4 (MDR_PORTB->RXTX & PB4)										// Reading pin PB4
//#define Read_PB5 (MDR_PORTB->RXTX & PB5)										// Reading pin PB5
//#define Read_PE7 (MDR_PORTE->RXTX & PE7)										// Reading pin PE7
//#define Read_PE6 (MDR_PORTE->RXTX & PE6)										// Reading pin PE6
//#define Read_PB10 (MDR_PORTE->RXTX & PB10)									// Reading pin PE6
//bool last_state_PB5 = 0;																		// Last state of PD7
//bool last_state_PB4 = 0; 																		// Last state of PB4
//bool last_state_PE7 = 0; 																		// Last state of PE7
//bool last_state_PE6 = 0;					
//													// Last state of PE6
//bool PE3_counter = 0;
//bool PE7_counter = 0;
//bool PB4_counter = 0;
//bool PD4_counter = 0;
//bool PE6_counter = 0;
//bool PB5_counter = 0;
//кнопочки определяются и всякое такое
/*------------------------------------------------------------*/
//Определяем нажатие кнопок и их четность
							//PE3
//#define PE3  1 << 3																// bit of the button PE3
//#define Read_PE3 (MDR_PORTE->RXTX & PE3)										// Reading pin PE3
//bool last_state_PE3 = 0;														// Last state of PE3
//bool checkPE3()
//{
//	if((Read_PE3)&& !last_state_PE3)// Чтение состояния PE7 если состояние отличается
//	{
//		PE3_counter = 0;
//		return false;
//	}
//	else {return true;}
//}
							//PE7
//void BUTTON_PE7()
//{
//	if((Read_PE7)&& !last_state_PE7)// Чтение состояния PE7 если состояние отличается
//	{
//		if(!PE7_counter){STEPPER_SPEED(0);}
//		if(PE7_counter){STEPPER_SPEED(1);} //обработка состояний, кароч тык работает, тык не работает, каеф
//		PE7_counter=!PE7_counter; //меняем счетчик на противоположное.
//			last_state_PE7 = Read_PE7;
//	}
//	else {last_state_PE7 = Read_PE7;}
//}
							//PB4
//void BUTTON_PB4()
//{
//	if((Read_PB4)&& !last_state_PB4)// Чтение состояния PE7 если состояние отличается
//	{
//		if(!PB4_counter){MDR_PORTA->RXTX = 0x00;}
//		if(PB4_counter){MDR_PORTA->RXTX = 0xF0;} //обработка состояний, кароч тык работает, тык не работает, каеф
//		PB4_counter=!PB4_counter; //меняем счетчик на противоположное.
//			last_state_PB4 = Read_PB4;
//	}
//	else {last_state_PB4 = Read_PB4;}
//}
							//PE6
//void Button_PE6()
//{
//		if((Read_PE6)&&!last_state_PE6)							// Reading button PE6
//		{
//			last_state_PE6 = 1 << 6;
//			if(!PE6_counter){MDR_PORTA->RXTX = 0x00;}
//			if(PE6_counter){MDR_PORTA->RXTX = 0xFF;}
//			PE6_counter=!PE6_counter;
//			last_state_PE6 = Read_PE6;
//		}
//		else {last_state_PE6 = Read_PE6;}												// Change of variable last state button
//}
							//PB5
//void BUTTON_PB5()
//{
//	if((Read_PB5)&& !last_state_PB5)
//	{
//		if(!PB5_counter){ MDR_PORTA->RXTX = 0x00;}
//		if(PB5_counter){MDR_PORTA->RXTX = 0xFF;} //обработка состояний, кароч тык работает, тык не работает, каеф
//		PB5_counter=!PB5_counter; //меняем счетчик на противоположное.
//			last_state_PB5 = Read_PB5;
//	}
//	else {last_state_PB5 = Read_PB5;}
//}



enum CellType { Empty, Wall, Snake, SnakeHead, Apple, };
enum State { NewGame, Standard, Death, Restart};

struct s_Cell //по сути змейка
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

void spawnApple(struct s_Cell* apple)
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

void drawBlock(int Y, int X, char* what) //what - по сути палка, тут надо либо делать такое для квадратов, либо более сложное
{
	//LCD_page_set(Y);
	for (size_t i = 0; i < 8; i++)
	{
		//LCD_column_set(8 * X + i);
		//LCD_wr_byte(what[i]);
	}
}

void fillMap(struct s_Cell* snake, size_t size, struct s_Cell *apple) //ТОЛЬКО ПЕРЕРИСОВКА КАРТЫ!
{
	for (int i = 1; i < 7; i++) //очищение, копирование карты
		for (int j = 1; j < 15; j++)
			m_map[i][j] = clr_map[i][j];

	for (int i = 0; i < size; i++) //добавляем змейку
		m_map[snake[i].y][snake[i].x] = snake->cellType;

	m_map[apple->y][apple->x] = apple->cellType;

	for (int Y = 0; Y < 8; Y++) //отрисовка карты на дисплее !!!!!!!!!!!!!!!!!!!!!!!!!!
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

//ИЗБАВЛЯЮСЬ!?
enum CellType doAction(struct s_Cell* snake, size_t* size, struct s_Cell* apple)
{
	switch (m_map[snake[0].y][snake[0].x])
	{
	case Empty: break;
	case Wall: return Wall; break;
	case Snake: return Snake; break;
	case Apple: 
		snake[*size].x = snake[*size - 1].x;
		snake[*size].y = snake[*size - 1].y;
		*size += 1; 
		spawnApple(apple);
		return Apple; break;
	default:
		break;
	}
}
//ИЗБАВЛЯЮСЬ!?

//uint16_t sec_counter = 0; // переменная-счетчик секунд
//void Timer1_init(void)
//{ // настройка Т1 на генерирование прерывания каждую секунду
// MDR_RST_CLK->TIM_CLOCK |= (1<< 24); // вкл. тактирование Таймера 1
// // режим счета – вверх,начальное значение – число из регистра CNT
// MDR_TIMER1->CNTRL = 0x00000000; 
// MDR_TIMER1->PSG = 1; // предделитель частоты 
// MDR_TIMER1->ARR = 999; // основание счета = CNT + 1 = 1000
// MDR_TIMER1->CNT = 0; // начальное значение счетчика
// MDR_TIMER1->IE = 2; // разрешение генерир. прерывание при CNT=ARR
//}
//
//void Timer1_IRQHandler(void)
//{ // обработчик прерывания от Т1
// sec_counter++; // инкремент счетчика секунд
// MDR_TIMER1->CNT = 0; // сброс счетчика таймера
// MDR_TIMER1->STATUS = 0; // сброс статуса прерывания
// NVIC_ClearPendingIRQ(Timer1_IRQn); // сброс статуса прерывания
//} 
//void Timer1_start(void) // команда запуска Таймера 1
//{ 
// MDR_TIMER1->CNTRL = 1;
//}


enum State  stateNewGame( bool start)
{
	//LCD_set_cursor(1);
	//LCD_column_set(8*6);
	//LCD_print_text("Hello");
	//LCD_set_cursor(2);
	//LCD_print_text("PB4 - DOWN");
	//LCD_set_cursor(3);
	//LCD_print_text("PE6 - UP");
	//LCD_set_cursor(4);
	//LCD_print_text("PE7 - LEFT");
	//LCD_set_cursor(5);
	//LCD_print_text("PE3 - RIGHT");
	//LCD_set_cursor(6);
	//LCD_print_text("POT - SPEED CONTROL");
	//LCD_set_cursor(7);
	//LCD_print_text("PRESS ANY BUTTON");
	//delay_ms(20);
	if (start) return Restart;
	else NewGame;
}
void stateStandart(struct s_Cell* snake, size_t *size, struct s_Cell* apple, char *dir)
{
	switch (doAction(snake, &size, &apple))
	{
	case Wall:
		return Death;
		break;
	case Snake:
		return Death;
		break;
	default:
		//MCU_ADC_set_ch(4);
		//ch_4_counts = MCU_ADC_read() - 38;

		//if(!Read_PB4) *dir = 'd';
		//else if(!Read_PE6) *dir = 'u';
		//else if(!Read_PE7) *dir = 'l';
		//else if(!Read_PE3) *dir = 'r';
		//else;
		//if(sec_counter > ch_4_counts)
	{
		//sec_counter = 0;
		step(snake, size, dir);
		//state = doAction(snake, &snake_Length, &apple);
		fillMap(snake, size, &apple);
	}
	break;
	}
}
enum State stateGameOver(size_t score, bool start)
{
	//LCD_set_cursor(3);
	//LCD_column_set(8*6);
	//LCD_print_text(Wall == gameState ? "WALL" : "SNAKE");
	//LCD_set_cursor(4);
	//LCD_column_set(8*2);
	//LCD_print_text("Your score is: ");
	//LCD_print_num(score);
	//delay_ms(10000);
	if (start) return Restart;
	else Death;
}
void stateRestart(struct s_Cell* snake, size_t size, struct s_Cell* apple)
{
	size = 3;
	for (size_t i = 0; i < size; i++)
	{
		snake[i].x = 5;
		snake[i].y = 2;
		snake[i].prevX = snake[i].x;
		snake[i].prevY = snake[i].y;
		snake[i].cellType = Snake;
	}
	snake[0].cellType = SnakeHead;

	apple->x = 1; apple->y = 1; apple->cellType = Apple;
	spawnApple(apple);
}

int main()
{
//setub
	////куча бесполезных инициализаций
	//MDR_RST_CLK->PER_CLOCK = 0xffffffff; 
	//MDR_PORTA->OE = 0xffff; // настройка PORTA на выход
	//MDR_PORTA->FUNC = 0x0000; // функция - порт, основная функция
	//MDR_PORTA->PWR = 0xffff; // максимально быстрый фронт
	//MDR_PORTA->ANALOG = 0xffff; // режим работы - цифровой ввод/вывод
	//MDR_PORTA->RXTX = 0x0000; // запись числа 255 для включения
	//
	//MDR_PORTB->FUNC = 0x0000; // функция - порт, основная функция
	//MDR_PORTB->ANALOG = 0xffff; // режим работы - цифровой ввод/вывод
	//
	//MDR_RST_CLK->PER_CLOCK = 0xffffffff; // вкл. тактирования периферии МК 
	//MDR_PORTE->OE = 0xff37; // биты 7,6,3 PORTE - входы (кнопки), др. - выходы
	//MDR_PORTE->FUNC = 0x0000; // функция - порт, основная функция
	//MDR_PORTE->PWR = 0xff37; // макс. быстрый фронт
	//MDR_PORTE->ANALOG = 0xffff; // режим работы порта - цифровой ввод/вывод
	//
	//delay_init(); // инициализация системы задержек
	//LCD_init(); // инициализация дисплея
	//LCD_clear(); // очистка дисплея
	//LCD_led_en; // вкл. подсветки дисплея
	//
	//Timer1_init(); // инициализация Таймера 1
	//NVIC_EnableIRQ(Timer1_IRQn); // разрешение прерывания от Таймера 1
	//__enable_irq(); // глобальное разрешение прерываний
	//Timer1_start(); // запуск Таймера 1
	
	size_t snake_Length = 3;
	struct s_Cell snake[85];

	struct s_Cell apple; 
	apple.x = 1; apple.y = 1; apple.cellType = Apple;
	spawnApple(&apple);
	fillMap(snake, snake_Length, &apple);
	char dir = 'd'; //в проекте убрать иль нет, хммммммммммммм
	
	enum State state = NewGame;
	enum CellType gameState;
	
					float ch_4_counts = 0;
	
//setub end
	while (true)
	{
		switch (state)
		{
		case NewGame:
			if(stateNewGame(true) == Restart) state = Restart; //заменить true (!Read_PB4)||(!Read_PE6)||(!Read_PE7)||(!Read_PE3)
			break;
		case Standard:
			stateStandart(snake, &snake_Length, &apple, &dir);
			break;

		case Death:
			if(stateGameOver(snake_Length - 3, true) == Restart) state = Restart; //заменить 3 на переменную, true на (!Read_PB4)||(!Read_PE6)||(!Read_PE7)||(!Read_PE3)
			break;
		case Restart:
			stateRestart(snake, snake_Length, &apple);
			state = Standard;
			//REinit
			break;
		default:
			break;
		}
	}
	return 0;
}
