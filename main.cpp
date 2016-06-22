#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
extern "C" {
#include"./sdl/include/SDL.h"
#include"./sdl/include/SDL_main.h"
#include"./sdl/include/SDL_mixer.h"

}

double roundr(double fValue) 
{
	return fValue < 0 ? ceil(fValue - 0.5)
		: floor(fValue + 0.5);
}

#define SCREEN_WIDTH	700
#define SCREEN_HEIGHT	800
int czarny, zielony, niebieski, czerwony, bialy, ciemny, fioletowy, jasny, pomaranczowy, oliwkowy;

// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};
// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};
// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};
// rysowanie prostokπta o d≥ugoúci bokÛw l i k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

typedef struct {
	int length;
	int width;
	int x;
	int y;
} rect_t;



//#define TICK 0.5
//#define TPX 22
#define LINES 5
#define CHARS_IN_LINE 30
#define CHARS_IN_NAME 10
#define X0 10
#define Y0 25
int TPX; //sta≥e ktÛre ustawia program do ustawienia interfejsu
#define NX 15 //ile kratek w poziomie
#define NY 30 //ile kratek w pionie
#define N 4 //spadajaca tablica ma N x N
#define INF_W 7 // //ile szerokoúci jednego klocka ma mieÊ pasek po boku
#define LEVEL_CHANGE 30.0 //po ilu sekundach zmieni sie poziom
#define DIFICULTY 0.9 //im mniej, tym trudniej
#define STATEMENT_HEIGHT 10
#define STATEMENT_WIDTH 20
#define BASE_DIFICULTY 0.2
void DrawCircuit(SDL_Surface *screen, Uint32 color) {
	DrawLine(screen, X0 - 2, Y0 - 2, TPX*NX + 4, 1, 0, color);
	DrawLine(screen, X0 - 2, Y0 - 2, TPX*NY + 4, 0, 1, color);
	DrawLine(screen, X0 - 2, Y0 - 2 + TPX*NY + 4, TPX*NX + 4, 1, 0, color);
	DrawLine(screen, X0 - 2 + TPX*NX + 4, Y0 - 2, TPX*NY + 4, 0, 1, color);
}
void DrawNet(SDL_Surface *screen, Uint32 color) {
	for (int i = 0; i <= NX; i++) {
		DrawLine(screen, X0 + TPX*i, Y0, TPX*NY, 0, 1, color);
	}
	for (int i = 0; i <= NY; i++) {
		DrawLine(screen, X0, Y0 + TPX*i, TPX*NX, 1, 0, color);
	}
}
void DrawAllBlocks(SDL_Surface *screen, int allBlocksTab[][NY+N]) {
	for (int i = 0; i < NX; i++) {
		for (int j = 0; j < NY; j++) {
			if (allBlocksTab[i][j] != 0) {
				DrawRectangle(screen, X0 + i*TPX+1, Y0 + j*TPX+1, TPX-1, TPX-1, allBlocksTab[i][j], allBlocksTab[i][j]);
			}
		}
	}
}
void DrawFallingBlocks(SDL_Surface *screen, double yDistance, double xDistance, int x, int y, int fallTab[][N]) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (fallTab[i][j] != 0) {
				//DrawRectangle(screen, X0 + (x + i)* TPX + 1, Y0 + yDistance - TPX + j*TPX + 1, TPX - 1, TPX - 1, fallTab[i][j], fallTab[i][j]);
				DrawRectangle(screen, X0 + roundr(xDistance) + i*TPX + 1, Y0 + roundr(yDistance) + j*TPX + 1 - TPX, TPX-1, TPX-1, fallTab[i][j], fallTab[i][j]);
			}
		}
	}
}
void DrawHelpfulLine(SDL_Surface *screen, double xDistance, int fallTab[][N]) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (fallTab[i][j] != 0) {
				DrawLine(screen, X0 + roundr(xDistance) + i*TPX + 1, Y0 + NY*TPX + 5, TPX, 1, 0, bialy);
			}
		}
	}
	
}
void DrawCircuitInfo(SDL_Surface *screen, rect_t rect_info, Uint32 color) {
	DrawLine(screen, rect_info.x, rect_info.y, rect_info.width, 1, 0, color);
	DrawLine(screen, rect_info.x, rect_info.y, rect_info.length, 0, 1, color);
	DrawLine(screen, rect_info.x, rect_info.y + rect_info.length, rect_info.width, 1, 0, color);
	DrawLine(screen, rect_info.x + rect_info.width, rect_info.y, rect_info.length, 0, 1, color);
}
void DrawInfo(SDL_Surface *screen, SDL_Surface *charset, char *text, rect_t rect_info, int currentLevel, double worldTime, int prize, char **scores, double fps, int loadCounter) {
	sprintf(text, "Poziom: %d", currentLevel);
	DrawString(screen, rect_info.x + 8, rect_info.y + 15 * 1, text, charset);
	sprintf(text, "Czas gry: %.3f s", worldTime);
	DrawString(screen, rect_info.x + 8, rect_info.y + 15 * 2, text, charset);
	sprintf(text, "Wynik: %d", prize);
	DrawString(screen, rect_info.x + 8, rect_info.y + 15 * 3, text, charset);
	sprintf(text, "Najlepsze wyniki: ");
	DrawString(screen, rect_info.x + 8, rect_info.y + 15 * 4, text, charset);
	for (int i = 0; i < LINES; i++) {
		sprintf(text, "%s", scores[i]);
		DrawString(screen, rect_info.x + 8, rect_info.y + 15 * (5+i), text, charset);
	} 
	sprintf(text, "Mozliwe zapisy: %d", loadCounter);
	DrawString(screen, rect_info.x + 8, rect_info.y + 15 * (5+LINES), text, charset);
}
void DrawStatementRect(SDL_Surface *screen, rect_t rect_statement) {
	DrawRectangle(screen, rect_statement.x, rect_statement.y, rect_statement.width, rect_statement.length, bialy, ciemny);
}
void DrawStatementInfo(SDL_Surface *screen, SDL_Surface *charset, char *text, rect_t rect_statement, int prize, int stage, char *playerName) {
	sprintf(text, "Koniec gry!");
	DrawString(screen, rect_statement.x + rect_statement.width / 2 - strlen(text) * 8 / 2, rect_statement.y + 15, text, charset);
	sprintf(text, "Uzyskales wynik: %d", prize);
	DrawString(screen, rect_statement.x + rect_statement.width / 2 - strlen(text) * 8 / 2, rect_statement.y + 15 * 2, text, charset);
	sprintf(text, "Zajales miejsce: %d", stage);
	DrawString(screen, rect_statement.x + rect_statement.width / 2 - strlen(text) * 8 / 2, rect_statement.y + 15 * 3, text, charset);
	if (stage > LINES) {
		sprintf(text, "Pocwicz troche!");
		DrawString(screen, rect_statement.x + rect_statement.width / 2 - strlen(text) * 8 / 2, rect_statement.y + 15 * 4, text, charset);
	} else {
		sprintf(text, "Gratuluje, zajales miejsce: %d", stage);
		DrawString(screen, rect_statement.x + rect_statement.width / 2 - strlen(text) * 8 / 2, rect_statement.y + 15 * 4, text, charset);
		sprintf(text, "Podaj swoje imie, abym Cie dodal: %s", playerName);
		DrawString(screen, rect_statement.x + rect_statement.width / 2 - strlen(text) * 8 / 2, rect_statement.y + 15 * 5, text, charset);

	}
}
void fillFallTabWithZero(int fallTab[][N]);
void getRandomBlocks(int fallTab[][N]) {
	fillFallTabWithZero(fallTab);
	int tempRand;
	tempRand = rand() % 7;
	switch (tempRand) {
	case 0:
		fallTab[0][0] = jasny;
		fallTab[0][1] = jasny;
		fallTab[0][2] = jasny;
		fallTab[0][3] = jasny;
		break;
	case 1:
		fallTab[0][0] = oliwkowy;
		fallTab[1][0] = oliwkowy;
		fallTab[2][0] = oliwkowy;
		fallTab[1][1] = oliwkowy;
		break;
	case 2:
		fallTab[0][0] = fioletowy;
		fallTab[1][0] = fioletowy;
		fallTab[0][1] = fioletowy;
		fallTab[1][1] = fioletowy;
		break;
	case 3:
		fallTab[0][0] = zielony;
		fallTab[0][1] = zielony;
		fallTab[0][2] = zielony;
		fallTab[1][2] = zielony;
		break;
	case 4:
		fallTab[1][0] = czerwony;
		fallTab[1][1] = czerwony;
		fallTab[1][2] = czerwony;
		fallTab[0][2] = czerwony;
		break;
	case 5:
		fallTab[1][0] = niebieski;
		fallTab[2][0] = niebieski;
		fallTab[0][1] = niebieski;
		fallTab[1][1] = niebieski;
		break;
	case 6:
		fallTab[0][0] = pomaranczowy;
		fallTab[1][0] = pomaranczowy;
		fallTab[1][1] = pomaranczowy;
		fallTab[2][1] = pomaranczowy;
		break;
	}
}
void rewriteTab(int fallTab[][N], int **tempTab) {
	for (int i = 0; i<N; i++) {
		for (int j = 0; j<N; j++) {
			fallTab[i][j] = tempTab[i][j];
		}
	}
}
void fillTemporeryTabWithZero(int **tempTab) {
	for (int i = 0; i<N; i++) {
		for (int j = 0; j<N; j++) {
			tempTab[i][j] = 0;
		}
	}
}
void fillMainTabWithZero(int tab[][NY+N]) {
	for (int i = 0; i < NX+N; i++) {
		for (int j = 0; j < NY+N; j++) {
			tab[i][j] = 0;
		}
	}
}
void fillFallTabWithZero(int fallTab[][N]) {
	for (int i = 0; i<N; i++) {
		for (int j = 0; j<N; j++) {
			fallTab[i][j] = 0;
		}
	}
}
void moveTabToCorner(int fallTab[][N]) {
	int minX = N;
	int minY = N;
	for (int i = 0; i<N; i++) {
		for (int j = 0; j<N; j++) {
			if (fallTab[i][j] != 0) {
				if (i<minX) {
					minX = i;
				}
				if (j<minY) {
					minY = j;
				}
			}
		}
	}
	int **tempTab;
	tempTab = (int**)malloc(N*sizeof(int*));
	for (int i = 0; i<N; i++) {
		tempTab[i] = (int*)malloc(N*sizeof(int));
	}
	fillTemporeryTabWithZero(tempTab);
	for (int i = minX; i<N; i++) {
		for (int j = minY; j<N; j++) {
			tempTab[i - minX][j - minY] = fallTab[i][j];
		}
	}
	rewriteTab(fallTab, tempTab);
	for (int i = 0; i<N; i++) {
		free(tempTab[i]);
	}
	free(tempTab);

}
void rotateFalltab(int fallTab[][N]) {
	int **tempTab;
	tempTab = (int**)malloc(N*sizeof(int*));
	for (int i = 0; i<N; i++) {
		tempTab[i] = (int*)malloc(N*sizeof(int));
	}
	for (int i = 0; i<N; i++) {
		for (int j = 0; j<N; j++) {
			tempTab[i][j] = fallTab[j][N - 1 - i];
		}
	}
	rewriteTab(fallTab, tempTab);
	moveTabToCorner(fallTab);
	for (int i = 0; i<N; i++) {
		free(tempTab[i]);
	}
	free(tempTab);
}
bool check_collison(int fallTab[][N], int allBlocksTab[][NY+N], int x, int y) {
	if (x<0 || y<0 || x>NX || y>NY) {
		return false;
	}
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if(fallTab[i][j]!=0 && allBlocksTab[i+x][j+y]!=0){
				return false;
			}
			if (fallTab[i][j]!=0 && (i+x >= NX || j+y >= NY )) {
				return false;
			}
		}
	}
	return true;
}
void addBlocksToMainTab(int fallTab[][N], int allBlocksTab[][NY+N], int x, int y) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (fallTab[i][j] != 0) {
				allBlocksTab[i + x][j + y] = fallTab[i][j];
			}
		}
	}
}
int checkForFullLines(int allBlocksTab[][NY + N]) {
	bool fullLineFound = false;
	for (int i = 0; i < NY; i++) {
		for (int j = 0; j < NX; j++) {
			if (allBlocksTab[j][i] == 0) {
				fullLineFound = false;
				break;
			}
			if (j == NX - 1) {
				fullLineFound = true;
			}
		}
		if (fullLineFound) return i;
	}
	return -1;
}
void removeLine(int allBlocksTab[][NY + N], int n) {
	for (int i = 0; i < NX; i++) {
		allBlocksTab[i][n] = 0;
	}
	for (int i = n; i > 0; i--) {
		for (int j = 0; j < NX; j++) {
			allBlocksTab[j][i] = allBlocksTab[j][i - 1];
		}
	}
}
void stabilizeX(int *x, double *xDistance, int speedX, double TICK, double delta) {
	if ((double)((*x)*TPX) > roundr(*xDistance)) {
		*xDistance += (speedX + 0.05*((double)((*x)*TPX) - roundr(*xDistance))) * TPX / 0.3 * delta;
	}
	if ((double)((*x)*TPX) < roundr(*xDistance)) {
		*xDistance -= (speedX + 0.05*((double)(-(*x)*TPX) + roundr(*xDistance))) * TPX / 0.3 * delta;
	}
}
void stabilizeY(int *y, double *yDistance, double TICK, double delta) {
	if ((double)(((*y) + 1)*TPX) > roundr(*yDistance)) {
		*yDistance += TPX / TICK * delta;
	}
	if ((double)(((*y) + 1)*TPX) < roundr(*yDistance)) {
		*yDistance -= TPX / TICK * delta;
	}
}
void countFrames(double *fpsTimer, double *fps, int *frames) {
	if (*fpsTimer > 0.5) {
		*fps = *frames * 2;
		*frames = 0;
		*fpsTimer -= 0.5;
	};
}
void removeFullLines(int allBlocksTab[][NY + N], int line, int *prizeCounter) {
	if (line < 0) {
		return;
	} else {
		removeLine(allBlocksTab, line);
		(*prizeCounter)++;
		removeFullLines(allBlocksTab, checkForFullLines(allBlocksTab), prizeCounter);
	}
}
void setDefault(int *x, int *y, double *xDistance, double *yDistance, double *TICK, double baseTICK) {
	*x = NX / 2 - 1;
	*xDistance = (NX / 2 - 1)*TPX;
	*y = 0;
	*yDistance = 0;
	*TICK = baseTICK;
}
int setLevel(double *worldTime, double *baseTICK, bool reset) {
	static int level = 1;
	level = reset ? 1 : level;
	if (reset) *baseTICK = BASE_DIFICULTY;
	if (floor(*worldTime / LEVEL_CHANGE) >= level ) {
		*baseTICK = BASE_DIFICULTY * pow(DIFICULTY, level);
		level++;
	} 
	level = level < 10 ? level : 10;
	return level;
}
int countPrize(int *prizeCounter, int level) {
	static bool extraPrize = false;
	switch (*prizeCounter) {
	case 0:
		extraPrize = false;
		return 0;
	case 1:
		extraPrize = false;
		*prizeCounter = 0;
		return 100 * level;
	case 2:
		extraPrize = false;
		*prizeCounter = 0;
		return 200 * level;
	case 3:
		extraPrize = false;
		*prizeCounter = 0;
		return 400 * level;
	case 4:
		*prizeCounter = 0;
		if (extraPrize) {
			return 1200 * level;
		} else {
			extraPrize = true;
			return 800 * level;
		}
	default:
		return -1000;
	}
}
void getScores(char ***scores) {
	FILE *file;
	file = fopen("score.txt", "r");
	char **temp;
	temp = (char**)malloc(LINES*sizeof(char*));
	for (int i = 0; i<LINES; i++) {
		temp[i] = (char*)malloc(CHARS_IN_LINE*sizeof(char));
	}
	for (int i = 0; i<LINES; i++) {
		fgets(temp[i], CHARS_IN_LINE, file);
		for (int j = 0; j < CHARS_IN_LINE; j++) {
			if (temp[i][j] == 10) {
				temp[i][j] = 0;
			}
		}
	}
	*scores = temp;
}
void subScores(int **iSocres, char **scores) {
	int *temp;
	char *cTemp;
	temp = (int*)malloc(LINES*sizeof(int));
	cTemp = (char*)malloc(CHARS_IN_LINE*sizeof(char));
	int j = 0;
	int k = 0;
	int spaces = 0;
	for (int i = 0; i < LINES; i++) {
		for (int j = 0; j < CHARS_IN_LINE; j++) {
			if (scores[i][j] == ' ') {
				spaces++;
			}
			if (spaces == 1) {
				k = j + 1;
				while (scores[i][k] != 0) {
					cTemp[k - j - 1] = scores[i][k];
					k++;
				}
				cTemp[k - j - 1] = 0;
				break;
			}
		}
		temp[i] = strtol(cTemp, NULL, 0);
		spaces = 0;
		k = 0;
	}
	*iSocres = temp;
}
bool checkForEnd(int y) {
	if (y < N-1) {
		return true;
	} else {
		return false;
	}
}
int stage(int *iScores, int prize) {
	int i = 0;
	while (i < LINES) {
		if (iScores[i] < prize) {
			return i + 1;
		}
		i++;
	}
	return LINES + 1;
}
bool addToScores(int *iScores, int prize, char ***scores, char *playerName) {
	char **temp;
	int thatLine = stage(iScores, prize);
	if (thatLine>LINES) return false;
	temp = (char**)malloc(LINES*sizeof(char*));
	for (int i = 0; i<LINES; i++) {
		temp[i] = (char*)malloc(CHARS_IN_LINE*sizeof(char));
	}
	for (int i = 0; i<LINES; i++) {
		if (i == thatLine - 1) {
			sprintf(temp[i], "%s %d", playerName, prize);
			continue;
		}
		else if (i>thatLine - 1) {
			sprintf(temp[i], (*scores)[i - 1]);
			continue;
		}
		sprintf(temp[i], (*scores)[i]);
	}
	for (int i = 0; i<LINES; i++) {
		free((*scores)[i]);
	}
	free(*scores);
	*scores = temp;
	return true;
}
void saveScoreToFIle(char **scores) {
	FILE *savior;
	savior = fopen("score.txt", "w");
	for (int i = 0; i<LINES; i++) {
		fputs(scores[i], savior);
		if (i != LINES - 1) {
			fputs("\n", savior);
		}
	}
	fclose(savior);
}
char *newName() {
	char *temp = (char*)malloc(CHARS_IN_NAME*sizeof(char));
	for (int i = 0; i<CHARS_IN_NAME; i++) {
		temp[i] = 0;
	}
	return temp;
}

