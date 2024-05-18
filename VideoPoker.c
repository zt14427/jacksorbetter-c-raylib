#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#define DEFAULT_RESOLUTION_X 640
#define DEFAULT_RESOLUTION_Y 480
#define TARGET_FPS 60
#define BETM_ROYALFLUSH 250
#define BETM_STRAIGHTFLUSH 50
#define BETM_4OFAKIND 25
#define BETM_FULLHOUSE 9
#define BETM_FLUSH 6
#define BETM_STRAIGHT 4
#define BETM_3OFAKIND 3
#define BETM_2P 2
#define	BETM_JACKS 1
#define FRAME_DELAY 10

typedef struct {
	char suit;
	int value;
	int redraw;
	int facedown;
} Card;

void PlayJacksOrBetter();
void BuildDeck(Card* deck);
void Shuffle(Card* deck);
int ButtonCheck(int mx, int my, int x0, int y0, int x1, int y1);
int WinCheck(Card* hand, int bet);
int _royalFlushCheck(Card hand[5], int values[5]);
int _flushCheck(Card hand[5]);
int _straightCheck(int values[5]);
int _nOfAKindCheck(int values[5], int n);
int _fullHouseCheck(int values[5]);
int _twoPairCheck(int values[5]);
int _jacksOrBetterCheck(int values[5]);
int compare_ints(const void* a, const void* b);
void DrawTextWithShadow(const char* text, int x, int y, int fontSize, Color color, Color shadowColor, int shadowOffset);
char* strcpy(char* destination, const char* source);
void __debugPrintHand(Card hand[], int state);
void __CheatHand(Card* hand);

int main() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(DEFAULT_RESOLUTION_X, DEFAULT_RESOLUTION_Y, "Casino GOLD");
	SetTargetFPS(TARGET_FPS);
	PlayJacksOrBetter();
}


void BuildDeck(Card* deck) {
	char suits[4] = { 's', 'd', 'h', 'c' };
	for (int i = 0; i < 4; i++) {
		for (int j = 1; j < 14; j++) {
			Card card = {.suit=suits[i], .value=j, .redraw=1, .facedown=1};
			deck[(i * 13) + j - 1] = card;
		}
	}
}

void Shuffle(Card* deck) {
	int n = 52;
	if (n > 1) {
		int i;
		for (i = 0; i < n - 1; i++) {
			int j = i + rand() / (RAND_MAX / (n - i) + 1);
			Card t = deck[j];
			deck[j] = deck[i];
			deck[i] = t;
		}
	}
}

