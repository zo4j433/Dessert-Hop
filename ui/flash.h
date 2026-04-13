
#ifndef FLASH_H
#define FLASH_H
#define MAX_FLASH 64      

typedef struct {
    int r, c;
    int frames_left;
} FlashTile;

void  flash_init(void);                 
void  add_flash_tile(int r,int c);      
void  flash_update(void);               
void  flash_draw(void);               

#endif