//zapis i odczyt
void saveMainTab(FILE *temp, int tab[][NY + N]) {
	for (int i = 0; i<NY + N; i++) {
		for (int j = 0; j<NX + N; j++) {
			fprintf(temp, "%d ", tab[j][i]);
		}
		fprintf(temp, "$\n");
	}
}
void saveFallTab(FILE *temp, int fallTab[][N]) {
	for (int i = 0; i<N; i++) {
		for (int j = 0; j<N; j++) {
			fprintf(temp, "%d ", fallTab[j][i]);
		}
		fprintf(temp, "$\n");
	}
}
void saveInt(FILE *temp, int x) {
	fprintf(temp, "%d\n", x);
}
void saveDouble(FILE *temp, double x) {
	fprintf(temp, "%f\n", x);
}
void saveALL(int tab[][NY + N], int fallTab[][N], int x, int y, double xDistance,
	double yDistance, double TICK, double baseTICK, double tickTimer,
	double worldTime, int currentLevel, int prizeCounter, int prize) {
	FILE *temp;
	temp = fopen("save.txt", "w");
	saveMainTab(temp, tab);
	saveFallTab(temp, fallTab);
	saveInt(temp, x);
	saveInt(temp, y);
	saveInt(temp, currentLevel);
	saveInt(temp, prizeCounter);
	saveInt(temp, prize);
	saveDouble(temp, xDistance);
	saveDouble(temp, yDistance);
	saveDouble(temp, TICK);
	saveDouble(temp, baseTICK);
	saveDouble(temp, tickTimer);
	saveDouble(temp, worldTime);
	fclose(temp);
}

