#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <sys/ioctl.h>
struct winsize w;

// Screen width and height
int WIDTH = 0;
int HEIGHT = 0;

// The framerate (in miliseconds)
#define FRAMERATE 100

// The playground
char **playground;

enum Direction{
	UP,
	RIGHT,
	DOWN,
	LEFT,
	NO_DIR
};

struct Vector{
	int x;
	int y;
};
struct Object{
	int x;
	int y;
	struct Vector vel;
	int width;
	int height;
	char symbol;
	enum Direction dir;
};

#define PLAYER_SPEED 1
struct Object player1;
struct Object player2;
#define BALL_SPEED 1
struct Object ball;

// The secret AI algorithm :)
enum Direction AI(int y, int playerHeight, int ballY, int ballHeight){
	int playerCenter = y + playerHeight / 2;
	int ballCenter = ballY + ballHeight / 2;

	if(playerCenter < ballCenter){
		return UP;
	} else if(playerCenter > ballCenter){
		return DOWN;
	}
	
	return NO_DIR;
}

// Detects collision with border
enum Direction detectBorderCollision(int x, int y){
	if(y <= 0){
		return UP;
	} else if(y >= HEIGHT - 1){
		return DOWN;
	} else if(x <= 0){
		return LEFT;
	} else if(x >= WIDTH - 1){
		return RIGHT;
	}

	return NO_DIR;
}

// Detects collision
int detectCollision(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2){
	if((x1 >= x2 && x1 <= x2 + width2) || (x1 + width1 >= x2 && x1 + width1 <= x2 + width2)){
		if((y1 >= y2 && y1 <= y2 + height2) || (y1 + height1 >= y2 && y1 + width1 <= y2 + height2)){
			return 1;
		}
	}

	return 0;
}

// Function for display the playground variable
void displayPlayground(){
	system("clear");

	int borderWidth = WIDTH * 2 - 1;

	for(int i = 0; i < borderWidth; i++){
		printf("#");
	}
	printf("\n");

	for(int i = 0; i < HEIGHT; i++){
		for(int j = 0; j < WIDTH; j++){
			if(playground[i][j] == '\0'){
				printf(" ");
			} else { 
				printf("%c", playground[i][j]);
			}
			printf(" ");
		}
		printf("\n");
	}

	for(int i = 0; i < borderWidth; i++){
		printf("#");
	}
	printf("\n");
}

// Initializes all of the game variables
void initializeGameVariables(){
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	WIDTH = w.ws_col / 2;
	HEIGHT = w.ws_row - 3;

	playground = malloc(HEIGHT * sizeof(char *));
	for(int i = 0; i < HEIGHT; i++){
		playground[i] = malloc(WIDTH * sizeof(char) + 1);
	}

	// Player 1
	player1.width = 1;
	player1.height = 5;
	player1.x = 1;
	player1.y = HEIGHT / 2 - player1.height / 2;
	player1.symbol = 'H';
	player1.vel.x = player1.vel.y = 0;

	// Player 2
	player2.width = player1.width;
	player2.height = player1.height;
	player2.x = WIDTH - 2;
	player2.y = player1.y;
	player2.symbol = 'H';
	player2.vel.x = player1.vel.y = 0;

	// Ball
	ball.width = 1;
	ball.height = 1;
	ball.x = WIDTH / 2 - ball.width / 2;
	ball.y = HEIGHT / 2 - ball.height / 2;
	ball.symbol = 'O';
	ball.vel.x = PLAYER_SPEED * -1;
	ball.vel.y = 0;
	ball.dir = LEFT;
}

void updatePlayground(){
	for(int i = 0; i < HEIGHT; i++){
		for(int j = 0; j < WIDTH; j++){
			playground[i][j] = '\0';
		}
	}

	// Updating player1
	for(int i = 0; i < player1.height; i++){
		for(int j = 0; j < player1.width; j++){
			playground[player1.y + i][player1.x + j] = player1.symbol;
		}
	}

	// Updating player2
	for(int i = 0; i < player2.height; i++){
		for(int j = 0; j < player2.width; j++){
			playground[player2.y + i][player2.x + j] = player2.symbol;
		}
	}

	// Updating ball
	for(int i = 0; i < ball.height; i++){
		for(int j = 0; j < ball.width; j++){
			playground[ball.y + i][ball.x + j] = ball.symbol;
		}
	}
}

// Updates the positions of the players and ball
void updatePositions(){
	srand(time(NULL));
	if(detectCollision(ball.x, ball.y, ball.width, ball.height, player1.x, player1.y, player1.width, player1.height)){
		ball.vel.x = BALL_SPEED * 1;
		ball.vel.y = rand() % 2 == 0 ? -1 : 1;
		ball.dir = RIGHT;
	} else if(detectCollision(ball.x, ball.y, ball.width, ball.height, player2.x, player2.y, player2.width, player2.height)){
		ball.vel.x = BALL_SPEED * -1;
		ball.vel.y = rand() % 2 == 0 ? -1 : 1;
		ball.dir = LEFT;
	}

	ball.x += ball.vel.x;
	ball.y += ball.vel.y;
	enum Direction borderCollision = detectBorderCollision(ball.x, ball.y);
	if(borderCollision == UP){
		ball.vel.y *= -1;
	} else if(borderCollision == DOWN){
		ball.vel.y *= -1;
	}

	int targetY = ball.y;
	int targetHeight = ball.height;
	if(ball.dir == RIGHT){
		targetY = HEIGHT / 2;
		targetHeight = 1;
	}
	enum Direction direction = AI(player1.y, player1.height, targetY, targetHeight);
	if(direction == UP){
		player1.vel.y = 1;
	} else if(direction == DOWN){
		player1.vel.y = -1;
	} else if(direction == NO_DIR){
		player1.vel.y = 0;
	}

	player1.x += player1.vel.x;
	player1.y += player1.vel.y;
	if(detectBorderCollision(player1.x, player1.y + player1.height - 2) == DOWN){
		player1.y = HEIGHT - player1.height;
		player1.vel.y = 0;
	} else if(detectBorderCollision(player1.x, player1.y) == UP){
		player1.y = 0;
		player1.vel.y = 0;
	}

	targetY = ball.y;
	targetHeight = ball.height;
	if(ball.dir == LEFT){
		targetY = HEIGHT / 2;
		targetHeight = 1;
	}
	direction = AI(player2.y, player2.height, targetY, targetHeight);
	if(direction == UP){
		player2.vel.y = 1;
	} else if(direction == DOWN){
		player2.vel.y = -1;
	} else if(direction == NO_DIR){
		player2.vel.y = 0;
	}

	player2.x += player2.vel.x;
	player2.y += player2.vel.y;
	if(detectBorderCollision(player2.x, player2.y + player2.height - 2) == DOWN){
		player2.y = HEIGHT - player2.height;
		player2.vel.y = 0;
	} else if(detectBorderCollision(player2.x, player2.y) == UP){
		player2.y = 0;
		player2.vel.y = 0;
	}
}

int main(int argc, char *argv[]){
	initializeGameVariables();

	while(1){
		updatePositions();
		updatePlayground();

		displayPlayground();
		usleep(FRAMERATE * 1000);
	}
	

	return 0;
}
