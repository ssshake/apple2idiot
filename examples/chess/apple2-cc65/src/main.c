#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <apple2.h>
#include <peekpoke.h>

#include "globals.h"
#include "menu.h"
#include "apple2idiot.h"
#include "../../chess_commands.h"
#include "../../../../arduino-lib/Apple2Idiot/A2I_commands.h"

unsigned char i = 0;
char pause_char;
char player_move[5];
char ai_move[5];
char state = STATE_NONE;
unsigned int address_offset = 0;

char game_board[10][24] = {
    "8: r n b q k b n r",
    "7: p p p p p p p p",
    "6: . . . . . . . .",
    "5: . . . . . . . .",
    "4: . . . . . . . .",
    "3: . . . . . . . .",
    "2: P P P P P P P P",
    "1: R N B Q K B N R",
    "   ---------------",
    "   a b c d e f g h"
};


void displayCurrentBoard(void);
void createNewBoard(void);  // deprecated
void spinner(void);
void awaitMoveResponse(void);
void popup(unsigned char* message);

void popup(unsigned char* message) {
    //cclearxy(x, y, SIZE_X); 
    //gotoxy(x,y);
    cclearxy(0, 7, SIZE_X); 
    cclearxy(0, 8, SIZE_X); 
    cclearxy(0, 9, SIZE_X); 
    cclearxy(0, 10, SIZE_X); 
    gotoxy(0,7);
    printf("+-------------------------------------+\n");
    printf("! %s \n", message);
    printf("+-------------------------------------+\n");
    printf("(Press a key)\n");
    if (kbhit()) {
        cgetc();
    }
    pause_char = cgetc();
}

void main(void) {
    unsigned char key;
    while (state != STATE_QUIT) {
        switch(state) {
            case STATE_LOCAL_GAME:
                displayCurrentBoard();
                cclearxy(0, 23, SIZE_X); 
                gotoxy(0,23);
                printf("Your Move:");
                fgets(player_move, sizeof player_move, stdin);
                player_move[strcspn(player_move, "\n")] = 0;
                write_string_to_ram(RAM_DATA_START_ADDRESS, player_move);
                write_byte(APPLE_COMMAND_ADDRESS, CHESS_MAKE_MOVE);
                awaitMoveResponse();
                break;
            case STATE_NONE:
                clrscr();
                drawMenuBar();
                gotoxy(0,23);
                printf("Menu Selection:");
                cursor(1);
                key = toupper(cgetc());
                switch(key) {
                    case ASCII_W:
                        cclearxy(0, 11, SIZE_X); 
                        gotoxy(0,11);
                        printf("+--------------------------------------+\n");
                        printf("! Not implemented yet.                 !\n");
                        printf("+--------------------------------------+\n");
                        printf("(Press a key)\n");
                        pause_char = cgetc();
                        break;
                    case ASCII_L:
                        //createNewBoard();
                        state = STATE_LOCAL_GAME;
                        break;
                    case ASCII_Q:
                        state = STATE_QUIT;
                        break;
                    default:
                        popup("Not implemented yet.");
                        //cclearxy(0, 11, SIZE_X); 
                        //gotoxy(0,11);
                        //printf("+--------------------------------------+\n");
                        //printf("! Not implemented yet.                 !\n");
                        //printf("+--------------------------------------+\n");
                        //printf("(Press a key)\n");
                        //pause_char = cgetc();
                }
                break;
            default:
                break;
        }
    }
}

void spinner(void) {
    // From https://github.com/bobbimanners/emailler
    static char chars[] = "|/-\\";
    //static uint8_t i = 0;
    static char i = 0;
    //putchar(BACKSPACE);
    //putchar(chars[(i++) % 4]);
    gotoxy(10+4+2,23);
    cputc(chars[(i++) % 4]);
}


void awaitMoveResponse(void) {
    int delay_count = 0;
    int timeout_count = 0;
    unsigned char read_char;
    int received_esp_response = 0;
    gotoxy(10+4+2,23);
    spinner();

    while (received_esp_response==0 && timeout_count < ESP_TIMEOUT) {
        timeout_count++;
        read_char = read_byte(ESP_COMMAND_ADDRESS);
            switch(read_char) {
                case STATUS_IN_PROGRESS:
                    received_esp_response = 1;
                    break;
                case STATUS_BLACK_WON:
                    received_esp_response = 1;
                    break;
                case STATUS_WHITE_WON:
                    received_esp_response = 1;
                    break;
                case STATUS_STALEMATE:
                    received_esp_response = 1;
                    break;
                case STATUS_FIFTY_RULE_MOVE:
                    received_esp_response = 1;
                    break;
                case STATUS_UNKNOWN:
                    received_esp_response = 1;
                    break;
                case STATUS_UNHANDLED:
                    received_esp_response = 1;
                    break;
                case STATUS_ERROR:
                    received_esp_response = 1;
                    break;
                case CHESS_INVALID_MOVE:
                    received_esp_response = 1;
                    break;
                default:
                    break;
            }
        // delay
        for (i=0; delay_count < 5555; ++delay_count) {
            spinner();
        }
    }
    if (timeout_count == ESP_TIMEOUT) {
        popup("ESP TIMEOUT");
        return;
    }

    //getBoard();
    switch(read_char) {
        case STATUS_IN_PROGRESS:
            popup("STATUS_IN_PROGRESS");
            read_string_from_ram(RAM_DATA_START_ADDRESS, ai_move, sizeof ai_move-1);
            popup(sprintf("AI Move: %s", ai_move));
            address_offset = 5;
            for (i=0; i<9; i++) {
                read_string_from_ram(RAM_DATA_START_ADDRESS + address_offset, game_board[0], sizeof game_board[0]-1);
                address_offset += strlen(game_board[0]) + 1;
                read_string_from_ram(RAM_DATA_START_ADDRESS + address_offset, game_board[1], sizeof game_board[1]-1);
                address_offset += strlen(game_board[1]) + 1;
                read_string_from_ram(RAM_DATA_START_ADDRESS + address_offset, game_board[2], sizeof game_board[2]-1);
                address_offset += strlen(game_board[2]) + 1;
                read_string_from_ram(RAM_DATA_START_ADDRESS + address_offset, game_board[3], sizeof game_board[3]-1);
                address_offset += strlen(game_board[3]) + 1;
            }
            break;
        case STATUS_BLACK_WON:
            popup("You lose.");
            break;
        case STATUS_WHITE_WON:
            popup("You win.");
            break;
        case STATUS_STALEMATE:
            popup("Stalemate");
            break;
        case STATUS_FIFTY_RULE_MOVE:
            popup("Fifty move rule.");
            break;
        case STATUS_UNKNOWN:
            popup("STATUS_UNKNOWN");
            break;
        case STATUS_UNHANDLED:
            popup("STATUS_UNHANDLED");
            break;
        case STATUS_ERROR:
            popup("STATUS_ERROR");
            break;
        case CHESS_INVALID_MOVE:
            popup("CHESS_INVALID_MOVE");
            break;
        default:
            popup("Unrecognised response from ESP.");
            break;
    }
}