void subLineFallTab(char *line, int fallTab[][N], int iLine) {
	char *temp = (char*)malloc(15 * sizeof(char));
	for (int i = 0; i<15; i++) {
		temp[i] = 0;
	}
	int record = 0;
	int iter = 0;
	int k = 0;
	while (line[iter] != '$') {
		while (line[iter] != ' ') {
			temp[k] = line[iter];
			iter++; k++;
		}
		fallTab[record][iLine] = strtol(temp, NULL, 0);
		for (int i = 0; i <= k; i++) {
			temp[i] = 0;
		}
		k = 0;
		iter++;
		record++;
	}
	free(temp);
}
void subFallTab(FILE *save, int fallTab[][N]) {
	fillFallTabWithZero(fallTab);
	char *tempLine = (char*)malloc(N * 12 * sizeof(char));
	for (int i = 0; i<N; i++) {
		fgets(tempLine, N * 12, save);
		subLineFallTab(tempLine, fallTab, i);
	}
}
void subLineMainTab(char *line, int tab[][NY + N], int iLine) {
	char *temp = (char*)malloc(15 * sizeof(char));
	for (int i = 0; i<15; i++) {
		temp[i] = 0;
	}
	int record = 0;
	int iter = 0;
	int k = 0;
	while (line[iter] != '$') {
		while (line[iter] != ' ') {
			temp[k] = line[iter];
			iter++; k++;
		}
		tab[record][iLine] = strtol(temp, NULL, 0);
		for (int i = 0; i <= k; i++) {
			temp[i] = 0;
		}
		k = 0;
		iter++;
		record++;
	}
	free(temp);
}
void subMainTab(FILE *save, int tab[][NY + N]) {
	fillMainTabWithZero(tab);
	char *tempLine = (char*)malloc(NX * 12 * sizeof(char));
	for (int i = 0; i<NY + N; i++) {
		fgets(tempLine, NX * 12, save);
		subLineMainTab(tempLine, tab, i);
	}
	free(tempLine);
}
void subInt(FILE *save, int *x) {
	char *tempLine = (char*)malloc(15 * sizeof(char));
	fgets(tempLine, 15, save);
	*x = strtol(tempLine, NULL, 0);
	free(tempLine);
}
void subDouble(FILE *save, double *x) {
	char *tempLine = (char*)malloc(15 * sizeof(char));
	fgets(tempLine, 15, save);
	sscanf(tempLine, "%lf", x);
	free(tempLine);
}
void loadALL(int tab[][NY + N], int fallTab[][N], int *x, int *y, int *currentLevel, int *prizeCounter, int *prize,
	double *xDistance, double *yDistance, double *TICK, double *baseTICK, double *tickTimer, double *worldTime) {
	FILE *temp;
	temp = fopen("save.txt", "r");
	subMainTab(temp, tab);
	subFallTab(temp, fallTab);
	subInt(temp, x);
	subInt(temp, y);
	subInt(temp, currentLevel);
	subInt(temp, prizeCounter);
	subInt(temp, prize);
	subDouble(temp, xDistance);
	subDouble(temp, yDistance);
	subDouble(temp, TICK);
	subDouble(temp, baseTICK);
	subDouble(temp, tickTimer);
	subDouble(temp, worldTime);
}
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps;
	srand(time(NULL));
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Surface *eti, *eti_2;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;
	Mix_Chunk *deploySound;

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 9192) == -1)
	{
		return false;
	}

	deploySound = Mix_LoadWAV("sounds/sound1.wav");
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Szablon do zdania drugiego 2014");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy≥πczenie widocznoúci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);
	//tu ustawiam interfejs
	//ile szerokoúci jednego klocka ma mieÊ pasek po boku
	TPX = (SCREEN_WIDTH-40) / (NX + INF_W);
	if (SCREEN_HEIGHT < 40 + NY*TPX) {
		TPX = (SCREEN_HEIGHT - 40) / NY;
	}
	char text[128];
	czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	bialy = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF);
	ciemny = SDL_MapRGB(screen->format, 0x14, 0x24, 0x1A);
	fioletowy = SDL_MapRGB(screen->format, 0x16, 0x3F, 0x60);
	jasny = SDL_MapRGB(screen->format, 0xDC, 0xD2, 0xD1);
	czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	zielony = SDL_MapRGB(screen->format, 0x42, 0xAE, 0x6B);
	niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	oliwkowy = SDL_MapRGB(screen->format, 0x19, 0xF0, 0x9B);
	pomaranczowy = SDL_MapRGB(screen->format, 0xF6, 0x9F, 0x13);
	
	t1 = SDL_GetTicks();
	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	//zmienne "mechaniczne"
	double speedX = 2;
	int allBlocksTab[NX + N][NY + N];
	int fallTab[N][N];
	
	rect_t rect_info;
	rect_info.x = X0 + TPX*NX + 10;
	rect_info.y = NY - 8;
	rect_info.width = SCREEN_WIDTH - rect_info.x - 10;
	rect_info.length = TPX * NY + 8;

	rect_t rect_statement;
	rect_statement.x = (SCREEN_WIDTH - STATEMENT_WIDTH * TPX) / 2;
	rect_statement.y = (SCREEN_HEIGHT - STATEMENT_HEIGHT * TPX) / 2;
	rect_statement.width = STATEMENT_WIDTH * TPX;
	rect_statement.length = STATEMENT_HEIGHT * TPX;


	int x, y;
	double xDistance, yDistance;
	double TICK;
	double baseTICK = BASE_DIFICULTY;
	double tickTimer = 0;
	bool preventFromMoving = false;
	bool pauseON = false;
	bool getingName = false;
	bool showStatement = false;
	int currentLevel = 1;
	int prizeCounter = 0;
	int prize = 0;
	int loadCounter = 3;

	int *iScores;
	char **scores;
	char *playerName;
	int nameIterator = 0;
	playerName = newName();

	int actualStage;

	getScores(&scores);
	subScores(&iScores, scores);

	setDefault(&x, &y, &xDistance, &yDistance, &TICK, baseTICK);
	getRandomBlocks(fallTab);
	fillMainTabWithZero(allBlocksTab);

	Mix_PlayChannel(-1, deploySound, 100);
	while(!quit) {
		SDL_FillRect(screen, NULL, czarny);
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		
		stabilizeX(&x, &xDistance, speedX, baseTICK, delta);
		stabilizeY(&y, &yDistance, TICK, delta);
		fpsTimer += delta;
		if(!pauseON) tickTimer += delta;
		if(!pauseON) worldTime += delta;
		countFrames(&fpsTimer, &fps, &frames);
		if (tickTimer > TICK) {
			if (check_collison(fallTab, allBlocksTab, x, y + 1)) {
				y++;
			} else {
				addBlocksToMainTab(fallTab, allBlocksTab, x, y);
				removeFullLines(allBlocksTab, checkForFullLines(allBlocksTab), &prizeCounter);
				prize += countPrize(&prizeCounter, currentLevel);
				preventFromMoving = false;

				if (checkForEnd(y)) {
					pauseON = true;
					preventFromMoving = true;
					showStatement = true;
					actualStage = stage(iScores, prize);
					if (actualStage <= LINES) getingName = true;
				} 
				currentLevel = setLevel(&worldTime, &baseTICK, false);
				setDefault(&x, &y, &xDistance, &yDistance, &TICK, baseTICK);
				getRandomBlocks(fallTab);
			}
			tickTimer = 0;
		}

		DrawNet(screen, ciemny);
		DrawCircuit(screen, bialy);
		DrawAllBlocks(screen, allBlocksTab);
		DrawFallingBlocks(screen, yDistance, xDistance, x, y, fallTab);
		DrawHelpfulLine(screen, xDistance, fallTab);
		DrawCircuitInfo(screen, rect_info, bialy);
		DrawInfo(screen, charset, text, rect_info, currentLevel, worldTime, prize, scores, fps, loadCounter);
		if (showStatement) {
			DrawStatementRect(screen, rect_statement);
			DrawStatementInfo(screen, charset, text, rect_statement, prize, actualStage, playerName);
		}
		

		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);

		while(SDL_PollEvent(&event)) {

			switch(event.type) {
				case SDL_KEYDOWN:
					if (getingName) {
						if (event.key.keysym.sym >= 97 && event.key.keysym.sym <= 122) { //ma≥e a i ma≥e z
							if (nameIterator < 10) {
								playerName[nameIterator] = event.key.keysym.sym - 32; //aby z malych zrobic duze
								nameIterator++;
							}
						} else if (event.key.keysym.sym == SDLK_BACKSPACE) {
							if (nameIterator > 0) {
								nameIterator--;
								playerName[nameIterator] = 0;
							}
						} else if (event.key.keysym.sym == SDLK_RETURN) {
							pauseON = false;
							preventFromMoving = false;
							showStatement = false;
							getingName = false;
							if (addToScores(iScores, prize, &scores, playerName)) saveScoreToFIle(scores);

							subScores(&iScores, scores);
							free(playerName);
							playerName = newName();
							nameIterator = 0;

							worldTime = 0;
							prize = 0;
							loadCounter = 3;

							currentLevel = setLevel(&worldTime, &baseTICK, true);
							setDefault(&x, &y, &xDistance, &yDistance, &TICK, baseTICK);
							fillMainTabWithZero(allBlocksTab);
							getRandomBlocks(fallTab);
							
						}
					} else {
						if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
						else if (event.key.keysym.sym == SDLK_UP) {}
						else if (event.key.keysym.sym == SDLK_DOWN) {
							if (!preventFromMoving) {
								TICK = 0.02;
								preventFromMoving = true;
							}
						}
						else if (event.key.keysym.sym == SDLK_RIGHT) {
							if (check_collison(fallTab, allBlocksTab, x + 1, y) && !preventFromMoving) {
								x++; 
							}
						}
						else if (event.key.keysym.sym == SDLK_LEFT) {
							if (check_collison(fallTab, allBlocksTab, x - 1, y) && !preventFromMoving) {
								x--; 
							}
						}
						else if (event.key.keysym.sym == SDLK_SPACE) {
							if (!preventFromMoving) {
								rotateFalltab(fallTab);
								if (!check_collison(fallTab, allBlocksTab, x, y)) {
									for (int i = 0; i < 3; i++) {
										rotateFalltab(fallTab);
									}
								}
							}
						}
						else if (event.key.keysym.sym == SDLK_s) {
							if (loadCounter > 0) {
								loadCounter--;
								saveALL(allBlocksTab, fallTab, x, y, xDistance, yDistance, TICK, baseTICK, tickTimer,
									worldTime, currentLevel, prizeCounter, prize);
							}
						}
						else if (event.key.keysym.sym == SDLK_l) {
							loadALL(allBlocksTab, fallTab, &x, &y, &currentLevel, &prizeCounter, &prize,
								&xDistance, &yDistance, &TICK, &baseTICK, &tickTimer, &worldTime);
						}
						else if (event.key.keysym.sym == SDLK_RETURN) {
							if (showStatement) {

								pauseON = false;
								preventFromMoving = false;
								showStatement = false;

								loadCounter = 3;
								currentLevel = 1;
								
								worldTime = 0;
								prize = 0;

								currentLevel = setLevel(&worldTime, &baseTICK, true);
								setDefault(&x, &y, &xDistance, &yDistance, &TICK, baseTICK);
								fillMainTabWithZero(allBlocksTab);
								getRandomBlocks(fallTab);
								
							}
						}
						else if (event.key.keysym.sym == SDLK_p) {
							pauseON = !pauseON;
							preventFromMoving = !preventFromMoving;
						}
					}
					break;
				case SDL_KEYUP:
					if (event.key.keysym.sym == SDLK_DOWN) {
						
					}
					break;
				case SDLK_ESCAPE:
					quit = 1;
					break;
				};
			};
		frames++;
		};

	// zwolnienie powierzchni
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	Mix_FreeChunk(deploySound);
	Mix_CloseAudio();

	SDL_Quit();
	return 0;
	};