void PlayJacksOrBetter() {
	Color blue = (Color){0,0,255,255};
	Color yellow = (Color){255,255,0,255};
	Color gray = (Color){128,128,128,255};
	Color black = (Color){0,0,0,255};
	Color white = (Color){255,255,255,255};
	
	InitAudioDevice();
	Sound betSound = LoadSound("resources/betSound.ogg");
	Sound buttonSound = LoadSound("resources/buttonSound.ogg");
	Sound winSound = LoadSound("resources/winSound.ogg");
	Image cardBackImage = LoadImage("resources/cardBack.png");
	Image cardFaceImage = LoadImage("resources/cardFace.png");
	Image holdImage = LoadImage("resources/holdButton.png");
	Image holdLitImage = LoadImage("resources/holdLitButton.png");
	Image betImage = LoadImage("resources/betButton.png");
	Image maxBetImage = LoadImage("resources/maxBetButton.png");
	Image drawImage = LoadImage("resources/drawButton.png");
	Image spadeImage = LoadImage("resources/spadeSuit.png");
	Image diamondImage = LoadImage("resources/diamondSuit.png");
	Image heartImage = LoadImage("resources/heartSuit.png");
	Image clubImage = LoadImage("resources/clubSuit.png");

	Card joker = { 'j', 0, 0, 0 };

	Card deck[52];
	Card hand[5] = { joker ,joker ,joker ,joker ,joker };

	BuildDeck(deck);
	Shuffle(deck);
	int drawPtr = 0;
	for (int i = 0; i < 5; i++) {
		hand[i] = deck[drawPtr++];
	}

	int x = GetScreenWidth();
	int y = GetScreenHeight();
	float xMargin = (float)0.1;
	float xMarginInv = (float)(1 - xMargin);
	float yMargin = (float)0.2;
	float yMarginInv = (float)(1 - yMargin);
	int cardx = (int)(x * xMarginInv / 5);
	int cardy = (int)(y * yMarginInv / 2);
	int buttonx = (int)(x * xMarginInv / 5 / 2);
	int buttony = (int)(y * yMargin);
	int suitx = (int)(x * xMarginInv / 5 / 2);
	int suity = (int)(y * yMargin);

	ImageResize(&cardBackImage, cardx, cardy);
	ImageResize(&cardFaceImage, cardx, cardy);
	ImageResize(&holdImage, buttonx, buttony);
	ImageResize(&holdLitImage, buttonx, buttony);
	ImageResize(&betImage, buttonx, buttony);
	ImageResize(&maxBetImage, buttonx, buttony);
	ImageResize(&drawImage, buttonx, buttony);
	ImageResize(&spadeImage, suitx, suity);
	ImageResize(&diamondImage, suitx, suity);
	ImageResize(&heartImage, suitx, suity);
	ImageResize(&clubImage, suitx, suity);

	Texture2D backTexture = LoadTextureFromImage(cardBackImage);
	Texture2D faceTexture = LoadTextureFromImage(cardFaceImage);
	Texture2D holdTexture = LoadTextureFromImage(holdImage);
	Texture2D holdLitTexture = LoadTextureFromImage(holdLitImage);
	Texture2D betTexture = LoadTextureFromImage(betImage);
	Texture2D maxBetTexture = LoadTextureFromImage(maxBetImage);
	Texture2D drawTexture = LoadTextureFromImage(drawImage);
	Texture2D spadeTexture = LoadTextureFromImage(spadeImage);
	Texture2D diamondTexture = LoadTextureFromImage(diamondImage);
	Texture2D heartTexture = LoadTextureFromImage(heartImage);
	Texture2D clubTexture = LoadTextureFromImage(clubImage);
	Texture2D cardSuitTexture;

	int playing = 1;
	int bet = 1;
	int state = 0;
	int holdButtonAnchor[5][2] = { {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1}, {-1,-1} };
	int betButtonAnchor[2] = { -1,-1 };
	int maxBetButtonAnchor[2] = { -1,-1 };
	int drawButtonAnchor[2] = { -1,-1 };
	int nx = -1;
	int ny = -1;
	const Color tint = { 255,255,255,255 };
	int cardxP = -1;
	int cardyP = -1;
	int mousex = -1;
	int mousey = -1;
	int drawCounter = 0;
	int frame = 0;
	int credits = 199;

	const char titleText[] = "JACKS OR BETTER";
	float titleFontSize = (float)(y / 24);
	int titleFontSizeInt = (int)titleFontSize;
	int titleTextWidth = MeasureText(titleText, titleFontSizeInt);
	int titleX = (int)(x / 2 - titleTextWidth / 2);
	int titleY = 0;
	float fontSize = (float)(y / 36);
	int fontSizeInt = (int)fontSize;
	int cardFontSize = (int)(y / 12);
	int cardFontSizeInt = (int)cardFontSize;
	char* tempChar2 = malloc(3);
	char cardTextStr[] = "10";
	int cardTextWidth = MeasureText(cardTextStr, cardFontSize);
	int winnings = 0;
	int wincheck = 0;
	int betReset = 1;
	int drawWinFlag = 0;
	int scoreTableX = 0;
	int scoreTableY = 0;
	int scoreTableW = 0;
	int scoreTableH = 0;
	int scoreCoords[7][9][2] = {0};
	int scoreCol = 0;
	int scoreRow = 0;
	char betName[9][16] = { "Royal Flush", "Straight Flush", "Four of a Kind", "Full House", "Flush", "Straight", "Three of a Kind", "Two Pair", "Jacks or Better" };
	int betLookup[9] = { BETM_ROYALFLUSH, BETM_STRAIGHTFLUSH, BETM_4OFAKIND, BETM_FULLHOUSE, BETM_FLUSH, BETM_STRAIGHT, BETM_3OFAKIND, BETM_2P, BETM_JACKS };

	Color cardColor;

	while (playing && !WindowShouldClose()) {
		nx = GetScreenWidth();
		ny = GetScreenHeight();
		if (x != nx || y != ny) {
			x = nx;
			y = ny;
			cardBackImage = LoadImage("resources/cardBack.png");
			cardFaceImage = LoadImage("resources/cardFace.png");
			holdImage = LoadImage("resources/holdButton.png");
			holdLitImage = LoadImage("resources/holdLitButton.png");
			betImage = LoadImage("resources/betButton.png");
			maxBetImage = LoadImage("resources/maxBetButton.png");
			drawImage = LoadImage("resources/drawButton.png");
			spadeImage = LoadImage("resources/spadeSuit.png");
			diamondImage = LoadImage("resources/diamondSuit.png");
			heartImage = LoadImage("resources/heartSuit.png");
			clubImage = LoadImage("resources/clubSuit.png");

			cardx = (int)(x * xMarginInv / 5);
			cardy = (int)(y * yMarginInv / 2);
			buttonx = (int)(x * xMarginInv / 5 / 2);
			buttony = (int)(y * yMargin);
			suitx = (int)(x * xMarginInv / 5 / 2);
			suity = (int)(y * yMargin);

			ImageResize(&cardBackImage, cardx, cardy);
			ImageResize(&cardFaceImage, cardx, cardy);
			ImageResize(&holdImage, buttonx, buttony);
			ImageResize(&holdLitImage, buttonx, buttony);
			ImageResize(&betImage, buttonx, buttony);
			ImageResize(&maxBetImage, buttonx, buttony);
			ImageResize(&drawImage, buttonx, buttony);
			ImageResize(&drawImage, buttonx, buttony);
			ImageResize(&spadeImage, suitx, suity);
			ImageResize(&diamondImage, suitx, suity);
			ImageResize(&heartImage, suitx, suity);
			ImageResize(&clubImage, suitx, suity);

			backTexture = LoadTextureFromImage(cardBackImage);
			faceTexture = LoadTextureFromImage(cardFaceImage);
			holdTexture = LoadTextureFromImage(holdImage);
			holdLitTexture = LoadTextureFromImage(holdLitImage);
			betTexture = LoadTextureFromImage(betImage);
			maxBetTexture = LoadTextureFromImage(maxBetImage);
			drawTexture = LoadTextureFromImage(drawImage);
			spadeTexture = LoadTextureFromImage(spadeImage);
			diamondTexture = LoadTextureFromImage(diamondImage);
			heartTexture = LoadTextureFromImage(heartImage);
			clubTexture = LoadTextureFromImage(clubImage);

			titleX = (int)(x / 2 - titleTextWidth / 2);
			titleY = 0;
			titleFontSize = (float)(y / 24);
			titleFontSizeInt = (int)titleFontSize;
			fontSize = (float)(y / 36);
			fontSizeInt = (int)fontSize;
			cardFontSize = (int)(y / 12);
			cardFontSizeInt = (int)cardFontSize;
			cardTextWidth = MeasureText("10", cardFontSize);
		}

		if (drawCounter) {
			if (drawCounter < 6 && state == 0) {
				drawWinFlag = 0;
				if (frame < FRAME_DELAY) {
					frame++;
				} else {
					PlaySound(buttonSound);
					hand[drawCounter - 1].facedown = 0;
					frame = 0;
					drawCounter++;
				}
			}
			if (drawCounter > 6 && drawCounter < 12 && state == 1) {
				drawWinFlag = 0;
				if (frame < FRAME_DELAY) {
					frame++;
				} else {
					if (hand[drawCounter - 7].redraw) {
						PlaySound(buttonSound);
						hand[drawCounter - 7] = deck[++drawPtr];
						hand[drawCounter - 7].facedown = 0;
					}
					frame = 0;
					drawCounter++;
				}
			}
			if (drawCounter < 6 && state == 3) {
				drawWinFlag = 0;
				if (frame < FRAME_DELAY) {
					frame++;
				} else {
					if (drawCounter == 1) {
						Shuffle(deck);
						drawPtr = 0;
					}
					if (hand[drawCounter - 1].redraw) {
						PlaySound(buttonSound);
						hand[drawCounter - 1] = deck[++drawPtr];
						hand[drawCounter - 1].facedown = 0;
					}
					frame = 0;

					drawCounter++;
				}
			}
			if (drawCounter == 6 || drawCounter == 18) {
				state = 1;
			}
			if (drawCounter == 12) {
				state = 2;
			}
		}

		BeginDrawing();
			ClearBackground(blue);
			float UIbufferX=(float)0.01;
			float UIbufferY=(float)0.05;
			//draw "UI"
			Vector2 creditDimensions = MeasureTextEx(GetFontDefault(), TextFormat("CREDITS: %04i", credits), fontSize, 0);
			DrawRectangle((int)(x*UIbufferX), (int)(y*UIbufferY), (int)(x*(1-2*UIbufferX)), (int)(y*.3), yellow);
			scoreTableX = (int)(x * UIbufferX) * 2;
			scoreTableY = (int)(y * UIbufferY) + (int)(x * UIbufferX);
			scoreTableW = x - ((int)(x * UIbufferX) * 4);
			scoreTableH = (int)(y * .3) - (int)(x * UIbufferX * 2);
			DrawRectangle(scoreTableX, scoreTableY, scoreTableW, scoreTableH, gray);
			DrawTextWithShadow(titleText, titleX, titleY, titleFontSizeInt, white, black, 2);
			scoreCol = (int)((float)scoreTableW/7);
			scoreRow = (int)((float)scoreTableH/9);
			for (int c = 0; c < 7; c++) {
				for (int r = 0; r < 9; r++) {
					scoreCoords[c][r][0] = scoreCol * c + scoreTableX;
					scoreCoords[c][r][1] = scoreRow * r + scoreTableY;
				}
			}
			DrawText(TextFormat("CREDIT: %04i", credits), scoreCoords[0][0][0], scoreCoords[0][0][1], fontSizeInt, white);
			DrawText(TextFormat("BET: %i", bet), scoreCoords[0][1][0], scoreCoords[0][1][1] + (int)creditDimensions.y, fontSizeInt, white);

			for (int r = 0; r < 9; r++) {
				DrawText(betName[r], scoreCoords[1][r][0], scoreCoords[1][r][1], fontSizeInt, white);
				for (int c = 0; c < 5; c++) {
					DrawText(TextFormat("%i", betLookup[r] * (c + 1)), scoreCoords[c + 2][r][0], scoreCoords[c + 2][r][1], fontSizeInt, white);
				}
			}

			__CheatHand(hand);

			// draw cards
			for (int i = 0; i < 5; i++) {
				holdButtonAnchor[i][0] = (int)((i + 2) * ((float)x / 9) + (x * xMargin / 9 / 2));
				holdButtonAnchor[i][1] = (int)(y - y * yMargin);
				cardxP = (int)(i * ((float)x / 5) + ((float)x * (float)xMargin / 5 / 2));
				cardyP = (int)(y * (float)yMarginInv / 2);
					
				if (hand[i].facedown) {
					DrawTexture(backTexture, cardxP, cardyP, tint);
				} else {
					switch(hand[i].suit) {
						case 's':
							cardColor = black;
							cardSuitTexture = spadeTexture;
							break;
						case 'd':
							cardColor = RED;
							cardSuitTexture = diamondTexture;
							break;
						case 'h':
							cardColor = RED;
							cardSuitTexture = heartTexture;
							break;
						case 'c':
							cardColor = black;
							cardSuitTexture = clubTexture;
							break;
					}
					DrawTexture(faceTexture, cardxP, cardyP, tint);
					switch (hand[i].value) {
						case 1:
							strcpy(cardTextStr, "A");
							break;
						case 11:
							strcpy(cardTextStr, "J");
							break;
						case 12:
							strcpy(cardTextStr, "Q");
							break;
						case 13:
							strcpy(cardTextStr, "K");
							break;
						default:
							snprintf(tempChar2, 3, "%d", hand[i].value);
							strcpy(cardTextStr, tempChar2);
					}
					DrawTexture(cardSuitTexture, cardxP+cardTextWidth, cardyP, tint);
					DrawText(cardTextStr, cardxP + cardx / 10, cardyP + cardy / 20, cardFontSize, cardColor);
				}
				if (hand[i].redraw) {
					DrawTexture(holdTexture, holdButtonAnchor[i][0], holdButtonAnchor[i][1], tint);
				} else {
					DrawTexture(holdLitTexture, holdButtonAnchor[i][0], holdButtonAnchor[i][1], tint);
				}
			}


			betButtonAnchor[0] = (int)(1 * (float)x / 9 + (x * xMargin / 9 / 2));
			betButtonAnchor[1] = (int)(y - y * yMargin);
			maxBetButtonAnchor[0] = (int)(7 * (float)x / 9 + (x * xMargin / 9 / 2));
			maxBetButtonAnchor[1] = (int)(y - y * yMargin);
			drawButtonAnchor[0] = (int)(8 * (float)x / 9 + (x * xMargin / 9 / 2));
			drawButtonAnchor[1] = (int)(y - y * yMargin);

			DrawTexture(betTexture, betButtonAnchor[0], betButtonAnchor[1], tint);
			DrawTexture(maxBetTexture, maxBetButtonAnchor[0], maxBetButtonAnchor[1], tint);
			DrawTexture(drawTexture, drawButtonAnchor[0], drawButtonAnchor[1], tint);

			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
				__debugPrintHand(hand, state);
			}

			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (drawCounter == 0 || drawCounter == 6 || drawCounter == 12)) {
				mousex = GetMouseX();
				mousey = GetMouseY();
				//Bet Button Check
				if (state == 0 || state == 3) {
					if (ButtonCheck(mousex, mousey, betButtonAnchor[0], betButtonAnchor[1], betButtonAnchor[0] + buttonx, betButtonAnchor[1] + buttony)) {
						if (state == 3 && betReset) {
							betReset = 0;
							if (bet != 1){
								bet = 0;
							}
						}
						if (bet < 5) {
							PlaySound(betSound);
							credits--;
							bet++;
							drawWinFlag = 0;
						}
					}
					// Max Bet button check
					if (ButtonCheck(mousex, mousey, maxBetButtonAnchor[0], maxBetButtonAnchor[1], maxBetButtonAnchor[0] + buttonx, maxBetButtonAnchor[1] + buttony)) {
						if (bet < 5) {
							PlaySound(betSound);
							bet = 5;
							drawWinFlag = 0;
						}
					}
				}
				// Draw button check
				if (ButtonCheck(mousex, mousey, drawButtonAnchor[0], drawButtonAnchor[1], drawButtonAnchor[0] + buttonx, drawButtonAnchor[1] + buttony)) {
						drawCounter++;
				}
				if (state == 1) {
					// Hold button checks
					for (int i = 0; i < 5; i++) {
						if (ButtonCheck(mousex, mousey, holdButtonAnchor[i][0], holdButtonAnchor[i][1], holdButtonAnchor[i][0] + buttonx, holdButtonAnchor[i][1] + buttony)) {
							PlaySound(buttonSound);
							hand[i].redraw = !hand[i].redraw;
						}
					}
				}
			}

			if (drawWinFlag) {
				DrawRectangle((int)((float)x/10), (int)(y* yMarginInv / 2 * 1.25), (int)((float)x / 10 * 8), (int)(cardy/5*3), blue);
				if (winnings){
					DrawTextWithShadow("WIN", (int)((float)x/2 - (float)(MeasureText("WIN", fontSizeInt))/2), cardyP + (int)((float)cardy / 2), fontSizeInt, white, black, 2);
				} else {
					DrawTextWithShadow("LOSE", (int)((float)x / 2 - (float)(MeasureText("WIN", fontSizeInt)) / 2), cardyP+(int)((float)cardy/2), fontSizeInt, white, black, 2);
				}
			}


		EndDrawing();

		if(!wincheck){
			winnings = WinCheck(hand, bet);
			wincheck = 1;
		}

		if (state == 2) {
			winnings = WinCheck(hand, bet);
			if (winnings) {
				PlaySound(winSound);
			}
			for (int i = 0; i < 5; i++) {
				hand[i].redraw = 1;
			}
			credits += winnings;
			drawCounter = 0;
			betReset = 1;
			drawWinFlag = 1;
			state = 3;
		}
	}
	free(tempChar2);
	UnloadSound(betSound);
	UnloadSound(buttonSound);
	UnloadSound(winSound);
	CloseAudioDevice();
}