void displayCurrentBoard(void) {
    unsigned char i;
    for (i=0; i<10; i++) {
        cclearxy(0, i+7, SIZE_X);	
        gotoxy(10,i+7);
        printf("%s", game_board[i]); 
    }
}

void createNewBoard(void) {

	cclearxy(0, 7, SIZE_X);	
    gotoxy(10,7);
	printf("8: r n b q k b n r"); 

	cclearxy(0, 8, SIZE_X);	
    gotoxy(10,8);
	printf("7: p p p p p p p p"); 

    gotoxy(10,9);
	printf("6: . . . . . . . ."); 
    gotoxy(10,10);
	printf("5: . . . . . . . ."); 
    gotoxy(10,11);
	printf("4: . . . . . . . ."); 
    gotoxy(10,12);
	printf("3: . . . . . . . ."); 
    gotoxy(10,13);
	printf("2: P P P P P P P P"); 
    gotoxy(10,14);
	printf("1: R N B Q K B N R"); 
    gotoxy(10,15);
	printf("   ---------------"); 
    gotoxy(10,16);
	printf("   a b c d e f g h"); 
}


/*
    while (key != ASCII_5) {    // Last menu item should be Quit
        drawMenuBar();
        gotoxy(0,SIZE_Y-3);
        //printf("cgetc() -> \"%c\"", key);

        switch((int)key) {
        case ASCII_0:
            break;
        case ASCII_5:
            break;
        case ASCII_1:
            clrscr();
            drawMenuBar();
            gotoxy(0,SIZE_Y-1);
            printf("Enter Country Code:");
            fgets(country_code, sizeof country_code, stdin);
            strncpy(country_code, country_code, strlen(country_code));
            //country_code[strcspn(country_code, "\n")] = 0;
            write_string_to_ram(RAM_DATA_START_ADDRESS, country_code);
            write_byte(APPLE_COMMAND_ADDRESS, COMMAND_SET_COUNTRY);
            //printf("C:[%s]", country_code);
            //pause_char = cgetc();
            break;
        case ASCII_2:
            clrscr();
            drawMenuBar();
            gotoxy(0,SIZE_Y-1);
            printf("Enter City: ");
            fgets(city, sizeof city, stdin);
            city[strcspn(city, "\n")] = 0;
            write_string_to_ram(RAM_DATA_START_ADDRESS, city);
            write_byte(APPLE_COMMAND_ADDRESS, COMMAND_SET_CITY);
            //printf("C:[%s]", city);
            //pause_char = cgetc();
            break;
        case ASCII_3:
            clrscr();
            drawMenuBar();
            write_byte(APPLE_COMMAND_ADDRESS, COMMAND_FETCH_WEATHER);
            break;
        case ASCII_4:
            clrscr();
            drawMenuBar();
            read_string_from_ram(RAM_DATA_START_ADDRESS + address_offset, temperature, sizeof temperature-1);
            address_offset += strlen(temperature) + 1;
            read_string_from_ram(RAM_DATA_START_ADDRESS + address_offset, humidity, sizeof humidity-1);
            address_offset += strlen(humidity) + 1;
            read_string_from_ram(RAM_DATA_START_ADDRESS + address_offset, wind_speed, sizeof wind_speed-1);
            address_offset += strlen(wind_speed) + 1;
            read_string_from_ram(RAM_DATA_START_ADDRESS + address_offset, wind_direction, sizeof wind_direction-1);
            address_offset += strlen(wind_direction) + 1;
            read_string_from_ram(RAM_DATA_START_ADDRESS + address_offset, weather1, sizeof weather1-1);
            address_offset += strlen(weather1) + 1;
            read_string_from_ram(RAM_DATA_START_ADDRESS + address_offset, weather2, sizeof weather2-1);
            gotoxy(0,SIZE_Y-11);
            printf("%s (%s)\n", city, country_code);
            printf("-------------------------------------\n");
            printf("Temp:       %s K\n", temperature);
            printf("Humidity:   %s%%\n", humidity);
            printf("Wind Speed: %s m/s\n", wind_speed);
            printf("Wind Dir:   %s \n", wind_direction);
            printf("Summary:    %s, %s \n", weather1, weather2);
            printf("-------------------------------------\n");
            break;
        default:
            break;
        }

    }
            */
