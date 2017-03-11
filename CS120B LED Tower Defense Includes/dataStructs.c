//Data structures
//task struct in scheduler.h

typedef struct tower{
	//tower struct containing damage, attack speed, and cost
	unsigned int damage;
	unsigned int attackSpeed;
	unsigned int cost;
	unsigned char purchased; //if turret is purchased, display on matrix
} tower;

typedef struct enemy{
	//enemy struct containing health, movement speed, and gold dropped
	unsigned int health;
	unsigned int moveSpeed;
	unsigned int goldDrop;
} enemy;
//End data structures