int ButtonCheck(int mx, int my, int x0, int y0, int x1, int y1) {
	if (mx > x0 && mx < x1) {
		if (my > y0 && my < y1) {
			return 1;
		}
	}
	return 0;
}


int WinCheck(Card* hand, int bet) {
	int values[5] = {0,0,0,0,0};
	for (int i = 0; i < 5; i++) {
		values[i] = hand[i].value;
	}
	//Royal Flush
	if(_royalFlushCheck(hand, values)) {
		return BETM_ROYALFLUSH*bet;
	}
	// Straight Flush
	if(_flushCheck(hand) && _straightCheck(values)) {
		return BETM_STRAIGHTFLUSH*bet;
	}
	//Four of a Kind
	if(_nOfAKindCheck(values, 4)) {
		return BETM_4OFAKIND*bet;
	}
	//Full House
	if(_fullHouseCheck(values)) {
		return BETM_FULLHOUSE*bet;
	}
	//Flush
	if(_flushCheck(hand)) {
		return BETM_FLUSH*bet;
	}
	//Straight
	if(_straightCheck(values)) {
		return BETM_STRAIGHT*bet;
	}
	//Three of a kind
	if(_nOfAKindCheck(values, 3)) {
		return BETM_3OFAKIND*bet;
	}
	//Two Pair
	if(_twoPairCheck(values)) {
		return BETM_2P*bet;
	}
	//Jacks or Better
	if(_jacksOrBetterCheck(values)) {
		return BETM_JACKS*bet;
	}	
	return 0;
}

int _royalFlushCheck(Card hand[], int values[5]) {
	int logicCheck=0;
	if (_flushCheck(hand) && _straightCheck(values)) {
		for (int i = 0; i < 5; i++) {
			if (hand[i].value == 10) {
				logicCheck += 1;
			}
			if (hand[i].value == 11) {
				logicCheck += 2;
			}
			if (hand[i].value == 12) {
				logicCheck += 4;
			}
			if (hand[i].value == 13) {
				logicCheck += 8;
			}
			if (hand[i].value == 1) {
				logicCheck += 16;
			}
		}
		if (logicCheck==31) {
			return 1;
		}
	}
	return 0;
}

int _flushCheck(Card hand[]) {
	if (hand[0].suit == hand[1].suit && hand[1].suit == hand[2].suit && hand[2].suit == hand[3].suit && hand[3].suit == hand[4].suit) {
		return 1;
	}
	return 0;
}

int _straightCheck(int values[5]) {
	if (values[0]+1 == values[1] && values[1]+1 == values[2] && values[2]+1 == values[3] && values[3]+1 == values[4]) {
		return 1;
	}
	// Count Ace-High
	if (values[0] == 1) {
		values[0] = 13;
		qsort(values, 5, sizeof(int), compare_ints);
		if (values[0]+1 == values[1] && values[1]+1 == values[2] && values[2]+1 == values[3] && values[3]+1 == values[4]) {
			values[0] = 1;
			qsort(values, 5, sizeof(int), compare_ints);
			return 1;
		}
	}
	return 0;
}

int _nOfAKindCheck(int values[5], int n) {
	int last = -1;
	int counter = 0;
	for (int i = 0; i < 5; i++) {
		if (counter == n) {
			return 1;
		}
		if (values[i] == last) {
			counter++;
		} else {
			counter = 0;
		}
		last = values[i];
	}
	return 0;
}

int _fullHouseCheck(int values[5]) {
	if(
		// 2 + 3
		values[0] == values[1] && values[2] == values[3] && values[3] == values[4]
	  ||
	    // 3 + 2
		values[0] == values[1] && values[1] == values[2] && values[3] == values[4]
	  ) {
		return 1;
	}
	return 0;
}

int _twoPairCheck(int values[5]) {
	if(
	  values[0] == values[1] && values[2] == values[3]
	  ||
	  values[0] == values[1] && values[3] == values[4]
	  ||
	  values[1] == values[2] && values[3] == values[4]
	  ) {
		  return 1;
	  }
	  return 0;
}

int  _jacksOrBetterCheck(int values[5]) {
	int counterJ = 0;
	int counterQ = 0;
	int counterK = 0;
	int counterA = 0;
	for (int i = 0; i < 5; i++) {
		switch (values[i]) {
			case 11:
				counterJ++;
				break;
			case 12:
				counterQ++;
				break;
			case 13:
				counterK++;
				break;
			case 1:
				counterA++;
				break;
		}
			
	}
	if(counterJ>1 || counterQ>1 || counterK>1 || counterA>1) {
		return 1;
	}
	return 0;
}

int compare_ints(const void* a, const void* b) {
    int arg1 = *(const int*)a;
    int arg2 = *(const int*)b;
 
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

void DrawTextWithShadow(const char* text, int x, int y, int fontSize, Color color, Color shadowColor, int shadowOffset) {
	DrawText(text, x+shadowOffset, y+shadowOffset, fontSize, shadowColor);
	DrawText(text, x, y, fontSize, color);
}

void __debugPrintHand(Card hand[], int state) {
	printf("\n\n\n\n\n\n\n\nSTATE:%i\n\n", state);
	for (int i = 0; i < 5; i++) {
		printf("\n\nCardNo: %i\nCardVal: %i", i, hand[i].value);
	}
}

void __CheatHand(Card* hand) {
	hand[0].value = 11;
	hand[1].value = 11;
	hand[2].value = 3;
	hand[3].value = 2;
	hand[4].value = 1;
}